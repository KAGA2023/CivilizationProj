// Fill out your copyright notice in the Description page of Project Settings.

#include "CityProductionSlotUI.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Engine/Texture2D.h"

UCityProductionSlotUI::UCityProductionSlotUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UCityProductionSlotUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 클릭 이벤트 바인딩
	if (StartProductionBtn)
	{
		StartProductionBtn->OnClicked.AddDynamic(this, &UCityProductionSlotUI::OnStartProductionBtnClicked);
	}
}

void UCityProductionSlotUI::OnStartProductionBtnClicked()
{
	// 델리게이트 브로드캐스트
	OnProductionSlotClicked.Broadcast(ProductionID);
}

void UCityProductionSlotUI::SetProductionImage(UTexture2D* Texture)
{
	if (ProductionImg && Texture)
	{
		ProductionImg->SetBrushFromTexture(Texture);
	}
}

void UCityProductionSlotUI::SetProductionItemText(const FString& Text)
{
	if (ProductionItemTxt)
	{
		ProductionItemTxt->SetText(FText::FromString(Text));
	}
}

