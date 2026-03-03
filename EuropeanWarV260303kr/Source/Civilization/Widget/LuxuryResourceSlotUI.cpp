// Fill out your copyright notice in the Description page of Project Settings.

#include "LuxuryResourceSlotUI.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void ULuxuryResourceSlotUI::NativeConstruct()
{
	Super::NativeConstruct();
}

void ULuxuryResourceSlotUI::SetResourceIcon(UTexture2D* Texture)
{
	if (ResourceIconImg && Texture)
	{
		ResourceIconImg->SetBrushFromTexture(Texture);
	}
}
