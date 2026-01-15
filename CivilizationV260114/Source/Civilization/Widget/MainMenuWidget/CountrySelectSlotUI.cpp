// Fill out your copyright notice in the Description page of Project Settings.

#include "CountrySelectSlotUI.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UCountrySelectSlotUI::SetBorderColor(const FLinearColor& Color)
{
	if (ColorBrd)
	{
		ColorBrd->SetBrushColor(Color);
	}
}

void UCountrySelectSlotUI::SetCountryImage(UTexture2D* Texture)
{
	if (CountryImg && Texture)
	{
		CountryImg->SetBrushFromTexture(Texture);
	}
}

void UCountrySelectSlotUI::SetCountryText(const FString& Text)
{
	if (CountryTxt)
	{
		CountryTxt->SetText(FText::FromString(Text));
	}
}
