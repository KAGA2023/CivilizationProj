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

void UCityProductionSlotUI::SetTurnText(int32 Turns)
{
	if (TurnTxt)
	{
		if (Turns <= 0)
		{
			TurnTxt->SetText(FText::GetEmpty());
		}
		else
		{
			FString TurnString = FString::Printf(TEXT("%d"), Turns);
			TurnTxt->SetText(FText::FromString(TurnString));
		}
		TurnTxt->SetVisibility(ESlateVisibility::Visible);
	}
	if (TimeImage)
	{
		TimeImage->SetVisibility(ESlateVisibility::Visible);
	}
	if (ResourceTurnTxt) { ResourceTurnTxt->SetVisibility(ESlateVisibility::Collapsed); }
	if (ResourceTimeImage) { ResourceTimeImage->SetVisibility(ESlateVisibility::Collapsed); }
	if (ResourceStockTxt) { ResourceStockTxt->SetVisibility(ESlateVisibility::Collapsed); }
	if (ResourceIconImg) { ResourceIconImg->SetVisibility(ESlateVisibility::Collapsed); }
}

void UCityProductionSlotUI::SetStrategicResourceDisplay(int32 Turns, int32 RequiredAmount, UTexture2D* ResourceIconTexture)
{
	// 턴 표시 영역 숨김
	if (TurnTxt) { TurnTxt->SetVisibility(ESlateVisibility::Collapsed); }
	if (TimeImage) { TimeImage->SetVisibility(ESlateVisibility::Collapsed); }

	// 전략자원 영역 표시
	if (ResourceTurnTxt)
	{
		ResourceTurnTxt->SetVisibility(ESlateVisibility::Visible);
		if (Turns <= 0)
		{
			ResourceTurnTxt->SetText(FText::GetEmpty());
		}
		else
		{
			ResourceTurnTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), Turns)));
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

