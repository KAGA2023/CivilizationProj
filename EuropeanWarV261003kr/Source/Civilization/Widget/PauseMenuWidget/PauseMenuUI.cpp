// Fill out your copyright notice in the Description page of Project Settings.

#include "PauseMenuUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "../../SuperGameInstance.h"
#include "../../SaveLoad/SaveLoadManager.h"
#include "../../SaveLoad/SaveLoadStruct.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

void UPauseMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 선택된 슬롯 초기화
	SelectedSlotIndex = 0;

	// 슬롯 정보 텍스트 초기화 (기본 Hidden)
	if (SlotExplainTxt)
	{
		SlotExplainTxt->SetText(FText::FromString(TEXT("")));
		SlotExplainTxt->SetVisibility(ESlateVisibility::Hidden);
	}

	// 버튼 클릭 이벤트 바인딩
	if (SaveBtn)
	{
		SaveBtn->OnClicked.AddDynamic(this, &UPauseMenuUI::OnSaveButtonClicked);
	}

	if (SaveSlot1Btn)
	{
		SaveSlot1Btn->OnClicked.AddDynamic(this, &UPauseMenuUI::OnSaveSlot1ButtonClicked);
	}

	if (SaveSlot2Btn)
	{
		SaveSlot2Btn->OnClicked.AddDynamic(this, &UPauseMenuUI::OnSaveSlot2ButtonClicked);
	}

	if (SaveSlot3Btn)
	{
		SaveSlot3Btn->OnClicked.AddDynamic(this, &UPauseMenuUI::OnSaveSlot3ButtonClicked);
	}

	if (SaveSlot4Btn)
	{
		SaveSlot4Btn->OnClicked.AddDynamic(this, &UPauseMenuUI::OnSaveSlot4ButtonClicked);
	}

	if (SaveSlot5Btn)
	{
		SaveSlot5Btn->OnClicked.AddDynamic(this, &UPauseMenuUI::OnSaveSlot5ButtonClicked);
	}

	if (TitleMenuBtn)
	{
		TitleMenuBtn->OnClicked.AddDynamic(this, &UPauseMenuUI::OnTitleMenuButtonClicked);
	}

	if (GameEndBtn)
	{
		GameEndBtn->OnClicked.AddDynamic(this, &UPauseMenuUI::OnGameEndButtonClicked);
	}

	if (BackBtn)
	{
		BackBtn->OnClicked.AddDynamic(this, &UPauseMenuUI::OnBackButtonClicked);
	}

	if (DeleteBtn)
	{
		DeleteBtn->OnClicked.AddDynamic(this, &UPauseMenuUI::OnDeleteButtonClicked);
	}
}

void UPauseMenuUI::OnSaveButtonClicked()
{
	// 선택된 슬롯이 없으면 저장하지 않음
	if (SelectedSlotIndex <= 0)
	{
		return;
	}

	// GameInstance에서 SaveLoadManager 가져오기
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			if (USaveLoadManager* SaveLoadManager = SuperGameInst->GetSaveLoadManager())
			{
				// 선택된 슬롯에 저장 (SaveGameName은 비어있으면 자동 생성)
				SaveLoadManager->SaveGameToSlot(SelectedSlotIndex, TEXT(""));

				// 저장 직후 SlotExplainTxt에 방금 저장한 슬롯 정보로 갱신
				UpdateSlotInfo(SelectedSlotIndex);
			}
		}
	}
}

void UPauseMenuUI::OnSaveSlot1ButtonClicked()
{
	SelectedSlotIndex = 1;
	UpdateSlotInfo(1);
}

void UPauseMenuUI::OnSaveSlot2ButtonClicked()
{
	SelectedSlotIndex = 2;
	UpdateSlotInfo(2);
}

void UPauseMenuUI::OnSaveSlot3ButtonClicked()
{
	SelectedSlotIndex = 3;
	UpdateSlotInfo(3);
}

void UPauseMenuUI::OnSaveSlot4ButtonClicked()
{
	SelectedSlotIndex = 4;
	UpdateSlotInfo(4);
}

void UPauseMenuUI::OnSaveSlot5ButtonClicked()
{
	SelectedSlotIndex = 5;
	UpdateSlotInfo(5);
}

void UPauseMenuUI::OnTitleMenuButtonClicked()
{
	// SlotExplainTxt Hidden 처리
	if (SlotExplainTxt)
	{
		SlotExplainTxt->SetVisibility(ESlateVisibility::Hidden);
	}

	// 타이틀 메뉴 레벨로 이동
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(World, TEXT("MainMenu"));
	}
}

void UPauseMenuUI::OnGameEndButtonClicked()
{
	// SlotExplainTxt Hidden 처리
	if (SlotExplainTxt)
	{
		SlotExplainTxt->SetVisibility(ESlateVisibility::Hidden);
	}

	// 게임 종료
	if (GetWorld())
	{
		UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
	}
}

void UPauseMenuUI::OnBackButtonClicked()
{
	// SlotExplainTxt Hidden 처리
	if (SlotExplainTxt)
	{
		SlotExplainTxt->SetVisibility(ESlateVisibility::Hidden);
	}

	// 뒤로가기 버튼 클릭 델리게이트 브로드캐스트
	OnBackButtonClickedDelegate.Broadcast();
}

void UPauseMenuUI::OnDeleteButtonClicked()
{
	// SlotExplainTxt Hidden 처리
	if (SlotExplainTxt)
	{
		SlotExplainTxt->SetVisibility(ESlateVisibility::Hidden);
	}

	// 선택된 슬롯이 없으면 삭제하지 않음
	if (SelectedSlotIndex <= 0)
	{
		return;
	}

	// GameInstance에서 SaveLoadManager 가져오기
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			if (USaveLoadManager* SaveLoadManager = SuperGameInst->GetSaveLoadManager())
			{
				// 선택된 슬롯의 세이브 데이터 삭제
				SaveLoadManager->DeleteSaveGame(SelectedSlotIndex);
				
				// 슬롯 선택 초기화
				SelectedSlotIndex = 0;
			}
		}
	}
}

void UPauseMenuUI::UpdateSlotInfo(int32 SlotIndex)
{
	if (!SlotExplainTxt)
	{
		return;
	}

	// GameInstance에서 SaveLoadManager 가져오기
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	USaveLoadManager* SaveLoadManager = SuperGameInst->GetSaveLoadManager();
	if (!SaveLoadManager)
	{
		return;
	}

	// 슬롯에 세이브 파일이 있는지 확인
	if (!SaveLoadManager->DoesSaveGameExist(SlotIndex))
	{
		SlotExplainTxt->SetText(FText::FromString(TEXT("")));
		return;
	}

	// 세이브 파일 로드
	FString SlotName = FString::Printf(TEXT("SaveSlot%d"), SlotIndex);
	USuperSaveGame* SaveGameObject = Cast<USuperSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

	if (!SaveGameObject)
	{
		SlotExplainTxt->SetText(FText::FromString(TEXT("로드 실패")));
		return;
	}

	// SaveData에서 정보 가져오기
	FGameSaveData& SaveData = SaveGameObject->SaveData;

	// 저장 시간
	FDateTime SaveDateTime = SaveData.SaveDateTime;
	FString DateTimeString = FString::Printf(
		TEXT("%04d-%02d-%02d %02d:%02d"),
		SaveDateTime.GetYear(),
		SaveDateTime.GetMonth(),
		SaveDateTime.GetDay(),
		SaveDateTime.GetHour(),
		SaveDateTime.GetMinute()
	);

	// 현재 라운드
	int32 CurrentRound = SaveData.CurrentRound;

	// 플레이어 국가 (PlayerIndex 0)
	FString CountryName = TEXT("");
	if (SaveData.PlayerDataArray.Num() > 0)
	{
		FName CountryRowName = SaveData.PlayerDataArray[0].CountryRowName;
		
		// CountryRowName을 국가 이름으로 변환
		// SuperPlayerState의 CountryDataTable을 통해 국가 이름을 가져올 수 있지만,
		// 여기서는 간단하게 RowName을 표시
		CountryName = CountryRowName.ToString();
	}

	// 정보 텍스트 생성
	FString InfoText = FString::Printf(
		TEXT("저장 일시: %s\n소속국: %s\n경과 라운드: %d"),
		*DateTimeString,
		*CountryName,
		CurrentRound
	);

	SlotExplainTxt->SetText(FText::FromString(InfoText));
	// SlotExplainTxt를 Visible로 변경
	SlotExplainTxt->SetVisibility(ESlateVisibility::Visible);
}
