// Fill out your copyright notice in the Description page of Project Settings.

#include "AIPlayerLoseUI.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "../../SuperGameInstance.h"
#include "../../SuperPlayerState.h"

void UAIPlayerLoseUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 계속하기 버튼 클릭 이벤트 바인딩
	if (ContinueBtn)
	{
		ContinueBtn->OnClicked.AddDynamic(this, &UAIPlayerLoseUI::OnContinueButtonClicked);
	}
}

void UAIPlayerLoseUI::SetupForDefeatedPlayer(int32 DefeatedPlayerIndex)
{
	if (!GetWorld())
	{
		return;
	}
	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}
	ASuperPlayerState* DefeatedPlayerState = SuperGameInst->GetPlayerState(DefeatedPlayerIndex);
	if (!DefeatedPlayerState)
	{
		return;
	}

	// KingImg: 해당 플레이어의 CountryKingImg
	if (KingImg && !DefeatedPlayerState->CountryKingImg.IsNull())
	{
		UTexture2D* KingTexture = DefeatedPlayerState->CountryKingImg.LoadSynchronous();
		if (KingTexture)
		{
			KingImg->SetBrushFromTexture(KingTexture);
		}
	}

	// CountryImg: 해당 플레이어의 CountryLargeImg
	if (CountryImg && !DefeatedPlayerState->CountryLargeImg.IsNull())
	{
		UTexture2D* CountryTexture = DefeatedPlayerState->CountryLargeImg.LoadSynchronous();
		if (CountryTexture)
		{
			CountryImg->SetBrushFromTexture(CountryTexture);
		}
	}

	// ResultTxt: "CountryName Lose" (예: "England Lose")
	if (ResultTxt)
	{
		FString ResultString = DefeatedPlayerState->CountryName + TEXT(" 敗北");
		ResultTxt->SetText(FText::FromString(ResultString));
	}
}

void UAIPlayerLoseUI::OnContinueButtonClicked()
{
	// 계속하기 버튼 클릭 델리게이트 브로드캐스트
	OnContinueButtonClickedDelegate.Broadcast();
}
