// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerLoseUI.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UPlayerLoseUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 타이틀 메뉴 버튼 클릭 이벤트 바인딩
	if (TitleMenuBtn)
	{
		TitleMenuBtn->OnClicked.AddDynamic(this, &UPlayerLoseUI::OnTitleMenuButtonClicked);
	}
}

void UPlayerLoseUI::OnTitleMenuButtonClicked()
{
	// 타이틀 메뉴 레벨로 이동
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(World, TEXT("MainMenu"));
	}
}
