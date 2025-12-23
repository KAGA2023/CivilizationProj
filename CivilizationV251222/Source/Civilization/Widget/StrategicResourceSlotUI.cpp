// Fill out your copyright notice in the Description page of Project Settings.

#include "StrategicResourceSlotUI.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UStrategicResourceSlotUI::NativeConstruct()
{
	Super::NativeConstruct();
}

void UStrategicResourceSlotUI::SetResourceIcon(UTexture2D* Texture)
{
	if (ResourceIconImg && Texture)
	{
		ResourceIconImg->SetBrushFromTexture(Texture);
	}
}

void UStrategicResourceSlotUI::SetResourceStockText(int32 Stock)
{
	if (ResourceStockTxt)
	{
		FString StockString = FString::Printf(TEXT("%d"), Stock);
		ResourceStockTxt->SetText(FText::FromString(StockString));
	}
}

