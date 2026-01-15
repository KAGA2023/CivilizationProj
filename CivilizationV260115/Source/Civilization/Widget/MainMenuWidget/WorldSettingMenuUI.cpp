// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldSettingMenuUI.h"
#include "Components/Button.h"

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
}

void UWorldSettingMenuUI::OnStartButtonClicked()
{
	// 시작 버튼 클릭 시 처리할 로직
	// TODO: 게임 시작 로직 구현
}

void UWorldSettingMenuUI::OnBackButtonClicked()
{
	// 뒤로가기 버튼 클릭 델리게이트 브로드캐스트
	OnBackButtonClickedDelegate.Broadcast();
}