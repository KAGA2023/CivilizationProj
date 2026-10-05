// Fill out your copyright notice in the Description page of Project Settings.

#include "CityBuildingSlotInfoUI.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "../../City/CityStruct.h"

void UCityBuildingSlotInfoUI::SetupFromBuildingData(const FBuildingData& Data)
{
	if (IconImg)
	{
		if (!Data.BuildingIcon.IsNull())
		{
			UTexture2D* IconTexture = Data.BuildingIcon.LoadSynchronous();
			if (IconTexture)
			{
				IconImg->SetBrushFromTexture(IconTexture);
			}
		}
		IconImg->SetVisibility(ESlateVisibility::Visible);
	}
	if (NameTxt)
	{
		NameTxt->SetText(FText::FromString(Data.BuildingName));
	}
	if (ScienceTxt)
	{
		ScienceTxt->SetText(FText::FromString(FString::Printf(TEXT("+%d"), Data.ScienceYield)));
	}
	if (GoldTxt)
	{
		GoldTxt->SetText(FText::FromString(FString::Printf(TEXT("+%d"), Data.GoldYield)));
	}
	if (FoodTxt)
	{
		FoodTxt->SetText(FText::FromString(FString::Printf(TEXT("+%d"), Data.FoodYield)));
	}
	if (ProductionTxt)
	{
		ProductionTxt->SetText(FText::FromString(FString::Printf(TEXT("+%d"), Data.ProductionYield)));
	}
	if (HealthTxt)
	{
		HealthTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), Data.MaxHealth)));
	}
}
