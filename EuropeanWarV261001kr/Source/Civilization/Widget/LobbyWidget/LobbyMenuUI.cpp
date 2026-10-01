// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyMenuUI.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/UniformGridPanel.h"
#include "Components/Image.h"
#include "../MainMenuWidget/CountrySelectSlotUI.h"
#include "../MainMenuWidget/CountrySelectMiniSlot.h"
#include "../../SuperGameInstance.h"
#include "../../LobbyGameState.h"
#include "../../SuperGameController.h"
#include "../../Multiplayer/MultiplayerSessionSubsystem.h"
#include "../../Country/CountryStruct.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"

void ULobbyMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartBtn)
	{
		StartBtn->OnClicked.AddDynamic(this, &ULobbyMenuUI::OnStartButtonClicked);
	}

	if (BackBtn)
	{
		BackBtn->OnClicked.AddDynamic(this, &ULobbyMenuUI::OnBackButtonClicked);
	}

	if (WorldSizeSlider)
	{
		WorldSizeSlider->OnValueChanged.AddDynamic(this, &ULobbyMenuUI::OnWorldSizeSliderValueChanged);
		OnWorldSizeSliderValueChanged(WorldSizeSlider->GetValue());
	}

	if (ForestRatioSlider)
	{
		ForestRatioSlider->OnValueChanged.AddDynamic(this, &ULobbyMenuUI::OnForestRatioSliderValueChanged);
		OnForestRatioSliderValueChanged(ForestRatioSlider->GetValue());
	}

	if (TemperateRatioSlider)
	{
		TemperateRatioSlider->OnValueChanged.AddDynamic(this, &ULobbyMenuUI::OnTemperateRatioSliderValueChanged);
		OnTemperateRatioSliderValueChanged(TemperateRatioSlider->GetValue());
	}

	if (PlainRatioSlider)
	{
		PlainRatioSlider->OnValueChanged.AddDynamic(this, &ULobbyMenuUI::OnPlainRatioSliderValueChanged);
		OnPlainRatioSliderValueChanged(PlainRatioSlider->GetValue());
	}

	if (DesertRatioSlider)
	{
		DesertRatioSlider->OnValueChanged.AddDynamic(this, &ULobbyMenuUI::OnDesertRatioSliderValueChanged);
		OnDesertRatioSliderValueChanged(DesertRatioSlider->GetValue());
	}

	if (HillRatioSlider)
	{
		HillRatioSlider->OnValueChanged.AddDynamic(this, &ULobbyMenuUI::OnHillRatioSliderValueChanged);
		OnHillRatioSliderValueChanged(HillRatioSlider->GetValue());
	}

	BindCountrySlot(HostCountrySelect);
	BindCountrySlot(ClientCountrySelect);
	InitializeSlotWithNoSelect(HostCountrySelect);
	InitializeSlotWithNoSelect(ClientCountrySelect);
	InitializeCountrySelectMiniSlots();
	InitializeHoveredCountryUI();
	HideWarningText();
	ApplyInteractableRules();
	TryCreateHostSession();
	BindLobbyGameState();
}

void ULobbyMenuUI::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BindLobbyGameStateTimerHandle);
	}

	if (ALobbyGameState* LobbyGameState = GetLobbyGameState())
	{
		LobbyGameState->OnLobbyStateChanged.RemoveDynamic(this, &ULobbyMenuUI::OnLobbyStateChanged);
	}

	if (UMultiplayerSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->OnDestroySessionComplete.RemoveDynamic(this, &ULobbyMenuUI::OnDestroySessionComplete);
		SessionSubsystem->OnCreateSessionComplete.RemoveDynamic(this, &ULobbyMenuUI::OnHostCreateSessionComplete);
	}

	Super::NativeDestruct();
}

void ULobbyMenuUI::TryCreateHostSession()
{
	if (!IsLobbyHost())
	{
		return;
	}

	UMultiplayerSessionSubsystem* SessionSubsystem = GetSessionSubsystem();
	if (!SessionSubsystem || SessionSubsystem->IsHostingSession())
	{
		return;
	}

	SessionSubsystem->OnCreateSessionComplete.RemoveDynamic(this, &ULobbyMenuUI::OnHostCreateSessionComplete);
	SessionSubsystem->OnCreateSessionComplete.AddDynamic(this, &ULobbyMenuUI::OnHostCreateSessionComplete);
	SessionSubsystem->CreateSession(UMultiplayerSessionSubsystem::DefaultMaxPlayers);
}

void ULobbyMenuUI::OnHostCreateSessionComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		SetWarningText(TEXT("참가자를 기다리는 중..."));
		return;
	}

	SetWarningText(TEXT("방 만들기에 실패했습니다!"));
}

bool ULobbyMenuUI::IsLobbyHost() const
{
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController && GetWorld())
	{
		PlayerController = GetWorld()->GetFirstPlayerController();
	}

	if (PlayerController)
	{
		return PlayerController->IsLocalController() && PlayerController->HasAuthority();
	}
	return false;
}

UCountrySelectSlotUI* ULobbyMenuUI::GetLocalCountrySlot() const
{
	return IsLobbyHost() ? HostCountrySelect : ClientCountrySelect;
}

void ULobbyMenuUI::ApplyInteractableRules()
{
	const bool bIsHost = IsLobbyHost();

	if (WorldSizeSlider) { WorldSizeSlider->SetIsEnabled(bIsHost); }
	if (ForestRatioSlider) { ForestRatioSlider->SetIsEnabled(bIsHost); }
	if (TemperateRatioSlider) { TemperateRatioSlider->SetIsEnabled(bIsHost); }
	if (PlainRatioSlider) { PlainRatioSlider->SetIsEnabled(bIsHost); }
	if (DesertRatioSlider) { DesertRatioSlider->SetIsEnabled(bIsHost); }
	if (HillRatioSlider) { HillRatioSlider->SetIsEnabled(bIsHost); }

	if (StartBtn)
	{
		StartBtn->SetVisibility(bIsHost ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void ULobbyMenuUI::BindCountrySlot(UCountrySelectSlotUI* CountrySlot)
{
	if (!CountrySlot)
	{
		return;
	}

	CountrySlot->OnCountrySelectSlotClicked.RemoveDynamic(this, &ULobbyMenuUI::OnCountrySelectSlotClicked);
	CountrySlot->OnCountrySelectSlotClicked.AddDynamic(this, &ULobbyMenuUI::OnCountrySelectSlotClicked);
}

void ULobbyMenuUI::SetWarningText(const FString& Message)
{
	if (!WarningTxt)
	{
		return;
	}

	WarningTxt->SetText(FText::FromString(Message));
	WarningTxt->SetVisibility(ESlateVisibility::Visible);
}

void ULobbyMenuUI::HideWarningText()
{
	if (WarningTxt)
	{
		WarningTxt->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ULobbyMenuUI::SnapIntegerSlider(USlider* Slider, float Value, UTextBlock* LabelTxt)
{
	const int32 IntValue = FMath::RoundToInt(Value);
	const float RoundedValue = static_cast<float>(IntValue);
	if (Slider && FMath::Abs(Value - RoundedValue) > 0.01f)
	{
		Slider->SetValue(RoundedValue);
	}
	if (LabelTxt)
	{
		LabelTxt->SetText(FText::AsNumber(IntValue));
	}
}

void ULobbyMenuUI::SnapRatioSlider(USlider* Slider, float Value, UTextBlock* LabelTxt, bool bInvertPercent)
{
	const float RoundedValue = FMath::RoundToFloat(Value * 10.0f) / 10.0f;
	if (Slider && FMath::Abs(Value - RoundedValue) > 0.001f)
	{
		Slider->SetValue(RoundedValue);
	}
	if (LabelTxt)
	{
		const int32 PercentValue = FMath::RoundToInt(RoundedValue * 100.0f);
		LabelTxt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), bInvertPercent ? (100 - PercentValue) : PercentValue)));
	}
}

void ULobbyMenuUI::ApplyWorldConfig(const FWorldConfig& WorldConfig)
{
	bIsApplyingWorldConfig = true;

	if (WorldSizeSlider)
	{
		WorldSizeSlider->SetValue(static_cast<float>(WorldConfig.WorldRadius));
		OnWorldSizeSliderValueChanged(WorldSizeSlider->GetValue());
	}
	if (ForestRatioSlider)
	{
		ForestRatioSlider->SetValue(WorldConfig.ForestPercentage);
		OnForestRatioSliderValueChanged(ForestRatioSlider->GetValue());
	}
	if (TemperateRatioSlider)
	{
		TemperateRatioSlider->SetValue(WorldConfig.TemperatePercentage);
		OnTemperateRatioSliderValueChanged(TemperateRatioSlider->GetValue());
	}
	if (PlainRatioSlider)
	{
		PlainRatioSlider->SetValue(WorldConfig.PlainsPercentage);
		OnPlainRatioSliderValueChanged(PlainRatioSlider->GetValue());
	}

	const float NonTemperate = 1.0f - WorldConfig.TemperatePercentage;
	if (DesertRatioSlider && NonTemperate > KINDA_SMALL_NUMBER)
	{
		DesertRatioSlider->SetValue(WorldConfig.TundraPercentage / NonTemperate);
		OnDesertRatioSliderValueChanged(DesertRatioSlider->GetValue());
	}

	const float NonPlain = 1.0f - WorldConfig.PlainsPercentage;
	if (HillRatioSlider && NonPlain > KINDA_SMALL_NUMBER)
	{
		HillRatioSlider->SetValue(WorldConfig.MountainPercentage / NonPlain);
		OnHillRatioSliderValueChanged(HillRatioSlider->GetValue());
	}

	bIsApplyingWorldConfig = false;
}

void ULobbyMenuUI::ApplyCountryToSlot(bool bHostSlot, FName CountryRowName)
{
	ApplyCountryDataToSlot(bHostSlot ? HostCountrySelect : ClientCountrySelect, CountryRowName);
}

FWorldConfig ULobbyMenuUI::BuildWorldConfigFromSliders() const
{
	FWorldConfig NewWorldConfig;
	NewWorldConfig.PlayerCount = 2;
	NewWorldConfig.OceanPercentage = 0.4f;

	if (WorldSizeSlider)
	{
		NewWorldConfig.WorldRadius = FMath::RoundToInt(WorldSizeSlider->GetValue());
	}
	if (ForestRatioSlider)
	{
		NewWorldConfig.ForestPercentage = ForestRatioSlider->GetValue();
	}
	if (TemperateRatioSlider && DesertRatioSlider)
	{
		const float TemperateSliderValue = TemperateRatioSlider->GetValue();
		const float DesertSliderValue = DesertRatioSlider->GetValue();
		NewWorldConfig.TemperatePercentage = TemperateSliderValue;
		const float NonTemperate = 1.0f - TemperateSliderValue;
		NewWorldConfig.DesertPercentage = NonTemperate * (1.0f - DesertSliderValue);
		NewWorldConfig.TundraPercentage = NonTemperate * DesertSliderValue;
	}
	if (PlainRatioSlider && HillRatioSlider)
	{
		const float PlainSliderValue = PlainRatioSlider->GetValue();
		const float HillSliderValue = HillRatioSlider->GetValue();
		NewWorldConfig.PlainsPercentage = PlainSliderValue;
		const float NonPlain = 1.0f - PlainSliderValue;
		NewWorldConfig.HillsPercentage = NonPlain * (1.0f - HillSliderValue);
		NewWorldConfig.MountainPercentage = NonPlain * HillSliderValue;
	}

	return NewWorldConfig;
}

bool ULobbyMenuUI::HasUnselectedCountry() const
{
	auto IsUnselected = [](const UCountrySelectSlotUI* CountrySlot)
	{
		if (!CountrySlot)
		{
			return true;
		}
		const FName CountryName = CountrySlot->GetRowName();
		return CountryName == NAME_None || CountryName == TEXT("NoSelect");
	};

	return IsUnselected(HostCountrySelect) || IsUnselected(ClientCountrySelect);
}

void ULobbyMenuUI::OnStartButtonClicked()
{
	if (!IsLobbyHost())
	{
		return;
	}

	if (!GetWorld() || GetWorld()->GetNumPlayerControllers() < 2)
	{
		SetWarningText(TEXT("참가자를 기다려 주세요!"));
		return;
	}

	if (HasUnselectedCountry())
	{
		SetWarningText(TEXT("모든 플레이어가 국가를 선택해야 합니다!"));
		return;
	}

	const FName HostCountry = HostCountrySelect ? HostCountrySelect->GetRowName() : NAME_None;
	const FName ClientCountry = ClientCountrySelect ? ClientCountrySelect->GetRowName() : NAME_None;
	if (HostCountry == ClientCountry)
	{
		SetWarningText(TEXT("같은 국가는 선택할 수 없습니다!"));
		return;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GameInstance)
	{
		return;
	}

	GameInstance->SetWorldConfig(BuildWorldConfigFromSliders());
	GameInstance->SetCountryNames({ HostCountry, ClientCountry });
	GameInstance->SetTargetLevel(TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Civilization/Maps/InGame.InGame"))));

	if (ALobbyGameState* LobbyGameState = GetLobbyGameState())
	{
		LobbyGameState->SetWorldConfig(BuildWorldConfigFromSliders());
		LobbyGameState->CommitLobbyToGameInstances();
	}
}

void ULobbyMenuUI::OnBackButtonClicked()
{
	HideWarningText();

	if (UMultiplayerSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->OnDestroySessionComplete.RemoveDynamic(this, &ULobbyMenuUI::OnDestroySessionComplete);
		SessionSubsystem->OnDestroySessionComplete.AddDynamic(this, &ULobbyMenuUI::OnDestroySessionComplete);
		SessionSubsystem->DestroySession();
		return;
	}

	ReturnToMainMenu();
}

void ULobbyMenuUI::OnDestroySessionComplete(bool bWasSuccessful)
{
	if (UMultiplayerSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->OnDestroySessionComplete.RemoveDynamic(this, &ULobbyMenuUI::OnDestroySessionComplete);
	}

	ReturnToMainMenu();
}

void ULobbyMenuUI::ReturnToMainMenu()
{
	UGameplayStatics::OpenLevel(this, TEXT("MainMenu"));
}

UMultiplayerSessionSubsystem* ULobbyMenuUI::GetSessionSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
	}
	return nullptr;
}

ALobbyGameState* ULobbyMenuUI::GetLobbyGameState() const
{
	if (UWorld* World = GetWorld())
	{
		return World->GetGameState<ALobbyGameState>();
	}
	return nullptr;
}

void ULobbyMenuUI::BindLobbyGameState()
{
	if (ALobbyGameState* LobbyGameState = GetLobbyGameState())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(BindLobbyGameStateTimerHandle);
		}

		LobbyGameState->OnLobbyStateChanged.RemoveDynamic(this, &ULobbyMenuUI::OnLobbyStateChanged);
		LobbyGameState->OnLobbyStateChanged.AddDynamic(this, &ULobbyMenuUI::OnLobbyStateChanged);
		OnLobbyStateChanged();
		PushWorldConfigIfHost();
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			BindLobbyGameStateTimerHandle,
			this,
			&ULobbyMenuUI::BindLobbyGameState,
			0.1f,
			true);
	}
}

void ULobbyMenuUI::OnLobbyStateChanged()
{
	ALobbyGameState* LobbyGameState = GetLobbyGameState();
	if (!LobbyGameState)
	{
		return;
	}

	ApplyWorldConfig(LobbyGameState->GetWorldConfig());
	ApplyCountryToSlot(true, LobbyGameState->GetHostCountryName());
	ApplyCountryToSlot(false, LobbyGameState->GetClientCountryName());
}

void ULobbyMenuUI::PushWorldConfigIfHost()
{
	if (!IsLobbyHost() || bIsApplyingWorldConfig)
	{
		return;
	}

	if (ALobbyGameState* LobbyGameState = GetLobbyGameState())
	{
		LobbyGameState->SetWorldConfig(BuildWorldConfigFromSliders());
	}
}

void ULobbyMenuUI::RequestLocalCountry(FName CountryRowName)
{
	if (IsLobbyHost())
	{
		if (ALobbyGameState* LobbyGameState = GetLobbyGameState())
		{
			LobbyGameState->SetHostCountry(CountryRowName);
		}
		return;
	}

	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController && GetWorld())
	{
		PlayerController = GetWorld()->GetFirstPlayerController();
	}

	if (ASuperGameController* GameController = Cast<ASuperGameController>(PlayerController))
	{
		GameController->RequestSetLobbyCountry(CountryRowName);
	}
}

void ULobbyMenuUI::OnWorldSizeSliderValueChanged(float Value)
{
	if (!IsLobbyHost() && !bIsApplyingWorldConfig)
	{
		return;
	}
	SnapIntegerSlider(WorldSizeSlider, Value, WorldSizeTxt);
	PushWorldConfigIfHost();
}

void ULobbyMenuUI::OnForestRatioSliderValueChanged(float Value)
{
	if (!IsLobbyHost() && !bIsApplyingWorldConfig)
	{
		return;
	}
	SnapRatioSlider(ForestRatioSlider, Value, ForestRatioTxt, false);
	PushWorldConfigIfHost();
}

void ULobbyMenuUI::OnTemperateRatioSliderValueChanged(float Value)
{
	if (!IsLobbyHost() && !bIsApplyingWorldConfig)
	{
		return;
	}
	SnapRatioSlider(TemperateRatioSlider, Value, TemperateRatioTxt, false);
	PushWorldConfigIfHost();
}

void ULobbyMenuUI::OnPlainRatioSliderValueChanged(float Value)
{
	if (!IsLobbyHost() && !bIsApplyingWorldConfig)
	{
		return;
	}
	SnapRatioSlider(PlainRatioSlider, Value, PlainRatioTxt, false);
	PushWorldConfigIfHost();
}

void ULobbyMenuUI::OnDesertRatioSliderValueChanged(float Value)
{
	if (!IsLobbyHost() && !bIsApplyingWorldConfig)
	{
		return;
	}
	SnapRatioSlider(DesertRatioSlider, Value, DesertRatioTxt, true);
	if (TundraRatioTxt)
	{
		const float RoundedValue = FMath::RoundToFloat(Value * 10.0f) / 10.0f;
		const int32 PercentValue = FMath::RoundToInt(RoundedValue * 100.0f);
		TundraRatioTxt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PercentValue)));
	}
	PushWorldConfigIfHost();
}

void ULobbyMenuUI::OnHillRatioSliderValueChanged(float Value)
{
	if (!IsLobbyHost() && !bIsApplyingWorldConfig)
	{
		return;
	}
	SnapRatioSlider(HillRatioSlider, Value, HillRatioTxt, true);
	if (MountainRatioTxt)
	{
		const float RoundedValue = FMath::RoundToFloat(Value * 10.0f) / 10.0f;
		const int32 PercentValue = FMath::RoundToInt(RoundedValue * 100.0f);
		MountainRatioTxt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PercentValue)));
	}
	PushWorldConfigIfHost();
}

void ULobbyMenuUI::OnCountrySelectSlotClicked(UCountrySelectSlotUI* ClickedSlot)
{
	if (!ClickedSlot || ClickedSlot != GetLocalCountrySlot())
	{
		return;
	}

	LastSelectedSlot = ClickedSlot;
	if (OpenCountrySelectBrd)
	{
		PlayAnimation(OpenCountrySelectBrd, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
	}
}

void ULobbyMenuUI::InitializeSlotWithNoSelect(UCountrySelectSlotUI* TargetSlot)
{
	ApplyCountryDataToSlot(TargetSlot, TEXT("NoSelect"));
}

void ULobbyMenuUI::InitializeHoveredCountryUI()
{
	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	UDataTable* CountryDataTable = SuperGameInst->GetCountryDataTable();
	if (!CountryDataTable)
	{
		return;
	}

	FCountryData* NoSelectData = CountryDataTable->FindRow<FCountryData>(TEXT("NoSelect"), TEXT("InitializeHoveredCountryUI"));
	if (!NoSelectData)
	{
		return;
	}

	if (HoveredCountryImg && !NoSelectData->CountryKingImg.IsNull())
	{
		if (UTexture2D* NoSelectImage = NoSelectData->CountryKingImg.LoadSynchronous())
		{
			HoveredCountryImg->SetBrushFromTexture(NoSelectImage);
		}
	}
	if (HoveredCountryTxt)
	{
		HoveredCountryTxt->SetText(FText::FromString(NoSelectData->CountryName));
	}
	if (HoveredCountryColorBrd)
	{
		HoveredCountryColorBrd->SetBrushColor(NoSelectData->BorderColor);
	}
}

void ULobbyMenuUI::InitializeCountrySelectMiniSlots()
{
	if (!CountrySelectUGP)
	{
		return;
	}

	CountrySelectUGP->ClearChildren();

	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	UDataTable* CountryDataTable = SuperGameInst->GetCountryDataTable();
	if (!CountryDataTable)
	{
		return;
	}

	UClass* MiniSlotClass = LoadClass<UCountrySelectMiniSlot>(nullptr, TEXT("/Game/Civilization/Widget/MainMenuWidget/W_CountrySelectMiniSlot.W_CountrySelectMiniSlot_C"));
	if (!MiniSlotClass)
	{
		return;
	}

	const TArray<FName> CountryRowNames = {
		FName(TEXT("England")),
		FName(TEXT("France")),
		FName(TEXT("Rome")),
		FName(TEXT("Maly")),
		FName(TEXT("Norway")),
		FName(TEXT("Poland")),
		FName(TEXT("Islam")),
		FName(TEXT("Byzantium")),
		FName(TEXT("Spain")),
	};

	const int32 Columns = 3;
	for (int32 i = 0; i < CountryRowNames.Num(); i++)
	{
		UCountrySelectMiniSlot* MiniSlot = CreateWidget<UCountrySelectMiniSlot>(this, MiniSlotClass);
		if (!MiniSlot)
		{
			continue;
		}

		CountrySelectUGP->AddChildToUniformGrid(MiniSlot, i / Columns, i % Columns);
		MiniSlot->SetVisibility(ESlateVisibility::Visible);
		MiniSlot->SetRowName(CountryRowNames[i]);

		if (FCountryData* CountryData = CountryDataTable->FindRow<FCountryData>(CountryRowNames[i], TEXT("InitializeCountrySelectMiniSlots")))
		{
			if (!CountryData->CountryLargeImg.IsNull())
			{
				if (UTexture2D* CountryImage = CountryData->CountryLargeImg.LoadSynchronous())
				{
					MiniSlot->SetCountryImage(CountryImage);
				}
			}
		}

		MiniSlot->OnCountrySelectMiniSlotHovered.AddDynamic(this, &ULobbyMenuUI::OnCountrySelectMiniSlotHovered);
		MiniSlot->OnCountrySelectMiniSlotUnhovered.AddDynamic(this, &ULobbyMenuUI::OnCountrySelectMiniSlotUnhovered);
		MiniSlot->OnCountrySelectMiniSlotClicked.AddDynamic(this, &ULobbyMenuUI::OnCountrySelectMiniSlotClicked);
	}
}

void ULobbyMenuUI::OnCountrySelectMiniSlotHovered(UCountrySelectMiniSlot* HoveredSlot)
{
	if (!HoveredSlot || !GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst || !SuperGameInst->GetCountryDataTable())
	{
		return;
	}

	FCountryData* CountryData = SuperGameInst->GetCountryDataTable()->FindRow<FCountryData>(HoveredSlot->GetRowName(), TEXT("OnCountrySelectMiniSlotHovered"));
	if (!CountryData)
	{
		return;
	}

	if (HoveredCountryImg && !CountryData->CountryKingImg.IsNull())
	{
		if (UTexture2D* CountryImage = CountryData->CountryKingImg.LoadSynchronous())
		{
			HoveredCountryImg->SetBrushFromTexture(CountryImage);
		}
	}
	if (HoveredCountryTxt)
	{
		HoveredCountryTxt->SetText(FText::FromString(CountryData->CountryName));
	}
	if (HoveredCountryColorBrd)
	{
		HoveredCountryColorBrd->SetBrushColor(CountryData->BorderColor);
	}
}

void ULobbyMenuUI::OnCountrySelectMiniSlotUnhovered()
{
	InitializeHoveredCountryUI();
}

void ULobbyMenuUI::OnCountrySelectMiniSlotClicked(UCountrySelectMiniSlot* ClickedSlot)
{
	if (!ClickedSlot)
	{
		return;
	}

	if (CloseCountrySelectBrd)
	{
		PlayAnimation(CloseCountrySelectBrd, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
	}

	InitializeHoveredCountryUI();
	ApplyCountryDataToSelectedSlot(ClickedSlot->GetRowName());
}

void ULobbyMenuUI::ApplyCountryDataToSelectedSlot(FName RowName)
{
	ApplyCountryDataToSlot(LastSelectedSlot, RowName);
	RequestLocalCountry(RowName);
	if (!HasUnselectedCountry())
	{
		HideWarningText();
	}
}

void ULobbyMenuUI::ApplyCountryDataToSlot(UCountrySelectSlotUI* TargetSlot, FName RowName)
{
	if (!TargetSlot || !GetWorld())
	{
		return;
	}

	if (RowName.IsNone())
	{
		RowName = TEXT("NoSelect");
	}

	TargetSlot->SetRowName(RowName);
	TargetSlot->SetVisibility(ESlateVisibility::Visible);

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst || !SuperGameInst->GetCountryDataTable())
	{
		TargetSlot->SetCountryText(RowName.ToString());
		return;
	}

	FCountryData* CountryData = SuperGameInst->GetCountryDataTable()->FindRow<FCountryData>(RowName, TEXT("ApplyCountryDataToSlot"));
	if (!CountryData)
	{
		TargetSlot->SetCountryText(RowName.ToString());
		return;
	}

	TargetSlot->SetCountryText(CountryData->CountryName);
	if (!CountryData->CountryLargeImg.IsNull())
	{
		if (UTexture2D* CountryImage = CountryData->CountryLargeImg.LoadSynchronous())
		{
			TargetSlot->SetCountryImage(CountryImage);
		}
	}
}
