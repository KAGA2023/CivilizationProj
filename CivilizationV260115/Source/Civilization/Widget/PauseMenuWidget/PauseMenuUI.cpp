// Fill out your copyright notice in the Description page of Project Settings.

#include "PauseMenuUI.h"
#include "Components/Button.h"
#include "../../SuperGameInstance.h"
#include "../../SaveLoad/SaveLoadManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

void UPauseMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 선택된 슬롯 초기화
	SelectedSlotIndex = 0;

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
			}
		}
	}
}

void UPauseMenuUI::OnSaveSlot1ButtonClicked()
{
	SelectedSlotIndex = 1;
}

void UPauseMenuUI::OnSaveSlot2ButtonClicked()
{
	SelectedSlotIndex = 2;
}

void UPauseMenuUI::OnSaveSlot3ButtonClicked()
{
	SelectedSlotIndex = 3;
}

void UPauseMenuUI::OnSaveSlot4ButtonClicked()
{
	SelectedSlotIndex = 4;
}

void UPauseMenuUI::OnSaveSlot5ButtonClicked()
{
	SelectedSlotIndex = 5;
}

void UPauseMenuUI::OnTitleMenuButtonClicked()
{
	// 타이틀 메뉴 레벨로 이동
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(World, TEXT("MainMenu"));
	}
}

void UPauseMenuUI::OnGameEndButtonClicked()
{
	// 게임 종료
	if (GetWorld())
	{
		UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
	}
}

void UPauseMenuUI::OnBackButtonClicked()
{
	// 뒤로가기 버튼 클릭 델리게이트 브로드캐스트
	OnBackButtonClickedDelegate.Broadcast();
}
