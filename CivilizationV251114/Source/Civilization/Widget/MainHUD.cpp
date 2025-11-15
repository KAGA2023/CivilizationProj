// Fill out your copyright notice in the Description page of Project Settings.

#include "MainHUD.h"
#include "Components/TextBlock.h"
#include "../SuperGameInstance.h"
#include "../SuperPlayerState.h"
#include "../World/WorldComponent.h"
#include "../SuperGameModeBase.h"
#include "../Turn/TurnComponent.h"
#include "../Turn/TurnStruct.h"
#include "../World/WorldTileActor.h"
#include "Kismet/GameplayStatics.h"

void UMainHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기 텍스트 설정
	if (FaithTxt)
	{
		FaithTxt->SetText(FText::FromString(TEXT("0")));
	}

	if (GoldTxt)
	{
		GoldTxt->SetText(FText::FromString(TEXT("0")));
	}

	if (PopulationTxt)
	{
		PopulationTxt->SetText(FText::FromString(TEXT("0/0")));
	}

	if (ScienceTxt)
	{
		ScienceTxt->SetText(FText::FromString(TEXT("+0")));
	}

	// GetGameMode -> CastToSuperGameModeBase -> GetTurnComponent의 OnTurnChanged 델리게이트 바인딩
	if (GetWorld())
	{
		if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
		{
			if (ASuperGameModeBase* SuperGameMode = Cast<ASuperGameModeBase>(GameMode))
			{
				if (UTurnComponent* TurnComponent = SuperGameMode->GetTurnComponent())
				{
					TurnComponent->OnTurnChanged.AddDynamic(this, &UMainHUD::OnTurnChanged);
				}
			}
		}
	}

	// 0.5초 후 모든 WorldTileActor의 OnPlayerCityTileClicked 델리게이트 바인딩
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(BindCityTileTimerHandle, this, &UMainHUD::BindCityTileClickedDelegates, 0.5f, false);
	}

	// PlayerState의 골드/인구 변경 델리게이트 바인딩
	BindPlayerStateDelegates();
}

void UMainHUD::UpdateHUDData()
{
	// GetGameInstance -> CastToSuperGameInstance -> GetPlayerState(0)
	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0);
	if (!PlayerState)
	{
		return;
	}

	// Gold를 GoldTxt에 표시
	if (GoldTxt)
	{
		FString GoldString = FString::Printf(TEXT("%d"), PlayerState->Gold);
		GoldTxt->SetText(FText::FromString(GoldString));
	}

	// Faith를 FaithTxt에 표시
	if (FaithTxt)
	{
		FString FaithString = FString::Printf(TEXT("%d"), PlayerState->Faith);
		FaithTxt->SetText(FText::FromString(FaithString));
	}

	// CalculateTotalScienceYield를 +x 형식으로 ScienceTxt에 표시
	if (ScienceTxt)
	{
		UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent();
		if (WorldComponent)
		{
			int32 ScienceYield = PlayerState->CalculateTotalScienceYield(WorldComponent);
			FString ScienceString = FString::Printf(TEXT("+%d"), ScienceYield);
			ScienceTxt->SetText(FText::FromString(ScienceString));
		}
		else
		{
			ScienceTxt->SetText(FText::FromString(TEXT("+0")));
		}
	}

	// GetPopulation()과 GetLimitPopulation()을 x/y 형식으로 PopulationTxt에 표시
	if (PopulationTxt)
	{
		int32 Population = PlayerState->GetPopulation();
		int32 LimitPopulation = PlayerState->GetLimitPopulation();
		FString PopulationString = FString::Printf(TEXT("%d/%d"), Population, LimitPopulation);
		PopulationTxt->SetText(FText::FromString(PopulationString));
	}
}

void UMainHUD::OnTurnChanged(FTurnStruct NewTurn)
{
	// 턴이 변경될 때마다 HUD 데이터 업데이트
	UpdateHUDData();
}

void UMainHUD::OnPlayerCityTileClicked()
{
	// 도시 타일 클릭 시 OnCityTileClicked 델리게이트 브로드캐스트
	OnCityTileClicked.Broadcast();
}

void UMainHUD::BindCityTileClickedDelegates()
{
	if (!GetWorld())
	{
		return;
	}

	// 모든 WorldTileActor 찾기
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWorldTileActor::StaticClass(), FoundActors);

	// 각 WorldTileActor의 OnPlayerCityTileClicked 델리게이트 바인딩
	for (AActor* Actor : FoundActors)
	{
		if (AWorldTileActor* TileActor = Cast<AWorldTileActor>(Actor))
		{
			TileActor->OnPlayerCityTileClicked.AddDynamic(this, &UMainHUD::OnPlayerCityTileClicked);
		}
	}
}

void UMainHUD::BindPlayerStateDelegates()
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

	ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0);
	if (!PlayerState)
	{
		return;
	}

	// 기존 바인딩 해제
	PlayerState->OnGoldChanged.RemoveDynamic(this, &UMainHUD::OnGoldChanged);
	PlayerState->OnPopulationChanged.RemoveDynamic(this, &UMainHUD::OnPopulationChanged);

	// 새로운 바인딩
	PlayerState->OnGoldChanged.AddDynamic(this, &UMainHUD::OnGoldChanged);
	PlayerState->OnPopulationChanged.AddDynamic(this, &UMainHUD::OnPopulationChanged);
}

void UMainHUD::OnGoldChanged(int32 NewGold)
{
	// 골드 변경 시 HUD 업데이트
	UpdateHUDData();
}

void UMainHUD::OnPopulationChanged(int32 NewPopulation)
{
	// 인구 변경 시 HUD 업데이트
	UpdateHUDData();
}

void UMainHUD::NativeDestruct()
{
	// 타이머 해제
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(BindCityTileTimerHandle);
	}

	// 델리게이트 바인딩 해제
	if (GetWorld())
	{
		// TurnComponent 델리게이트 바인딩 해제
		if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
		{
			if (ASuperGameModeBase* SuperGameMode = Cast<ASuperGameModeBase>(GameMode))
			{
				if (UTurnComponent* TurnComponent = SuperGameMode->GetTurnComponent())
				{
					TurnComponent->OnTurnChanged.RemoveDynamic(this, &UMainHUD::OnTurnChanged);
				}
			}
		}

		// 모든 WorldTileActor의 OnPlayerCityTileClicked 델리게이트 바인딩 해제
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWorldTileActor::StaticClass(), FoundActors);

		for (AActor* Actor : FoundActors)
		{
			if (AWorldTileActor* TileActor = Cast<AWorldTileActor>(Actor))
			{
				TileActor->OnPlayerCityTileClicked.RemoveDynamic(this, &UMainHUD::OnPlayerCityTileClicked);
			}
		}

		// PlayerState 델리게이트 바인딩 해제
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance()))
		{
			if (ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0))
			{
				PlayerState->OnGoldChanged.RemoveDynamic(this, &UMainHUD::OnGoldChanged);
				PlayerState->OnPopulationChanged.RemoveDynamic(this, &UMainHUD::OnPopulationChanged);
			}
		}
	}

	Super::NativeDestruct();
}

