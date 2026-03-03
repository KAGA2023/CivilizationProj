// Fill out your copyright notice in the Description page of Project Settings.

#include "CreditUI.h"
#include "Components/Button.h"

void UCreditUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackBtn)
	{
		BackBtn->OnClicked.AddDynamic(this, &UCreditUI::OnBackButtonClicked);
	}
}

void UCreditUI::OnBackButtonClicked()
{
	OnBackButtonClickedDelegate.Broadcast();
}
