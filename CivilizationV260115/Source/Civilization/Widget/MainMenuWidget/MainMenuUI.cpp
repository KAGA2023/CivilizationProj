// Fill out your copyright notice in the Description page of Project Settings.

#include "MainMenuUI.h"
#include "Components/Button.h"
#include "WorldSettingMenuUI.h"
#include "LoadMenuUI.h"
#include "Kismet/KismetSystemLibrary.h"

void UMainMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 클릭 이벤트 바인딩
	if (PlayBtn)
	{
		PlayBtn->OnClicked.AddDynamic(this, &UMainMenuUI::OnPlayButtonClicked);
	}

	if (EndBtn)
	{
		EndBtn->OnClicked.AddDynamic(this, &UMainMenuUI::OnEndButtonClicked);
	}

	if (LoadBtn)
	{
		LoadBtn->OnClicked.AddDynamic(this, &UMainMenuUI::OnLoadButtonClicked);
	}

	// WorldSettingMenuUI 위젯 초기화 (Hidden으로 설정)
	if (WorldSettingMenuUIWidget)
	{
		WorldSettingMenuUIWidget->SetVisibility(ESlateVisibility::Hidden);
		// 델리게이트 바인딩
		BindWorldSettingMenuUIDelegate();
	}

	// LoadMenuUI 위젯 초기화 (Hidden으로 설정)
	if (LoadMenuUIWidget)
	{
		LoadMenuUIWidget->SetVisibility(ESlateVisibility::Hidden);
		// 델리게이트 바인딩
		BindLoadMenuUIDelegate();
	}
}

void UMainMenuUI::OnPlayButtonClicked()
{
	// WorldSettingMenuUI 위젯 표시
	if (WorldSettingMenuUIWidget)
	{
		WorldSettingMenuUIWidget->SetVisibility(ESlateVisibility::Visible);
	}

	// MainMenuCanvas 숨기기
	if (MainMenuCanvas)
	{
		MainMenuCanvas->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMainMenuUI::OnEndButtonClicked()
{
	// 게임 종료
	if (GetWorld())
	{
		UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
	}
}

void UMainMenuUI::OnLoadButtonClicked()
{
	// LoadMenuUI 위젯 표시
	if (LoadMenuUIWidget)
	{
		LoadMenuUIWidget->SetVisibility(ESlateVisibility::Visible);
	}

	// MainMenuCanvas 숨기기
	if (MainMenuCanvas)
	{
		MainMenuCanvas->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMainMenuUI::SetWorldSettingMenuUI(UWorldSettingMenuUI* InWorldSettingMenuUI)
{
	WorldSettingMenuUIWidget = InWorldSettingMenuUI;

	// 델리게이트 바인딩
	if (WorldSettingMenuUIWidget)
	{
		BindWorldSettingMenuUIDelegate();
	}
}

void UMainMenuUI::BindWorldSettingMenuUIDelegate()
{
	if (WorldSettingMenuUIWidget)
	{
		// 기존 바인딩 제거 후 새로 바인딩
		WorldSettingMenuUIWidget->OnBackButtonClickedDelegate.RemoveDynamic(this, &UMainMenuUI::ShowMainMenu);
		WorldSettingMenuUIWidget->OnBackButtonClickedDelegate.AddDynamic(this, &UMainMenuUI::ShowMainMenu);
	}
}

void UMainMenuUI::ShowMainMenu()
{
	// WorldSettingMenuUI 위젯 숨기기
	if (WorldSettingMenuUIWidget)
	{
		WorldSettingMenuUIWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	// LoadMenuUI 위젯 숨기기
	if (LoadMenuUIWidget)
	{
		LoadMenuUIWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	// MainMenuCanvas 표시
	if (MainMenuCanvas)
	{
		MainMenuCanvas->SetVisibility(ESlateVisibility::Visible);
	}
}

void UMainMenuUI::SetLoadMenuUI(ULoadMenuUI* InLoadMenuUI)
{
	LoadMenuUIWidget = InLoadMenuUI;

	// 델리게이트 바인딩
	if (LoadMenuUIWidget)
	{
		BindLoadMenuUIDelegate();
	}
}

void UMainMenuUI::BindLoadMenuUIDelegate()
{
	if (LoadMenuUIWidget)
	{
		// 기존 바인딩 제거 후 새로 바인딩
		LoadMenuUIWidget->OnBackButtonClickedDelegate.RemoveDynamic(this, &UMainMenuUI::ShowMainMenu);
		LoadMenuUIWidget->OnBackButtonClickedDelegate.AddDynamic(this, &UMainMenuUI::ShowMainMenu);
	}
}
