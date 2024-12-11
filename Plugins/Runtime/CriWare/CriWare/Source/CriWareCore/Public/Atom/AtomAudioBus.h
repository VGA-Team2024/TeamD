
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "Extensions/IAtomProxyInitializer.h"

#include "AtomAudioBus.generated.h"

// Forward Declarations
class FAtomRuntime;

// The number of channels to mix audio into the source bus
UENUM(BlueprintType)
enum class EAtomAudioBusChannels : uint8
{
	Mono = 0,
	Stereo = 1,
	StereoWithCenter = 2,
	Quad = 3,
	QuadWithCenter = 4,
	FivePointOne = 5,
	SevenPointOne = 7
};

class CRIWARECORE_API FAtomAudioBusProxy final
	: public Atom::TProxyData<FAtomAudioBusProxy>
{
public:
	IMPL_ATOMPROXY_CLASS(FAtomAudioBusProxy);

	explicit FAtomAudioBusProxy(UAtomAudioBus* InAudioBus);

	FAtomAudioBusProxy(const FAtomAudioBusProxy& Other) = default;

	virtual ~FAtomAudioBusProxy() override {}

	uint32 AudioBusId = INDEX_NONE;
	int32 NumChannels = INDEX_NONE;
};

using FAtomAudioBusProxyPtr = TSharedPtr<FAtomAudioBusProxy, ESPMode::ThreadSafe>;

// An audio bus is an object which represents an audio patch cord. Audio can be sent to it. It can be sonified using USoundSourceBuses.
// Instances of the audio bus are created in the audio engine. 
UCLASS(ClassGroup = Sound, meta = (BlueprintSpawnableComponent))
class CRIWARECORE_API UAtomAudioBus
	: public UObject
	, public IAtomProxyDataFactory
{
	GENERATED_BODY()

public:

	UAtomAudioBus(const FObjectInitializer& ObjectInitializer);

	/** Number of channels to use for the Audio Bus. */
	UPROPERTY(EditAnywhere, Category = BusProperties)
	EAtomAudioBusChannels AudioBusChannels = EAtomAudioBusChannels::Mono;

	// Returns the number of channels of the audio bus in integer format
	int32 GetNumChannels() const { return (int32)AudioBusChannels + 1; }

	//~ Begin UObject Interface.
	virtual void BeginDestroy() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	//~ End UObject Interface.

	//~ Begin IAtomProxy Interface
	virtual TSharedPtr<Atom::IProxyData> CreateProxyData(const Atom::FProxyDataInitParams& InitParams) override;
	//~ End IAtomProxy Interface
};