
#include "Atom/Mixer/AtomMixerSourceVoice.h"

#include "Atom/Mixer/AtomMixer.h"
#include "Atom/Mixer/AtomMixerSource.h"
#include "Atom/Mixer/AtomMixerSourceManager.h"
#include "Atom/AtomRuntime.h"

namespace Atom
{
	/**
	* FMixerSourceVoice Implementation
	*/

	FMixerSourceVoice::FMixerSourceVoice()
	{
		Reset(nullptr);
	}

	FMixerSourceVoice::~FMixerSourceVoice()
	{
	}

	void FMixerSourceVoice::Reset(FAtomRuntime* InAtomRuntime)
	{
		if (InAtomRuntime)
		{
			AtomRuntime = InAtomRuntime;
			SourceManager = AtomRuntime->GetSourceManager();
		}
		else
		{
			AtomRuntime = nullptr;
			SourceManager = nullptr;
		}

		//Pitch = -1.0f;
		//Volume = -1.0f;
		//DistanceAttenuation = -1.0f;
		//Distance = -1.0f;
		//LPFFrequency = -1.0f;
		//HPFFrequency = -1.0f;
		SourceID = INDEX_NONE;
		//bIsPlaying = false;
		//bIsPaused = false;
		//bIsActive = false;
		bIsBus = false;
		//bEnableBusSends = false;
		//bEnableBaseSubmix = false;
		//bEnableSubmixSends = false;
		//bStopFadedOut = false;

		//PitchModBase = TNumericLimits<float>::Max();
		//VolumeModBase = TNumericLimits<float>::Max();
		//LPFFrequencyModBase = TNumericLimits<float>::Max();
		//HPFFrequencyModBase = TNumericLimits<float>::Max();

		//SubmixSends.Reset();

		// destroy the pool/port
		VoicePool = nullptr;
		PoolType = EAtomVoicePoolType::Undefined;
		PoolID = INDEX_NONE;
		MaxInputVoices = 0;
		MaxInputChannels = 0;
		MaxInputSampleRate = 0;

		VoiceEffect.Reset();
	}

	bool FMixerSourceVoice::Init(const FMixerSourceVoiceInitParams& InitParams)
	{
		ATOM_MIXER_CHECK_GAME_THREAD(AtomRuntime);

		if (SourceManager->GetFreeSourceID(SourceID))
		{
			ATOM_MIXER_CHECK(InitParams.SourceListener != nullptr);
			ATOM_MIXER_CHECK(InitParams.NumInputChannels > 0);

			bIsBus = InitParams.AudioBusID != INDEX_NONE;

			SourceManager->InitSource(SourceID, InitParams);
			return true;
		}

		return false;
	}

	void FMixerSourceVoice::Release()
	{
		ATOM_MIXER_CHECK_GAME_THREAD(AtomRuntime);

		SourceManager->ReleaseSourceID(SourceID);
	}

	void FMixerSourceVoice::SetVoiceEffect(const FAtomSourceVoiceEffect& Params)
	{
		ATOM_MIXER_CHECK_GAME_THREAD(AtomRuntime);

		// update the attached dsp effect to the voice pool
		if (VoiceEffect.Type != Params.Type && VoicePool.IsValid())
		{
			// always call detach if something is set or not
			FCriWareApi::criAtomExVoicePool_DetachDsp(VoicePool);

			switch (Params.Type)
			{
			case EAtomSourceVoiceEffectType::PitchShifter:
			{
				CriAtomExDspPitchShifterConfig Config;
				criAtomExVoicePool_SetDefaultConfigForDspPitchShifter(&Config);
				Config.max_channels = MaxInputChannels;
				Config.max_sampling_rate = MaxInputSampleRate;
				Config.num_dsp = MaxInputVoices;
				FCriWareApi::criAtomExVoicePool_AttachDspPitchShifter(VoicePool, &Config, nullptr, 0);
			}
			break;
			case EAtomSourceVoiceEffectType::TimeStretch:
			{
				CriAtomExDspTimeStretchConfig Config;
				criAtomExVoicePool_SetDefaultConfigForDspTimeStretch(&Config);
				Config.max_channels = MaxInputChannels;
				Config.max_sampling_rate = MaxInputSampleRate;
				Config.num_dsp = MaxInputVoices;
				FCriWareApi::criAtomExVoicePool_AttachDspTimeStretch(VoicePool, &Config, nullptr, 0);
			}
			break;
			case EAtomSourceVoiceEffectType::Disabled:
			default:
			{
				// nothing to do
			}
			}

			// params at attach time
			VoiceEffect = Params;
		}

		SourceManager->SetVoiceEffect(SourceID, Params);
	}

	/*void FMixerSourceVoice::OnMixBus(FMixerSourceVoiceBuffer* OutMixerSourceBuffer)
	{
		ATOM_MIXER_CHECK_AUDIO_PLAT_THREAD(AtomRuntime);

		check(OutMixerSourceBuffer->AudioData.Num() > 0);

		for (int32 i = 0; i < OutMixerSourceBuffer->AudioData.Num(); ++i)
		{
			OutMixerSourceBuffer->AudioData[i] = 0.0f;
		}
	}*/

	float FMixerSourceVoice::GetModVolumeEnd()
	{
		ATOM_MIXER_CHECK_GAME_THREAD(AtomRuntime);

		if (SourceID != INDEX_NONE)
		{
			auto& SourceInfo = SourceManager->SourceInfos[SourceID];

			//const bool bHasProcessed = SourceInfo.VolumeModulation.GetHasProcessed();
			//const float ModVolumeStart = SourceInfo.VolumeModulation.GetValue();
			SourceInfo.VolumeModulation.ProcessControl(SourceInfo.VolumeModulationBase);
			const float ModVolumeEnd = SourceInfo.VolumeModulation.GetValue();
			//VolumeStart *= bHasProcessed ? ModVolumeStart : ModVolumeEnd;

			return ModVolumeEnd;
		}

		return 1.0f;
	}

	float FMixerSourceVoice::GetModPitchEnd()
	{
		ATOM_MIXER_CHECK_GAME_THREAD(AtomRuntime);

		if (SourceID != INDEX_NONE)
		{
			auto& SourceInfo = SourceManager->SourceInfos[SourceID];

			SourceInfo.PitchModulation.ProcessControl(SourceInfo.PitchModulationBase);
			const float ModPitchEnd = SourceInfo.PitchModulation.GetValue();

			return ModPitchEnd;
		}

		return 1.0f;
	}

	float FMixerSourceVoice::GetModLPFEnd()
	{
		ATOM_MIXER_CHECK_GAME_THREAD(AtomRuntime);

		if (SourceID != INDEX_NONE)
		{
			auto& SourceInfo = SourceManager->SourceInfos[SourceID];

			SourceInfo.LowpassModulation.ProcessControl(SourceInfo.LowpassModulationBase);
			const float ModLPFEnd = SourceInfo.LowpassModulation.GetValue();

			return ModLPFEnd;
		}

		return 1.0f;
	}

	float FMixerSourceVoice::GetModHPFEnd()
	{
		ATOM_MIXER_CHECK_GAME_THREAD(AtomRuntime);

		if (SourceID != INDEX_NONE)
		{
			auto& SourceInfo = SourceManager->SourceInfos[SourceID];

			SourceInfo.HighpassModulation.ProcessControl(SourceInfo.HighpassModulationBase);
			const float ModHPFEnd = SourceInfo.HighpassModulation.GetValue();

			return ModHPFEnd;
		}

		return 1.0f;
	}

	bool FMixerSourceVoice::IterateOverAisacModulations(TFunction<bool(FAtomAisacControl& Control, FAtomModulationDestination& Modulation, float ModulationBase)> Func)
	{
		ATOM_MIXER_CHECK_GAME_THREAD(AtomRuntime);

		if (SourceID != INDEX_NONE)
		{
			for (auto& AisacControlModulation : SourceManager->SourceInfos[SourceID].AisacControlModulations)
			{
				if (!Func(AisacControlModulation.Key, AisacControlModulation.Value.Modulation, AisacControlModulation.Value.ModulationBase))
				{
					return false;
				}
			}
		}

		return true;
	}

	bool FMixerSourceVoice::IsUsingHRTFSpatializer(bool bDefaultValue) const
	{
		ATOM_MIXER_CHECK_GAME_THREAD(AtomRuntime);

		if (SourceID != INDEX_NONE)
		{
			return SourceManager->IsUsingHRTFSpatializer(SourceID);
		}

		return bDefaultValue;
	}

	void FMixerSourceVoice::SetModVolume(const float InVolumeModBase)
	{
		ATOM_MIXER_CHECK_GAME_THREAD(AtomRuntime);

		if (InVolumeModBase != VolumeModBase)
		{
			VolumeModBase = InVolumeModBase;
			SourceManager->SetModVolume(SourceID, VolumeModBase);
		}
	}

	void FMixerSourceVoice::SetModPitch(const float InPitchModBase)
	{
		ATOM_MIXER_CHECK_GAME_THREAD(AtomRuntime);

		if (InPitchModBase != PitchModBase)
		{
			PitchModBase = InPitchModBase;
			SourceManager->SetModPitch(SourceID, InPitchModBase);
		}
	}

	void FMixerSourceVoice::SetModHPFFrequency(const float InHPFFrequencyModBase)
	{
		ATOM_MIXER_CHECK_GAME_THREAD(AtomRuntime);

		if (InHPFFrequencyModBase != HPFFrequencyModBase)
		{
			HPFFrequencyModBase = InHPFFrequencyModBase;
			SourceManager->SetModHPFFrequency(SourceID, InHPFFrequencyModBase);
		}
	}

	void FMixerSourceVoice::SetModLPFFrequency(const float InLPFFrequencyModBase)
	{
		ATOM_MIXER_CHECK_GAME_THREAD(AtomRuntime);

		if (InLPFFrequencyModBase != LPFFrequencyModBase)
		{
			LPFFrequencyModBase = InLPFFrequencyModBase;
			SourceManager->SetModLPFFrequency(SourceID, InLPFFrequencyModBase);
		}
	}

	void FMixerSourceVoice::SetModulationRouting(FAtomSoundModulationDefaultSettings& RoutingSettings)
	{
		ATOM_MIXER_CHECK_GAME_THREAD(AtomRuntime);

		SourceManager->SetModulationRouting(SourceID, RoutingSettings);
	}
} // namespace
