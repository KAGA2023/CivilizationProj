// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerMenuUI.h"
#include "CreateSessionUI.h"
#include "FindSessionUI.h"
#include "Components/Button.h"

void UMultiplayerMenuUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackBtn)
	{
		BackBtn->OnClicked.AddDynamic(this, &UMultiplayerMenuUI::OnBackButtonClicked);
	}
}

void UMultiplayerMenuUI::OnBackButtonClicked()
{
	OnBackButtonClickedDelegate.Broadcast();
}
