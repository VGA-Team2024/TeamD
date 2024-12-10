/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCore
 * File     : Atom.h
 *
 ****************************************************************************/

#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "UObject/Interface.h"
#include "Curves/CurveFloat.h"
#include "Stats/Stats.h"

#include "CriWareApi.h"
#include "CriWareUtils.h"
#include "CriWareDefines.h"
#include "AtomAttenuation.h"
#include "AtomEnvelope.h"
#include "AtomSourceVoiceEffect.h"
#include "AtomAisacPatch.h"
#include "AtomParameter.h"
#include "Extensions/IAtomExtensionPlugin.h"
#include "Extensions/IAtomRuntimePlugin.h"

#include "Atom.generated.h"

// Atom log
CRIWARECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogCriWareAtom, Display, All);

// Special log category used for temporary programmer debugging code of audio
CRIWARECORE_API DECLARE_LOG_CATEGORY_EXTERN(LogCriWareAtomDebug, Display, All);

/**
 * Atom stats
 */
DECLARE_STATS_GROUP(TEXT("Atom"), STATGROUP_Atom, );
DECLARE_STATS_GROUP(TEXT("Atom Commands"), STATGROUP_AtomCommands, STATCAT_Advanced);

DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Active Sounds"), STAT_AtomActiveSounds, STATGROUP_Atom, );
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Virtual Sounds"), STAT_AtomVirtualLoops, STATGROUP_Atom, );
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Sources"), STAT_AtomSources, STATGROUP_Atom, );
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Playback Instances"), STAT_AtomPlaybackInstances, STATGROUP_Atom, );
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Playback Instances Dropped"), STAT_AtomPlaybacksDroppedDueToPriority, STATGROUP_Atom, );
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Audible Playback Instances Dropped"), STAT_AtomAudiblePlaybacksDroppedDueToPriority, STATGROUP_Atom, );
DECLARE_CYCLE_STAT_EXTERN(TEXT("Atom Evaluate Concurrency"), STAT_AtomEvaluateConcurrency, STATGROUP_Atom, );
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Max Channels"), STAT_AtomMaxChannels, STATGROUP_Atom, );
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Max Reserved Sources"), STAT_AtomMaxReservedSources, STATGROUP_Atom, );
DECLARE_DWORD_COUNTER_STAT_EXTERN(TEXT("Finished delegates called"), STAT_AtomSoundFinishedDelegatesCalled, STATGROUP_Atom, );
DECLARE_MEMORY_STAT_EXTERN(TEXT("Atom Memory Used"), STAT_AtomMemorySize, STATGROUP_Atom, );
DECLARE_CYCLE_STAT_EXTERN(TEXT("Gathering PlaybackInstances"), STAT_AtomGatherPlaybackInstances, STATGROUP_Atom, );
DECLARE_CYCLE_STAT_EXTERN(TEXT("Processing Sources"), STAT_AtomStartSources, STATGROUP_Atom, );
DECLARE_CYCLE_STAT_EXTERN(TEXT("Updating Sources"), STAT_AtomUpdateSources, STATGROUP_Atom, CRIWARECORE_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Source Init"), STAT_AtomSourceInitTime, STATGROUP_Atom, CRIWARECORE_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("Finished delegates time"), STAT_AtomSoundFinishedDelegates, STATGROUP_Atom, );
DECLARE_CYCLE_STAT_EXTERN(TEXT("Finding Nearest Location"), STAT_AtomFindNearestLocation, STATGROUP_Atom, );

// Forward declarations
class FCriWareCore;
class FAtomRuntime;
class UAtomRackBase;
class UAtomRack;
class UAtomBus;
class UAtomSoundBase;
class UAtomSoundWave;
class UAtomSoundClass;
class UAtomSourceDataOverridePluginSourceSettingsBase;
struct FAtomActiveSound;
struct FAtomBusSend;
struct FAtomPlaybackInstance;
struct FAtomAttenuationBusSendSettings;

/**
 * Typed identifier for Atom Runtime Id
 */
using FAtomRuntimeId = uint32;

/**
 * Typed identifier for Atom Resource Id
 */
using FAtomResourceId = uint32;

// Internal use
enum class EAtomLoopingMode : uint8
{
	/** One shot sound */
	LoopNever,
	/** Call the user callback on each loop for dynamic control */
	LoopWithNotification,
	/** Loop the sound forever */
	LoopForever
};

UENUM(BlueprintType)
enum class EAtomFormat : uint8
{
	/** No Format */
	None = 0,

	/** CRI's Adaptive Differential Extended sound. */
	ADX,

	/** CRI's High Compression Audio sound. */
	HCA,

	/** CRI's High Compression Audio Mix sound. */
	HCAMX		UMETA(DisplayName = "HCA-MX"),

	/** AIFF format sound */
	AIFF,

	/** Wave format sound */
	Wave,

	/** Raw PCM sound */
	RawPCM		UMETA(DisplayName = "Raw PCM"),

	/** Haptic vibrations */
	Vibration,

	/** Audio buffer */
	AudioBuffer UMETA(DisplayName = "Audio Buffer"),

	/** Synthesizer */
	Instrument,

	/** Hardware 1 sound (VAG or OPUS) */
	Hardware1,

	/** Hardware 2 sound (ATRAC) */
	Hardware2,

	Num	UMETA(hidden),
};

inline const TCHAR* ToString(EAtomFormat Format)
{
	switch (Format) {
	case EAtomFormat::None:			return TEXT("None");
	case EAtomFormat::ADX:			return TEXT("ADX");
	case EAtomFormat::HCA:			return TEXT("HCA");
	case EAtomFormat::HCAMX:		return TEXT("HCA-MX");
	case EAtomFormat::AIFF:			return TEXT("AIFF");
	case EAtomFormat::Wave:			return TEXT("Wave");
	case EAtomFormat::RawPCM:		return TEXT("Raw PCM");
	case EAtomFormat::Vibration:	return TEXT("Vibration");
	case EAtomFormat::AudioBuffer:	return TEXT("Audio Buffer");
	case EAtomFormat::Instrument:	return TEXT("Instrument");
	case EAtomFormat::Hardware1:	return TEXT("Hardware1");
	case EAtomFormat::Hardware2:	return TEXT("Hardware2");
	default:						return TEXT("Unknown");
	}
}

/**
 * Channel definitions for multistream waves
 *
 * These are in the sample order Atom expects for a 7.1.4 sound.
 *
 */
UENUM()
enum class EAtomSpeaker : uint8
{
	FrontLeft,
	FrontRight,
	FrontCenter,
	LowFrequency,
	SurroundLeft,
	SurroundRight,
	SurroundBackLeft,
	SurroundBackRight,
	TopFrontLeft,
	TopFrontRight,
	TopRearLeft,
	TopRearRight,
	Count,
	Unknown,
	DefaultChannel = FrontLeft
};

inline const TCHAR* ToString(EAtomSpeaker InSpeaker)
{
	switch (InSpeaker)
	{
	case EAtomSpeaker::FrontLeft:			return TEXT("FrontLeft");
	case EAtomSpeaker::FrontRight:			return TEXT("FrontRight");
	case EAtomSpeaker::FrontCenter:			return TEXT("FrontCenter");
	case EAtomSpeaker::LowFrequency:		return TEXT("LowFrequency");
	case EAtomSpeaker::SurroundLeft:		return TEXT("SurroundLeft");
	case EAtomSpeaker::SurroundRight:		return TEXT("SurroundRight");
	case EAtomSpeaker::SurroundBackLeft:	return TEXT("SurroundBackLeft");
	case EAtomSpeaker::SurroundBackRight:	return TEXT("SurroundBackRight");
	case EAtomSpeaker::TopFrontLeft:		return TEXT("TopFrontLeft");
	case EAtomSpeaker::TopFrontRight:		return TEXT("TopFrontRight");
	case EAtomSpeaker::TopRearLeft:			return TEXT("TopRearLeft");
	case EAtomSpeaker::TopRearRight:		return TEXT("TopRearRight");
	case EAtomSpeaker::Unknown:				return TEXT("Unknown");

	default:
		return TEXT("UNSUPPORTED");
	}
}

#if WITH_EDITOR
inline const TCHAR* ToAbbreviatedString(EAtomSpeaker InSpeaker)
{
	switch (InSpeaker)
	{
	case EAtomSpeaker::FrontLeft:			return TEXT("FL");
	case EAtomSpeaker::FrontRight:			return TEXT("FR");
	case EAtomSpeaker::FrontCenter:			return TEXT("FC");
	case EAtomSpeaker::LowFrequency:		return TEXT("LFE");
	case EAtomSpeaker::SurroundLeft:		return TEXT("SL");
	case EAtomSpeaker::SurroundRight:		return TEXT("SR");
	case EAtomSpeaker::SurroundBackLeft:	return TEXT("SBL");
	case EAtomSpeaker::SurroundBackRight:	return TEXT("SBR");
	case EAtomSpeaker::TopFrontLeft:		return TEXT("TFL");
	case EAtomSpeaker::TopFrontRight:		return TEXT("TFR");
	case EAtomSpeaker::TopRearLeft:			return TEXT("TRL");
	case EAtomSpeaker::TopRearRight:		return TEXT("TRR");
	case EAtomSpeaker::Unknown:				return TEXT("Unknown");

	default:
		return TEXT("UNSUPPORTED");
	}
}
#endif

/** DSP Bus send Stage */
UENUM(BlueprintType)
enum class EAtomBusSendStage : uint8
{
	PreEffect,
	PostEffect,
};

UENUM(BlueprintType)
enum class EAtomPcmBitDepth : uint8
{
	/** Signed Integer 16-bits */
	Int16,

	/** Float 32-bits */
	Float32,

	Num	UMETA(hidden),
};

inline const TCHAR* ToString(EAtomPcmBitDepth PcmBitDepth)
{
	switch (PcmBitDepth) {
	case EAtomPcmBitDepth::Int16:	return TEXT("Int16");
	case EAtomPcmBitDepth::Float32:	return TEXT("Float32");
	default:						return TEXT("Unknown");
	}
}

USTRUCT(BlueprintType)
struct CRIWARECORE_API FAtomWaveInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atom")
	int32 WaveID = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atom")
	EAtomFormat Format = EAtomFormat::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atom")
	int32 SampleRate = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atom")
	int32 NumChannels = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atom")
	int32 NumFrames = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atom")
	uint32 bIsStreamed : 1;

	FORCEINLINE float GetDuration() const
	{
		if (SampleRate > KINDA_SMALL_NUMBER)
		{
			return (float)(NumFrames) / SampleRate;
		}
		return 0.0f;
	}

	FAtomWaveInfo()
		: bIsStreamed(false) 
	{}
};

/** Common base Interface for Atom sound assets (SoundBank and SoundBase) **/
UINTERFACE(MinimalApi, meta = (CannotImplementInterfaceInBlueprint))
class UAtomSoundAsset : public UInterface
{
	GENERATED_BODY()
};

class IAtomSoundAsset
{
	GENERATED_BODY()
};

/** Native Sound Renderer Type */
UENUM()
enum class EAtomSoundRendererType : uint8
{
	/* Default output */
	Default UMETA(DisplayName = "Default"),

	/* Main virtual output */
	Main UMETA(DisplayName = "Main"),

	/* BGM virtual output */
	BGM UMETA(DisplayName = "BGM"),

	/* Voice virtual output */
	Voice UMETA(DisplayName = "Voice"),

	/* Pad Speaker output */
	Pad UMETA(DisplayName = "Pad/Controller Speaker"),

	/* Haptic pad/device output */
	Haptic UMETA(DisplayName = "Haptic/Vibrations"),

	/* User virtual output */
	User UMETA(DisplayName = "User"),

	/* Auxiliary virtual output */
	Aux UMETA(DisplayName = "Aux"),

	// internal
	/* Output to another sound renderer. */
	Asr UMETA(Hidden),

	/* Do not output. */
	Muted UMETA(Hidden),

	Num UMETA(Hidden)
};

inline const TCHAR* ToString(EAtomSoundRendererType SoundRendererType)
{
	switch (SoundRendererType) {
	case EAtomSoundRendererType::Default:	return TEXT("Default");
	case EAtomSoundRendererType::Main:		return TEXT("Main");
	case EAtomSoundRendererType::BGM:		return TEXT("BGM");
	case EAtomSoundRendererType::Voice:		return TEXT("Voice");
	case EAtomSoundRendererType::Pad:		return TEXT("Pad");
	case EAtomSoundRendererType::Haptic:	return TEXT("Haptic");
	case EAtomSoundRendererType::User:		return TEXT("User");
	case EAtomSoundRendererType::Aux:		return TEXT("Aux");
	case EAtomSoundRendererType::Asr:		return TEXT("Asr");
	case EAtomSoundRendererType::Muted:		return TEXT("Muted");
	}
	ensure(false);
	return TEXT("Unknown");
}

/** Native Soundfield Renderer Type */
UENUM()
enum class EAtomSoundfieldRendererType : uint8
{
	/* Default channel spatialization without any soundfield encoding. Same as a default Rack with default settings. */
	Default UMETA(DisplayName = "Default - No Encoding"),

	/* Channel based spatialization. Encode to HRTF spatialization when binauralizer is enabled. */
	Spatializer UMETA(DisplayName = "Channel Spatialization"),

	/* Ambisonics based spatialization. Encode to HRTF spatialization when binauralizer is enabled. */
	Ambisonics UMETA(DisplayName = "Ambisonics"),

	/* Object based spatialization (Experimental: limited to 16 monaural sources). Encode to HRTF spatialization when binauralizer is enabled. */
	SoundObject UMETA(DisplayName = "Sound Object (Experimental)"),

	Num UMETA(Hidden)
};

inline const TCHAR* ToString(EAtomSoundfieldRendererType SoundRendererType)
{
	switch (SoundRendererType) {
	case EAtomSoundfieldRendererType::Default:		return TEXT("Default");
	case EAtomSoundfieldRendererType::Spatializer:	return TEXT("Spatializer");
	case EAtomSoundfieldRendererType::Ambisonics:	return TEXT("Ambisonics");
	case EAtomSoundfieldRendererType::SoundObject:	return TEXT("SoundObject");
	}
	ensure(false);
	return TEXT("Unknown");
}

/** Enumeration for each voices pool type. */
UENUM()
enum class EAtomVoicePoolType : uint8
{
	StandardMemory = 0,
	StandardStreaming,
	HcaMxMemory,
	HcaMxStreaming,
	WaveMemory,
	WaveStreaming,
	AiffMemory,
	AiffStreaming,
	RawPcm,
	InputPort,
	External,

	Audio3dMemory,
	Audio3dStreaming,
	OpusMemory,
	OpusStreaming,

	UserDefined,

	Undefined = 0xff
};

inline const TCHAR* ToString(EAtomVoicePoolType VoicePoolType)
{
	switch (VoicePoolType) {
	case EAtomVoicePoolType::StandardMemory:	return TEXT("StandardMemory");
	case EAtomVoicePoolType::StandardStreaming:	return TEXT("StandardStreaming");
	case EAtomVoicePoolType::HcaMxMemory:		return TEXT("HcaMxMemory");
	case EAtomVoicePoolType::HcaMxStreaming:	return TEXT("HcaMxStreaming");
	case EAtomVoicePoolType::WaveMemory:		return TEXT("WaveMemory");
	case EAtomVoicePoolType::WaveStreaming:		return TEXT("WaveStreaming");
	case EAtomVoicePoolType::AiffMemory:		return TEXT("AiffMemory");
	case EAtomVoicePoolType::AiffStreaming:		return TEXT("AiffStreaming");
	case EAtomVoicePoolType::RawPcm:			return TEXT("RawPcm");
	case EAtomVoicePoolType::InputPort:			return TEXT("InputPort");
	case EAtomVoicePoolType::External:			return TEXT("External");
	case EAtomVoicePoolType::Audio3dMemory:		return TEXT("Audio3dMemory");
	case EAtomVoicePoolType::Audio3dStreaming:	return TEXT("Audio3dStreaming");
	case EAtomVoicePoolType::OpusMemory:		return TEXT("OpusMemory");
	case EAtomVoicePoolType::OpusStreaming:		return TEXT("OpusStreaming");
	}
	ensure(false);
	return TEXT("Unknown");
}

/**
 * Note: not yet usable per AtomSoundWave -> only AtomSoundBank currently.  
 * Determines how we are going to load or retain a given Atom asset.
 * An UAtomSoundBank's loading behavior can be overridden by cvars.
 */
UENUM()
enum class EAtomSoundWaveLoadingBehavior : uint8
{
	// If set use the default behavior defined via the atom.streamcache cvars.
	Inherited = 0,
	// If a sound must play with zero latency under any circumstance. The first chunk of any audio wave data for this asset will be loaded when this asset is loaded and retained by cache until a given UAtomWaveBank or UAtomCueSheet is either destroyed or UAtomSoundBank::Release() is called.
	RetainOnLoad = 1,
	// If you expect sounds to play shortly after they are loaded. The first chunk of any audio wave data for this asset will be loaded into cache from disk when this asset is loaded, but may be evicted to make room for other audio bank if it isn't played for a while.
	PrimeOnLoad = 2,
	// For controlled lifespan sounds. The first chunk of any audio wave data for this asset will not be loaded until this asset is played or primed with PrimeSound() function.
	LoadOnDemand = 3,
	// Force all data for this asset to live outside cache and use the non-streaming pathways. Only do it if necessary since it uses a lot of memory. (Editor will use this when asset is not saved to disk.)
	ForceInline = 4			UMETA(DisplayName = "Force Inline"),
	// This value is used to delineate when the value of EAtomSoundWaveLoadingBehavior hasn't been cached on a UAtomSoundBank yet.
	Uninitialized = 0xff	UMETA(Hidden)
};

inline const TCHAR* ToString(EAtomSoundWaveLoadingBehavior InCurrentState)
{
	switch (InCurrentState) {
	case EAtomSoundWaveLoadingBehavior::Inherited:		return TEXT("Inherited");
	case EAtomSoundWaveLoadingBehavior::RetainOnLoad:	return TEXT("RetainOnLoad");
	case EAtomSoundWaveLoadingBehavior::PrimeOnLoad:	return TEXT("PrimeOnLoad");
	case EAtomSoundWaveLoadingBehavior::LoadOnDemand:	return TEXT("LoadOnDemand");
	case EAtomSoundWaveLoadingBehavior::ForceInline:	return TEXT("ForceInline");
	case EAtomSoundWaveLoadingBehavior::Uninitialized:	return TEXT("Uninitialized");
	}
	ensure(false);
	return TEXT("Unknown");
}

/** Queries for settings a plugin of the given type. */
CRIWARECORE_API UClass* GetAtomPluginCustomSettingsClass(EAtomPlugin PluginType);
CRIWARECORE_API TArray<UClass*> GetAtomRuntimePluginCustomSettingsClasses();

UENUM(BlueprintType)
enum class EAtomSendLevelControlMethod : uint8
{
	// A send based on linear interpolation between a distance range and send-level range
	Linear,

	// A send based on a supplied curve
	CustomCurve,

	// A manual send level (Uses the specified constant send level value. Useful for 2D sounds.)
	Manual,
};

// Class used to send audio to rack from UAtomSoundBase
USTRUCT(BlueprintType)
struct CRIWARECORE_API FAtomSoundToRackSend
{
	GENERATED_BODY()

	FAtomSoundToRackSend()
		: Rack(nullptr)
		, SendLevel(0.0f)
	{
	}

	// The rack to send the audio to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RackSend)
	TObjectPtr<UAtomRackBase> Rack;

	// The amount of audio to send
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RackSend)
	float SendLevel;
};

// Class used to send audio to bus from UAtomSoundBase
USTRUCT(BlueprintType)
struct CRIWARECORE_API FAtomSoundToBusSend
{
	GENERATED_BODY()

	FAtomSoundToBusSend()
		: SendLevelControlMethod(EAtomSendLevelControlMethod::Manual)
		, Bus(nullptr)
		, SendLevel(0.0f)
		, DisableManualSendClamp(false)
		, MinSendLevel(0.0f)
		, MaxSendLevel(1.0f)
		, MinSendDistance(100.0f)
		, MaxSendDistance(1000.0f)
	{
	}

	/*
		Manual: Use Send Level only
		Linear: Interpolate between Min and Max Send Levels based on listener distance (between Distance Min and Distance Max)
		Custom Curve: Use the float curve to map Send Level to distance (0.0-1.0 on curve maps to Distance Min - Distance Max)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	EAtomSendLevelControlMethod SendLevelControlMethod;

	// The bus to send the audio to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	TObjectPtr<UAtomBus> Bus;

	// The amount of audio to send
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	float SendLevel;

	// Whether to disable the 0-1 clamp for manual SendLevel control
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	bool DisableManualSendClamp;

	// The amount to send to master when sound is located at a distance equal to value specified in the min send distance.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	float MinSendLevel;

	// The amount to send to master when sound is located at a distance equal to value specified in the max send distance.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	float MaxSendLevel;

	// The min distance to send to the master
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	float MinSendDistance;

	// The max distance to send to the master
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	float MaxSendDistance;

	// The custom reverb send curve to use for distance-based send level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BusSend)
	FRuntimeFloatCurve CustomSendLevelCurve;
};

/* ~ Bus Send */

/** A structure with beat information from cue sequence's BeatSync when a beat appends while playing. */
USTRUCT(BlueprintType)
struct FAtomBeatSyncInfo
{
	GENERATED_BODY()

public:
	
	/** The current bar count .*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atom|Sequencer|BeatSync")
	int32 BarCount = 0;

	/** The current beat count in current bar. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atom|Sequencer|BeatSync")
	int32 BeatCount = 0;
	
	/** The current beat progression before next beat if offsetted. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atom|Sequencer|BeatSync", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BeatProgress = 0.0f;
	
	/** The currently used BPM (Beats Per Minute) value. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atom|Sequencer|BeatSync", meta = (ClampMin = "20", ClampMax = "980"))
	float BPM = 0.0f;

	/** The used BeatSync Offset value. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atom|Sequencer|BeatSync", meta = (Units = ms))
	int32 Offset = 0;
	
	/** The number of beats per bar. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atom|Sequencer|BeatSync")
	int32 NumBeatsPerBar = 0;
};

/** A structure with event information from cue's sequencer when a marker with callback is reached while playing. */
USTRUCT(BlueprintType)
struct FAtomSequencerEventInfo
{
	GENERATED_BODY()

public:

	/** Marker's Time in sequence. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atom|Sequencer|Event")
	FTimespan Time;

	/** Marker's Id. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atom|Sequencer|Event")
	int32 ID = INDEX_NONE;

	/** Marker's Tag. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atom|Sequencer|Event")
	FString Tag;
};

// Tfunction for PCM filter
using FAtomPcmFilterFunction = TFunction<void(EAtomPcmBitDepth BitDepth, int32 NumChannels, int32 NumFrames, void* Data[])>;

/**
 * Interface notified when a playback instance finishes to play.
 */
UINTERFACE(MinimalAPI)
class UAtomPlaybackFinishedNotifiedObject : public UInterface
{
	GENERATED_BODY()
};

class IAtomPlaybackFinishedNotifiedObject
{
	GENERATED_BODY()

public:

	virtual bool NotifyPlaybackInstanceFinished(FAtomPlaybackInstance* PlaybackInstanceHash) { return false; };
};

/**
 * Structure to held object to notify when a playback instance finishes to play.
 */
struct CRIWARECORE_API FAtomNotifyPlaybackFinishedHooks
{
	void AddNotify(TScriptInterface<IAtomPlaybackFinishedNotifiedObject> NotifyObject, UPTRINT PlaybackInstanceHash);
	UPTRINT GetHashForSound(TScriptInterface<IAtomPlaybackFinishedNotifiedObject> NotifyObject) const;
	void AddReferencedObjects(FReferenceCollector& Collector);
	void DispatchNotifies(FAtomPlaybackInstance* PlaybackInstance, const bool bStopped);

	friend FArchive& operator<<(FArchive& Ar, FAtomNotifyPlaybackFinishedHooks& NotifyHook);

private:

	struct FNotifyPlaybackDetails
	{
        TScriptInterface<IAtomPlaybackFinishedNotifiedObject> NotifyObject;
		UPTRINT NotifyPlaybackInstanceHash;

		FNotifyPlaybackDetails()
			: NotifyObject(nullptr)
			, NotifyPlaybackInstanceHash(0)
		{
		}

		FNotifyPlaybackDetails(TScriptInterface<IAtomPlaybackFinishedNotifiedObject> InNotifyObject, UPTRINT InHash)
			: NotifyObject(InNotifyObject)
			, NotifyPlaybackInstanceHash(InHash)
		{
		}
	};

	TArray<FNotifyPlaybackDetails> Notifies;
};


UENUM(BlueprintType)
enum class EAtomSoundPlaybackType : uint8
{
	Cue,
	Wave,
	InputPort,
	RawPCM,
	Unknown
};

USTRUCT(BlueprintType)
struct FAtomSoundPlayback
{
	GENERATED_BODY()

public:

	/** The Type of playback being played. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playbacks")
	EAtomSoundPlaybackType Type = EAtomSoundPlaybackType::Unknown;

	/** The Atom ID of the sound being played by the playback. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playbacks")
	int32 SoundID = INDEX_NONE;

	/** The name of the sound being played by the playback. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Playbacks")
	FName SoundName;

	// ID to identify the active sound.
	uint32 InstanceID = INDEX_NONE;

	// Hash to identify the playback instance.
	UPTRINT PlaybackInstanceHash = INDEX_NONE;
};

struct FAtomPlaybackInfo
{
	EAtomSoundPlaybackType Type = EAtomSoundPlaybackType::Unknown;
	int32 SoundID = INDEX_NONE;
	FName SoundName;
};

/**
 * Structure encapsulating all information required to play on a source.
 */
struct CRIWARECORE_API FAtomPlaybackInstance
{
	/** Static helper to create good unique type hashes */
	static uint32 TypeHashCounter;

	/** Sound data */
	TObjectPtr<UAtomSoundBase> SoundData;

	/** Sound class */
	TObjectPtr<UAtomSoundClass> SoundClass;

	/** Sound Rack or Endpoint object to send audio to for mixing.  */
	UAtomRackBase* SoundRack;

	/** Sound bus sends */
	TArray<FAtomSoundToBusSend> SoundBusSends;

	/** The source bus and/or audio bus sends. */
	//TArray<FSoundSourceBusSendInfo> BusSends[(int32)EBusSendType::Count];

	/** Sound effect chain */
	//USoundEffectSourcePresetChain* SourceEffectChain;

	/** Objects to notify when the current playback finishes */
	FAtomNotifyPlaybackFinishedHooks NotifyPlaybackFinishedHooks;

	/** Active Sound this playback instance belongs to. */
	FAtomActiveSound* ActiveSound;

	/** Quantized Request data */
	//TUniquePtr<Audio::FQuartzQuantizedRequestData> QuantizedRequestData;

	/** Source Buffer listener */
	//FSharedISourceBufferListenerPtr SourceBufferListener;
	//bool bShouldSourceBufferListenerZeroBuffer = false;

	FAtomPlaybackInfo SoundInfo;

private:

	/** Current volume */
	float Volume;

	/** Volume attenuation due to distance. */
	float DistanceAttenuation;

	/** Volume attenuation due to occlusion. */
	float OcclusionAttenuation;

	/** Current volume multiplier - used to zero the volume without stopping the source */
	float VolumeMultiplier;

	/** The current envelope value of the wave instance. */
	//float EnvelopValue;

public:

	/** The envelope follower attack time in milliseconds. */
	//int32 EnvelopeFollowerAttackTime;

	/** The envelope follower release time in milliseconds. */
	//int32 EnvelopeFollowerReleaseTime;

	/** An audio component priority value that scales with volume (post all gain stages) and is used to determine voice playback priority. */
	float Priority;

	/** Voice center channel volume */
	//float VoiceCenterChannelVolume;

	/** Volume of the radio filter effect */
	//float RadioFilterVolume;

	/** The volume at which the radio filter kicks in */
	//float RadioFilterVolumeThreshold;

	/** The amount of a sound to bleed to the LFE channel */
	//float LFEBleed;

	/** Looping mode - None, loop with notification, forever */
	EAtomLoopingMode LoopingMode;

	/** An offset/seek time to play this wave instance. */
	float StartTime;

	/** Whether or not to enable sending this audio's output to buses.*/
	uint32 bEnableBusSends : 1;

	/** Whether or not to render to the main rack. */
	uint32 bEnableSoundRack : 1;

	/** Whether or not to enable bus sends in addition to the Main rack. */
	uint32 bEnableSoundBusSends : 1;

	/** Whether or not to use source data overrides. */
	uint32 bEnableSourceDataOverride : 1;

	/** Set to true if the sound nodes state that the radio filter should be applied */
	//uint32 bApplyRadioFilter : 1;

	/** Whether wave instanced has been started */
	uint32 bIsStarted : 1;

	/** Whether wave instanced is finished */
	uint32 bIsFinished : 1;

	/** Whether the notify finished hook has been called since the last update/parsenodes */
	uint32 bAlreadyNotifiedHook : 1;

	/** Whether the spatialization method is an external send */
	uint32 bSpatializationIsExternalSend : 1;

private:

	/** Whether to use spatialization */
	uint32 bUseSpatialization : 1;

public:

	// Whether we have enabled amplitude envelope of this sound
	uint32 bEnableAmplitudeEnvelope : 1;

	/** Whether or not to enable the low pass filter */
	uint32 bEnableLowPassFilter : 1;

	/** Whether or not the sound is occluded. */
	uint32 bIsOccluded : 1;

	/** Whether or not this sound plays when the game is paused in the UI */
	uint32 bIsUISound : 1;

	/** Whether or not this wave is music */
	//uint32 bIsMusic : 1;

	/** Whether or not this wave has reverb applied */
	//uint32 bReverb : 1;

	/** Whether or not this sound class forces sounds to the center channel */
	uint32 bCenterChannelOnly : 1;

	/** Whether or not this sound is manually paused */
	uint32 bIsPaused : 1;

	/** Prevent spamming of spatialization of surround sounds by tracking if the warning has already been emitted */
	uint32 bReportedSpatializationWarning : 1;

	/** Whether or not this wave instance is ambisonics. */
	uint32 bIsAmbisonics : 1;

	/** Whether or not this wave instance is stopping. */
	uint32 bIsStopping : 1;

	/** Which spatialization method to use to spatialize 3d sounds. */
	EAtomSpatializationAlgorithm SpatializationMethod;

	/** The occlusion plugin settings to use for the wave instance. */
	//USpatializationPluginSourceSettingsBase* SpatializationPluginSettings;

	/** The occlusion plugin settings to use for the wave instance. */
	//UOcclusionPluginSourceSettingsBase* OcclusionPluginSettings;

	/** The occlusion plugin settings to use for the wave instance. */
	//UReverbPluginSourceSettingsBase* ReverbPluginSettings;

	/** The source data override plugin settings to use for the wave instance. */
	UAtomSourceDataOverridePluginSourceSettingsBase* SourceDataOverridePluginSettings;

	/** The runtime plugin settings array to use for the playback instance. */
	TArray<UAtomRuntimePluginSettingsBase*> AtomRuntimePluginSettingsArray;

	// Atom Categories names - per instance - cannot be changed while playback
	TArray<FName> CategoryNames;

	// AISACs - per instance
	// /!\ not per instance - TODO: remove -> this should be accessed via ActiveSound/SoundData.
	TArray<FName> AdditionalAisacPatchNames;
	// Control values.
	TArray<FAtomAisacControlParameterInfo> AisacControlParams;

	// Cue Parameters - per instance
	// Cue Selector labels.
	TArray<FAtomSelectorParam> CueSelectorParams;
	// Cue Next block index.
	int32 CueNextBlockIndex;
	// Cue BeatSync offset.
	int32 CueBeatSyncOffset;

	/** Which output target the sound should play on. */
	//EAudioOutputTarget::Type OutputTarget;

	/** The envelope to apply to volume amplitude. */
	FAtomEnvelope AmplitudeEnvelope;

	// The source voice effect (TimeStretch or PitchShift) to apply
	FAtomSourceVoiceEffect SourceVoiceEffect;

	/** The low pass filter frequency to use */
	float LowPassFilterFrequency;

	/** The low pass filter frequency to use from sound class. */
	float SoundClassFilterFrequency;

	/** The high pass filter frequency to use from sound class. */
	float SoundClassHighPassFilterFrequency;

	/** The low pass filter frequency to use if the sound is occluded. */
	float OcclusionFilterFrequency;

	/** The low pass filter frequency to use due to ambient zones. */
	float AmbientZoneFilterFrequency;

	/** The low pass filter frequency to use due to distance attenuation. */
	float AttenuationLowpassFilterFrequency;

	/** The high pass filter frequency to use due to distance attenuation. (using in audio mixer only) */
	float AttenuationHighpassFilterFrequency;

	/** Current pitch scale. */
	float Pitch;

	/** Current location and direction (scale not used). */
	FTransform Transform;

	/** At what distance we start transforming into non-spatialized soundsource */
	float NonSpatializedRadiusStart;

	/** At what distance we are fully non-spatialized*/
	float NonSpatializedRadiusEnd;

	/** How we are doing the non-spatialized radius feature. */
	EAtomNonSpatializedRadiusSpeakerMapMode NonSpatializedRadiusMode;

	/** Amount of spread for 3d multi-channel asset spatialization */
	float StereoSpread;

	/** Distance over which the sound is attenuated. */
	float AttenuationDistance;

	/** The distance from this wave instance to the closest listener. */
	float ListenerToSoundDistance;

	/** The distance from this wave instance to the closest listener. (ignoring attenuation override) */
	float ListenerToSoundDistanceForPanning;

	/** The absolute position of the wave instance relative to forward vector of listener. */
	float AbsoluteAzimuth;

	/** The playback time of the wave instance. Updated from active sound. */
	float PlaybackTime;

	/** The output reverb send level to use for tje wave instance. */
	//float ReverbSendLevel;

	/** The submix send settings to use. */
	TArray<FAtomAttenuationBusSendSettings> BusSendSettings;

	/** Cached type hash */
	uint32 TypeHash;

	/** Hash value for finding the playback instance based on the path through the cue to get to it */
	UPTRINT PlaybackInstanceHash;

	/** User / Controller index that owns the sound */
	uint8 UserIndex;

	/** Constructor, initializing all member variables. */
	FAtomPlaybackInstance(const UPTRINT PlaybackInstanceHash, FAtomActiveSound& ActiveSound);

	FAtomPlaybackInstance(FAtomPlaybackInstance&&) = default;
	FAtomPlaybackInstance& operator=(FAtomPlaybackInstance&&) = default;

	/** Stops the wave instance without notifying NotifyWaveInstanceFinishedHook. */
	void StopWithoutNotification();

	/** Notifies the wave instance that the current playback has finished. */
	void NotifyFinished(const bool bStopped = false);

	/** Friend archive function used for serialization. */
	friend FArchive& operator<<(FArchive & Ar, FAtomPlaybackInstance * PlaybackInstance);

	/** Function used by the GC. */
	void AddReferencedObjects(FReferenceCollector & Collector);

	/** Setters for various values on wave instances. */
	void SetVolume(const float InVolume) { Volume = InVolume; }
	void SetDistanceAttenuation(const float InDistanceAttenuation) { DistanceAttenuation = InDistanceAttenuation; }
	void SetOcclusionAttenuation(const float InOcclusionAttenuation) { OcclusionAttenuation = InOcclusionAttenuation; }
	void SetPitch(const float InPitch) { Pitch = InPitch; }
	void SetVolumeMultiplier(const float InVolumeMultiplier) { VolumeMultiplier = InVolumeMultiplier; }

	void SetStopping(bool bInIsStopping) { bIsStopping = bInIsStopping; }
	bool IsStopping() const { return bIsStopping; }

	/** Returns whether or not the WaveInstance is actively playing sound or set to play when silent. */
	bool IsPlaying() const;

	/** Returns the volume multiplier on the wave instance. */
	float GetVolumeMultiplier() const { return VolumeMultiplier; }

	/** Returns the actual volume the wave instance will play at, including all gain stages. */
	float GetActualVolume() const;

	/** Returns the volume of the sound including distance attenuation. */
	float GetVolumeWithDistanceAndOcclusionAttenuation() const;

	/** Returns the combined distance and occlusion attenuation of the source voice. */
	float GetDistanceAndOcclusionAttenuation() const;

	/** Returns the distance attenuation of the source voice. */
	float GetDistanceAttenuation() const;

	/** Returns the occlusion attenuation of the source voice. */
	float GetOcclusionAttenuation() const;

	/** Returns the dynamic volume of the sound. */
	float GetDynamicVolume() const;

	/** Returns the pitch of the playback instance. */
	float GetPitch() const;

	/** Returns the volume of the wave instance (ignoring application muting). */
	float GetVolume() const;

	/** Returns the time stretch ratio of the playback instance. */
	float GetTimeStretchRatio() const;

	/** Returns the actual volume the playback instance will play at. */
	bool ShouldStopDueToMaxConcurrency() const;

	/** Returns the weighted priority of the wave instance. */
	float GetVolumeWeightedPriority() const;

	bool IsSeekable() const;

	/** Checks whether wave is streaming and streaming is supported. */
	bool IsStreaming() const;

	/** Returns the sound name of the contained USoundBase. */
	FString GetName() const;

	/** Sets the envelope value of the wave instance. Only set if the wave instance is actually generating real audio with a source voice. Only implemented in the audio mixer. */
	//void SetEnvelopeValue(const float InEnvelopeValue) { EnvelopValue = InEnvelopeValue; }

	/** Gets the envelope value of the waveinstance. Only returns non-zero values if it's a real voice. Only implemented in the audio mixer. */
	//float GetEnvelopeValue() const { return EnvelopValue; }

	/** Whether to use spatialization, which controls 3D effects like panning */
	void SetUseSpatialization(const bool InUseSpatialization) { bUseSpatialization = InUseSpatialization; }

	/** Whether this playback will be spatialized, which controls 3D effects like panning */
	bool GetUseSpatialization() const;

	/** Whether spatialization is an external send */
	void SetSpatializationIsExternalSend(const bool InSpatializationIsExternalSend) { bSpatializationIsExternalSend = InSpatializationIsExternalSend; }

	/** Whether spatialization is an external send */
	bool GetSpatializationIsExternalSend() const { return bSpatializationIsExternalSend; }
};

inline uint32 GetTypeHash(FAtomPlaybackInstance* A) { return A->TypeHash; }

/**
* FSpatializationParams
* Struct for retrieving parameters needed for computing spatialization and occlusion plugins.
*/
struct FAtomSpatializationParams
{
	/** The listener position (is likely at the origin). */
	FVector ListenerPosition = FVector::ZeroVector;

	/** The listener orientation. */
	FQuat ListenerOrientation = FQuat::Identity;

	/** The emitter position relative to listener. */
	FVector EmitterPosition = FVector::ZeroVector;

	/** The emitter world position. */
	FVector EmitterWorldPosition = FVector::ZeroVector;

	/** The emitter world rotation. */
	FQuat EmitterWorldRotation = FQuat::Identity;

	/** The emitter world rotation on callback ago. */
	FQuat LastEmitterWorldRotation = FQuat::Identity;

	/** The left channel position. */
	FVector LeftChannelPosition = FVector::ZeroVector;

	/** The right channel position. */
	FVector RightChannelPosition = FVector::ZeroVector;

	/** The distance between listener and emitter. */
	float Distance = 0.0f;

	/** The distance used to compute attenuation. Maybe different from the distance between listener and emitter if it's overridden. */
	float AttenuationDistance = 0.0f;

	/** The amount of non-spatialized this source is. 1.0 means fully 2D, 0.0 means fully 3D. */
	float NonSpatializedAmount = 0.0f;

	/** The time when this spatialization params was built. */
	double AtomClock = 0.0;
};

/**
* This represent a Voice
* 
* Note: AtomExPlayer starts and stops mutiples voices, so it represnted also as a single playback voice in Unreal.
* but it can add playback voices.
*/
class FAtomSource
{
public:

	CRIWARECORE_API FAtomSource(FAtomRuntime* InAtomRuntime)
		: AtomRuntime(InAtomRuntime)
		, PlaybackInstance(nullptr)
		//, Buffer(nullptr)
		//, LFEBleed(0.5f)
		, LPFFrequency(ATOM_MAX_FILTER_FREQUENCY)
		, HPFFrequency(ATOM_MIN_FILTER_FREQUENCY)
		, LastLPFFrequency(ATOM_MAX_FILTER_FREQUENCY)
		, LastHPFFrequency(ATOM_MIN_FILTER_FREQUENCY)
		, PlaybackTime(0.0f)
		, Pitch(1.0f)
		, LastUpdate(0)
		, LastHeardUpdate(0)
		, TickCount(0)
		, LeftChannelSourceLocation(0)
		, RightChannelSourceLocation(0)
		, NumFramesPlayed(0)
		, NumTotalFrames(1)
		, StartFrame(0)
		//, VoiceId(-1)
		, bIsPlaying(false)
		//, bReverbApplied(false)
		, bIsPausedByGame(false)
		, bIsManuallyPaused(false)
		, bIsPaused(false)
		, bIsInitialized(false)
		, bIsPreviewSound(false)
		, bIsVirtual(false)
	{}

	CRIWARECORE_API virtual ~FAtomSource() = 0;

	virtual void* GetNativeHandle() const = 0;

	/** Prepares the source for initialization. */
	virtual bool PrepareForInitialization(FAtomPlaybackInstance* InPlaybackInstance) { return true; }

	/** Returns if the source is prepared to initialize. */
	virtual bool IsPreparedToInit() { return true; }

	/** Initializes the Atom source. */
	virtual bool Init(FAtomPlaybackInstance* InPlaybackInstance) = 0;

	/** Returns whether or not the sound source has initialized. */
	virtual bool IsInitialized() const { return bIsInitialized; };

	/** Updates the sound source. */
	virtual void Update() = 0;

	/** Plays the sound source. */
	virtual void Play() = 0;

	/** Stops the sound source. */
	CRIWARECORE_API virtual void Stop();

	CRIWARECORE_API virtual void StopNow() { Stop(); };

	/** Whether or not the source is stopping. Only implemented in audio mixer. */
	virtual bool IsStopping() { return false; }

	/** Returns true if the sound source has finished playing. */
	virtual	bool IsFinished() = 0;

	/** Pause the source from game pause */
	void SetPauseByGame(bool bInIsPauseByGame);

	/** Pause the source manually */
	void SetPauseManually(bool bInIsPauseManually);

	/** Returns a string describing the source (subclass can override, but it should call the base and append). */
	CRIWARECORE_API virtual FString Describe(bool bUseLongName);

	/** Returns source is an in-game only. Will pause when in UI. */
	bool IsGameOnly() const;

	/** Returns the playback instance of this sound source. */
	const FAtomPlaybackInstance* GetPlaybackInstance() const { return PlaybackInstance; }
	const FAtomWaveInfo& GetWaveInfo() const { return WaveInfo; }

	/** Returns whether or not the sound source is playing. */
	bool IsPlaying() const { return bIsPlaying; }

	/**  Returns true if the sound is paused. */
	bool IsPaused() const { return bIsPaused; }

	/**  Returns true if the sound is paused by game pause. */
	bool IsPausedByGame() const { return bIsPausedByGame; }

	/**  Returns true if the sound is paused manually. */
	bool IsPausedManually() const { return bIsManuallyPaused; }

	/** Returns true if reverb should be applied. */
	//bool IsReverbApplied() const { return bReverbApplied; }

	/** Set the bReverbApplied variable. */
	//CRIWARECORE_API bool SetReverbApplied(bool bHardwareAvailable);

	/** Updates and sets the LFEBleed variable. */
	//CRIWARECORE_API float SetLFEBleed();

	/** Updates the FilterFrequency value. */
	CRIWARECORE_API void SetFilterFrequency();

	/** Updates the stereo emitter positions of this voice. */
	CRIWARECORE_API void UpdateStereoEmitterPositions();

	/** Gets parameters necessary for computing 3d spatialization of sources. */
	CRIWARECORE_API FAtomSpatializationParams GetSpatializationParams();

	/** Returns the contained sound buffer object. */
	//virtual const FSoundBuffer* GetBuffer() const { return Buffer; }

	/** Initializes any source effects for this sound source. */
	//virtual void InitializeSourceEffects(uint32 InEffectVoiceId) {}

	/** Sets if this voice is virtual. */
	void SetVirtual()
	{
		bIsVirtual = true;
	}

	/** Returns the source's playback percent. */
	CRIWARECORE_API virtual float GetPlaybackPercent() const;

	/** Returns the source's envelope at the callback block rate. Only implemented in audio mixer. */
	//CRIWARECORE_API virtual float GetEnvelopeValue() const { return 0.0f; };

	CRIWARECORE_API void GetChannelLocations(FVector& Left, FVector& Right) const;

	void NotifyPlaybackData();

	virtual bool IsVoiceSource() const { return true; }

protected:

	/** Initializes common data for all sound source types. */
	CRIWARECORE_API void InitCommon();

	/** Updates common data for all sound source types. */
	CRIWARECORE_API void UpdateCommon();

	/** Pauses the sound source. */
	virtual void Pause() = 0;

	/** Updates this source's pause state */
	void UpdatePause();

	/** Returns the volume of the sound source after evaluating debug commands */
	CRIWARECORE_API float GetDebugVolume(const float InVolume);

	/** Owning Atom runtime. */
	FAtomRuntime* AtomRuntime;

	/** Contained playback instance. */
	FAtomPlaybackInstance* PlaybackInstance; // a playback
	FAtomWaveInfo WaveInfo; // move this in playbackinstance

	/** Cached sound buffer associated with currently bound wave instance. */
	//FSoundBuffer* Buffer;

	/** The amount of a sound to bleed to the LFE speaker */
	//float LFEBleed;

	/** What frequency to set the LPF filter to. Note this could be caused by occlusion, manual LPF application, or LPF distance attenuation. */
	float LPFFrequency;

	/** What frequency to set the HPF filter to. Note this could be caused by HPF distance attenuation. */
	float HPFFrequency;

	/** The last LPF frequency set. Used to avoid making API calls when parameter doesn't changing. */
	float LastLPFFrequency;

	/** The last HPF frequency set. Used to avoid making API calls when parameter doesn't changing. */
	float LastHPFFrequency;

	/** The virtual current playback time. Used to trigger notifications when finished. */
	float PlaybackTime;

	/** The pitch of the sound source. */
	float Pitch;

	/** Last tick when this source was active */
	int32 LastUpdate;

	/** Last tick when this source was active *and* had a hearable volume */
	int32 LastHeardUpdate;

	/** Update tick count. Used to stop oldest stopping sound source. */
	int32 TickCount;

	/** The location of the left-channel source for stereo spatialization. */
	FVector LeftChannelSourceLocation;

	/** The location of the right-channel source for stereo spatialization. */
	FVector RightChannelSourceLocation;

	/** The number of frames (Samples / NumChannels) played by the sound source. */
	int32 NumFramesPlayed;

	/** The total number of frames of audio for the sound wave */
	int32 NumTotalFrames;

	/** The frame we started on. */
	int32 StartFrame;

	/** Effect ID of this sound source in the audio device sound source array. */
	//uint32 VoiceId;

	/** Whether we are playing or not. */
	FThreadSafeBool bIsPlaying;

	/** Cached sound mode value used to detect when to switch outputs. */
	//uint8 bReverbApplied : 1;

	/** Whether we are paused by game state or not. */
	uint8 bIsPausedByGame : 1;

	/** Whether or not we were paused manually. */
	uint8 bIsManuallyPaused : 1;

	/** Whether or not we are actually paused. */
	uint8 bIsPaused : 1;

	/** Whether or not the sound source is initialized. */
	uint8 bIsInitialized : 1;

	/** Whether or not the sound is a preview sound. */
	uint8 bIsPreviewSound : 1;

	/** True if this isn't a real hardware voice */
	uint32 bIsVirtual : 1;

	friend FAtomRuntime;
	friend FAtomActiveSound;

#if ENABLE_ATOM_DEBUG
public:

	/** Struct containing the debug state of a SoundSource */
	struct CRIWARECORE_API FDebugInfo
	{
		/** True if this sound has been soloed. */
		bool bIsSoloed = false;

		/** True if this sound has been muted . */
		bool bIsMuted = false;

		/** Reason why this sound is mute/soloed. */
		FString MuteSoloReason;

		/** Basic CS so we can pass this around safely. */
		FCriticalSection CS;
	};

	TSharedPtr<FDebugInfo, ESPMode::ThreadSafe> DebugInfo;
	friend struct FDebugInfo;
#endif //ENABLE_ATOM_DEBUG
};

/** Simple class that wraps the math involved with interpolating a parameter
  * over time based on audio device update time.
  */
class CRIWARECORE_API FAtomDynamicParameter
{
public:

	explicit FAtomDynamicParameter(float Value);

	void Set(float Value, float InDuration);
	void Update(float DeltaTime);

	bool IsDone() const
	{
		return CurrTimeSec >= DurationSec;
	}

	float GetValue() const
	{
		return CurrValue;
	}

	float GetTargetValue() const
	{
		return TargetValue;
	}

private:
	float CurrValue;
	float StartValue;
	float DeltaValue;
	float CurrTimeSec;
	float DurationSec;
	float LastTime;
	float TargetValue;
};


/// A test -to remove
/*class CRIWARECORE_API FAtomBus
{
public:
	FAtomBus(FName Name) : BusName(Name) {}

private:
	FName BusName;

	TMap<UAtomEffectPreset*, FAtomBus> AsrBuses;
};

class CRIWARECORE_API FAtomRack
{
public:
	FAtomRack(FCriAtomExAsrRackId&& InAsrRackId) { AsrRackId = Forward<FCriAtomExAsrRackId>(InAsrRackId); }
	FAtomRack(FAtomRack&&) = default;
	FAtomRack& operator= (FAtomRack&&) = default;
	FAtomRack(const FAtomRack&) = delete;
	FAtomRack& operator= (const FAtomRack&) = delete;

	void SetDspBusSnapShot(const FName& SnapshotName, float FadeSeconds)
	{
		const CriSint32 TimeMs = static_cast<CriSint32>(FMath::Min(FadeSeconds, 0.f) * 1000.f);
		if (!SnapshotName.IsNone())
		{
			FCriWareApi::criAtomExAsrRack_ApplyDspBusSnapshot(AsrRackId, TCHAR_TO_UTF8(*SnapshotName.ToString()), TimeMs);
		}
		else
		{
			FCriWareApi::criAtomExAsrRack_ApplyDspBusSnapshot(AsrRackId, nullptr, TimeMs);
		}
	}



private:
	FCriAtomExAsrRackId AsrRackId;

	TMap<UAtomBus*, FAtomBus> AsrBuses;
};*/

// DSP utilites - from UE
namespace Atom
{
	// Low precision, high performance approximation of sine using parabolic polynomial approx
	// Valid on interval [-PI, PI]
	static FORCEINLINE float FastSin(const float X)
	{
		return (4.0f * X) / UE_PI * (1.0f - FMath::Abs(X) / UE_PI);
	}

	// Function to convert linear scale volume to decibels.
	static FORCEINLINE float ConvertToDecibels(const float InLinear, const float InFloor = UE_SMALL_NUMBER)
	{
		return 20.0f * FMath::LogX(10.0f, FMath::Max(InLinear, InFloor));
	}

	// Function to convert decibel to linear scale.
	static FORCEINLINE float ConvertToLinear(const float InDecibels)
	{
		return FMath::Pow(10.0f, InDecibels / 20.0f);
	}

	// Returns the frequency multiplier to scale a base frequency given the input semitones
	static FORCEINLINE float GetFrequencyMultiplier(const float InPitchSemitones)
	{
		if (InPitchSemitones == 0.0f)
		{
			return 1.0f;

		}
		return FMath::Pow(2.0f, InPitchSemitones / 12.0f);
	}

	// Returns the number of semitones relative to a base frequency given the input frequency multiplier
	static FORCEINLINE float GetSemitones(const float InMultiplier)
	{
		if (InMultiplier <= 0.0f)
		{
			return 12.0f * FMath::Log2(UE_SMALL_NUMBER);
		}
		return 12.0f * FMath::Log2(InMultiplier);
	}

	// Returns the log frequency of the input value. Maps linear domain and range values to log output (good for linear slider controlling frequency)
	static FORCEINLINE float GetLogFrequencyClamped(const float InValue, const FVector2D& Domain, const FVector2D& Range)
	{
		// Check if equal as well as less than to avoid round error in case where at edges.
		if (InValue <= Domain.X)
		{
			return UE_REAL_TO_FLOAT(Range.X);
		}

		if (InValue >= Domain.Y)
		{
			return UE_REAL_TO_FLOAT(Range.Y);
		}

		//Handle edge case of NaN
		if (FMath::IsNaN(InValue))
		{
			return UE_REAL_TO_FLOAT(Range.X);
		}

		const FVector2D RangeLog(FMath::Max(FMath::Loge(Range.X), UE_SMALL_NUMBER), FMath::Min(FMath::Loge(Range.Y), UE_BIG_NUMBER));
		const float FreqLinear = (float)FMath::GetMappedRangeValueUnclamped(Domain, RangeLog, (FVector2D::FReal)InValue);
		return FMath::Exp(FreqLinear);
	}

	// Returns the linear frequency of the input value. Maps log domain and range values to linear output (good for linear slider representation/visualization of log frequency). Reverse of GetLogFrequencyClamped.
	static FORCEINLINE float GetLinearFrequencyClamped(const float InFrequencyValue, const FVector2D& Domain, const FVector2D& Range)
	{
		// Check if equal as well as less than to avoid round error in case where at edges.
		if (InFrequencyValue <= Range.X)
		{
			return UE_REAL_TO_FLOAT(Domain.X);
		}

		if (InFrequencyValue >= Range.Y)
		{
			return UE_REAL_TO_FLOAT(Domain.Y);
		}

		//Handle edge case of NaN
		if (FMath::IsNaN(InFrequencyValue))
		{
			return UE_REAL_TO_FLOAT(Domain.X);
		}

		const FVector2D RangeLog(FMath::Max(FMath::Loge(Range.X), UE_SMALL_NUMBER), FMath::Min(FMath::Loge(Range.Y), UE_BIG_NUMBER));
		const FVector2D::FReal FrequencyLog = FMath::Loge(InFrequencyValue);
		return UE_REAL_TO_FLOAT(FMath::GetMappedRangeValueUnclamped(RangeLog, Domain, FrequencyLog));
	}

	// todo move to criware defines ?
	#define ATOM_BUFFER_ALIGNMENT 16

	/** Aligned allocator used for fast operations. */
	using FAtomBufferAlignedAllocator = TAlignedHeapAllocator<ATOM_BUFFER_ALIGNMENT>;

	using FAlignedByteBuffer = TArray<uint8, FAtomBufferAlignedAllocator>;
	using FAlignedFloatBuffer = TArray<float, FAtomBufferAlignedAllocator>;
	using FAlignedInt32Buffer = TArray<int32, FAtomBufferAlignedAllocator>;
}
