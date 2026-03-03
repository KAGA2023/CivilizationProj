// Fill out your copyright notice in the Description page of Project Settings.

#include "CityUnitSlotInfoUI.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "../../Status/UnitStatusStruct.h"

void UCityUnitSlotInfoUI::SetupFromUnitData(const FUnitBaseStat& Data)
{
	if (IconImg)
	{
		if (!Data.UnitIcon.IsNull())
		{
			UTexture2D* IconTexture = Data.UnitIcon.LoadSynchronous();
			if (IconTexture)
			{
				IconImg->SetBrushFromTexture(IconTexture);
			}
		}
		IconImg->SetVisibility(ESlateVisibility::Visible);
	}
	if (NameTxt)
	{
		NameTxt->SetText(Data.UnitName);
	}
	if (AttackTxt)
	{
		AttackTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), Data.AttackStrength)));
	}
	if (DefenseAttackTxt)
	{
		DefenseAttackTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), Data.DefenseStrength)));
	}
	if (HealthTxt)
	{
		HealthTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), Data.MaxHealth)));
	}
	if (MovementTxt)
	{
		MovementTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), Data.MovementPoints)));
	}
	if (RangeTxt)
	{
		RangeTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), Data.Range)));
	}
}
