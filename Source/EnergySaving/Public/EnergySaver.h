#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "EnergySaver.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnergySavingEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnergySavingDisabled);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRenderingEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRenderingDisabled);

UCLASS()
class ENERGYSAVING_API UEnergySaver : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UEnergySaver();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	// Use this property to disallow energy saving temporarily, i.e. for cutscenes
	// If energy saving is already enabled, disallowing it won't disable it.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=EnergySaving)
	bool bIsEnergySavingAllowed = true;

	// A multicast delegate that is called any time energy saving gets enabled
	UPROPERTY(BlueprintAssignable, Category=EnergySaving)
	FOnEnergySavingEnabled OnEnergySavingEnabled;

	// A multicast delegate that is called any time energy saving gets disabled
	UPROPERTY(BlueprintAssignable, Category=EnergySaving)
	FOnEnergySavingDisabled OnEnergySavingDisabled;

	// A multicast delegate that is called any time rendering gets enabled
	UPROPERTY(BlueprintAssignable, Category=EnergySaving)
	FOnRenderingEnabled OnRenderingEnabled;

	// A multicast delegate that is called any time rendering gets disabled
	UPROPERTY(BlueprintAssignable, Category=EnergySaving)
	FOnRenderingDisabled OnRenderingDisabled;

private:
	float LastInteractionTime = 0.0;

	float MaxScreenPercentageToRestore;
	float MaxFpsToRestore;
	bool bWorldRenderingToRestore;

	bool bPrevEnergySavingEnabled;
	bool bPrevRenderingDisabled;

	bool bEnergySavingEnabled;
	bool bRenderingDisabled;

	bool bInitialized = false;

	void SetIsEnergySavingEnabled(bool NewIsEnergySavingEnabled);
	void SetIsRenderingDisabled(bool NewIsRenderingSuspended);
};
