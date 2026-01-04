#include "EnergySaver.h"

float GEnergySaverInactivityTimePluggedIn = 30;
static FAutoConsoleVariableRef CVarEnergySaverInactivityTimePluggedIn(
	TEXT("EnergySaver.InactivityTime.PluggedIn"),
	GEnergySaverInactivityTimePluggedIn,
	TEXT("Idle time threshold at which energy saving kicks in while the device is plugged in (seconds). Set to 0 to disable"),
	ECVF_Default);

float GEnergySaverInactivityTimeOnBattery = 10;
static FAutoConsoleVariableRef CVarEnergySaverInactivityTimeOnBattery(
	TEXT("EnergySaver.InactivityTime.OnBattery"),
	GEnergySaverInactivityTimeOnBattery,
	TEXT("Idle time threshold at which energy saving kicks in while running on battery (seconds). Set to 0 to disable"),
	ECVF_Default);

int32 GEnergySaverMaxFps = 30;
static FAutoConsoleVariableRef CVarEnergySaverMaxFps(
	TEXT("EnergySaver.MaxFps"),
	GEnergySaverMaxFps,
	TEXT("Max FPS for the energy saving mode. Set to 0 to disable"),
	ECVF_Default);

int32 GEnergySaverMaxScreenPercentage = 50;
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

ETickableTickType UEnergySaver::GetTickableTickType() const
{
	// If this is a template or has not been initialized yet, set to never tick and it will be enabled when it is initialized
	if (IsTemplate() || !bInitialized)
	{
		return ETickableTickType::Never;
	}

	// Otherwise default to conditional
	return ETickableTickType::Conditional;
}

void UEnergySaver::Initialize(FSubsystemCollectionBase& Collection)
{
	check(!bInitialized);
	bInitialized = true;

	// Refresh the tick type after initialization
	SetTickableTickType(GetTickableTickType());
}

void UEnergySaver::Deinitialize()
{
	check(bInitialized);
	bInitialized = false;

	// Always cancel tick as this is about to be destroyed
	SetTickableTickType(ETickableTickType::Never);
}

void UEnergySaver::Tick(float DeltaTime)
{
	const bool bIsRunningOnBattery = FPlatformMisc::IsRunningOnBattery();
	const float Threshold = bIsRunningOnBattery ? GEnergySaverInactivityTimeOnBattery : GEnergySaverInactivityTimePluggedIn;
	bPrevEnergySavingEnabled = bEnergySavingEnabled;

	if (FMath::IsNearlyZero(Threshold))
	{
		bEnergySavingEnabled = false;
	}
	else
	{
		// GetLastUserInteractionTime() sometimes returns inconsistent values which are lower than in the previous frame, that's why the value is not allowed to become lower.
		// On PS5, make sure to set Slate.Input.MotionFiresUserInteractionEvents to false, otherwise energy saving won't work
		LastInteractionTime = FMath::Max(FSlateApplication::Get().GetLastUserInteractionTime(), LastInteractionTime);

		const double TimeSinceLastInteraction = FSlateApplication::Get().GetCurrentTime() - LastInteractionTime;
		bEnergySavingEnabled = TimeSinceLastInteraction > Threshold || (GEnergySaverWhenWindowInactive && !FApp::HasFocus());
	}

	if (bEnergySavingEnabled != bPrevEnergySavingEnabled)
	{
		// With dynamic resolution enabled, use r.DynamicRes.ThrottlingMaxScreenPercentage. Otherwise, use r.screenpercentage (not great, because it can cause a hitch)
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
		}
	}
}

TStatId UEnergySaver::GetStatId() const
{
	return GetStatID();
}