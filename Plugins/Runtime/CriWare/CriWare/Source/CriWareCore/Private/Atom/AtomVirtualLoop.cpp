
#include "Atom/AtomVirtualLoop.h"

#include "Atom/AtomActiveSound.h"
#include "Atom/AtomDebug.h"
#include "Atom/AtomRuntime.h"
#include "Atom/AtomSoundBase.h"

static int32 bAtomVirtualLoopsEnabledCVar = 1;
FAutoConsoleVariableRef CVarAtomVirtualLoopsEnabled(
	TEXT("atom.VirtualLoops.Enabled"),
	bAtomVirtualLoopsEnabledCVar,
	TEXT("Enables or disables whether virtualizing is supported for audio loops.\n"),
	ECVF_Default);

static float AtomVirtualLoopsPerfDistanceCVar = 15000.0f;
FAutoConsoleVariableRef CVarAtomVirtualLoopsPerfDistance(
	TEXT("atom.VirtualLoops.PerfDistance"),
	AtomVirtualLoopsPerfDistanceCVar,
	TEXT("Sets virtual loop distance to scale update rate between min and max beyond max audible distance of sound.\n"),
	ECVF_Default);

static float AtomVirtualLoopsForceUpdateListenerMoveDistanceCVar = 2500.0f;
FAutoConsoleVariableRef CVarAtomVirtualLoopsForceUpdateListenerMoveDistance(
	TEXT("atom.VirtualLoops.ForceUpdateListenerMoveDistance"),
	AtomVirtualLoopsForceUpdateListenerMoveDistanceCVar,
	TEXT("Sets distance threshold required to force an update on virtualized sounds to check for if listener moves in a single frame over the given distance.\n"),
	ECVF_Default);

static float AtomVirtualLoopsUpdateRateMinCVar = 0.1f;
FAutoConsoleVariableRef CVarAtomVirtualLoopsUpdateRateMin(
	TEXT("atom.VirtualLoops.UpdateRate.Min"),
	AtomVirtualLoopsUpdateRateMinCVar,
	TEXT("Sets minimum rate to check if sound becomes audible again at sound's max audible distance.\n"),
	ECVF_Default);

static float AtomVirtualLoopsUpdateRateMaxCVar = 3.0f;
FAutoConsoleVariableRef CVarAtomVirtualLoopsUpdateRateMax(
	TEXT("atom.VirtualLoops.UpdateRate.Max"),
	AtomVirtualLoopsUpdateRateMaxCVar,
	TEXT("Sets maximum rate to check if sound becomes audible again (at beyond sound's max audible distance + perf scaling distance).\n"),
	ECVF_Default);

FAtomVirtualLoop::FAtomVirtualLoop()
	: TimeSinceLastUpdate(0.0f)
	, TimeVirtualized(0.0f)
	, UpdateInterval(0.0f)
	, ActiveSound(nullptr)
{
}

bool FAtomVirtualLoop::Virtualize(FAtomActiveSound& InActiveSound, bool bDoRangeCheck, FAtomVirtualLoop& OutVirtualLoop)
{
	FAtomRuntime* AtomRuntime = InActiveSound.GetAtomRuntime();
	check(AtomRuntime);

	return Virtualize(InActiveSound, *AtomRuntime, bDoRangeCheck, OutVirtualLoop);
}

bool FAtomVirtualLoop::Virtualize(FAtomActiveSound& InActiveSound, FAtomRuntime& InAtomRuntime, bool bDoRangeCheck, FAtomVirtualLoop& OutVirtualLoop)
{
	UAtomSoundBase* Sound = InActiveSound.GetSound();
	check(Sound);

	const EAtomVirtualizationMode VirtualizationMode = Sound->GetVirtualizationMode();
	if (VirtualizationMode == EAtomVirtualizationMode::Normal
		|| VirtualizationMode == EAtomVirtualizationMode::StopWhenSilent)
	{
		return false;
	}

	if (!bAtomVirtualLoopsEnabledCVar || InActiveSound.bIsPreviewSound /*|| !InActiveSound.IsLooping()*/)
	{
		return false;
	}

	if (InActiveSound.FadeOut != FAtomActiveSound::EFadeOut::None || InActiveSound.bIsStopping)
	{
		return false;
	}

	/*if (InAtomRuntime.CanHaveMultipleActiveSounds(InActiveSound.GetAtomComponentID()))
	{
		return false;
	}*/

	if (bDoRangeCheck && IsInAudibleRange(InActiveSound, &InAtomRuntime))
	{
		return false;
	}

	if (!InActiveSound.IsPlayWhenSilent())
	{
		// Create a virtual copy (original active sound will be stopped)
		FAtomActiveSound* ActiveSound = FAtomActiveSound::CreateVirtualCopy(InActiveSound, InAtomRuntime);
		OutVirtualLoop.ActiveSound = ActiveSound;
	}
	else
	{
		// Set active sound as virtualized
		InActiveSound.SetVirtualized(true);
		OutVirtualLoop.ActiveSound = &InActiveSound;
	}

	OutVirtualLoop.CalculateUpdateInterval();
	
	return true;
}

void FAtomVirtualLoop::CalculateUpdateInterval()
{
	check(ActiveSound);
	FAtomRuntime* AtomRuntime = ActiveSound->GetAtomRuntime();
	check(AtomRuntime);

	const float DistanceToListener = AtomRuntime->GetDistanceToNearestListener(ActiveSound->Transform.GetLocation());
	const float DistanceRatio = (DistanceToListener - ActiveSound->MaxDistance) / FMath::Max(AtomVirtualLoopsPerfDistanceCVar, 1.0f);
	const float DistanceRatioClamped = FMath::Clamp(DistanceRatio, 0.0f, 1.0f);
	UpdateInterval = FMath::Lerp(AtomVirtualLoopsUpdateRateMinCVar, AtomVirtualLoopsUpdateRateMaxCVar, DistanceRatioClamped);
}

float FAtomVirtualLoop::GetTimeVirtualized() const
{
	return TimeVirtualized;
}

float FAtomVirtualLoop::GetUpdateInterval() const
{
	return UpdateInterval;
}

FAtomActiveSound& FAtomVirtualLoop::GetActiveSound()
{
	check(ActiveSound);
	return *ActiveSound;
}

const FAtomActiveSound& FAtomVirtualLoop::GetActiveSound() const
{
	check(ActiveSound);
	return *ActiveSound;
}

bool FAtomVirtualLoop::IsEnabled()
{
	return bAtomVirtualLoopsEnabledCVar != 0;
}

bool FAtomVirtualLoop::IsInAudibleRange(const FAtomActiveSound& InActiveSound, const FAtomRuntime* InAtomRuntime)
{
	if (!InActiveSound.bAllowSpatialization)
	{
		return true;
	}

	const FAtomRuntime* AtomRuntime = InAtomRuntime;
	if (!AtomRuntime)
	{
		AtomRuntime = InActiveSound.GetAtomRuntime();
	}
	check(AtomRuntime);

	if (InActiveSound.IsPlayWhenSilent())
	{
		return true;
	}

	float DistanceScale = 1.0f;
	if (InActiveSound.bHasAttenuationSettings)
	{
		// If we are not using distance-based attenuation, this sound will be audible regardless of distance.
		if (!InActiveSound.AttenuationSettings.bAttenuate)
		{
			return true;
		}

		DistanceScale = InActiveSound.FocusData.DistanceScale;
	}

	DistanceScale = FMath::Max(DistanceScale, UE_KINDA_SMALL_NUMBER);
	const FVector Location = InActiveSound.Transform.GetLocation();
	return AtomRuntime->LocationIsAudible(Location, InActiveSound.MaxDistance / DistanceScale);
}

void FAtomVirtualLoop::UpdateFocusData(float DeltaTime)
{
	check(ActiveSound);

	if (!ActiveSound->bHasAttenuationSettings)
	{
		return;
	}

	// If we are not using distance-based attenuation, this sound will be audible regardless of distance.
	if (!ActiveSound->AttenuationSettings.bAttenuate)
	{
		return;
	}

	check(ActiveSound->GetAtomRuntime());
	const FAtomRuntime& AtomRuntime = *ActiveSound->GetAtomRuntime();
	const int32 ClosestListenerIndex = AtomRuntime.FindClosestListenerIndex(ActiveSound->Transform);

	FAtomAttenuationListenerData ListenerData = FAtomAttenuationListenerData::Create(AtomRuntime, ClosestListenerIndex, ActiveSound->Transform, ActiveSound->AttenuationSettings);
	ActiveSound->UpdateFocusData(DeltaTime, ListenerData);
}

bool FAtomVirtualLoop::Update(float DeltaTime, bool bForceUpdate)
{
	// Keep playback time up-to-date as it may be used to evaluate whether or
	// not virtual sound is eligible for playback when compared against
	// actively playing sounds in concurrency checks.
	const float DeltaTimePitchCorrected = DeltaTime * ActiveSound->MinCurrentPitch;
	ActiveSound->PlaybackTime += DeltaTimePitchCorrected;
	TimeVirtualized += DeltaTimePitchCorrected;

	const float UpdateDelta = TimeSinceLastUpdate + DeltaTime;
	if (bForceUpdate)
	{
		TimeSinceLastUpdate = 0.0f;
	}
	else if (UpdateInterval > 0.0f)
	{
		TimeSinceLastUpdate = UpdateDelta;
		if (UpdateInterval > TimeSinceLastUpdate)
		{
			return false;
		}
		TimeSinceLastUpdate = 0.0f;
	}

#if ENABLE_ATOM_DEBUG
	Atom::FAtomDebugger::DrawDebugInfo(*this);
#endif // ENABLE_ATOM_DEBUG

	// if sound was virtualized by Atom SDK, update when will be checked by Atom SDK, so return false here.
	if (ActiveSound && !ActiveSound->bVirtualizedDueToMaxConcurrency && ActiveSound->IsPlayWhenSilent())
	{
		return false;
	}

	UpdateFocusData(UpdateDelta);

	// If not audible, update when will be checked again and return false
	if (!IsInAudibleRange(*ActiveSound))
	{
		CalculateUpdateInterval();
		return false;
	}

	return true;
}

bool FAtomVirtualLoop::ShouldListenerMoveForceUpdate(const FTransform& LastTransform, const FTransform& CurrentTransform)
{
	const float DistanceSq = FVector::DistSquared(LastTransform.GetTranslation(), CurrentTransform.GetTranslation());
	const float ForceUpdateDistSq = AtomVirtualLoopsForceUpdateListenerMoveDistanceCVar * AtomVirtualLoopsForceUpdateListenerMoveDistanceCVar;
	return DistanceSq > ForceUpdateDistSq;
}

bool FAtomVirtualLoop::IsUsingVirtualVoice() const
{
	return ActiveSound && ActiveSound->IsPlayWhenSilent();
}

bool FAtomVirtualLoop::IsVirtualizedDueToMaxConcurrency() const
{
	return ActiveSound && ActiveSound->bVirtualizedDueToMaxConcurrency;
}
