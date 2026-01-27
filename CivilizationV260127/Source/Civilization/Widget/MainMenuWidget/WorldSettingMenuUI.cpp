// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldSettingMenuUI.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Border.h"
#include "Components/UniformGridPanel.h"
#include "Components/Image.h"
#include "CountrySelectSlotUI.h"
#include "CountrySelectMiniSlot.h"
#include "../../SuperGameInstance.h"
#include "../../Country/CountryStruct.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"

void UWorldSettingMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 클릭 이벤트 바인딩
	if (StartBtn)
	{
		StartBtn->OnClicked.AddDynamic(this, &UWorldSettingMenuUI::OnStartButtonClicked);
	}

	if (BackBtn)
	{
		BackBtn->OnClicked.AddDynamic(this, &UWorldSettingMenuUI::OnBackButtonClicked);
	}

	// 플레이어 수 슬라이더 이벤트 바인딩
	if (PlayerCountSlider)
	{
		PlayerCountSlider->OnValueChanged.AddDynamic(this, &UWorldSettingMenuUI::OnPlayerCountSliderValueChanged);
		
		// 초기 텍스트 값 설정
		if (PlayerCountTxt)
		{
			int32 InitialValue = FMath::RoundToInt(PlayerCountSlider->GetValue());
			PlayerCountTxt->SetText(FText::AsNumber(InitialValue));
		}

		// 초기 AI 플레이어 슬롯 업데이트
		UpdateAIPlayerSlots();
	}

	// 국가 선택 미니 슬롯 그리드 초기화
	InitializeCountrySelectMiniSlots();

	// 호버된 국가 UI 기본값 설정 (NoSelect)
	InitializeHoveredCountryUI();

	// 경고 텍스트 초기화 (초기에는 숨김)
	if (WarningTxt)
	{
		WarningTxt->SetVisibility(ESlateVisibility::Hidden);
	}

	// 월드 크기 슬라이더 이벤트 바인딩
	if (WorldSizeSlider)
	{
		WorldSizeSlider->OnValueChanged.AddDynamic(this, &UWorldSettingMenuUI::OnWorldSizeSliderValueChanged);
		
		// 초기 텍스트 값 설정
		if (WorldSizeTxt)
		{
			int32 InitialValue = FMath::RoundToInt(WorldSizeSlider->GetValue());
			WorldSizeTxt->SetText(FText::AsNumber(InitialValue));
		}
	}

	// 숲 비율 슬라이더 이벤트 바인딩
	if (ForestRatioSlider)
	{
		ForestRatioSlider->OnValueChanged.AddDynamic(this, &UWorldSettingMenuUI::OnForestRatioSliderValueChanged);
		
		// 초기 텍스트 값 설정
		if (ForestRatioTxt)
		{
			float InitialValue = ForestRatioSlider->GetValue();
			float RoundedValue = FMath::RoundToFloat(InitialValue * 10.0f) / 10.0f;
			int32 PercentValue = FMath::RoundToInt(RoundedValue * 100.0f);
			ForestRatioTxt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PercentValue)));
		}
	}

	// 온대 비율 슬라이더 이벤트 바인딩
	if (TemperateRatioSlider)
	{
		TemperateRatioSlider->OnValueChanged.AddDynamic(this, &UWorldSettingMenuUI::OnTemperateRatioSliderValueChanged);
		
		// 초기 텍스트 값 설정
		if (TemperateRatioTxt)
		{
			float InitialValue = TemperateRatioSlider->GetValue();
			float RoundedValue = FMath::RoundToFloat(InitialValue * 10.0f) / 10.0f;
			int32 PercentValue = FMath::RoundToInt(RoundedValue * 100.0f);
			TemperateRatioTxt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PercentValue)));
		}
	}

	// 평지 비율 슬라이더 이벤트 바인딩
	if (PlainRatioSlider)
	{
		PlainRatioSlider->OnValueChanged.AddDynamic(this, &UWorldSettingMenuUI::OnPlainRatioSliderValueChanged);
		
		// 초기 텍스트 값 설정
		if (PlainRatioTxt)
		{
			float InitialValue = PlainRatioSlider->GetValue();
			float RoundedValue = FMath::RoundToFloat(InitialValue * 10.0f) / 10.0f;
			int32 PercentValue = FMath::RoundToInt(RoundedValue * 100.0f);
			PlainRatioTxt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PercentValue)));
		}
	}

	// 사막 비율 슬라이더 이벤트 바인딩
	if (DesertRatioSlider)
	{
		DesertRatioSlider->OnValueChanged.AddDynamic(this, &UWorldSettingMenuUI::OnDesertRatioSliderValueChanged);
		
		// 초기 텍스트 값 설정
		if (DesertRatioTxt)
		{
			float InitialValue = DesertRatioSlider->GetValue();
			float RoundedValue = FMath::RoundToFloat(InitialValue * 10.0f) / 10.0f;
			int32 PercentValue = FMath::RoundToInt(RoundedValue * 100.0f);
			DesertRatioTxt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), (100 - PercentValue))));
		}

		// 툰드라 비율 초기 텍스트 값 설정
		if (TundraRatioTxt)
		{
			float InitialValue = DesertRatioSlider->GetValue();
			float RoundedValue = FMath::RoundToFloat(InitialValue * 10.0f) / 10.0f;
			int32 PercentValue = FMath::RoundToInt(RoundedValue * 100.0f);
			TundraRatioTxt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PercentValue)));
		}
	}

	// 언덕 비율 슬라이더 이벤트 바인딩
	if (HillRatioSlider)
	{
		HillRatioSlider->OnValueChanged.AddDynamic(this, &UWorldSettingMenuUI::OnHillRatioSliderValueChanged);
		
		// 초기 텍스트 값 설정
		if (HillRatioTxt)
		{
			float InitialValue = HillRatioSlider->GetValue();
			float RoundedValue = FMath::RoundToFloat(InitialValue * 10.0f) / 10.0f;
			int32 PercentValue = FMath::RoundToInt(RoundedValue * 100.0f);
			HillRatioTxt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), (100 - PercentValue))));
		}

		// 산 비율 초기 텍스트 값 설정
		if (MountainRatioTxt)
		{
			float InitialValue = HillRatioSlider->GetValue();
			float RoundedValue = FMath::RoundToFloat(InitialValue * 10.0f) / 10.0f;
			int32 PercentValue = FMath::RoundToInt(RoundedValue * 100.0f);
			MountainRatioTxt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PercentValue)));
		}
	}
}

void UWorldSettingMenuUI::OnStartButtonClicked()
{
	// NoSelect 상태인 슬롯이 있는지 체크
	bool bHasNoSelect = false;

	// PlayerSelect 체크
	if (PlayerSelect)
	{
		FName PlayerCountryName = PlayerSelect->GetRowName();
		if (PlayerCountryName == NAME_None || PlayerCountryName == TEXT("NoSelect"))
		{
			bHasNoSelect = true;
		}
	}
	else
	{
		// PlayerSelect가 없으면 시작 불가
		bHasNoSelect = true;
	}

	// AIPlayerSelectVB의 슬롯들 체크
	if (!bHasNoSelect && AIPlayerSelectVB)
	{
		for (int32 i = 0; i < AIPlayerSelectVB->GetChildrenCount(); i++)
		{
			UWidget* ChildWidget = AIPlayerSelectVB->GetChildAt(i);
			if (ChildWidget)
			{
				UCountrySelectSlotUI* CountrySlot = Cast<UCountrySelectSlotUI>(ChildWidget);
				if (CountrySlot)
				{
					FName CountryName = CountrySlot->GetRowName();
					if (CountryName == NAME_None || CountryName == TEXT("NoSelect"))
					{
						bHasNoSelect = true;
						break;
					}
				}
			}
		}
	}

	// NoSelect 상태인 슬롯이 있으면 시작 불가
	if (bHasNoSelect)
	{
		// 경고 텍스트를 visible로 표시
		if (WarningTxt)
		{
			WarningTxt->SetVisibility(ESlateVisibility::Visible);
		}
		return;
	}

	// 게임 인스턴스 가져오기
	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GameInstance)
	{
		return;
	}

	// FWorldConfig 구조체 생성 및 설정
	FWorldConfig NewWorldConfig;

	// 플레이어 수 설정 (PlayerCountSlider)
	if (PlayerCountSlider)
	{
		NewWorldConfig.PlayerCount = FMath::RoundToInt(PlayerCountSlider->GetValue());
	}

	// 바다 비율 고정 (0.4 = 40%)
	NewWorldConfig.OceanPercentage = 0.4f;

	// 월드 크기 설정 (WorldSizeSlider)
	if (WorldSizeSlider)
	{
		NewWorldConfig.WorldRadius = FMath::RoundToInt(WorldSizeSlider->GetValue());
	}

	// 숲 비율 설정 (ForestRatioSlider)
	if (ForestRatioSlider)
	{
		NewWorldConfig.ForestPercentage = ForestRatioSlider->GetValue();
	}

	// 온대, 사막, 툰드라 비율 설정 (TemperateRatioTxt 표시값 = 온대 비율로 그대로 적용)
	if (TemperateRatioSlider && DesertRatioSlider)
	{
		float TemperateSliderValue = TemperateRatioSlider->GetValue();
		float DesertSliderValue = DesertRatioSlider->GetValue();

		// TemperateRatioSlider value = 온대 비율 (UI 70% → TemperatePercentage 0.7)
		NewWorldConfig.TemperatePercentage = TemperateSliderValue;

		// 나머지 (1 - 온대)를 사막과 툰드라가 나눔
		float NonTemperate = 1.0f - TemperateSliderValue;
		NewWorldConfig.DesertPercentage = NonTemperate * (1.0f - DesertSliderValue);
		NewWorldConfig.TundraPercentage = NonTemperate * DesertSliderValue;
	}

	// 평지, 언덕, 산 비율 설정 (PlainRatioTxt 표시값 = 평지 비율로 그대로 적용)
	if (PlainRatioSlider && HillRatioSlider)
	{
		float PlainSliderValue = PlainRatioSlider->GetValue();
		float HillSliderValue = HillRatioSlider->GetValue();

		// PlainRatioSlider value = 평지 비율 (UI 70% → PlainsPercentage 0.7)
		NewWorldConfig.PlainsPercentage = PlainSliderValue;

		// 나머지 (1 - 평지)를 언덕과 산이 나눔
		float NonPlain = 1.0f - PlainSliderValue;
		NewWorldConfig.HillsPercentage = NonPlain * (1.0f - HillSliderValue);
		NewWorldConfig.MountainPercentage = NonPlain * HillSliderValue;
	}

	// WorldConfig 설정
	GameInstance->SetWorldConfig(NewWorldConfig);

	// 국가 배열 수집 및 설정
	TArray<FName> CollectedCountryNames = CollectCountryNames();
	GameInstance->SetCountryNames(CollectedCountryNames);

	// InGame 레벨로 이동
	FSoftObjectPath InGameLevelPath(TEXT("/Game/Civilization/Maps/InGame.InGame"));
	TSoftObjectPtr<UWorld> InGameLevel(InGameLevelPath);
	GameInstance->OpenLevel(InGameLevel);
}

void UWorldSettingMenuUI::OnBackButtonClicked()
{
	// 경고 텍스트 숨김
	if (WarningTxt)
	{
		WarningTxt->SetVisibility(ESlateVisibility::Hidden);
	}

	// 뒤로가기 버튼 클릭 델리게이트 브로드캐스트
	OnBackButtonClickedDelegate.Broadcast();
}

void UWorldSettingMenuUI::OnPlayerCountSliderValueChanged(float Value)
{
	// 슬라이더 값을 정수로 반올림
	int32 PlayerCount = FMath::RoundToInt(Value);
	float RoundedValue = static_cast<float>(PlayerCount);

	// 반올림된 값을 슬라이더에 다시 설정 (딱딱 끊기게)
	if (PlayerCountSlider && FMath::Abs(Value - RoundedValue) > 0.01f)
	{
		PlayerCountSlider->SetValue(RoundedValue);
	}

	// 텍스트 업데이트
	if (PlayerCountTxt)
	{
		PlayerCountTxt->SetText(FText::AsNumber(PlayerCount));
	}

	// CountrySelectBrd를 hidden으로 설정
	if (CountrySelectBrd)
	{
		CountrySelectBrd->SetVisibility(ESlateVisibility::Hidden);
	}

	// 경고 텍스트 숨김
	if (WarningTxt)
	{
		WarningTxt->SetVisibility(ESlateVisibility::Hidden);
	}

	// AI 플레이어 슬롯 업데이트
	UpdateAIPlayerSlots();
}

void UWorldSettingMenuUI::OnWorldSizeSliderValueChanged(float Value)
{
	// 슬라이더 값을 정수로 반올림
	int32 WorldSize = FMath::RoundToInt(Value);
	float RoundedValue = static_cast<float>(WorldSize);

	// 반올림된 값을 슬라이더에 다시 설정 (딱딱 끊기게)
	if (WorldSizeSlider && FMath::Abs(Value - RoundedValue) > 0.01f)
	{
		WorldSizeSlider->SetValue(RoundedValue);
	}

	// 텍스트 업데이트
	if (WorldSizeTxt)
	{
		WorldSizeTxt->SetText(FText::AsNumber(WorldSize));
	}
}

void UWorldSettingMenuUI::OnForestRatioSliderValueChanged(float Value)
{
	// 슬라이더 값을 0.1 단위로 반올림
	float RoundedValue = FMath::RoundToFloat(Value * 10.0f) / 10.0f;

	// 반올림된 값을 슬라이더에 다시 설정 (0.1 단위로 딱딱 끊기게)
	if (ForestRatioSlider && FMath::Abs(Value - RoundedValue) > 0.001f)
	{
		ForestRatioSlider->SetValue(RoundedValue);
	}

	// 텍스트 업데이트 (퍼센트로 표시)
	if (ForestRatioTxt)
	{
		int32 PercentValue = FMath::RoundToInt(RoundedValue * 100.0f);
		ForestRatioTxt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PercentValue)));
	}
}

void UWorldSettingMenuUI::OnTemperateRatioSliderValueChanged(float Value)
{
	// 슬라이더 값을 0.1 단위로 반올림
	float RoundedValue = FMath::RoundToFloat(Value * 10.0f) / 10.0f;

	// 반올림된 값을 슬라이더에 다시 설정 (0.1 단위로 딱딱 끊기게)
	if (TemperateRatioSlider && FMath::Abs(Value - RoundedValue) > 0.001f)
	{
		TemperateRatioSlider->SetValue(RoundedValue);
	}

	// 텍스트 업데이트 (퍼센트로 표시)
	if (TemperateRatioTxt)
	{
		int32 PercentValue = FMath::RoundToInt(RoundedValue * 100.0f);
		TemperateRatioTxt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PercentValue)));
	}
}

void UWorldSettingMenuUI::OnPlainRatioSliderValueChanged(float Value)
{
	// 슬라이더 값을 0.1 단위로 반올림
	float RoundedValue = FMath::RoundToFloat(Value * 10.0f) / 10.0f;

	// 반올림된 값을 슬라이더에 다시 설정 (0.1 단위로 딱딱 끊기게)
	if (PlainRatioSlider && FMath::Abs(Value - RoundedValue) > 0.001f)
	{
		PlainRatioSlider->SetValue(RoundedValue);
	}

	// 텍스트 업데이트 (퍼센트로 표시)
	if (PlainRatioTxt)
	{
		int32 PercentValue = FMath::RoundToInt(RoundedValue * 100.0f);
		PlainRatioTxt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PercentValue)));
	}
}

void UWorldSettingMenuUI::OnDesertRatioSliderValueChanged(float Value)
{
	// 슬라이더 값을 0.1 단위로 반올림
	float RoundedValue = FMath::RoundToFloat(Value * 10.0f) / 10.0f;

	// 반올림된 값을 슬라이더에 다시 설정 (0.1 단위로 딱딱 끊기게)
	if (DesertRatioSlider && FMath::Abs(Value - RoundedValue) > 0.001f)
	{
		DesertRatioSlider->SetValue(RoundedValue);
	}

	// 텍스트 업데이트 (퍼센트로 표시)
	if (DesertRatioTxt)
	{
		int32 PercentValue = FMath::RoundToInt(RoundedValue * 100.0f);
		DesertRatioTxt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), (100 - PercentValue))));
	}

	// 툰드라 비율 텍스트 업데이트 (퍼센트로 표시)
	if (TundraRatioTxt)
	{
		int32 PercentValue = FMath::RoundToInt(RoundedValue * 100.0f);
		TundraRatioTxt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PercentValue)));
	}
}

void UWorldSettingMenuUI::OnHillRatioSliderValueChanged(float Value)
{
	// 슬라이더 값을 0.1 단위로 반올림
	float RoundedValue = FMath::RoundToFloat(Value * 10.0f) / 10.0f;

	// 반올림된 값을 슬라이더에 다시 설정 (0.1 단위로 딱딱 끊기게)
	if (HillRatioSlider && FMath::Abs(Value - RoundedValue) > 0.001f)
	{
		HillRatioSlider->SetValue(RoundedValue);
	}

	// 텍스트 업데이트 (퍼센트로 표시)
	if (HillRatioTxt)
	{
		int32 PercentValue = FMath::RoundToInt(RoundedValue * 100.0f);
		HillRatioTxt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), (100 - PercentValue))));
	}

	// 산 비율 텍스트 업데이트 (퍼센트로 표시)
	if (MountainRatioTxt)
	{
		int32 PercentValue = FMath::RoundToInt(RoundedValue * 100.0f);
		MountainRatioTxt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PercentValue)));
	}
}

void UWorldSettingMenuUI::SetPlayerSelect(UCountrySelectSlotUI* InPlayerSelect)
{
	PlayerSelect = InPlayerSelect;

	// PlayerSelect 델리게이트 바인딩
	if (PlayerSelect)
	{
		PlayerSelect->OnCountrySelectSlotClicked.RemoveDynamic(this, &UWorldSettingMenuUI::OnCountrySelectSlotClicked);
		PlayerSelect->OnCountrySelectSlotClicked.AddDynamic(this, &UWorldSettingMenuUI::OnCountrySelectSlotClicked);

		// NoSelect 데이터로 초기화
		InitializeSlotWithNoSelect(PlayerSelect);
	}
}

void UWorldSettingMenuUI::UpdateAIPlayerSlots()
{
	if (!AIPlayerSelectVB || !PlayerCountSlider)
	{
		return;
	}

	// 플레이어 수 가져오기
	int32 PlayerCount = FMath::RoundToInt(PlayerCountSlider->GetValue());
	
	// AI 플레이어 수 = 전체 플레이어 수 - 1 (플레이어 본인 제외)
	int32 AIPlayerCount = FMath::Max(0, PlayerCount - 1);

	// 기존 슬롯 제거
	AIPlayerSelectVB->ClearChildren();

	// AI 플레이어 슬롯 생성
	for (int32 i = 0; i < AIPlayerCount; i++)
	{
		// CountrySelectSlotUI 블루프린트 클래스 로드
		UClass* SlotClass = LoadClass<UCountrySelectSlotUI>(nullptr, TEXT("/Game/Civilization/Widget/MainMenuWidget/W_CountrySelectSlot.W_CountrySelectSlot_C"));
		if (SlotClass)
		{
			// 위젯 생성
			UCountrySelectSlotUI* AISlot = CreateWidget<UCountrySelectSlotUI>(this, SlotClass);
			if (AISlot)
			{
				// VerticalBox에 추가
				AIPlayerSelectVB->AddChild(AISlot);

				// 위젯 가시성 설정
				AISlot->SetVisibility(ESlateVisibility::Visible);

				// NoSelect 데이터로 초기화
				InitializeSlotWithNoSelect(AISlot);
			}
		}
	}

	// AI 플레이어 슬롯 델리게이트 바인딩
	BindAIPlayerSlotDelegates();
}

void UWorldSettingMenuUI::BindAIPlayerSlotDelegates()
{
	if (!AIPlayerSelectVB)
	{
		return;
	}

	// AIPlayerSelectVB의 모든 자식 위젯에 델리게이트 바인딩
	int32 ChildCount = AIPlayerSelectVB->GetChildrenCount();
	for (int32 i = 0; i < ChildCount; i++)
	{
		if (UWidget* ChildWidget = AIPlayerSelectVB->GetChildAt(i))
		{
			if (UCountrySelectSlotUI* CountrySlot = Cast<UCountrySelectSlotUI>(ChildWidget))
			{
				// 기존 바인딩 제거 후 새로 바인딩
				CountrySlot->OnCountrySelectSlotClicked.RemoveDynamic(this, &UWorldSettingMenuUI::OnCountrySelectSlotClicked);
				CountrySlot->OnCountrySelectSlotClicked.AddDynamic(this, &UWorldSettingMenuUI::OnCountrySelectSlotClicked);
			}
		}
	}
}

void UWorldSettingMenuUI::OnCountrySelectSlotClicked(UCountrySelectSlotUI* ClickedSlot)
{
	if (!ClickedSlot)
	{
		return;
	}

	// 마지막으로 클릭한 슬롯 저장
	LastSelectedSlot = ClickedSlot;

	// CountrySelectBrd 보더를 visible로 설정
	if (CountrySelectBrd)
	{
		CountrySelectBrd->SetVisibility(ESlateVisibility::Visible);
	}

	// 여기에 국가 선택 슬롯 클릭 시 실행할 기능을 구현하세요
	// 예: 선택된 슬롯의 정보를 표시하거나, 다른 UI를 업데이트하는 등의 작업
}

void UWorldSettingMenuUI::InitializeSlotWithNoSelect(UCountrySelectSlotUI* TargetSlot)
{
	if (!TargetSlot || !GetWorld())
	{
		return;
	}

	// GameInstance에서 CountryDataTable 가져오기
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

	// NoSelect 행의 데이터 가져오기
	FName NoSelectRowName = FName(TEXT("NoSelect"));
	FCountryData* CountryData = CountryDataTable->FindRow<FCountryData>(NoSelectRowName, TEXT("InitializeSlotWithNoSelect"));
	if (!CountryData)
	{
		return;
	}

	// 슬롯에 데이터 설정
	TargetSlot->SetCountryText(CountryData->CountryName);
	TargetSlot->SetBorderColor(CountryData->BorderColor);
	TargetSlot->SetRowName(TEXT("NoSelect")); // RowName 저장

	// 이미지 설정
	if (!CountryData->CountryRoundImg.IsNull())
	{
		UTexture2D* CountryImage = CountryData->CountryRoundImg.LoadSynchronous();
		if (CountryImage)
		{
			TargetSlot->SetCountryImage(CountryImage);
		}
	}
}

void UWorldSettingMenuUI::InitializeCountrySelectMiniSlots()
{
	if (!CountrySelectUGP)
	{
		return;
	}

	// 기존 슬롯 제거
	CountrySelectUGP->ClearChildren();

	// 3열 4행 = 12개의 미니 슬롯 생성
	const int32 Columns = 3;
	const int32 Rows = 4;
	const int32 TotalSlots = Columns * Rows;

	// GameInstance에서 CountryDataTable 가져오기
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

	// NoSelect 행의 데이터 가져오기
	FName NoSelectRowName = FName(TEXT("NoSelect"));
	FCountryData* NoSelectData = CountryDataTable->FindRow<FCountryData>(NoSelectRowName, TEXT("InitializeCountrySelectMiniSlots"));
	if (!NoSelectData)
	{
		return;
	}

	// NoSelect 이미지 가져오기 (CountryRoundImg 사용)
	UTexture2D* NoSelectImage = nullptr;
	if (!NoSelectData->CountryRoundImg.IsNull())
	{
		NoSelectImage = NoSelectData->CountryRoundImg.LoadSynchronous();
	}

	// CountrySelectMiniSlot 블루프린트 클래스 로드
	UClass* MiniSlotClass = LoadClass<UCountrySelectMiniSlot>(nullptr, TEXT("/Game/Civilization/Widget/MainMenuWidget/W_CountrySelectMiniSlot.W_CountrySelectMiniSlot_C"));
	if (!MiniSlotClass)
	{
		return;
	}

	// 국가 이름 배열 (12개 슬롯 순서대로)
	TArray<FName> CountryRowNames = {
		FName(TEXT("England")),    // 0
		FName(TEXT("Italy")),      // 1
		FName(TEXT("France")),     // 2
		FName(TEXT("Germany")),    // 3
		FName(TEXT("Spain")),      // 4
		FName(TEXT("Portugal")),   // 5
		FName(TEXT("Russia")),     // 6
		FName(TEXT("Netherlands")), // 7
		FName(TEXT("Poland")),     // 8
		FName(TEXT("Sweden")),     // 9
		FName(TEXT("Swiss")),      // 10
		FName(TEXT("Turkey"))      // 11
	};

	// 각 국가의 이미지를 미리 로드
	TArray<UTexture2D*> CountryImages;
	CountryImages.SetNum(TotalSlots);
	
	for (int32 i = 0; i < TotalSlots && i < CountryRowNames.Num(); i++)
	{
		FCountryData* CountryData = CountryDataTable->FindRow<FCountryData>(CountryRowNames[i], TEXT("InitializeCountrySelectMiniSlots"));
		if (CountryData && !CountryData->CountryRoundImg.IsNull())
		{
			CountryImages[i] = CountryData->CountryRoundImg.LoadSynchronous();
		}
		else
		{
			// 국가 데이터를 찾지 못한 경우 NoSelect 이미지 사용
			CountryImages[i] = NoSelectImage;
		}
	}

	// 미니 슬롯 생성 및 그리드에 추가
	for (int32 i = 0; i < TotalSlots; i++)
	{
		// 위젯 생성
		UCountrySelectMiniSlot* MiniSlot = CreateWidget<UCountrySelectMiniSlot>(this, MiniSlotClass);
		if (MiniSlot)
		{
			// 그리드 위치 계산 (열, 행)
			int32 Column = i % Columns;
			int32 Row = i / Columns;

			// 그리드 패널에 추가
			CountrySelectUGP->AddChildToUniformGrid(MiniSlot, Row, Column);

			// 위젯 가시성 설정
			MiniSlot->SetVisibility(ESlateVisibility::Visible);

			// 슬롯 인덱스에 따라 국가 설정
			if (i < CountryRowNames.Num())
			{
				// 국가 데이터 가져오기
				FCountryData* CountryData = CountryDataTable->FindRow<FCountryData>(CountryRowNames[i], TEXT("InitializeCountrySelectMiniSlots"));
				
				// 국가 이미지 설정 (CountryRoundImg 사용)
				if (CountryImages[i])
				{
					MiniSlot->SetCountryImage(CountryImages[i]);
				}
				else if (NoSelectImage)
				{
					MiniSlot->SetCountryImage(NoSelectImage);
				}

				// 국경선 색상 설정
				if (CountryData)
				{
					MiniSlot->SetBorderColor(CountryData->BorderColor);
				}

				// RowName 설정
				MiniSlot->SetRowName(CountryRowNames[i]);
			}
			else
			{
				// 12개를 초과하는 경우 NoSelect
				if (NoSelectImage)
				{
					MiniSlot->SetCountryImage(NoSelectImage);
				}

				// NoSelect의 국경선 색상 설정
				MiniSlot->SetBorderColor(NoSelectData->BorderColor);

				// RowName 설정 (NoSelect)
				MiniSlot->SetRowName(NoSelectRowName);
			}

			// 호버 델리게이트 바인딩
			MiniSlot->OnCountrySelectMiniSlotHovered.RemoveDynamic(this, &UWorldSettingMenuUI::OnCountrySelectMiniSlotHovered);
			MiniSlot->OnCountrySelectMiniSlotHovered.AddDynamic(this, &UWorldSettingMenuUI::OnCountrySelectMiniSlotHovered);

			// 언호버 델리게이트 바인딩
			MiniSlot->OnCountrySelectMiniSlotUnhovered.RemoveDynamic(this, &UWorldSettingMenuUI::OnCountrySelectMiniSlotUnhovered);
			MiniSlot->OnCountrySelectMiniSlotUnhovered.AddDynamic(this, &UWorldSettingMenuUI::OnCountrySelectMiniSlotUnhovered);

			// 클릭 델리게이트 바인딩
			MiniSlot->OnCountrySelectMiniSlotClicked.RemoveDynamic(this, &UWorldSettingMenuUI::OnCountrySelectMiniSlotClicked);
			MiniSlot->OnCountrySelectMiniSlotClicked.AddDynamic(this, &UWorldSettingMenuUI::OnCountrySelectMiniSlotClicked);
		}
	}
}

void UWorldSettingMenuUI::InitializeHoveredCountryUI()
{
	if (!GetWorld())
	{
		return;
	}

	// GameInstance에서 CountryDataTable 가져오기
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

	// NoSelect 행의 데이터 가져오기
	FName NoSelectRowName = FName(TEXT("NoSelect"));
	FCountryData* NoSelectData = CountryDataTable->FindRow<FCountryData>(NoSelectRowName, TEXT("InitializeHoveredCountryUI"));
	if (!NoSelectData)
	{
		return;
	}

	// HoveredCountryImg 설정
	if (HoveredCountryImg)
	{
		if (!NoSelectData->CountryRoundImg.IsNull())
		{
			UTexture2D* NoSelectImage = NoSelectData->CountryRoundImg.LoadSynchronous();
			if (NoSelectImage)
			{
				HoveredCountryImg->SetBrushFromTexture(NoSelectImage);
			}
		}
	}

	// HoveredCountryTxt 설정
	if (HoveredCountryTxt)
	{
		HoveredCountryTxt->SetText(FText::FromString(NoSelectData->CountryName));
	}

	// HoveredCountryColorBrd 설정
	if (HoveredCountryColorBrd)
	{
		HoveredCountryColorBrd->SetBrushColor(NoSelectData->BorderColor);
	}
}

void UWorldSettingMenuUI::OnCountrySelectMiniSlotHovered(UCountrySelectMiniSlot* HoveredSlot)
{
	if (!HoveredSlot || !GetWorld())
	{
		return;
	}

	// 슬롯의 RowName 가져오기
	FName RowName = HoveredSlot->GetRowName();
	if (RowName == NAME_None)
	{
		return;
	}

	// GameInstance에서 CountryDataTable 가져오기
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

	// 해당 RowName의 데이터 가져오기
	FCountryData* CountryData = CountryDataTable->FindRow<FCountryData>(RowName, TEXT("OnCountrySelectMiniSlotHovered"));
	if (!CountryData)
	{
		return;
	}

	// HoveredCountryImg 설정
	if (HoveredCountryImg)
	{
		if (!CountryData->CountryRoundImg.IsNull())
		{
			UTexture2D* CountryImage = CountryData->CountryRoundImg.LoadSynchronous();
			if (CountryImage)
			{
				HoveredCountryImg->SetBrushFromTexture(CountryImage);
			}
		}
	}

	// HoveredCountryTxt 설정
	if (HoveredCountryTxt)
	{
		HoveredCountryTxt->SetText(FText::FromString(CountryData->CountryName));
	}

	// HoveredCountryColorBrd 설정
	if (HoveredCountryColorBrd)
	{
		HoveredCountryColorBrd->SetBrushColor(CountryData->BorderColor);
	}
}

void UWorldSettingMenuUI::OnCountrySelectMiniSlotUnhovered()
{
	// 언호버 시 NoSelect 데이터로 되돌리기
	InitializeHoveredCountryUI();
}

void UWorldSettingMenuUI::OnCountrySelectMiniSlotClicked(UCountrySelectMiniSlot* ClickedSlot)
{
	if (!ClickedSlot)
	{
		return;
	}

	// CountrySelectBrd를 hidden으로 설정
	if (CountrySelectBrd)
	{
		CountrySelectBrd->SetVisibility(ESlateVisibility::Hidden);
	}

	// HoveredCountryImg, HoveredCountryTxt, HoveredCountryColorBrd를 NoSelect로 되돌리기
	InitializeHoveredCountryUI();

	// 클릭한 슬롯의 RowName 가져오기
	FName RowName = ClickedSlot->GetRowName();
	if (RowName == NAME_None)
	{
		return;
	}

	// 마지막으로 선택된 슬롯에 국가 데이터 적용
	ApplyCountryDataToSelectedSlot(RowName);
}

void UWorldSettingMenuUI::ApplyCountryDataToSelectedSlot(FName RowName)
{
	// 마지막으로 선택된 슬롯이 없으면 무시
	if (!LastSelectedSlot)
	{
		return;
	}

	if (!GetWorld())
	{
		return;
	}

	// GameInstance에서 CountryDataTable 가져오기
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

	// 해당 RowName의 데이터 가져오기
	FCountryData* CountryData = CountryDataTable->FindRow<FCountryData>(RowName, TEXT("ApplyCountryDataToSelectedSlot"));
	if (!CountryData)
	{
		return;
	}

	// 선택된 슬롯에 데이터 설정
	LastSelectedSlot->SetCountryText(CountryData->CountryName);
	LastSelectedSlot->SetBorderColor(CountryData->BorderColor);
	LastSelectedSlot->SetRowName(RowName); // RowName 저장

	// 이미지 설정
	if (!CountryData->CountryRoundImg.IsNull())
	{
		UTexture2D* CountryImage = CountryData->CountryRoundImg.LoadSynchronous();
		if (CountryImage)
		{
			LastSelectedSlot->SetCountryImage(CountryImage);
		}
	}

	// 모든 슬롯이 선택되었는지 체크하여 경고 텍스트 숨김
	bool bHasNoSelect = false;

	// PlayerSelect 체크
	if (PlayerSelect)
	{
		FName PlayerCountryName = PlayerSelect->GetRowName();
		if (PlayerCountryName == NAME_None || PlayerCountryName == TEXT("NoSelect"))
		{
			bHasNoSelect = true;
		}
	}
	else
	{
		bHasNoSelect = true;
	}

	// AIPlayerSelectVB의 슬롯들 체크
	if (!bHasNoSelect && AIPlayerSelectVB)
	{
		for (int32 i = 0; i < AIPlayerSelectVB->GetChildrenCount(); i++)
		{
			UWidget* ChildWidget = AIPlayerSelectVB->GetChildAt(i);
			if (ChildWidget)
			{
				UCountrySelectSlotUI* CountrySlot = Cast<UCountrySelectSlotUI>(ChildWidget);
				if (CountrySlot)
				{
					FName CountryName = CountrySlot->GetRowName();
					if (CountryName == NAME_None || CountryName == TEXT("NoSelect"))
					{
						bHasNoSelect = true;
						break;
					}
				}
			}
		}
	}

	// 모든 슬롯이 선택되었으면 경고 텍스트 숨김
	if (!bHasNoSelect && WarningTxt)
	{
		WarningTxt->SetVisibility(ESlateVisibility::Hidden);
	}
}

TArray<FName> UWorldSettingMenuUI::CollectCountryNames() const
{
	TArray<FName> CountryNames;

	// PlayerSelect의 국가 추가 (Player 0)
	if (PlayerSelect)
	{
		FName PlayerCountryName = PlayerSelect->GetRowName();
		if (PlayerCountryName != NAME_None)
		{
			CountryNames.Add(PlayerCountryName);
		}
		else
		{
			// RowName이 없으면 기본값 "NoSelect" 추가
			CountryNames.Add(TEXT("NoSelect"));
		}
	}
	else
	{
		// PlayerSelect가 없으면 기본값 추가
		CountryNames.Add(TEXT("NoSelect"));
	}

	// AIPlayerSelectVB의 슬롯들 순서대로 추가 (Player 1~N)
	if (AIPlayerSelectVB)
	{
		for (int32 i = 0; i < AIPlayerSelectVB->GetChildrenCount(); i++)
		{
			UWidget* ChildWidget = AIPlayerSelectVB->GetChildAt(i);
			if (ChildWidget)
			{
				UCountrySelectSlotUI* CountrySlot = Cast<UCountrySelectSlotUI>(ChildWidget);
				if (CountrySlot)
				{
					FName CountryName = CountrySlot->GetRowName();
					if (CountryName != NAME_None)
					{
						CountryNames.Add(CountryName);
					}
					else
					{
						// RowName이 없으면 기본값 "NoSelect" 추가
						CountryNames.Add(TEXT("NoSelect"));
					}
				}
			}
		}
	}

	return CountryNames;
}