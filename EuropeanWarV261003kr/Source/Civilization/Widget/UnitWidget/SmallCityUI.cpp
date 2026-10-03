// Fill out your copyright notice in the Description page of Project Settings.

#include "SmallCityUI.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Engine/Texture2D.h"

void USmallCityUI::SetHPBar(float HealthPercent)
{
	if (HPBar)
	{
		float ClampedPercent = FMath::Clamp(HealthPercent, 0.0f, 1.0f);
		HPBar->SetPercent(ClampedPercent);
	}
}

void USmallCityUI::SetCountryNameTxt(const FString& CountryName)
{
	if (CountryNameTxt)
	{
		CountryNameTxt->SetText(FText::FromString(CountryName));
	}
}

void USmallCityUI::SetCountryImg(UTexture2D* CountryTexture)
{
	if (CountryImg)
	{
		if (CountryTexture)
		{
			CountryImg->SetBrushFromTexture(CountryTexture);
		}
		else
		{
			CountryImg->SetBrushFromTexture(nullptr);
		}
	}
}
