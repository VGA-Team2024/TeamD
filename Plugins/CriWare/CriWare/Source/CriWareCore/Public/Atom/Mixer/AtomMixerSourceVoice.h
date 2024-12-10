
#pragma once

#include "AtomMixerSourceManager.h"
#include "Atom/AtomSourceVoiceEffect.h"

// Forward Definitions
class FAtomRuntime;

namespace Atom
{
	// Forward Definitions
	//struct FMixerSourceVoiceBuffer;
	//struct FMixerSourceVoiceFilterParams;
	struct FMixerSourceVoiceInitParams;
	//class FMixerSubmix;
	class FMixerSource;
	class FAtomSourceManager;
	//class ISourceBufferQueueListener;

	// An Atom voice
	class FMixerSourceVoice
	{
	public:

		FMixerSourceVoice();
		~FMixerSourceVoice();

		// Resets the source voice state
		void Reset(FAtomRuntime* InAtomRuntime);

		// Initializes the mixer source voice
		bool Init(const FMixerSourceVoiceInitParams& InFormat);

		// Releases the source voice back to the source buffer pool
		void Release();

		// Returns the source's ID.
		int32 GetSourceID() const { return SourceID; }

		// Returns the Atom voice pool's ID to assign to MixerSource.
		int32 GetVoicePoolID() const { return PoolType != EAtomVoicePoolType::InputPort ? PoolID : INDEX_NONE; }

		// Returns the Atom input port if voice is used as a port.
		void* GetInputPort() const { return PoolType == EAtomVoicePoolType::InputPort ? (void*)VoicePool.Get() : nullptr; }

		// Sets the fixed voice effect for this voice pool.
		void SetVoiceEffect(const FAtomSourceVoiceEffect& Params);

		// Called when the source is a bus and needs to mix other sources together to generate output
		//void OnMixBus(FMixerSourceVoiceBuffer* OutMixerSourceBuffer);

		// Whether or not the voice is currently using HRTF spatialization.
		//
		// @param bDefaultValue - This value will be returned if voice does not have a valid source id.
		bool IsUsingHRTFSpatializer(bool bDefaultValue) const;

		// Sets the source voice modulation base pitch value.
		void SetModPitch(const float InPitch);

		// Sets the source voice's volume modulation base frequency.
		void SetModVolume(const float InVolume);

		// Sets the source voice's LPF filter modulation base frequency.
		void SetModLPFFrequency(const float InFrequency);

		// Sets the source voice's HPF filter modulation base frequency.
		void SetModHPFFrequency(const float InFrequency);

		void SetModulationRouting(FAtomSoundModulationDefaultSettings& RoutingSettings);

	public:

		// TEMP may move this to real time filter callback
		// Process modulation update and retun the final value 
		float GetModVolumeEnd();
		// Process modulation update and retun the final value 
		float GetModPitchEnd();
		// Process modulation update and retun the final value 
		float GetModLPFEnd();
		// Process modulation update and retun the final value 
		float GetModHPFEnd();
		// Process modulation update and retun the final value 
		bool IterateOverAisacModulations(TFunction<bool(FAtomAisacControl& Control, FAtomModulationDestination& Modulation, float ModulationBase)> Func);

	private:

		friend class FAtomSourceManager;

		FAtomSourceManager* SourceManager;
		FAtomRuntime* AtomRuntime;

		float PitchModBase;
		float VolumeModBase;
		float LPFFrequencyModBase;
		float HPFFrequencyModBase;
		int32 SourceID;
		bool bIsBus;

		// Atom use static voice pools so a voice here is a pool with one or more sub-voices.
		FCriAtomVoicePoolPtr VoicePool;

		// the pool type
		EAtomVoicePoolType PoolType = EAtomVoicePoolType::Undefined;
		int32 MaxInputVoices;
		int32 MaxInputChannels;
		int32 MaxInputSampleRate;

		// The Atom voice pool ID 
		int32 PoolID = INDEX_NONE;

		// Voice Effect and parameters at effect attach time
		FAtomSourceVoiceEffect VoiceEffect;
	};
} // namespace
