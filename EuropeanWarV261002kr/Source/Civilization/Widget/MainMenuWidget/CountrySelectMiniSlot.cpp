// Fill out your copyright notice in the Description page of Project Settings.

#include "CountrySelectMiniSlot.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Engine/Texture2D.h"

void UCountrySelectMiniSlot::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 클릭 이벤트 바인딩
	if (CountryBtn)
	{
		CountryBtn->OnClicked.AddDynamic(this, &UCountrySelectMiniSlot::OnCountryBtnClicked);
		CountryBtn->OnHovered.AddDynamic(this, &UCountrySelectMiniSlot::OnCountryBtnHovered);
		CountryBtn->OnUnhovered.AddDynamic(this, &UCountrySelectMiniSlot::OnCountryBtnUnhovered);
	}
}

void UCountrySelectMiniSlot::SetCountryImage(UTexture2D* Texture)
{
	if (CountryImg && Texture)
	{
		CountryImg->SetBrushFromTexture(Texture);
	}
}

void UCountrySelectMiniSlot::SetRowName(FName InRowName)
{
	RowName = InRowName;
}

void UCountrySelectMiniSlot::OnCountryBtnClicked()
{
	// 델리게이트 브로드캐스트
	OnCountrySelectMiniSlotClicked.Broadcast(this);
}

void UCountrySelectMiniSlot::OnCountryBtnHovered()
{
	// 호버 델리게이트 브로드캐스트
	OnCountrySelectMiniSlotHovered.Broadcast(this);
}

void UCountrySelectMiniSlot::OnCountryBtnUnhovered()
{
	// 언호버 델리게이트 브로드캐스트
	OnCountrySelectMiniSlotUnhovered.Broadcast();
}
