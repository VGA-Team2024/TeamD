/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2021 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine
 * Module   : CriWareCore
 * File     : AtomRackSubmixListener.h
 *
 ****************************************************************************/

#pragma once

#include "CoreMinimal.h"

#include "ISubmixBufferListener.h"
#include "AudioResampler.h"

class USoundSubmix;

// Simple locking circular buffer
// Uses 32bit wrap around logic explained here: 
// https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/
class FAtomCircularSampleBuffer
{
public:
	FAtomCircularSampleBuffer(int32 InInitialCapacity)
	{
		SetCapacity(InInitialCapacity);
	}

	// This doesn't preserve contents.
	void SetCapacity(int32 InCapacity)
	{
		check(InCapacity > 0);

		if (!FMath::IsPowerOfTwo(InCapacity))
		{
			InCapacity = FMath::RoundUpToPowerOfTwo(InCapacity);
		}

		Buffer.SetNumZeroed(InCapacity, true /* Allow shrinking */);
		Mask = InCapacity - 1;
		Read = 0; // Empty.
		Write = 0;

		check(Mask != 0);
	}
	int32 GetCapacity() const
	{
		return Buffer.Num();
	}
	int32 Num() const
	{
		return Write - Read;
	}

	// Get number of samples that can be pushed onto the buffer before it is full.
	int32 Remainder() const
	{
		return Buffer.Num() - Num();
	}

	int32 Push(const float* InBuffer, int32 InSize)
	{
		int32 CanPush = FMath::Min(Remainder(), InSize);
		for (int32 i = 0; i < CanPush; ++i)
		{
			Enqueue(InBuffer[i]);
		}
		return CanPush;
	}
	int32 Pop(float* OutBuffer, int32 InNumSamples)
	{
		int32 CanPop = FMath::Min(Num(), InNumSamples);
		for (int32 i = 0; i < CanPop; ++i)
		{
			OutBuffer[i] = Dequeue();
		}
		return CanPop;
	}

	int32 PushZeros(int32 InNumSamplesOfSilence)
	{
		int32 CanPush = FMath::Min(GetCapacity(), InNumSamplesOfSilence);
		for (int32 i = 0; i < CanPush; ++i)
		{
			Enqueue(0.f);
		}
		return CanPush;
	}
private:
	uint32 Read = 0;		// These grow indefinitely until wrap at 2^32,
	uint32 Write = 0;		// this allows us to use full capacity as write >= read.
	uint32 Mask = 0;
	TArray<float> Buffer;

	// NOTE: Not forceinline as compiler does better job without.
	void Enqueue(const float InFloat)
	{
		Buffer[Write++ & Mask] = InFloat;
	}
	float Dequeue()
	{
		return Buffer[Read++ & Mask];
	}
};

/** SubmixBufferListener used to Mix Atom sound rack output with unreal sound submix. */
class FAtomRackSubmixListener
	: public ISubmixBufferListener
{
public:
	
	FAtomRackSubmixListener(int32 InRackID, int32 InRackNumChannels, int32 InRackSampleRate, USoundSubmix* InSubmix = nullptr);
	virtual ~FAtomRackSubmixListener();

	void RegisterSubmixListener();

	// ISubmixBufferListener
	virtual void OnNewSubmixBuffer(const USoundSubmix* OwningSubmix, float* AudioData, int32 NumSamples, int32 NumChannels, const int32 SampleRate, double AudioClock) override;
	virtual bool IsRenderingAudio() const override;
	// ~ISubmixBufferListener

	struct FBufferParams
	{
		FBufferParams() {}
		
		const int32 ReceivingBufferSizeInFrames = 1024;
		const float RackToSubmixBufferRatio = 3.0f;
		const float InitialSilenceFillRatio = 1.0f;
	};

private:

	void UnregisterSubmixListener();

	class FAtomRackSubmixBuffer
	{
	public:

		FAtomRackSubmixBuffer(int32 InitialNumChannels = 2, FBufferParams&& Params = FBufferParams());

		int32 EnqueueData(TArray<TArray<float>>& AtomData, int32 NumChannels, int NumFrames, float SampleRateRatio = 1.0f);
		int32 PushData(TArray<float> InData);
		int32 PopData(int32 NumResquestedSamples, float* OutBuffer);
		int32 GetRemainder() const { return Buffer.Remainder(); }
		void Reset(int NumChannels, float SampleRateRatio = 1.0f);
		bool IsEmpty() const;
		
	private:
		
		bool IsFull() const;
		
		FBufferParams BufferParams;
		FAtomCircularSampleBuffer Buffer;
		Audio::FResampler AudioResampler;
		int32 NumChannels;
		float SampleRateRatio;

		TArray<float> TempInterleavedBuffer;
		TArray<float> SampleRateConversionBuffer;
	};

	FAtomRackSubmixBuffer RackBuffer;
	TArray<TArray<float>> Buffers;
	TArray<float*> OutputBuffers;

	int32 RackID;
	int32 RackNumChannels;
	int32 RackSampleRate;
	USoundSubmix* Submix;
};