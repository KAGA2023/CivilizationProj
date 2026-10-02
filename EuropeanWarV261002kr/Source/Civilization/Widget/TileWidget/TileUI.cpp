// Fill out your copyright notice in the Description page of Project Settings.

#include "TileUI.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UTileUI::NativeConstruct()
{
	Super::NativeConstruct();

	// PurchaseTileImg는 기본적으로 Visible
	if (PurchaseTileImg)
	{
		PurchaseTileImg->SetVisibility(ESlateVisibility::Visible);
	}

	// SelectPurchaseTileImg는 기본적으로 Hidden
	if (SelectPurchaseTileImg)
	{
		SelectPurchaseTileImg->SetVisibility(ESlateVisibility::Hidden);
	}

	// TileCostTxt 초기화
	if (TileCostTxt)
	{
		TileCostTxt->SetText(FText::GetEmpty());
	}
}

void UTileUI::UpdateTileCost(int32 Cost)
{
	if (TileCostTxt)
	{
		FString CostString = FString::Printf(TEXT("%d"), Cost);
		TileCostTxt->SetText(FText::FromString(CostString));
	}
}

void UTileUI::SetHovered(bool bHovered)
{
	if (bHovered)
	{
		// 호버 시: PurchaseTileImg 숨김, SelectPurchaseTileImg 표시
		if (PurchaseTileImg)
		{
			PurchaseTileImg->SetVisibility(ESlateVisibility::Hidden);
		}
		if (SelectPurchaseTileImg)
		{
			SelectPurchaseTileImg->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		// 호버 종료 시: PurchaseTileImg 표시, SelectPurchaseTileImg 숨김
		if (PurchaseTileImg)
		{
			PurchaseTileImg->SetVisibility(ESlateVisibility::Visible);
		}
		if (SelectPurchaseTileImg)
		{
			SelectPurchaseTileImg->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

