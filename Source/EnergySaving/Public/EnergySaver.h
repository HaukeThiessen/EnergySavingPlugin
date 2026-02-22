#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Tickable.h"
#include "EnergySaver.generated.h"

UCLASS()
class ENERGYSAVING_API UEnergySaver : public UEngineSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	UEnergySaver();

	virtual ETickableTickType GetTickableTickType() const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	// Use this property to disallow energy saving temporarily, i.e. for cutscenes
	// If energy saving is already enabled, disallowing it won't disable it. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=EnergySaving)
	bool bIsEnergySavingAllowed = true;

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
};
