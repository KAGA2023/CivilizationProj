// Fill out your copyright notice in the Description page of Project Settings.

#include "SmallUnitUI.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"

void USmallUnitUI::NativeConstruct()
{
	Super::NativeConstruct();
}

void USmallUnitUI::SetHPBar(float HealthPercent)
{
	if (HPBar)
	{
		// 0.0 ~ 1.0 범위로 클램프
		float ClampedPercent = FMath::Clamp(HealthPercent, 0.0f, 1.0f);
		HPBar->SetPercent(ClampedPercent);
	}
}

void USmallUnitUI::SetUnitImg(UTexture2D* UnitTexture)
{
	if (UnitImg)
	{
		if (UnitTexture)
		{
			UnitImg->SetBrushFromTexture(UnitTexture);
		}
		else
		{
			// 텍스처가 없으면 null로 설정
			UnitImg->SetBrushFromTexture(nullptr);
		}
	}
}

void USmallUnitUI::SetCountryImg(UTexture2D* CountryTexture)
{
	if (CountryImg)
	{
		if (CountryTexture)
		{
			CountryImg->SetBrushFromTexture(CountryTexture);
		}
		else
		{
			// 텍스처가 없으면 null로 설정
			CountryImg->SetBrushFromTexture(nullptr);
		}
	}
}
