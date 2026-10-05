// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerLoseUI.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "../../SuperGameInstance.h"
#include "../../SuperPlayerState.h"

void UPlayerLoseUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 이 기기의 슬롯 이미지 세팅 (CountryKingImg -> KingImg, CountryLargeImg -> CountryImg)
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			if (ASuperPlayerState* LocalPlayerState = SuperGameInst->GetLocalPlayerState())
			{
				if (KingImg && !LocalPlayerState->CountryKingImg.IsNull())
				{
					UTexture2D* KingTexture = LocalPlayerState->CountryKingImg.LoadSynchronous();
					if (KingTexture)
					{
						KingImg->SetBrushFromTexture(KingTexture);
					}
				}
				if (CountryImg && !LocalPlayerState->CountryLargeImg.IsNull())
				{
					UTexture2D* CountryTexture = LocalPlayerState->CountryLargeImg.LoadSynchronous();
					if (CountryTexture)
					{
						CountryImg->SetBrushFromTexture(CountryTexture);
					}
				}
			}
		}
	}

	// 타이틀 메뉴 버튼 클릭 이벤트 바인딩
	if (TitleMenuBtn)
	{
		TitleMenuBtn->OnClicked.AddDynamic(this, &UPlayerLoseUI::OnTitleMenuButtonClicked);
	}
}

void UPlayerLoseUI::OnTitleMenuButtonClicked()
{
	// 타이틀 메뉴 레벨로 이동
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::OpenLevel(World, TEXT("MainMenu"));
	}
}
