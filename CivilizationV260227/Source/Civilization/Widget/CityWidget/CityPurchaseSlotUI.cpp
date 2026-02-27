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
		GoldCostTxt->SetVisibility(ESlateVisibility::Visible);
	}
	if (TimeImage)
	{
		TimeImage->SetVisibility(ESlateVisibility::Visible);
	}
	if (ResourceGoldCostTxt) { ResourceGoldCostTxt->SetVisibility(ESlateVisibility::Collapsed); }
	if (ResourceTimeImage) { ResourceTimeImage->SetVisibility(ESlateVisibility::Collapsed); }
	if (ResourceStockTxt) { ResourceStockTxt->SetVisibility(ESlateVisibility::Collapsed); }
	if (ResourceIconImg) { ResourceIconImg->SetVisibility(ESlateVisibility::Collapsed); }
}

void UCityPurchaseSlotUI::SetStrategicResourceDisplay(int32 GoldCost, int32 RequiredAmount, UTexture2D* ResourceIconTexture)
{
	// 골드/시간 영역 숨김
	if (GoldCostTxt) { GoldCostTxt->SetVisibility(ESlateVisibility::Collapsed); }
	if (TimeImage) { TimeImage->SetVisibility(ESlateVisibility::Collapsed); }

	// 전략자원 영역 표시
	if (ResourceGoldCostTxt)
	{
		ResourceGoldCostTxt->SetVisibility(ESlateVisibility::Visible);
		if (GoldCost <= 0)
		{
			ResourceGoldCostTxt->SetText(FText::GetEmpty());
		}
		else
		{
			ResourceGoldCostTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), GoldCost)));
		}
	}
	if (ResourceTimeImage) { ResourceTimeImage->SetVisibility(ESlateVisibility::Visible); }
	if (ResourceStockTxt)
	{
		ResourceStockTxt->SetVisibility(ESlateVisibility::Visible);
		ResourceStockTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), RequiredAmount)));
	}
	if (ResourceIconImg)
	{
		ResourceIconImg->SetVisibility(ESlateVisibility::Visible);
		if (ResourceIconTexture)
		{
			ResourceIconImg->SetBrushFromTexture(ResourceIconTexture);
		}
	}
}

