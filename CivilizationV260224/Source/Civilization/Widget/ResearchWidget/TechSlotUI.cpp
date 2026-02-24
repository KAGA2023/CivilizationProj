// Fill out your copyright notice in the Description page of Project Settings.

#include "TechSlotUI.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Engine/Texture2D.h"

UTechSlotUI::UTechSlotUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UTechSlotUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 클릭 이벤트 바인딩
	if (StartDevelopingBtn)
	{
		StartDevelopingBtn->OnClicked.AddDynamic(this, &UTechSlotUI::OnStartDevelopingBtnClicked);
	}
}

void UTechSlotUI::OnStartDevelopingBtnClicked()
{
	// 델리게이트 브로드캐스트
	OnTechSlotClicked.Broadcast(TechRowName);
}

void UTechSlotUI::SetTechImage(UTexture2D* Texture)
{
	if (TechImg && Texture)
	{
		TechImg->SetBrushFromTexture(Texture);
	}
}

void UTechSlotUI::SetTechText(const FString& Text)
{
	if (TechTxt)
	{
		TechTxt->SetText(FText::FromString(Text));
	}
}

void UTechSlotUI::SetTurnText(int32 Turns)
{
	if (TurnTxt)
	{
		if (Turns <= 0)
		{
			TurnTxt->SetText(FText::GetEmpty());
		}
		else
		{
			FString TurnString = FString::Printf(TEXT("%d"), Turns);
			TurnTxt->SetText(FText::FromString(TurnString));
		}
	}
}

