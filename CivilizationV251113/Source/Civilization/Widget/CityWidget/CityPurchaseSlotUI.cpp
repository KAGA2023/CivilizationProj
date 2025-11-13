// Fill out your copyright notice in the Description page of Project Settings.

#include "CityPurchaseSlotUI.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Engine/Texture2D.h"

UCityPurchaseSlotUI::UCityPurchaseSlotUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UCityPurchaseSlotUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 클릭 이벤트 바인딩
	if (PurchaseBtn)
	{
		PurchaseBtn->OnClicked.AddDynamic(this, &UCityPurchaseSlotUI::OnPurchaseBtnClicked);
	}
}

void UCityPurchaseSlotUI::OnPurchaseBtnClicked()
{
	// 델리게이트 브로드캐스트
	OnPurchaseSlotClicked.Broadcast(PurchaseID);
}

void UCityPurchaseSlotUI::SetPurchaseImage(UTexture2D* Texture)
{
	if (PurchaseImg && Texture)
	{
		PurchaseImg->SetBrushFromTexture(Texture);
	}
}

void UCityPurchaseSlotUI::SetPurchaseItemText(const FString& Text)
{
	if (PurchaseItemTxt)
	{
		PurchaseItemTxt->SetText(FText::FromString(Text));
	}
}

void UCityPurchaseSlotUI::SetGoldCostText(int32 GoldCost)
{
	if (GoldCostTxt)
	{
		if (GoldCost <= 0)
		{
			GoldCostTxt->SetText(FText::GetEmpty());
		}
		else
		{
			FString GoldCostString = FString::Printf(TEXT("%d"), GoldCost);
			GoldCostTxt->SetText(FText::FromString(GoldCostString));
		}
	}
}

