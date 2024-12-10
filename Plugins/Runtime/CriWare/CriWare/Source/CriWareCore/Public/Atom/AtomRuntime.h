/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCore
 * File     : AtomRuntime.h
 *
 ****************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "UObject/StrongObjectPtr.h"
#include "Subsystems/SubsystemCollection.h"
#include "HAL/PlatformMisc.h"
#include "DSP/MultithreadedPatching.h"

#include "CriWare.h"
#include "CriWareApi.h"
#include "CriWareUtils.h"
#include "CriWareCoreSettings.h"
#include "AtomThread.h"
#include "AtomEngineSubsystem.h"
#include "AtomRuntimeManager.h"
#include "AtomListener.h"
#include "AtomVolume.h"
#include "AtomComponent.h"
#include "AtomSoundClass.h"
#include "AtomConcurrency.h"
#include "Extensions/IAtomExtensionPlugin.h"
#include "Extensions/IAtomRuntimePlugin.h"
#include "Extensions/IAtomEndpoint.h"
#include "Extensions/IManaRuntime.h"
#include "AtomVirtualLoop.h"

// Forward declarations.
class UAtomConfig;
class UAtomRackBase;
class UAtomRack;
class UAtomBus;
class UAtomBusEffectPreset;
class UAtomSoundBase;
class IAtomSoundResource;
class UAtomAttenuation;
class UAtomConcurrency;
class ICriWarePlatformAtom;
class FAtomSource;
class FAtomModulationSystem;

struct FAtomPanning;
struct FAtomBusSend;
struct FAtomActiveSound;
struct FAtomAttenuationSettings;
struct FAtomPlaybackInstance;

namespace Atom
{
	class FAtomSourceManager;
	class FMixerSourceVoice;
	class FAtomDebugger;
}

/**
 * Debug state of the Atom system
 */
enum class EAtomDebugState : uint8
{
	// No debug sounds
	None,
	// No reverb sounds
	//IsolateDryAudio,
	// Only reverb sounds
	//IsolateReverb,
	// Force LPF on all sources
	TestLPF,
	// Force LPF on all sources
	TestHPF,
	// Bleed all sounds to the LFE speaker
	//TestLFEBleed,
	// Disable any LPF filter effects
	DisableLPF,
	// Disable any LPF filter effects
	DisableHPF,
	// Disable any radio filter effects
	//DisableRadio,
	MAX,
};

/**
 * Current state of a Rack
 */
enum class EAtomRackState : uint8
{
	// Waiting to fade in
	Inactive,
	// Fading in
	FadingIn,
	// Fully active
	Active,
	// Fading out
	FadingOut,
	// Time elapsed, just about to be removed
	AwaitingRemoval,
};

static const TCHAR* GetString(EAtomRackState InState)
{
	switch (InState)
	{
	case EAtomRackState::Inactive: return TEXT("Inactive");
	case EAtomRackState::FadingIn: return TEXT("FadingIn");
	case EAtomRackState::Active: return TEXT("Active");
	case EAtomRackState::FadingOut: return TEXT("FadingOut");
	case EAtomRackState::AwaitingRemoval: return TEXT("AwaitingRemoval");
	default: return TEXT("unknown");
	}
}

enum class ESortedActivePlaybackGetType : uint8
{
	FullUpdate,
	PausedUpdate,
	QueryOnly,
};

struct FAtomRackState
{
	bool IsMasterRack;
	uint32 ActiveRefCount;
	uint32 PassiveRefCount;
	double StartTime;
	double FadeInStartTime;
	double FadeInEndTime;
	double FadeOutStartTime;
	double EndTime;
	float InterpValue;
	EAtomRackState CurrentState;
};

/*
 * Settings for global focus scaling
 */
struct FAtomGlobalFocusSettings
{
	float FocusAzimuthScale = 1.0f;
	float NonFocusAzimuthScale = 1.0f;
	float FocusDistanceScale = 1.0f;
	float NonFocusDistanceScale = 1.0f;
	float FocusVolumeScale = 1.0f;
	float NonFocusVolumeScale = 1.0f;
	float FocusPriorityScale = 1.0f;
	float NonFocusPriorityScale = 1.0f;
};

/*
 * Atom Runtime class 
 */
class CRIWARECORE_API FAtomRuntime
{
public:

	FAtomRuntime(ICriWarePlatformAtom* InPlatformAtom);
	virtual ~FAtomRuntime();

	/** Intialize this runtime and active it. */
	bool Init(FAtomRuntimeId InRuntimeID);

	/**
	 * Called after FAtomRuntime creation and init.
	 */
	void OnRuntimeCreated(FAtomRuntimeId InDeviceID);

	/**
	 * Called before FAtomRuntime teardown.
	 */
	void OnRuntimeDestroyed(FAtomRuntimeId InDeviceID);

	/** Remove runtime from active and unitialize it. */
	void Teardown();

	/** De-initialization step that occurs after runtime destroyed broadcast, but before removal from the runtime manager. */
	void Deinitialize();

	/**
	 * The Atom main "Tick" function.
	 *
	 * @param bGameTicking Whether the game is still ticking at the time of update.
	 */
	void Update(bool bGameTicking);

	/** Update called on game thread. */
	virtual void UpdateGameThread();

	/**
	 * Suspend/resume all sounds (global pause for device suspend/resume, etc.)
	 *
	 * @param bGameTicking Whether the game is still ticking at the time of suspend
	 */
	void Suspend(bool bGameTicking);

	/** Optional fadeout and fade in of audio to avoid clicks when closing or opening/reusing Atom runtime. */
	virtual void FadeOut() {}
	virtual void FadeIn() {}

	/**
	 * Stop all the Atom components and sources attached to the world. nullptr world means all components.
	 */
	void Flush(UWorld* WorldToFlush);

	/*
	 * Derived classes can override this method to do their own cleanup. Called at the end of FAtomRuntime::Flush()
	 */
	virtual void FlushExtended(UWorld* WorldToFlush/*, bool bClearActivatedReverb*/);

	/**
	 * Allows Atom rendering command queue to flush during Atom runtime flush. (Override in platforms if need extra native audio rendering command)
	 * @param bPumpSynchronously must be called in situations where the Atom render thread is not being called.
	 */
	/*virtual void FlushAtomRenderingCommands(bool bPumpSynchronously = false) {}

	void OnPreGarbageCollect();*/

	/**
	 * Track references to UObjects
	 */
	void AddReferencedObjects(FReferenceCollector& Collector);

	// Functions which check the thread it's called on and helps make sure functions are called from correct threads
	void CheckAtomThread() const;
	void CheckAudioRenderingThread() const;
	bool IsAudioRenderingThread() const;
	
private: // ?
	// Resets the thread ID used for audio rendering
	void ResetAudioRenderingThreadID();
public:

	/**
	 * Stop any playing sounds that are using a particular Sound with resource
	 *
	 * @param InSound						Resource to stop any sounds that are using it
	 * @param[out] StoppedComponents	List of Audio Components that were stopped
	 */
	void StopSoundsUsingResource(IAtomSoundResource* InSound, TArray<UAtomComponent*>* StoppedComponents = nullptr);

	void Activate(bool bFadeIn = false); // maybe use events
	void Deactivate(bool bFadeOut = false); // maybe use events
	bool IsActive() const;

	FORCEINLINE FAtomRuntimeId GetAtomRuntimeID() const { return RuntimeID; }

#if WITH_EDITOR
	/** Deals with anything Atom related that should happen when PIE starts */
	void OnBeginPIE(bool bIsSimulating);

	/** Deals with anything Atom related that should happen when PIE ends */
	void OnEndPIE(bool bIsSimulating);
#endif

private: // runtime API

	void InitializeAtomLibrary();
	void FinalizeAtomLibrary();

	const UAtomConfig* AtomConfig;

protected:

	/** Update the Atom library. */
	virtual void UpdateAtomLibrary();

protected: // Atom library platform API

	struct FAtomRackInitParams
	{
		int SamplingRate = CRIATOM_DEFAULT_OUTPUT_SAMPLING_RATE;
		int NumChannels = CRIATOM_DEFAULT_OUTPUT_CHANNELS;
		float ServerFrequency = 60.0f;
		EAtomSoundRendererType SoundRendererType = EAtomSoundRendererType::Default;
		CriAtomSoundRendererType RendererType = CRIATOM_SOUND_RENDERER_ASR;
		CriAtomSpeakerMapping SpeakerMapping = CRIATOM_SPEAKER_MAPPING_AUTO;
		CriAtomExAsrRackId OutputRackId = 0;
		const IAtomEndpointSettingsProxy* EndpointSettings = nullptr;
	};

	struct FAtomSoundfieldRackInitParams
	{
		int SamplingRate = CRIATOM_DEFAULT_OUTPUT_SAMPLING_RATE;
		int NumChannels = CRIATOM_DEFAULT_OUTPUT_CHANNELS;
		EAtomSoundfieldRendererType SoundfieldRendererType = EAtomSoundfieldRendererType::Default;
		CriAtomSoundRendererType RendererType = CRIATOM_SOUND_RENDERER_ASR;
		CriAtomSpeakerMapping SpeakerMapping = CRIATOM_SPEAKER_MAPPING_AUTO;
		const IAtomEndpointSettingsProxy* EndpointSettings = nullptr;
	};

	/** */
	virtual void InitializePlatformAtomLibrary() = 0;

	/** */
	virtual void FinalizePlatformAtomLibrary() = 0;
	
	/** */
	virtual FCriAtomExAsrRackId LoadPlatformAtomRack(const FAtomRackInitParams& InitParams)
	{
		return LoadGenericAtomRack(InitParams);
	}

	virtual FCriAtomExAsrRackId LoadPlatformAtomSoundfieldRack(const FAtomSoundfieldRackInitParams& InitParams)
	{
		return LoadGenericAtomSoundfieldRack(InitParams);
	}

	FORCEINLINE virtual TSubclassOf<UAtomEndpointSettingsBase> GetPlatformEndpointSettingsClass() const
	{
		return nullptr;
	}
	
	FORCEINLINE virtual CriAtomSoundRendererType GetPlatformAtomExSoundRendererType(EAtomSoundRendererType RendererType)
	{
		return GetAtomExSoundRendererType(RendererType);
	}
	
	FORCEINLINE virtual int32 GetPlatformSoundRendererNumChannels(EAtomSoundRendererType RendererType)
	{
		return GetAtomExSoundRendererNumChannels(RendererType);
	}

	FORCEINLINE virtual int32 GetPlatformSoundRendererNumPorts(EAtomSoundRendererType RendererType)
	{
		return GetAtomExSoundRendererNumPorts(RendererType);
	}

	FORCEINLINE virtual CriAtomSoundRendererType GetPlatformSoundfieldAtomExSoundRendererType(EAtomSoundfieldRendererType SoundfieldRendererType)
	{
		return GetSoundfieldAtomExSoundRendererType(SoundfieldRendererType);
	}

	FORCEINLINE virtual int32 GetPlatformSoundfieldRendererNumChannels(EAtomSoundfieldRendererType SoundfieldRendererType)
	{
		return GetSoundfieldAtomExSoundRendererNumChannels(SoundfieldRendererType);
	}

	FORCEINLINE virtual CriAtomSpeakerMapping GetPlatformSoundfieldRendererSpeakerMapping(EAtomSoundfieldRendererType SoundfieldRendererType)
	{
		return GetSoundfieldAtomExSoundRendererSpeakerMapping(SoundfieldRendererType);
	}

	/** Add a fully created voice pool. */
	bool AddAtomVoicePool(FCriAtomVoicePoolPtr&& VoicePool, EAtomVoicePoolType VoiceType);

	/** Convenient function to create and add standard memory and streaming voices pools. */
	void CreateAtomStandardVoicePools(const FCriWareStandardVoicesSettings& VoiceSettings);

	/** Convenient function to create and add HCA-MX memory and streaming voices pools. */
	void CreateAtomHcaMxVoicePools(const FCriWareHcaMxVoicesSettings& VoiceSettings);

	/** Add a fully created asr rack. */
	bool AddAtomAsrRack(FCriAtomExAsrRackId&& AsrRack, const UAtomRackBase& InAtomRack);

	/** Function to create and add a default standard asr rack. */
	FCriAtomExAsrRackId LoadGenericAtomRack(const FAtomRackInitParams& InitParams);

	/** Function to create and add a default spatializer asr rack. */
	FCriAtomExAsrRackId LoadGenericAtomSoundfieldRack(const FAtomSoundfieldRackInitParams& InitParams);

	/** Setup Atom D-bas data stream system. (Optional) */
	void SetupAtomDBAS();

public: // user API

	// NEW - Mixer: voices source management for Atom Runtime.

	Atom::FMixerSourceVoice* GetMixerSourceVoice();
	void ReleaseMixerSourceVoice(Atom::FMixerSourceVoice* InSourceVoice);

	Atom::FAtomSourceManager* GetSourceManager();
	const Atom::FAtomSourceManager* GetSourceManager() const;

	// move to rack ? use rack arg ? 
	Audio::FPatchOutputStrongPtr AddPatchForRack(uint32 InObjectId, float InPatchGain);

	Audio::FPatchOutputStrongPtr MakePatch(int32 InFrames, int32 InChannels, float InGain) const;

	// ---

	FAtomConcurrencyManager& GetConcurrencyManager() { return ConcurrencyManager; }
	const FAtomConcurrencyManager& GetConcurrencyManager() const { return ConcurrencyManager; }

	/** Whether the mixer source manager use user defined voice pools or generate voice pool on the fly for data asset sound. */
	bool IsUsingFixedVoicePools() const { return !bUseMixerVoiceManagement; }

	/** Whether Atom routes any rack audio outputs to Unreal audio. */
	virtual bool IsUsingUnrealSoundRenderer() const { return false; }

	/** Function to iterate over voice pool informations. */
	void IterateOverVoicePools(TFunctionRef<void(EAtomVoicePoolType Type, int32 ID, int32 NumUsedVoices, int32 MaxVoices, int32 MaxChannels, int32 MaxSampleRate)> Func) const;

	/** Register ACF to Atom runtime. */
	bool SetAtomConfiguration(const UAtomConfig* AtomConfig);

	/** Get the currently used Atom Config. (May be null, test-it before use.) */
	FORCEINLINE const UAtomConfig* GetAtomConfiguration() const { return AtomConfig; }

	/** Resister an AtomRack to the Atom runtime. */
	bool RegisterAtomRack(const UAtomRackBase* AtomRack);

	/** Unresister an AtomRack to the Atom runtime. */
	bool UnregisterAtomRack(const UAtomRackBase* AtomRack);

	/** Get the currently used master AtomRack. */
	FORCEINLINE UAtomRack* GetMasterRack() const { return MasterRack; }

	/** Get the currently used master AtomRack. */
	FORCEINLINE UAtomRackBase* GetSpatializationRack() const { return SpatializationRack; }

	int32 GetRuntimeSampleRate() const;
	int32 GetRuntimeNumOutputChannels() const;
	int32 GetRuntimeNumOutputFrames() const;

	/** Retrieves the names of output ports mapped with a given rack maps.*/
	TArray<FName> GetMappedOutputPortNames(const UAtomRackBase* AtomRack) const;

	/** */
	bool MapOutputPortToRack(FName OutputPortName, const UAtomRackBase* AtomRack);

	/** Retreive the native implementation id for a given AtomRack. Returns -1 if fails. */
	CriAtomExAsrRackId GetAsrRackId(const UAtomRackBase* InRack) const;

	/** */
	int32 GetNumRacks() { return AsrRacks.Num() + 1; }

	/** */
	void IterateOverRacks(TFunctionRef<void(const UAtomRackBase* Rack)> Func) const;

	/** */
	int32 GetRackSampleRate(UAtomRackBase* InRack) const;
	
	/** */
	int32 GetRackNumOutputChannels(UAtomRackBase* InRack) const;

	/** Attach the DspSetting from AtomRack to Atom runtime rack.*/
	bool SetRackDspBusSetting(const UAtomRack* InRack);

	/** Apply snapshot from AtomRack to Atom runtime rack with given fade time in seconds. */
	bool SetRackSnapshot(const UAtomRack* InRack, float FadeSeconds);

	/** Get snapshot name from Atom runtime rack. */
	FString GetRackSnapshotName(const UAtomRack* InRack) const;

	/** Apply bus volume setting from AtomBus to Atom runtime bus. */
	bool SetBusVolume(const UAtomBus* InBus, float InBusVolume);

	/** Get bus volume setting from Atom runtime bus. */
	float GetBusVolume(const UAtomBus* InBus) const;

	/** Apply spatialization panning settings from AtomBus to Atom runtime bus. */
	bool SetBusPanning(const UAtomBus* InBus);

	/** Get spatialization panning settings from Atom runtime bus. */
	bool GetBusPanning(const UAtomBus* InBus, FAtomPanning& OutPanning) const;

	/** Apply level matrix setting from AtomBus to Atom runtime bus. */
	bool SetBusChannelLevelMatrix(const UAtomBus* InBus);

	/** Apply a bus send level from AtomBus to Atom runtime bus. */
	bool SetBusSendLevel(const UAtomBus* InBus, const FAtomBusSend& InSend);

	/** Get bus effect parameter of AtomBusEffectPreset from Atom runtime bus. */
	float GetBusEffectParameter(const UAtomBus* InBus, const UAtomBusEffectPreset* InEffectPreset, int InParameterIndex) const;

	/** Apply a bus effect parameter from AtomBusEffectPreset to Atom runtime bus. */
	bool SetBusEffectParameter(const UAtomBus* InBus, const UAtomBusEffectPreset* InEffectPreset, int InParameterIndex);

	/** Get bus effect by-pass of AtomBusEffectPreset from Atom runtime bus. */
	bool GetBusEffectBypass(const UAtomBus* InBus, const UAtomBusEffectPreset* InEffectPreset) const;

	/** Apply a bus effect by-pass from AtomBusEffectPreset to Atom runtime bus. */
	bool SetBusEffectBypass(const UAtomBus* InBus, const UAtomBusEffectPreset* InEffectPreset);

	/** Get all settings of AtomBusEffectPreset from Atom runtime bus. */
	bool GetBusEffectPreset(const UAtomBus* InBus, UAtomBusEffectPreset* InEffectPreset);

	/** Apply all settings from AtomBusEffectPreset to Atom runtime bus. */
	bool SetBusEffectPreset(const UAtomBus* InBus, const UAtomBusEffectPreset* InEffectPreset);

	bool AttachAisacPatchToCategory(const FAtomCategory& Category, const UAtomAisacPatch* AisacPatch);

	bool DetachAisacPatchFromCategory(const FAtomCategory& Category, const UAtomAisacPatch* AisacPatch);

	// Bus Modulation Settings
	void UpdateBusModulationSettings(UAtomBus* InAtomBus, const TSet<TObjectPtr<UAtomModulatorBase>>& InOutputModulation);
	void SetBusModulationBaseLevels(UAtomBus* InAtomBus, float InVolumeModBase);

	/*FAsrRackWeakPtr GetRackInstance(const UAtomRackBase* AtomRack)
	{
		FAsrRackPtr AsrRack = GetMasterRackInstance(AtomRack);
		if (AsrRack.IsValid())
		{
			return AsrRack;
		}

		if (AtomRack)
		{
			return AsrRacks.FindRef(AtomRack);
		}

		return nullptr;
	}*/

	// Atom SDK callbacks handling
	void HandleSDKOnAtomExBeatSync(const CriAtomExBeatSyncInfo* BeatSyncInfo);
	void HandleSDKOnAtomExSequencerEvent(const CriAtomExSequenceEventInfo* EventInfo);

protected: // platform

	ICriWarePlatformAtom* PlatformAtom;

protected: // D-bas

	/* D-bas settings values */
	int DbasMaxBPS;
	int DbasMaxStreams;

	/* D-bas ID (Handle) */
	FCriAtomDbasId DbasID;

protected: // voices

	/** User defined voice pools classed by type.
	 * User pools share the same identifier and are assigned automaticaly by Atom.
	 * The AtomSourceManager will borrow them for Atom cue playback. */
	TMap<EAtomVoicePoolType, FCriAtomVoicePoolPtr> VoicesPools;

	/** Queue of mixer source voices controlled by AtomSourceManager. 
	 * A Mixer Source Voice contains information about voice pool and settings in use for a specific sound source to play. */
	TQueue<Atom::FMixerSourceVoice*> SourceVoices;

	/** The Atom source manager. */
	TUniquePtr<Atom::FAtomSourceManager> SourceManager;

	/** MPSC command queue to send commands to the game thread */
	TMpscQueue<TFunction<void()>> GameThreadCommandQueue;

	/** ThreadId for the game thread (or if Atom is running a separate thread, that ID) */
	mutable int32 GameOrAtomThreadID;

	/** ThreadId for the low-level ASR audio mixer. */
	mutable int32 ASRThreadID;

	/** The concurrency manager. */
	friend class FAtomConcurrencyManager;
	FAtomConcurrencyManager ConcurrencyManager;

private: // submixer

	/* Master Rack */
	UAtomRack* MasterRack;

	void InitMasterRack();
	void InitRacks();

	/* Default Binaural / Spatializer */
	UAtomRackBase* SpatializationRack;

	void InitSpatializationRack();
	void InitOuputPortMappings();

protected:

	void LoadMasterRack(const FSoftObjectPath& InObjectPath);
	bool IsMasterRack(const UAtomRackBase* InRack) const;
	bool IsSpatializationRack(const UAtomRackBase* InRack) const;
	bool LoadAtomRackBusSettings(const UAtomRack* Rack);
	//void InitSoundfieldAndEndpointDataForRack(const UAtomRackBase& InRack, CriAtomExAsrRackId InRackId, bool bAllowReInit);

	/** Create and register a new atom rack. */
	bool LoadAtomRack(const UAtomRackBase& InAtomRack);

	// Native handles
	// rack Id
	TMap<const UAtomRackBase*, FCriAtomExAsrRackId> AsrRacks;

	// Racks mapped to output ports.
	TMap<FName, TStrongObjectPtr<const UAtomRackBase>> OutputPortRackMap;

	// Bus effects to update struct
	struct FBusEffectToUpdateInfo
	{
		const UAtomRackBase* Rack = nullptr;
		const UAtomBus* Bus = nullptr;
		FName EffectName;

		bool operator==(const FBusEffectToUpdateInfo& Rhs) const
		{
			return Rack == Rhs.Rack && Bus == Rhs.Bus && EffectName == Rhs.EffectName;
		}

		friend uint32 GetTypeHash(const FBusEffectToUpdateInfo& Other)
		{
			return HashCombine(GetTypeHash(Other.Rack),
				HashCombine(GetTypeHash(Other.Bus),
				GetTypeHash(Other.EffectName)));
		}
	};
	// Bus effects to update in next AtomProcess
	TSet<FBusEffectToUpdateInfo> BusEffectsToUpdate;

protected: // listeners for 3d sound

	// Atom thread representation of listeners
	TArray<FAtomListener> Listeners;

	/** Inverse listener transformations, used for spatialization */
	TArray<FMatrix> InverseListenerTransforms;

	/** Cached parameters passed to the initialization of various Atom plugins */
	FAtomPluginInitializationParams PluginInitializationParams;

public:
	/** The source data override interface. */
	TAtomSourceDataOverridePtr SourceDataOverridePluginInterface;

	/** The Mana interface. */
	TManaRuntimePtr ManaRuntimeInterface;

	/** 3rd party Runtime plugin interfaces. */
	TArray<TAtomRuntimePluginPtr> RuntimePluginInterfaces;

	// Game thread cache of listener transforms
	TArray<FAtomListenerProxy> ListenerProxies;

public:

	/**
	 * Sets the details about the listener
	 * @param	World				The world the listener is being set on.
	 * @param   ListenerIndex		The index of the listener
	 * @param   ListenerTransform   The listener's world transform
	 * @param   DeltaSeconds		The amount of time over which velocity should be calculated.  If 0, then velocity will not be calculated.
	 */
	void SetListener(UWorld* World, const int32 ListenerIndex, const FTransform& ListenerTransform, const float InDeltaSeconds);

	/** Sets an override position for the specified listener to do attenuation calculations (also called focus point). */
	void SetListenerAttenuationOverride(int32 ListenerIndex, const FVector AttenuationPosition);

	/** Removes a listener attenuation override (also called focus point) for the specified listener. */
	void ClearListenerAttenuationOverride(int32 ListenerIndex);

	const TArray<FAtomListener>& GetListeners() const { check(IsInAtomThread()); return Listeners; }

	/** Returns the index of the listener closest to the given sound transform */
	static int32 FindClosestListenerIndex(const FTransform& SoundTransform, const TArray<FAtomListener>& InListeners);

	/** Returns the index of the listener closest to the given sound transform */
	int32 FindClosestListenerIndex(const FTransform& SoundTransform) const;
	int32 FindClosestListenerIndex(const FVector& Position, float& OutSqDistance, bool AllowAttenuationOverrides) const;

	/**
	 * Gets the direction of the given position vector transformed relative to listener.
	 * @param Position				Input position vector to transform relative to listener
	 * @param OutDistance			Optional output of distance from position to listener
	 * @return The input position relative to the listener.
	 */
	FVector GetListenerTransformedDirection(const FVector& Position, float* OutDistance);

	/**
	 * Returns a position from the appropriate listener representation, depending on calling thread.
	 *
	 * @param	ListenerIndex	index of the listener or proxy
	 * @param	OutPosition		filled in position of the listener or proxy
	 * @param	bAllowOverride	if true we will use the attenuation override for position, if set
	 * @return	true if successful
	 */
	bool GetListenerPosition(int32 ListenerIndex, FVector& OutPosition, bool bAllowOverride) const;

	/**
	 * Returns the transform of the appropriate listener representation, depending on calling thread
	 */
	bool GetListenerTransform(int32 ListenerIndex, FTransform& OutTransform) const;

	/**
	 * Returns the WorldID of the appropriate listener representation, depending on calling thread
	 */
	bool GetListenerWorldID(int32 ListenerIndex, uint32& OutWorldID) const;

	/**
	 * Checks to see if a coordinate is within a distance of any listener
	 */
	bool LocationIsAudible(const FVector& Location, const float MaxDistance) const;

	/**
	 * Checks to see if a coordinate is within a distance of a specific listener
	 */
	bool LocationIsAudible(const FVector& Location, int32 ListenerIndex, const float MaxDistance) const;

	/**
	 * Returns the distance to the nearest listener from the given location
	 */
	float GetDistanceToNearestListener(const FVector& Location) const;

	/**
	 * Sets OutSqDistance to the distance from location to the appropriate listener representation, depending on calling thread.
	 * Returns true if listener position is valid, false if not (in which case, OutSqDistance is undefined).
	 */
	bool GetDistanceSquaredToListener(const FVector& Location, int32 ListenerIndex, float& OutSqDistance) const;

	/**
	 * Sets OutSqDistance to the distance from location the closest listener, depending on calling thread.
	 * Returns true if listener position is valid, false if not (in which case, OutSqDistance is undefined).
	 */
	bool GetDistanceSquaredToNearestListener(const FVector& Location, float& OutSqDistance) const;

	/** Whether or not HRTF spatialization is enabled for all. */
	bool IsHRTFEnabledForAll() const;

	void SetHRTFEnabledForAll(bool InbHRTFEnabledForAll)
	{
		const bool bNewHRTFEnabledForAll = InbHRTFEnabledForAll;

		bHRTFEnabledForAll_OnGameThread = bNewHRTFEnabledForAll;

		DECLARE_CYCLE_STAT(TEXT("FAtomThreadTask.SetHRTFEnabledForAll"), STAT_SetHRTFEnabledForAll, STATGROUP_AtomThreadCommands);

		FAtomRuntime* AtomRuntime = this;
		FAtomThread::RunCommandOnAtomThread([AtomRuntime, bNewHRTFEnabledForAll]()
		{
			AtomRuntime->bHRTFEnabledForAll = bNewHRTFEnabledForAll;

		}, GET_STATID(STAT_SetHRTFEnabledForAll));
	}

	/** Whether or not HRTF is disabled. */
	bool IsHRTFDisabled() const;

	void SetHRTFDisabled(bool InIsHRTFDisabled)
	{
		const bool bNewHRTFDisabled = InIsHRTFDisabled;

		bHRTFDisabled_OnGameThread = bNewHRTFDisabled;

		DECLARE_CYCLE_STAT(TEXT("FAtomThreadTask.SetHRTFDisabled"), STAT_SetHRTFDisabled, STATGROUP_AtomThreadCommands);

		FAtomRuntime* AtomRuntime = this;
		FAtomThread::RunCommandOnAtomThread([AtomRuntime, bNewHRTFDisabled]()
		{
			AtomRuntime->bHRTFDisabled = bNewHRTFDisabled;
			FCriWareApi::criAtomExAsr_EnableBinauralizer(bNewHRTFDisabled ? CRI_FALSE : CRI_TRUE);

		}, GET_STATID(STAT_SetHRTFDisabled));
	}

private:

	/**
	 * Notifies all plugin listeners belonging to this audio device that
	 * the world changed. Called in the game thread.
	 *
	 * @param World: Pointer to the UWorld the listener is in.
	 */
	void NotifyPluginListenersWorldChanged(UWorld* World);

	/**
	 * This is overridden in Audio::FMixerDevice to propogate listener information to the audio thread.
	 */
	virtual void OnListenerUpdated(const TArray<FAtomListener>& InListeners);

public: // sound classes

	/**
	 * Registers a sound class with the Atom runtime.
	 */
	void RegisterSoundClass(UAtomSoundClass* InSoundClass);

	/**
	 * Unregisters a sound class.
	 */
	void UnregisterSoundClass(UAtomSoundClass* InSoundClass);

	/**
	 * Set up the sound class hierarchy.
	 */
	void InitSoundClasses();

	/**
	 * Gets the current properties of a sound class, if the sound class hasn't been registered, then it returns nullptr.
	 */
	FAtomSoundClassProperties* GetSoundClassCurrentProperties(UAtomSoundClass* InSoundClass);

	/** 
	 * Returns the parameters which are dynamic from the given sound class. 
	 */
	FAtomSoundClassDynamicProperties* GetSoundClassDynamicProperties(UAtomSoundClass* InSoundClass);

private:

	/**
	 * Parses the sound classes and propagates multiplicative properties down the tree.
	 */
	void ParseSoundClasses(float InDeltaTime);

	/**
	 * Construct the CurrentSoundClassProperties map
	 * @param DeltaTime The current frame delta. Used to interpolate sound class adjustments.
	 *
	 * This contains the original sound class properties propagated properly, and all adjustments.
	 */
	void UpdateSoundClassProperties(float DeltaTime);

	/**
	 * Internal helper function used by ParseSoundClasses to traverse the tree.
	 *
	 * @param CurrentClass		Subtree to deal with
	 * @param ParentProperties		Propagated properties of parent node
	 */
	void RecurseIntoSoundClasses(UAtomSoundClass* CurrentClass, FAtomSoundClassProperties& ParentProperties);

	/** Current properties of all sound classes */
	TMap<UAtomSoundClass*, FAtomSoundClassProperties> SoundClasses;
	TMap<UAtomSoundClass*, FAtomSoundClassDynamicProperties> DynamicSoundClassProperties;

public: // Interior volumes

	/**
	 * Gets the default reverb and interior settings for the provided world. Returns true if settings with WorldID were located.
	 */
	bool GetDefaultAtomSettings(uint32 WorldID, FAtomReverbSettings& OutReverbSettings, FAtomInteriorSettings& OutInteriorSettings) const;

	/**
	 * Sets the default reverb and interior settings for the provided world.
	 */
	void SetDefaultAtomSettings(UWorld* World, const FAtomReverbSettings& DefaultReverbSettings, const FAtomInteriorSettings& DefaultInteriorSettings);

public:

	FAtomModulationSystem* GetAtomModulationSystem() const { return ModulationSystem.Get(); }

private:

	TSharedPtr<FAtomModulationSystem> ModulationSystem;

private: // source players and active playback

	// all sources
	TArray<FAtomSource*> Sources;
	// pooled sources
	TArray<FAtomSource*> FreeSources;
	// unpooled sources to delete
	TArray<FAtomSource*> RemovedSources;

	friend FAtomSource;

	/** Set of sources used to play sounds. */
	TMap<FAtomPlaybackInstance*, FAtomSource*> PlaybackInstanceSourceMap;

	/** Anchor used to connect UAtomEngineSubsystems to FAtomRuntime */
	TStrongObjectPtr<UAtomSubsystemCollectionRoot> SubsystemCollectionRoot;

	/** Subsystems tied to this device's lifecycle */
	FAtomSubsystemCollection SubsystemCollection;

public: // UE4 object API

	/** Retreive the SDK handle for a given AtomPlaybackInstance. Returns nullptr if fails. */
	CriAtomExPlayerHn GetExPlayerHandle(const FAtomPlaybackInstance* InPlaybackInstance) const;
	CriAtomPlayerHn GetPlayerHandle(const FAtomPlaybackInstance* InPlaybackInstance) const;

	struct CRIWARECORE_API FCreateComponentParams
	{
		FCreateComponentParams();
		FCreateComponentParams(UWorld* World, AActor* Actor = nullptr);
		FCreateComponentParams(AActor* Actor);
		FCreateComponentParams(FAtomRuntime* AtomRuntime);

		UAtomAttenuation* AttenuationSettings;
		TSubclassOf<UAtomComponent> AtomComponentClass = UAtomComponent::StaticClass();
		TSet<UAtomConcurrency*> ConcurrencySet;
		bool bAutoDestroy;
		bool bPlay;
		bool bStopWhenOwnerDestroyed;

		void SetLocation(FVector Location);
		bool ShouldUseAttenuation() const;

	private:
		UWorld* World;
		AActor* Actor;
		FAtomRuntime* AtomRuntime;

		bool bLocationSet;
		FVector Location;

		void CommonInit();

		friend FAtomRuntime;
	};

	/**
	 * Creates an Atom component to handle playing a sound.
	 * @param   Sound				The UAtomSoundBase to play at the location.
	 * @param   Params				The parameter block of properties to create the component based on
	 * @return	The created Atom component if the function successfully created one or a nullptr if not successful. Note: if audio is disabled or if there were no hardware audio devices available, this will return nullptr.
	 */
	static UAtomComponent* CreateComponent(UAtomSoundBase* Sound, const FCreateComponentParams& Params = FCreateComponentParams());

	/**
	 * Plays a sound at the given location without creating an Atom component.
	 * @param   Sound				The UAtomSoundBase to play at the location.
	 * @param   World				The world this sound is playing in.
	 * @param   VolumeMultiplier	The volume multiplier to set on the sound.
	 * @param   PitchMultiplier		The pitch multiplier to set on the sound.
	 * @param	StartTime			The initial time offset for the sound.
	 * @param	Location			The sound's position.
	 * @param	Rotation			The sound's rotation.
	 * @param	AttenuationSettings	The sound's attenuation settings to use (optional). Will default to the UAtomSoundBase's AttenuationSettings if not specified.
	 * @param	ConcurrencySettings	The sound's concurrency settings to use (optional). Will use the UAtomSoundBase's UAtomSoundConcurrency if not specified.
	 * @param	Params				An optional list of Atom component params to immediately apply to a sound.
	 */
	void PlaySoundAtLocation(UAtomSoundBase* Sound, UWorld* World, float VolumeMultiplier, float PitchMultiplier, float StartTime, const FVector& Location, const FRotator& Rotation, UAtomAttenuation* AttenuationSettings = nullptr, UAtomConcurrency* ConcurrencySettings = nullptr, const TArray<FAtomParameter>* Params = nullptr, const AActor* OwningActor = nullptr);

	/**
	 * Adds an active sound to the Atom runtime
	 */
	void AddNewActiveSound(const FAtomActiveSound& ActiveSound);

	/** Returns the audio clock of the Atom runtime. */
	double GetAtomClock() const { return AtomClock; }

	/**
	 * Add provided loop (virtualized active sound)
	 */
	void AddVirtualLoop(const FAtomVirtualLoop& InVirtualLoop);

	/**
	 * Attempts to retrigger a provided loop (virtualized active sound)
	 */
	void RetriggerVirtualLoop(FAtomVirtualLoop& VirtualLoop);

	/**
	 * Attempts to realize provided loop (virtualized active sound)
	 */
	void RealizeVirtualLoop(FAtomActiveSound& ActiveSound);
	void RealizeVirtualLoop(FAtomVirtualLoop& VirtualLoop);

	/**
	 * Removes the active sound for the specified Atom component
	 */
	void StopActiveSound(uint64 AtomComponentID);

	/**
	 * Pauses the active sound for the specified Atom component
	 */
	void PauseActiveSound(uint64 AtomComponentID, const bool bInIsPaused);

	/** Notify that a pending async occlusion trace finished on the active sound. */
	void NotifyActiveSoundOcclusionTraceDone(FAtomActiveSound* ActiveSound, bool bIsOccluded);

	/**
	 * Whether a given Audio Component ID should be allowed to have multiple
	 * associated Active Sounds
	 */
	bool CanHaveMultipleActiveSounds(uint64 AtomComponentID) const;

	/**
	 * Set whether a given Audio Component ID should be allowed to have multiple
	 * associated Active Sounds
	 */
	void SetCanHaveMultipleActiveSounds(uint64 AtomComponentID, bool InCanHaveMultipleActiveSounds);

	/**
	 * Removes an active sound from the active sounds array
	 */
	void RemoveActiveSound(FAtomActiveSound* ActiveSound);

	/**
	 * Marks a sound to be stopped.  Returns true if added to stop,
	 * false if already pending stop.
	 */
	void AddSoundToStop(FAtomActiveSound* SoundToStop);

	/**
	 * Whether the provided ActiveSound is currently pending to stop.
	 */
	bool IsPendingStop(FAtomActiveSound* ActiveSound);

	/**
	 * Stops all game sounds (and possibly UI) sounds
	 *
	 * @param bShouldStopUISounds If true, this function will stop UI sounds as well
	 */
	virtual void StopAllSounds(bool bShouldStopUISounds = false);

	/** Performs an operation on all active sounds requested to execute by an audio component. */
	void SendCommandToActiveSounds(uint64 InAtomComponentID, TUniqueFunction<void(FAtomActiveSound&)> InFunc, const TStatId InStatId = TStatId());

	/** Disables ActiveSound from responding to calls from its associated AtomComponent. */
	void UnlinkActiveSoundFromComponent(const FAtomActiveSound& InActiveSound);

	const TArray<FAtomActiveSound*>& GetActiveSounds() const { check(IsInAtomThread()); return ActiveSounds; }

	const TMap<FAtomActiveSound*, FAtomVirtualLoop>& GetVirtualLoops() const { check(IsInAtomThread()); return VirtualLoops; }

	FAtomSource* GetSoundSource(const FAtomPlaybackInstance* InPlaybackInstance) const;

	const TArray<FAtomPlaybackInstance*>& GetActivePlaybackInstances() const { check(IsInAtomThread()); return ActivePlaybackInstances; }

	const TMap<FAtomPlaybackInstance*, FAtomSource*>& GetPlaybackInstanceSourceMap() const { return PlaybackInstanceSourceMap; }

	/** Overrides the attenuation scale used on a sound class. */
	void SetSoundClassDistanceScale(UAtomSoundClass* InSoundClass, float DistanceScale, float TimeSec);

	/** Returns the current audio device update delta time. */
	float GetRuntimeDeltaTime() const;

	/** Returns the game's delta time */
	float GetGameDeltaTime() const;

	/** Returns the max channels used by the runtime. */
	int32 GetMaxChannels() const;

	/** Returns the maximum sources used by the runtime set on initialization,
	  * including the number of stopping sources reserved. */
	int32 GetMaxSources() const;

	/** Returns global pitch range. */
	TRange<float> GetGlobalPitchRange() const;

	float GetTransientPrimaryVolume() const { check(IsInAtomThread()); return TransientPrimaryVolume; }
	void SetTransientPrimaryVolume(float TransientPrimaryVolume);

	/** Returns the volume that combines transient master volume and the FApp::GetVolumeMultiplier() value */
	float GetPrimaryVolume() const { return PrimaryVolume; }

	const FAtomGlobalFocusSettings& GetGlobalFocusSettings() const;
	void SetGlobalFocusSettings(const FAtomGlobalFocusSettings& NewFocusSettings);

	const FAtomDynamicParameter& GetGlobalPitchScale() const { check(IsInAtomThread()); return GlobalPitchScale; }
	void SetGlobalPitchModulation(float PitchScale, float TimeSec);
	float ClampPitch(float InPitchScale) const;

	bool IsAtomRuntimeMuted() const;

	void SetRuntimeMuted(bool bMuted);

	/** Returns the azimuth angle of the sound relative to the sound's nearest listener. Used for 3d audio calculations. */
	void GetAzimuth(const FAtomAttenuationListenerData& OutListenerData, float& OutAzimuth, float& AbsoluteAzimuth) const;

	/** Returns the focus factor of a sound based on its position and listener data. */
	float GetFocusFactor(const float Azimuth, const FAtomAttenuationSettings& AttenuationSettings) const;

	/** Gets the max distance and focus factor of a sound. */
	void GetMaxDistanceAndFocusFactor(UAtomSoundBase* Sound, const UWorld* World, const FVector& Location, const FAtomAttenuationSettings* AttenuationSettingsToApply, float& OutMaxDistance, float& OutFocusFactor);

	/**
	* Checks if the given sound would be audible.
	* @param Sound					The sound to check if it would be audible
	* @param World					The world the sound is playing in
	* @param Location				The location the sound is playing in the world
	* @param AttenuationSettings	The (optional) attenuation settings the sound is using
	* @param MaxDistance			The computed max distance of the sound.
	* @param FocusFactor			The focus factor of the sound.
	*
	* @return Returns true if the sound is audible, false otherwise.
	*/
	bool SoundIsAudible(UAtomSoundBase* Sound, const UWorld* World, const FVector& Location, const FAtomAttenuationSettings* AttenuationSettingsToApply, float MaxDistance, float FocusFactor) const;

	/** Set whether or not we force the use of attenuation for non-game worlds (as by default we only care about game worlds) */
	void SetUseAttenuationForNonGameWorlds(bool bInUseAttenuationForNonGameWorlds)
	{
		bUseAttenuationForNonGameWorlds = bInUseAttenuationForNonGameWorlds;
	}

	/** Resets all interpolating values to defaults. */
	void ResetInterpolation();

	/** Suspend any context related objects */
	virtual void SuspendContext() {}

	/** Resume any context related objects */
	virtual void ResumeContext() {}

	// Pushes the command to a MPSC queue to be executed on the game thread
	void GameThreadMPSCCommand(TFunction<void()> InCommand);

	// Subsystems

	/**
	 * Gathers data about interior volumes affecting the active sound (called on Atom thread)
	 */
	void GatherInteriorData(FAtomActiveSound& ActiveSound, FAtomSoundParseParameters& ParseParams) const;

	/**
	 * Applies interior settings from affecting volumes to the active sound (called on Atom thread)
	 */
	void ApplyInteriorSettings(FAtomActiveSound& ActiveSound, FAtomSoundParseParameters& ParseParams) const;

	/** Notifies subsystems an active sound is about to be deleted. */
	void NotifyPendingDelete(FAtomActiveSound& ActiveSound) const;

	/** Get a Subsystem of specified type. */
	UAtomEngineSubsystem* GetSubsystemBase(TSubclassOf<UAtomEngineSubsystem> SubsystemClass) const
	{
		return SubsystemCollection.GetSubsystem<UAtomEngineSubsystem>(SubsystemClass);
	}

	/** Get a Subsystem of specified type. */
	template <typename TSubsystemClass>
	TSubsystemClass* GetSubsystem() const
	{
		return SubsystemCollection.GetSubsystem<TSubsystemClass>(TSubsystemClass::StaticClass());
	}

	/**
	 * Get a Subsystem of specified type from the provided AtomRuntimeHandle.
	 * Returns nullptr if the Subsystem cannot be found or the AtomRuntimeHandle is invalid.
	 */
	template <typename TSubsystemClass>
	static FORCEINLINE TSubsystemClass* GetSubsystem(const FAtomRuntimeHandle& InHandle)
	{
		if (InHandle.IsValid())
		{
			return InHandle->GetSubsystem<TSubsystemClass>();
		}
		return nullptr;
	}

	/**
	 * Gets all Subsystems of specified type, this is only necessary for interfaces that can have multiple implementations instanced at a time.
	 * Do not hold onto this Array reference unless you are sure the lifetime is less than that of the Atom runtime.
	 */
	template <typename TSubsystemClass>
	const TArray<TSubsystemClass*>& GetSubsystemArray() const
	{
		return SubsystemCollection.GetSubsystemArray<TSubsystemClass>(TSubsystemClass::StaticClass());
	}

protected:

	/**
	 * Handle pausing/unpausing of sources when entering or leaving pause mode, or global pause (like device suspend)
	 */
	void HandlePause(bool bGameTicking, bool bGlobalPause = false);

	/**
	 * Set up the initial sound sources for cue.
	 * Allows us to initialize sound source early on.
	 */
	void InitSoundSources();

	/** Creates a new Atom source for any playback with single or mixed voices. (AtomExPlayer) */
	FAtomSource* CreateSoundSource();

	/** Creates a new Atom source for single, non 3d, voice playback. (AtomPlayer) */
	FAtomSource* CreateVoiceSource();

	/**
	 * Stop sources that need to be stopped, and touch the ones that need to be kept alive
	 * Stop sounds that are too low in priority to be played
	 */
	void StopSources(TArray<FAtomPlaybackInstance*>& InActiveSounds, int32 FirstActiveIndex);

	/**
	 * Start and/or update any sources that have a high enough priority to play
	 */
	void StartSources(TArray<FAtomPlaybackInstance*>& InActiveSounds, int32 FirstActiveIndex, bool bGameTicking);

	/**
	 * Iterate over the active AtomComponents for active sounds that could be playing.
	 *
	 * @return 
	 */
	int32 GetSortedActivePlaybackInstances(TArray<FAtomPlaybackInstance*>& InActiveSounds , const ESortedActivePlaybackGetType GetType);
	//int32 GetSortedPlaybackInstances(TArray<FAtomActiveSound*>& InActiveSounds);

	/**
	 * This is overridden in Audio::FMixerDevice to propogate listener information to the audio thread.
	 */
	//virtual void OnListenerUpdated(const TArray<FAtomListener>& InListeners) {};

	/** Processes the set of pending sounds that need to be stopped. */
	void ProcessingPendingActiveSoundStops(bool bForceDelete = false);

	/** Stops oldest sound source. */
	void StopOldestStoppingSource();

	/** */
	void VirtualizeInactiveLoops();

	/** Remove virtualized sound to list. */
	bool RemoveVirtualLoop(FAtomActiveSound& ActiveSound);

	/** Update status of dormant virtualized sounds/loops. */
	void UpdateVirtualLoops(bool bForceUpdate);

	/** Sets the update delta time for the audio frame */
	void UpdateRuntimeDeltaTime();

	/** Update bus effect parameters that have been changed. */
	void UpdateBusEffectParameters();

	/** */
	void UpdateAtomSoundRender();

	/** Update the active sound playback time. This is done here to do after all audio is updated. */
	void UpdateActiveSoundPlaybackTime(bool bIsGameTicking);

	/** Check whether we should use attenuation settings. */
	bool ShouldUseAttenuation(const UWorld* World) const;

	/**
	 * Adds an active sound to the Atom runtime.
	 */
	void AddNewActiveSoundInternal(const FAtomActiveSound& InActiveSound, FAtomVirtualLoop* InVirtualLoopToRetrigger = nullptr);

	/**
	 * Reports if a sound fails to start when attempting to create a new active sound.
	 */
	void ReportSoundFailedToStart(const uint64 AtomComponentID, FAtomVirtualLoop* VirtualLoop);

	/** 
	 * Stops quiet/low priority sounds due to being evaluated as not fulfilling concurrency requirements.
	 */
	void UpdateConcurrency(TArray<FAtomPlaybackInstance*>& PlaybackInstances, TArray<FAtomActiveSound*>& ActiveSoundsCopy);

	/**
	 * Checks if the given sound would be audible.
	 * @param NewActiveSound	The ActiveSound attempting to be created
	 * @return True if the sound is audible, false otherwise.
	 */
	bool SoundIsAudible(const FAtomActiveSound& NewActiveSound);

	/** Create our subsystem collection root object and initialize subsystems */
	void InitializeSubsystemCollection();

	/** Updates Atom engine subsystems on this runtimr. */
	void UpdateAtomEngineSubsystems();

	/** */
	void SendUpdateResultsToGameThread(int32 FirstActiveIndex);

	/** */
	void UpdateProfilers();

private:

	// Pumps the command queue
	void PumpGameThreadCommandQueue();

public: // static tools

	/** Returns the main Atom runtime of the engine */
	static FAtomRuntimeHandle GetMainAtomRuntime()
	{
		// Try to get GCriWare's main atom runtime
		FAtomRuntimeHandle AtomRuntime = GCriWare->GetMainAtomRuntime();

		// If we don't have a main Atom runtime (maybe we're running in a non-standard mode like a commandlet)
		if (!AtomRuntime)
		{
			// We should have an active runtime for device manager
			FAtomRuntimeManager* RuntimeManager = GCriWare->GetAtomRuntimeManager();
			return RuntimeManager->GetActiveAtomRuntime();
		}
		return AtomRuntime;
	}

	/** Returns the Atom runtime manager */
	static FAtomRuntimeManager* GetAtomRuntimeManager()
	{
		return GCriWare->GetAtomRuntimeManager();
	}

	/** Whether or not there's a source data override plugin enabled. */
	bool IsSourceDataOverridePluginEnabled() const
	{
		return bSourceDataOverrideInterfaceEnabled;
	}

	/** Whether or not there's a source data override plugin enabled. */
	static bool IsSourceDataOverridePluginLoaded()
	{
		if (FAtomRuntimeHandle MainAtomRuntime = GCriWare->GetMainAtomRuntime())
		{
			return MainAtomRuntime->bSourceDataOverrideInterfaceEnabled;
		}
		return false;
	}

	/** Whether or not there's a Mana runtime plugin enabled. */
	bool IsManaRuntimePluginEnabled() const
	{
		return bManaRuntimeInterfaceEnabled;
	}

	/** Generic platforms ASR settings */
	static CriAtomSoundRendererType GetAtomExSoundRendererType(EAtomSoundRendererType RendererType);
	static int32 GetAtomExSoundRendererNumChannels(EAtomSoundRendererType RendererType);
	static int32 GetAtomExSoundRendererNumPorts(EAtomSoundRendererType RendererType);
	static CriAtomSoundRendererType GetSoundfieldAtomExSoundRendererType(EAtomSoundfieldRendererType SoundfieldRendererType);
	static int32 GetSoundfieldAtomExSoundRendererNumChannels(EAtomSoundfieldRendererType SoundfieldRendererType);
	static CriAtomSpeakerMapping GetSoundfieldAtomExSoundRendererSpeakerMapping(EAtomSoundfieldRendererType SoundfieldRendererType);

	static EAtomFormat GetFormatFromAtomExFormat(CriAtomExFormat AtomExFormat);
	static EAtomPcmBitDepth GetPcmBitDepthFromAtomPcmFormat(CriAtomPcmFormat AtomPcmFormat);

	static int32 GetAtomDefaultVoicePoolIdentifier();

private:

	/** The handle for this Atom runtime used in the Atom manager. */
	FAtomRuntimeId RuntimeID;

	/** The number of sources to reserve for stopping sounds. */
	int32 NumReservedSources;

	/** The maximum number of sources. Value cannot change after initialization. */
	int32 MaxSources;

	/** Whether Atom is active (current Atom in-focus in PIE) */
	uint8 bIsRuntimeMuted : 1;

	/** Whether Atom has been initialized */
	uint8 bIsInitialized : 1;

	/** Whether Native Atom runtime library has been initialized */
	uint8 bIsAtomLibraryInitialized : 1;
	
	/** Whether master rack has been initialized */
	uint8 bIsMasterRackInitialized : 1;

	/** Gamethread representation of whether HRTF is enabled for all 3d sounds. (not bitpacked to avoid thread issues) */
	bool bHRTFEnabledForAll_OnGameThread;

	/** Gamethread representation of whether HRTF is disabled for all 3d sounds. */
	bool bHRTFDisabled_OnGameThread;

	/** Whether HRTF is enabled for all 3d sounds. This will automatically make all 3d mono sounds HRTF spatialized. */
	uint8 bHRTFEnabledForAll : 1;

	/** Whether or not HRTF is disabled. This will make any sounds which are set to HRTF spatialize to spatialize with panning. */
	uint8 bHRTFDisabled : 1;

	uint8 bGameWasTicking : 1;

	/** Whether or not the mixer voice management is being used. */
	uint8 bUseMixerVoiceManagement : 1;

	/** Whether or not Atom routes any rack audio outputs to Unreal audio. */
	uint8 bUseUnrealSoundRenderer : 1;

	/** Whether or not we force the use of attenuation for non-game worlds (as by default we only care about game worlds) */
	uint8 bUseAttenuationForNonGameWorlds : 1;

	/** Whether or not there's a source data override plugin enabled. */
	uint8 bSourceDataOverrideInterfaceEnabled : 1;

	/** Whether or not there's a Mana Runtime plugin enabled. */
	uint8 bManaRuntimeInterfaceEnabled : 1;

	/** Set of currently used sounds. */
	TArray<FAtomActiveSound*> ActiveSounds;

	/** Array of sound waves to add references to avoid GC until guaranteed to be done with precache or decodes. */
	TArray<UAtomSoundBase*> ReferencedSounds;

	void UpdateReferencedSounds();

	TArray<UAtomSoundBase*> ReferencedSounds_AtomThread;
	FCriticalSection ReferencedSoundCriticalSection;
	TArray<UAtomSoundBase*> PrecachingSounds;

	TArray<FAtomPlaybackInstance*> ActivePlaybackInstances;

	/** Array of dormant sound/loops stopped due to proximity/applicable concurrency rules that can be retriggered. */
	TMap<FAtomActiveSound*, FAtomVirtualLoop> VirtualLoops;

	/** Set of sounds which will be stopped next frame update */
	TSet<FAtomActiveSound*> PendingSoundsToStop;

	/** A set of sounds which need to be deleted but weren't able to be deleted due to pending async operations */
	TArray<FAtomActiveSound*> PendingSoundsToDelete;

	/** Map of Atom component to active sounds. */
	TMap<uint64, TArray<FAtomActiveSound*>> AtomComponentIDToActiveSoundMap;

	/** Used to determine if a given Audio Component should be allowed to have multiple simultaneous associated active sounds */
	TMap<uint64, bool> AtomComponentIDToCanHaveMultipleActiveSoundsMap;

	/** Map of default Atom volume settings for worlds. */
	TMap<uint32, TPair<FAtomReverbSettings, FAtomInteriorSettings>> WorldIDToDefaultAtomVolumeSettingsMap;

	uint64 CurrentTick;

	/** The audio clock from the Native Atom runtime. */
	double AtomClock;

	float RuntimeDeltaTime;

	/** Timestamp of the last update */
	double LastUpdateTime;

	/** A count of the number of one-shot active sounds. */
	uint32 OneShotCount;

	/** transient master volume multiplier that can be modified at runtime without affecting user settings automatically reset to 1.0 on level change */
	float TransientPrimaryVolume;

	/** The master volume of the game combines the FApp::GetVolumeMultipler() value and the TransientPrimaryVolume. */
	float PrimaryVolume;

	/** What the previous master volume was. */
	float PreviousPrimaryVolume;

	/** The global focus settings */
	FAtomGlobalFocusSettings GlobalFocusSettings;
	FAtomGlobalFocusSettings GlobalFocusSettings_GameThread;

	/** Global dynamic pitch scale parameter */
	FAtomDynamicParameter GlobalPitchScale;

	// Global min and max pitch scale, derived from audio settings
	float GlobalMinPitch;
	float GlobalMaxPitch;

	// Handle for our runtime created delegate
	FDelegateHandle RuntimeCreatedHandle;

	// Handle for our runtime destroyed delegate
	FDelegateHandle RuntimeDestroyedHandle;

public: // Debug & Tests

	/**
	 * Gets the current Atom debug state
	 */
	EAtomDebugState GetMixDebugState() const { return((EAtomDebugState)DebugState); }

	void SetMixDebugState(EAtomDebugState DebugState);

	/** The debug state of the audio device */
	EAtomDebugState DebugState;

#if !UE_BUILD_SHIPPING
	UAtomComponent* GetTestComponent(UWorld* InWorld);
	void StopTestComponent();

	/** Structure for collating info about sound categories */
	struct FAtomClassDebugInfo
	{
		int32 NumResident = 0;
		int32 SizeResident = 0;
		int32 NumRealTime = 0;
		int32 SizeRealTime = 0;
	};

private:

	/** An AtomComponent to play test sounds on */
	TStrongObjectPtr<UAtomComponent> TestAtomComponent;

public:

	void HandleAtomMemoryInfo(const TArray<FString>& Args, FOutputDevice& Ar);
	/**
	 * Exec command handlers.
	 */
	void HandleDumpSoundInfoCommand(FOutputDevice& Ar);

	/**
	 * Lists all the playing playback instances and their associated source.
	 */
	void HandleListPlaybacksCommand(FOutputDevice& Ar);
	
	/**
	 * Lists a summary of loaded sound collated by class.
	 */
	void HandleListSoundClassesCommand(FOutputDevice& Ar);
	void HandleListSoundClassVolumesCommand(FOutputDevice& Ar);

	void HandleListAtomComponentsCommand(FOutputDevice& Ar);
	void HandleListSoundDurationsCommand(FOutputDevice& Ar);
	
	/**
	 * shows sound class hierarchy.
	 */
	void HandleShowSoundClassHierarchyCommand(FOutputDevice& Ar);

	void HandleSoloCommand(FOutputDevice& Ar);
	void HandleClearSoloCommand(FOutputDevice& Ar);
	void HandleAtomSoloSoundClass(const TArray<FString>& Args);
	void HandleAtomSoloSoundWave(const TArray<FString>& Args);
	void HandleAtomSoloSoundCue(const TArray<FString>& Args);

	void HandlePlayAllPIEAtomCommand(FOutputDevice& Ar);
	//void HandleAtom3dVisualizeCommand(FOutputDevice& Ar);

	void HandleAtomMixDebugSound(const TArray<FString>& Args);
	void HandleAtomDebugSound(const TArray<FString>& Args);
	void HandleSetBaseSoundRackCommand(const TArray<FString>& Args, FOutputDevice& Ar);

	//void HandleIsolateDryAudioCommand(FOutputDevice& Ar);
	//void HandleIsolateReverbCommand(FOutputDevice& Ar);
	void HandleTestLPFCommand(FOutputDevice& Ar);
	void HandleTestHPFCommand(FOutputDevice& Ar);
	//void HandleTestLFEBleedCommand(FOutputDevice& Ar);
	void HandleDisableLPFCommand(FOutputDevice& Ar);
	void HandleDisableHPFCommand(FOutputDevice& Ar);
	//void HandleDisableRadioCommand(FOutputDevice& Ar);
	//void HandleEnableRadioCommand(FOutputDevice& Ar);
	void HandleEnableHRTFForAllCommand(FOutputDevice& Ar);
	//void HandleToggleSpatializationExtensionCommand(FOutputDevice& Ar);
	void HandleResetSoundStateCommand(FOutputDevice& Ar);

	void HandleResetAllDynamicSoundVolumesCommand();
	void HandleResetDynamicSoundVolumeCommand(const TArray<FString>& Args);
	void HandleGetDynamicSoundVolumeCommand(const TArray<FString>& Args, FOutputDevice& Ar);
	void HandleSetDynamicSoundCommand(const TArray<FString>& Args);

	/** Handles all argument parsing for the solo commands in one place */
	using FToggleSoloPtr = void (Atom::FAtomDebugger::*)(FName InName, bool bExclusive);
	void HandleAtomSoloCommon(const TArray<FString>& Args, FToggleSoloPtr FPtr);

	/**
	* Lists a summary of loaded sound collated by class
	*/
	void ShowSoundClassHierarchy(FOutputDevice& Ar, UAtomSoundClass* SoundClass = nullptr, int32 Indent = 0) const;

	/**
	* Gets a summary of loaded sound collated by class
	*/
	void GetSoundClassInfo(TMap<FName, FAtomClassDebugInfo>& AtomClassInfos);
#endif
};

template<typename T>
struct FCriWarePlatformAtomConfig
{
	/**
	 * Template to setup common default Atom and FileSystem values.
	 * Atom uses different structures per platform so we use a template.
	 */
	static void SetCommonAtomConfig(T& AtomConfig, CriFsConfig& FileSytemConfig)
	{
		auto Settings = GetDefault<UCriWareCoreSettings>();

		CriAtomSoundRendererType AtomRendererType = CRIATOM_SOUND_RENDERER_HW1;
		const int AsrNumChannels = CRIATOM_DEFAULT_OUTPUT_CHANNELS;

		auto SetAtomConfigLambda = [&](auto& atom_config, CriFsConfig& fs_config)
		{
			int MaxSources = Settings->MaxSoundSources + Settings->NumReservedSoundSources;

			atom_config.atom_ex.max_virtual_voices = Settings->MaxSoundSources;
			atom_config.atom_ex.max_pitch = Settings->MaxPitch;
			atom_config.atom_ex.max_parameter_blocks = MaxSources * 16;
			atom_config.atom_ex.max_categories = MaxSources;
			atom_config.atom_ex.max_sequences = MaxSources;
			atom_config.atom_ex.max_tracks = MaxSources * 2;
			atom_config.atom_ex.max_track_items = MaxSources * 2;
			atom_config.atom_ex.max_voice_limit_groups = 128;
			atom_config.atom_ex.max_categories = 128;
			atom_config.atom_ex.categories_per_playback = CRIATOMEXCATEGORY_MAX_CATEGORIES_PER_PLAYBACK;
			atom_config.asr.output_channels = AsrNumChannels;
			atom_config.asr.output_sampling_rate = CRIATOM_DEFAULT_OUTPUT_SAMPLING_RATE;
			atom_config.asr.num_buses = CRIATOMEXASR_MAX_BUSES;
			atom_config.asr.max_racks = 64;
			atom_config.asr.sound_renderer_type = AtomRendererType;
			atom_config.hca_mx.output_channels = CRIATOM_DEFAULT_OUTPUT_CHANNELS;
			atom_config.hca_mx.output_sampling_rate = Settings->HcaMxVoicesSettings.HcaMxVoiceSamplingRate;
			atom_config.hca_mx.max_sampling_rate = Settings->HcaMxVoicesSettings.HcaMxVoiceSamplingRate;
			atom_config.hca_mx.max_voices = Settings->HcaMxVoicesSettings.NumHcaMxMemoryVoices + Settings->HcaMxVoicesSettings.NumHcaMxStreamingVoices;
			atom_config.hca_mx.num_mixers = ((atom_config.hca_mx.max_voices > 0) ? 1 : 0);

			fs_config.num_binders = Settings->NumBinders;
			fs_config.max_binds = Settings->MaxBindings;
			fs_config.num_loaders = Settings->NumLoaders;
			fs_config.max_files = 0;
			fs_config.max_path = FPlatformMisc::GetMaxPathLength();
			atom_config.atom_ex.fs_config = &fs_config;
		};

		SetAtomConfigLambda(AtomConfig, FileSytemConfig);
	}
};
