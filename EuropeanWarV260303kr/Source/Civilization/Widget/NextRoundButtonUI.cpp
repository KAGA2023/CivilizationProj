// Fill out your copyright notice in the Description page of Project Settings.

#include "NextRoundButtonUI.h"
#include "Components/Button.h"
#include "../SuperGameModeBase.h"

void UNextRoundButtonUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 클릭 이벤트 바인딩
	if (NextRoundBtn)
	{
		NextRoundBtn->OnClicked.AddDynamic(this, &UNextRoundButtonUI::OnNextRoundButtonClicked);
	}
}

void UNextRoundButtonUI::OnNextRoundButtonClicked()
{
	if (!GetWorld())
	{
		return;
	}

	// GameMode 가져오기
	if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(GetWorld()->GetAuthGameMode()))
	{
		// 플레이어 0의 턴 종료 요청
		GameMode->RequestEndPlayerTurn();
		// 이후 자동으로 모든 AI 플레이어(1, 2, 3)의 턴이 순차적으로 처리됨
	}
}

