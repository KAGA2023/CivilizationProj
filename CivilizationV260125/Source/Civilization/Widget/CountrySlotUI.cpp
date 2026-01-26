// Fill out your copyright notice in the Description page of Project Settings.

#include "CountrySlotUI.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "../SuperPlayerState.h"
#include "Engine/Texture2D.h"

void UCountrySlotUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 클릭 이벤트 바인딩
	if (CountryBtn)
	{
		CountryBtn->OnClicked.AddDynamic(this, &UCountrySlotUI::OnCountryBtnClicked);
	}
}

void UCountrySlotUI::SetupForPlayer(int32 PlayerIndex, ASuperPlayerState* PlayerState)
{
	if (!PlayerState)
	{
		return;
	}

	// 플레이어 인덱스 저장
	TargetPlayerIndex = PlayerIndex;

	// CountryImg 설정 (작은 이미지 사용)
	if (CountryImg)
	{
		UTexture2D* Texture = PlayerState->GetCountrySmallImg();
		if (Texture)
		{
			CountryImg->SetBrushFromTexture(Texture);
		}
	}
}

void UCountrySlotUI::OnCountryBtnClicked()
{
	// 델리게이트 브로드캐스트
	OnCountrySlotClicked.Broadcast(TargetPlayerIndex);
}

