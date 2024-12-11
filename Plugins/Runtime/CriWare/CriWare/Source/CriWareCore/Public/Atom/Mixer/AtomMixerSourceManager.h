
#pragma once

#include "DSP/BufferVectorOperations.h"
#include "DSP/EnvelopeFollower.h"
#include "DSP/InterpolatedOnePole.h"
#include "DSP/ParamInterpolator.h"

//#include "IAudioLink.h"
//#include "IAudioLinkFactory.h"

#include "Atom/AtomRuntime.h"
#include "Atom/AtomAudioBus.h"
#include "Atom/AtomAudioBusSubsystem.h"
#include "Atom/Mixer/AtomMixer.h"
#include "Atom/Mixer/AtomMixerBus.h"
#include "Atom/Mixer/AtomMixerPlayer.h"

// Forward Definitions
struct FAtomSourceVoiceEffect;

namespace Atom
{
	// Forward Definitions
	class FMixerSourceVoice;

	class ISourceListener
	{
	public:
		virtual ~ISourceListener() = default;

		// Called before a source begins to generate audio. 
		virtual void OnBeginGenerate() = 0;

		// Called when a loop point is hit
		//virtual void OnLoopEnd() = 0;

		// Called when the source finishes on the ASR render thread
		virtual void OnDone() = 0;

		// Called when the source's effect tails finish on the ASR render thread.
		//virtual void OnEffectTailsDone() = 0;
	};

	// Struct holding mappings of bus ids (unique ids) to send level
	struct FInitAudioBusSend
	{
		uint32 AudioBusID = INDEX_NONE;
		float SendLevel = 0.0f;
		int32 BusChannels = 0;
	};

	struct FSourceManagerInitParams
	{
		// Total number of sources to use in the source manager
		int32 NumSources = 0;

		// Number of worker threads to use for the source manager.
		int32 NumSourceWorkers = 0;
	};

	struct FMixerSourceVoiceInitParams
	{
		//TSharedPtr<FAtomSourceBuffer, ESPMode::ThreadSafe> MixerSourceBuffer = nullptr;
		ISourceListener* SourceListener = nullptr;
		//TArray<FMixerSourceSubmixSend> SubmixSends;
		TArray<Atom::FInitAudioBusSend> AudioBusSends[(int32)EBusSendType::Count];
		uint32 AudioBusID = INDEX_NONE;
		int32 AudioBusChannels = 0;
		float SourceBusDuration = 0.0f;
		//uint32 SourceEffectChainId = INDEX_NONE;
		//TArray<FSourceEffectChainEntry> SourceEffectChain;
		FMixerSourceVoice* SourceVoice = nullptr;
		int32 NumInputChannels = 0;
		int32 NumInputFrames = 0;
		float EnvelopeFollowerAttackTime = 10.0f;
		float EnvelopeFollowerReleaseTime = 100.0f;
		float TimeStretch = 0.0f;
		FString DebugName;
		//USpatializationPluginSourceSettingsBase* SpatializationPluginSettings = nullptr;
		//UOcclusionPluginSourceSettingsBase* OcclusionPluginSettings = nullptr;
		//UReverbPluginSourceSettingsBase* ReverbPluginSettings = nullptr;
		UAtomSourceDataOverridePluginSourceSettingsBase* SourceDataOverridePluginSettings = nullptr;

		FAtomSoundModulationDefaultSettings ModulationSettings;

		//FQuartzQuantizedRequestData QuantizedRequestData;

		FSharedISourceBufferListenerPtr SourceBufferListener;

		//IBufferedAudioOutput
		//IAudioLinkFactory::FAudioLinkSourcePushedSharedPtr AudioLink;

		FName AtomComponentUserID;
		uint64 AtomComponentID = 0;
		bool bIs3D = false;
		bool bPlayEffectChainTails = false;
		bool bUseHRTFSpatialization = false;
		bool bIsExternalSend = false;
		bool bIsDebugMode = false;
		bool bEnableBusSends = false;
		bool bEnableBaseSubmix = false;
		bool bEnableSubmixSends = false;
		bool bIsVorbis = false;
		bool bIsSoundfield = false;
		//bool bIsSeeking = false;
		//bool bShouldSourceBufferListenerZeroBuffer = false;
		bool bUseFixedVoicePools = false;
		EAtomFormat InputFormat = EAtomFormat::None;
		int32 InputVoices = 1;
		int32 InputSampleRate = 48000;
		bool bIsStreamed = false;

		FCriAtomVoicePoolPtr ExternalVoicePool = nullptr;

		uint32 PlayOrder = INDEX_NONE;
	};

	// Atom sources and voice pool manager
	class FAtomSourceManager
	{
	public:

		FAtomSourceManager(FAtomRuntime* InAtomRuntime);
		~FAtomSourceManager();

		void Init(const FSourceManagerInitParams& InitParams);
		void Update(bool bTimedOut = false);

		bool GetFreeSourceID(int32& OutSourceID);
		int32 GetNumActiveSources() const;
		int32 GetNumActiveAudioBuses() const;

		void ReleaseSourceID(const int32 SourceID);
		void InitSource(const int32 SourceID, const FMixerSourceVoiceInitParams& InitParams);

		// creates and starts an audio player manually. (AtomExPlayer) Must be called from the Atom thread.
		void StartAudioPlayer(FAtomExPlayerKey InAudioPlayerKey);

		// Stops an audio player manually. (AtomExPlayer) Must be called from the Atom thread.
		void StopAudioPlayer(FAtomExPlayerKey InAudioPlayerKey);

		// Queries if an audio player is active. Must be called from the Atom thread.
		bool IsAudioPlayerActive(FAtomExPlayerKey InAudioPlayerKey) const;

		// Creates and starts an audio bus manually.
		void StartAudioBus(FAudioBusKey InAudioBusKey, int32 InNumChannels, bool bInIsAutomatic);

		// Stops an audio bus manually
		void StopAudioBus(FAudioBusKey InAudioBusKey);

		// Queries if an audio bus is active. Must be called from the audio thread.
		bool IsAudioBusActive(FAudioBusKey InAudioBusKey) const;

		// Returns the number of channels currently set for the audio bus associated with
		// the provided BusId.  Returns 0 if the audio bus is inactive.
		int32 GetAudioBusNumChannels(FAudioBusKey InAudioBusKey) const;

		// Adds a patch output for an audio bus from the Audio Render Thread
		void AddPatchOutputForAudioBus(FAudioBusKey InAudioBusKey, const Audio::FPatchOutputStrongPtr& InPatchOutputStrongPtr);

		// Adds a patch output for an audio bus from the Audio Thread
		void AddPatchOutputForAudioBus_AudioThread(FAudioBusKey InAudioBusKey, const Audio::FPatchOutputStrongPtr& InPatchOutputStrongPtr);

		// Adds a patch input for an audio bus
		void AddPatchInputForAudioBus(FAudioBusKey InAudioBusKey, const Audio::FPatchInput& InPatchInput);

		// Adds a patch input for an audio bus from the Audio Thread
		void AddPatchInputForAudioBus_AudioThread(FAudioBusKey InAudioBusKey, const Audio::FPatchInput& InPatchInput);

		// Sets the voice effect for a given source.
		void SetVoiceEffect(const int32 SourceID, const FAtomSourceVoiceEffect& Effect);

		// Check if this source is using HRFT spatialization.
		bool IsUsingHRTFSpatializer(const int32 SourceID) const;

		// Sets base (i.e. carrier) frequency of modulatable parameters
		void SetModPitch(const int32 SourceID, const float InModPitch);
		void SetModVolume(const int32 SourceID, const float InModVolume);
		void SetModLPFFrequency(const int32 SourceID, const float InModFrequency);
		void SetModHPFFrequency(const int32 SourceID, const float InModFrequency);

		void SetModulationRouting(const int32 SourceID, FAtomSoundModulationDefaultSettings& ModulationSettings);

		// Buffer getters
		const float* GetPreDistanceAttenuationBuffer(const int32 SourceId) const;
		const float* GetPreEffectBuffer(const int32 SourceId) const;
		const float* GetPreviousSourceBusBuffer(const int32 SourceId) const;
		const float* GetPreviousAudioBusBuffer(const int32 AudioBusId) const;
		int32 GetNumChannels(const int32 SourceId) const;
		int32 GetNumOutputFrames() const { return 256; } // return NumOutputFrames; }
		bool IsSourceBus(const int32 SourceId) const;

		// grab info
		void IterateOverVoicePools(TFunctionRef<void(EAtomVoicePoolType Type, int32 ID, int32 NumUsedVoices, int32 MaxVoices, int32 MaxChannels, int32 MaxSampleRate)> Func) const;

	private:

		void ReleaseSource(const int32 SourceId);

		void GenerateSourceAudio(const bool bGenerateBuses);
		void GenerateSourceAudio(const bool bGenerateBuses, const int32 SourceIdStart, const int32 SourceIdEnd);

		void ComputeBuses();
		void UpdateBuses();

		static const int32 NUM_BYTES_PER_SAMPLE = 2;

		FAtomRuntime* AtomRuntime;

		// Cached ptr to an optional source data override plugin
		TAtomSourceDataOverridePtr SourceDataOverridePlugin;

		// Array of pointers to game thread audio source objects
		TArray<FMixerSourceVoice*> MixerSources;

		struct FSourceInfo
		{
			FSourceInfo() {}
			~FSourceInfo() {}

			// What audio bus Id this source is sonfiying, if it is a source bus. This is INDEX_NONE for sources which are not source buses.
			uint32 AudioBusID;

			// What buses this source is sending its audio to. Used to remove this source from the bus send list.
			TArray<uint32> AudioBusSends[(int32)EBusSendType::Count];

			// A DSP object which tracks the amplitude envelope of a source.
			//Audio::FInlineEnvelopeFollower SourceEnvelopeFollower;
			//float SourceEnvelopeValue;

			// Time stretching
			float TimeStretch;

			// Modulation destinations
			FAtomModulationDestination VolumeModulation;
			FAtomModulationDestination PitchModulation;
			FAtomModulationDestination LowpassModulation;
			FAtomModulationDestination HighpassModulation;

			// Modulation Base (i.e. Carrier) Values
			float VolumeModulationBase;
			float PitchModulationBase;
			float LowpassModulationBase;
			float HighpassModulationBase;

			//Asiacs
			struct FSourceAisacModulation
			{
				FAtomModulationDestination Modulation;
				float ModulationBase = 1.0f;
			};
			TMap<FAtomAisacControl, FSourceAisacModulation> AisacControlModulations;

			uint8 bIs3D : 1;
			uint8 bIsCenterChannelOnly : 1;
			uint8 bIsActive : 1;
			uint8 bIsPlaying : 1;
			uint8 bIsPaused : 1;
			uint8 bIsPausedForQuantization : 1;
			//uint8 bDelayLineSet : 1;
			uint8 bIsStopping : 1;
			uint8 bHasStarted : 1;
			//uint8 bIsBusy : 1;
			uint8 bUseHRTFSpatializer : 1;
			uint8 bIsExternalSend : 1;
			//uint8 bUseOcclusionPlugin : 1;
			//uint8 bUseReverbPlugin : 1;
			//uint8 bIsDone : 1;
			//uint8 bIsLastBuffer : 1;
			uint8 bEnableBusSends : 1;
			//uint8 bEnableBaseSubmix : 1;
			//uint8 bEnableSubmixSends : 1;
			//uint8 bIsVorbis : 1;
			uint8 bIsSoundfield : 1;
			//uint8 bHasPreDistanceAttenuationSend : 1;
			uint8 bModFiltersUpdated : 1;
			uint8 bShouldSourceBufferListenerZeroBuffer : 1;

			// Source format info
			int32 NumInputChannels;
			int32 NumInputFrames;

			// ID for associated Atom Component if there is one, 0 otherwise
			uint64 AtomComponentID;

			// ID for playback if genererated from an AtomExPlayer.
			//FAtomExPlayer::FAtomPlaybackId PlaybackID;

			FORCEINLINE void ResetModulators(const FAtomRuntimeId InRuntimeID)
			{
				VolumeModulation.Init(InRuntimeID, FName("Volume"), false /* bInIsBuffered */, true /* bInValueLinear */);
				PitchModulation.Init(InRuntimeID, FName("Pitch"));
				HighpassModulation.Init(InRuntimeID, FName("HPFCutoffFrequency"));
				LowpassModulation.Init(InRuntimeID, FName("LPFCutoffFrequency"));

				VolumeModulationBase = 0.0f;
				PitchModulationBase = 0.0f;
				HighpassModulationBase = MIN_FILTER_FREQUENCY;
				LowpassModulationBase = MAX_FILTER_FREQUENCY;

				AisacControlModulations.Empty();
			}

#if ATOM_MIXER_ENABLE_DEBUG_MODE
			uint8 bIsDebugMode : 1;
			FString DebugName;
#endif // ATOM_MIXER_ENABLE_DEBUG_MODE
		};

		// Array of listener transforms
		TArray<FTransform> ListenerTransforms;

		// Array of source infos.
		TArray<FSourceInfo> SourceInfos;

		// Map of players Id's to AtomExPlayers.
		TMap<FAtomExPlayerKey, TSharedPtr<FAtomExPlayer>> AudioPlayers;

		// Map of bus object Id's to audio bus data. 
		TMap<FAudioBusKey, TSharedPtr<FMixerAudioBus>> AudioBuses;
		TArray<FAudioBusKey> AudioBusKeys_AudioThread;

		// General information about sources in source manager accessible from game thread.
		struct FGameThreadInfo
		{
			TArray<int32> FreeSourceIndices;
			TArray<bool> bIsBusy;
			TArray<bool> bNeedsSpeakerMap;
			TArray<bool> bIsDebugMode;
			TArray<bool> bIsUsingHRTFSpatializer;
		} GameThreadInfo;

		int32 NumActiveSources;
		int32 NumTotalSources;

		uint8 bInitialized : 1;
		uint8 bUsingSpatializationPlugin : 1;
		uint8 bUsingSourceDataOverridePlugin : 1;

		friend class FMixerSourceVoice;
	};
} // namespace
