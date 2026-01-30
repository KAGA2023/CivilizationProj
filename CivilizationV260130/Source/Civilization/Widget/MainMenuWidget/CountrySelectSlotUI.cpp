// Fill out your copyright notice in the Description page of Project Settings.

#include "CountrySelectSlotUI.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Engine/Texture2D.h"

void UCountrySelectSlotUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 클릭 이벤트 바인딩
	if (SelectBtn)
	{
		SelectBtn->OnClicked.AddDynamic(this, &UCountrySelectSlotUI::OnSelectBtnClicked);
	}
}

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

void UCountrySelectSlotUI::SetRowName(FName InRowName)
{
	RowName = InRowName;
}

void UCountrySelectSlotUI::OnSelectBtnClicked()
{
	// 델리게이트 브로드캐스트
	OnCountrySelectSlotClicked.Broadcast(this);
}
