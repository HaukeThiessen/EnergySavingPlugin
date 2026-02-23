#include "EnergySaver.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/App.h"

// The CVars below are supposed to be exposed to the user in your game's settings.
// If your game is only released on platforms that don't have batteries,
// feel free to remove the logic to detect batteries and adjust the settings accordingly

float GEnergySaverTimeThresholdForEnergySavingPluggedIn = 30;
static FAutoConsoleVariableRef CVarEnergySaverTimeThresholdForEnergySavingPluggedIn(
	TEXT("EnergySaver.TimeThresholdForEnergySaving.PluggedIn"),
	GEnergySaverTimeThresholdForEnergySavingPluggedIn,
	TEXT("Idle time threshold at which energy saving kicks in while the device is plugged in (seconds). Set to 0 to disable"),
	ECVF_Default);

float GEnergySaverTimeThresholdForEnergySavingOnBattery = 10;
static FAutoConsoleVariableRef CVarEnergySaverTimeThresholdForEnergySavingOnBattery(
	TEXT("EnergySaver.TimeThresholdForEnergySaving.OnBattery"),
	GEnergySaverTimeThresholdForEnergySavingOnBattery,
	TEXT("Idle time threshold at which energy saving kicks in while running on battery (seconds). Set to 0 to disable"),
	ECVF_Default);

float GEnergySaverTimeThresholdForDisabledRenderingPluggedIn = 120;
static FAutoConsoleVariableRef CVarEnergySaverTimeThresholdForDisabledRenderingPluggedIn(
	TEXT("EnergySaver.TimeThresholdForDisabledRendering.PluggedIn"),
	GEnergySaverTimeThresholdForDisabledRenderingPluggedIn,
	TEXT("Idle time threshold at which the rendering gets disabled while the device is plugged in (seconds). Set to 0 to disable"),
	ECVF_Default);

float GEnergySaverTimeThresholdForDisabledRenderingOnBattery = 60;
static FAutoConsoleVariableRef CVarEnergySaverTimeThresholdForDisabledRenderingOnBattery(
	TEXT("EnergySaver.TimeThresholdForDisabledRendering.OnBattery"),
	GEnergySaverTimeThresholdForDisabledRenderingOnBattery,
	TEXT("Idle time threshold at which the rendering gets disabled while running on battery (seconds). Set to 0 to disable"),
	ECVF_Default);

int32 GEnergySaverMaxFps = 33;
static FAutoConsoleVariableRef CVarEnergySaverMaxFps(
	TEXT("EnergySaver.MaxFps"),
	GEnergySaverMaxFps,
	TEXT("Max FPS for the energy saving mode. Set to 0 to disable"),
	ECVF_Default);

int32 GEnergySaverMaxScreenPercentage = 40;
static FAutoConsoleVariableRef CVarEnergySaverMaxScreenPercentage(
	TEXT("EnergySaver.MaxScreenPercentage"),
	GEnergySaverMaxScreenPercentage,
	TEXT("Max resolution percentage for the energy saving mode. Set to 0 to disable"),
	ECVF_Default);

bool GEnergySaverWhenWindowInactive = true;
static FAutoConsoleVariableRef EnergySaverWhenWindowInactive(
	TEXT("EnergySaver.WhenWindowInactive"),
	GEnergySaverWhenWindowInactive,
	TEXT("Whether to save energy when the window loses focus"),
	ECVF_Default);

UEnergySaver::UEnergySaver()
{
}

void UEnergySaver::Initialize(FSubsystemCollectionBase& Collection)
{
	SetTickableTickType((IsTemplate() || !bInitialized) ? ETickableTickType::Never : ETickableTickType::Always);
	Super::Initialize(Collection);
}

void UEnergySaver::Deinitialize()
{
	SetTickableTickType(ETickableTickType::Never);
	Super::Deinitialize();
}

void UEnergySaver::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UEnergySaver::Tick);
	const int BatteryLevel = FPlatformMisc::GetBatteryLevel();

	// IsRunningOnBattery() on Windows just returns whether the device has a battery, making it unusable to detect if a laptop is currently not being charged.
	// Since there is no easy way to check if the battery is being charged, having a battery that is not charged 100% is considered as running on a battery
	const bool bIsRunningOnBattery = (0 <= BatteryLevel && BatteryLevel < 100);
	const float EnergySavingThreshold = bIsRunningOnBattery ? GEnergySaverTimeThresholdForEnergySavingOnBattery : GEnergySaverTimeThresholdForEnergySavingPluggedIn;
	const float DisableRenderingThreshold = bIsRunningOnBattery ? GEnergySaverTimeThresholdForDisabledRenderingOnBattery : GEnergySaverTimeThresholdForDisabledRenderingPluggedIn;

	bPrevEnergySavingEnabled = bEnergySavingEnabled;
	bPrevRenderingDisabled = bRenderingDisabled;

	// GetLastUserInteractionTime() sometimes returns inconsistent values which are lower than in the previous frame, that's why the value is not allowed to become lower.
	// On PS5, make sure to set Slate.Input.MotionFiresUserInteractionEvents to false, otherwise energy saving won't work
	LastInteractionTime = FMath::Max(FSlateApplication::Get().GetLastUserInteractionTime(), LastInteractionTime);
	const double TimeSinceLastInteraction = FSlateApplication::Get().GetCurrentTime() - LastInteractionTime;

	if (FMath::IsNearlyZero(EnergySavingThreshold))
	{
		bEnergySavingEnabled = false;
	}
	else
	{
		bEnergySavingEnabled = TimeSinceLastInteraction > EnergySavingThreshold || (GEnergySaverWhenWindowInactive && !FApp::HasFocus());
	}

	if (FMath::IsNearlyZero(DisableRenderingThreshold))
	{
		bRenderingDisabled = false;
	}
	else
	{
		bRenderingDisabled = TimeSinceLastInteraction > DisableRenderingThreshold;
	}
	
	if (bRenderingDisabled != bPrevRenderingDisabled)
	{
		SetIsRenderingDisabled(bRenderingDisabled);
	}

	if (bEnergySavingEnabled != bPrevEnergySavingEnabled)
	{
		SetIsEnergySavingEnabled(bEnergySavingEnabled);
	}
}

TStatId UEnergySaver::GetStatId() const
{
	return GetStatID();
}

void UEnergySaver::SetIsEnergySavingEnabled(bool NewIsEnergySavingEnabled)
{
	// With dynamic resolution enabled, use r.DynamicRes.ThrottlingMaxScreenPercentage.
	// Otherwise, use r.screenpercentage (not great, because it can cause a hitch).
	static IConsoleVariable* CVarDynamicResOperationMode = IConsoleManager::Get().FindConsoleVariable(TEXT("r.DynamicRes.OperationMode"));
	const int32 DynamicResOperationMode = CVarDynamicResOperationMode->GetInt();

	static IConsoleVariable* CvarMaxScreenPercentage = DynamicResOperationMode == 0 ?
		IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage")) :
		IConsoleManager::Get().FindConsoleVariable(TEXT("r.DynamicRes.ThrottlingMaxScreenPercentage"));
	static IConsoleVariable* CvarMaxFps = IConsoleManager::Get().FindConsoleVariable(TEXT("t.maxfps"));

	if (bEnergySavingEnabled && bIsEnergySavingAllowed)
	{
		if (GEnergySaverMaxScreenPercentage > 0)
		{
			MaxScreenPercentageToRestore = CvarMaxScreenPercentage->GetFloat();
			CvarMaxScreenPercentage->AsVariable()->SetWithCurrentPriority(GEnergySaverMaxScreenPercentage);
		}
		if (GEnergySaverMaxFps > 0)
		{
			MaxFpsToRestore = CvarMaxFps->GetInt();
			CvarMaxFps->AsVariable()->SetWithCurrentPriority(GEnergySaverMaxFps);
		}
		OnEnergySavingEnabled.Broadcast();
	}
	else
	{
		if (GEnergySaverMaxScreenPercentage > 0)
		{
			CvarMaxScreenPercentage->AsVariable()->SetWithCurrentPriority(MaxScreenPercentageToRestore);
		}
		if (GEnergySaverMaxFps > 0)
		{
			CvarMaxFps->AsVariable()->SetWithCurrentPriority(MaxFpsToRestore);
		}
		OnEnergySavingDisabled.Broadcast();
	}
}

void UEnergySaver::SetIsRenderingDisabled(bool NewIsRenderingSuspended)
{
	if (GEngine->GameViewport)
	{
		FWorldContext* WorldContext = GEngine->GetWorldContextFromGameViewport(GEngine->GameViewport);
		if (WorldContext)
		{
			if (bRenderingDisabled && bIsEnergySavingAllowed)
			{
				bWorldRenderingToRestore = UGameplayStatics::GetEnableWorldRendering(WorldContext->World());
				UGameplayStatics::SetEnableWorldRendering(WorldContext->World(), false);
				OnRenderingDisabled.Broadcast();
			}
			else
			{
				UGameplayStatics::SetEnableWorldRendering(WorldContext->World(), bWorldRenderingToRestore);
				OnRenderingEnabled.Broadcast();
			}
		}
	}
}