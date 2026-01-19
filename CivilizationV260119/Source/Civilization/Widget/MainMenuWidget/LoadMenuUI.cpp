// Fill out your copyright notice in the Description page of Project Settings.

#include "LoadMenuUI.h"
#include "Components/Button.h"
#include "../../SuperGameInstance.h"
#include "../../SaveLoad/SaveLoadManager.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/SoftObjectPath.h"

void ULoadMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 선택된 슬롯 초기화
	SelectedSlotIndex = 0;

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
}

void ULoadMenuUI::OnSaveSlot1ButtonClicked()
{
	SelectedSlotIndex = 1;
}

void ULoadMenuUI::OnSaveSlot2ButtonClicked()
{
	SelectedSlotIndex = 2;
}

void ULoadMenuUI::OnSaveSlot3ButtonClicked()
{
	SelectedSlotIndex = 3;
}

void ULoadMenuUI::OnSaveSlot4ButtonClicked()
{
	SelectedSlotIndex = 4;
}

void ULoadMenuUI::OnSaveSlot5ButtonClicked()
{
	SelectedSlotIndex = 5;
}

void ULoadMenuUI::OnLoadButtonClicked()
{
	// 선택된 슬롯이 없으면 로드하지 않음
	if (SelectedSlotIndex <= 0)
	{
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
			// 세이브 파일이 없으면 아무 동작도 하지 않음 (필요시 에러 메시지 표시 가능)
		}
	}
}

void ULoadMenuUI::OnBackButtonClicked()
{
	// 뒤로가기 버튼 클릭 델리게이트 브로드캐스트
	OnBackButtonClickedDelegate.Broadcast();
}
