// Fill out your copyright notice in the Description page of Project Settings.

#include "LoadMenuUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "../../SuperGameInstance.h"
#include "../../SaveLoad/SaveLoadManager.h"
#include "../../SaveLoad/SaveLoadStruct.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/SoftObjectPath.h"

void ULoadMenuUI::NativeConstruct()
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

	// 경고 텍스트 초기화 (기본 Hidden)
	if (WarningTxt)
	{
		WarningTxt->SetText(FText::FromString(TEXT("")));
		WarningTxt->SetVisibility(ESlateVisibility::Hidden);
	}

	// 버튼 클릭 이벤트 바인딩
	if (SaveSlot1Btn)
	{
		SaveSlot1Btn->OnClicked.AddDynamic(this, &ULoadMenuUI::OnSaveSlot1ButtonClicked);
	}

	if (SaveSlot2Btn)
	{
		SaveSlot2Btn->OnClicked.AddDynamic(this, &ULoadMenuUI::OnSaveSlot2ButtonClicked);
	}

	if (SaveSlot3Btn)
	{
		SaveSlot3Btn->OnClicked.AddDynamic(this, &ULoadMenuUI::OnSaveSlot3ButtonClicked);
	}

	if (SaveSlot4Btn)
	{
		SaveSlot4Btn->OnClicked.AddDynamic(this, &ULoadMenuUI::OnSaveSlot4ButtonClicked);
	}

	if (SaveSlot5Btn)
	{
		SaveSlot5Btn->OnClicked.AddDynamic(this, &ULoadMenuUI::OnSaveSlot5ButtonClicked);
	}

	if (LoadBtn)
	{
		LoadBtn->OnClicked.AddDynamic(this, &ULoadMenuUI::OnLoadButtonClicked);
	}

	if (BackBtn)
	{
		BackBtn->OnClicked.AddDynamic(this, &ULoadMenuUI::OnBackButtonClicked);
	}

	if (DeleteBtn)
	{
		DeleteBtn->OnClicked.AddDynamic(this, &ULoadMenuUI::OnDeleteButtonClicked);
	}
}

void ULoadMenuUI::OnSaveSlot1ButtonClicked()
{
	SelectedSlotIndex = 1;
	UpdateSlotInfo(1);
}

void ULoadMenuUI::OnSaveSlot2ButtonClicked()
{
	SelectedSlotIndex = 2;
	UpdateSlotInfo(2);
}

void ULoadMenuUI::OnSaveSlot3ButtonClicked()
{
	SelectedSlotIndex = 3;
	UpdateSlotInfo(3);
}

void ULoadMenuUI::OnSaveSlot4ButtonClicked()
{
	SelectedSlotIndex = 4;
	UpdateSlotInfo(4);
}

void ULoadMenuUI::OnSaveSlot5ButtonClicked()
{
	SelectedSlotIndex = 5;
	UpdateSlotInfo(5);
}

void ULoadMenuUI::OnLoadButtonClicked()
{
	// SlotExplainTxt Hidden 처리
	if (SlotExplainTxt)
	{
		SlotExplainTxt->SetVisibility(ESlateVisibility::Hidden);
	}

	// WarningTxt Hidden 처리 (초기화)
	if (WarningTxt)
	{
		WarningTxt->SetVisibility(ESlateVisibility::Hidden);
	}

	// 선택된 슬롯이 없으면 경고 메시지 표시
	if (SelectedSlotIndex <= 0)
	{
		if (WarningTxt)
		{
			WarningTxt->SetText(FText::FromString(TEXT("불러올 슬롯이 선택되지 않았습니다!")));
			WarningTxt->SetVisibility(ESlateVisibility::Visible);
		}
		return;
	}

	// GameInstance에서 SaveLoadManager 가져오기
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			// 세이브 파일 존재 여부 확인
			if (SuperGameInst->GetSaveLoadManager() && 
			    SuperGameInst->GetSaveLoadManager()->DoesSaveGameExist(SelectedSlotIndex))
			{
				// 로드 성공 시 WarningTxt Hidden 처리
				if (WarningTxt)
				{
					WarningTxt->SetVisibility(ESlateVisibility::Hidden);
				}

				// GameInstance에 로드 슬롯 인덱스 저장 (Loading 레벨에서 사용)
				SuperGameInst->PendingLoadSlotIndex = SelectedSlotIndex;

				// 게임 플레이 레벨로 이동 (Loading 레벨을 거쳐서)
				// SuperGameInstance::OpenLevel은 TSoftObjectPtr<UWorld>를 받아서
				// TargetLevel에 저장하고 Loading 레벨로 이동합니다.
				// 에러 로그를 보면 MainMenu가 /Game/Civilization/Maps/MainMenu 경로를 사용하므로
				// InGame도 동일한 경로 구조를 사용 (/Game/Civilization/Maps/InGame)
				FSoftObjectPath InGameLevelPath(TEXT("/Game/Civilization/Maps/InGame"));
				TSoftObjectPtr<UWorld> InGameLevel = TSoftObjectPtr<UWorld>(InGameLevelPath);

				// OpenLevel 호출 (Loading 레벨로 이동)
				// SuperGameInstance::OpenLevel은 내부에서 Loading 레벨로 이동하고
				// GetTargetLevelName()을 통해 InGame 레벨 이름을 반환합니다
				SuperGameInst->OpenLevel(InGameLevel);
			}
			else
			{
				// 세이브 파일이 없으면 경고 메시지 표시
				if (WarningTxt)
				{
					WarningTxt->SetText(FText::FromString(TEXT("해당 슬롯에 저장된 데이터가 없습니다!")));
					WarningTxt->SetVisibility(ESlateVisibility::Visible);
				}
			}
		}
	}
}

void ULoadMenuUI::OnBackButtonClicked()
{
	// SlotExplainTxt Hidden 처리
	if (SlotExplainTxt)
	{
		SlotExplainTxt->SetVisibility(ESlateVisibility::Hidden);
	}

	// WarningTxt Hidden 처리
	if (WarningTxt)
	{
		WarningTxt->SetVisibility(ESlateVisibility::Hidden);
	}

	// 뒤로가기 버튼 클릭 델리게이트 브로드캐스트
	OnBackButtonClickedDelegate.Broadcast();
}

void ULoadMenuUI::OnDeleteButtonClicked()
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

void ULoadMenuUI::UpdateSlotInfo(int32 SlotIndex)
{
	if (!SlotExplainTxt)
	{
		return;
	}

	// 슬롯 선택 시 경고 메시지 숨기기
	if (WarningTxt)
	{
		WarningTxt->SetVisibility(ESlateVisibility::Hidden);
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
		SlotExplainTxt->SetText(FText::FromString(TEXT("세이브 로드 실패")));
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
		TEXT("게임 저장 시간: %s\n플레이어 국가: %s\n현재 라운드: %d"),
		*DateTimeString,
		*CountryName,
		CurrentRound
	);

	SlotExplainTxt->SetText(FText::FromString(InfoText));
	// SlotExplainTxt를 Visible로 변경
	SlotExplainTxt->SetVisibility(ESlateVisibility::Visible);
}
