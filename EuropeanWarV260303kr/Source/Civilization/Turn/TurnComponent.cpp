// Fill out your copyright notice in the Description page of Project Settings.

#include "TurnComponent.h"
#include "../SuperGameInstance.h"
#include "Kismet/GameplayStatics.h"

UTurnComponent::UTurnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// CurrentTurn은 구조체의 기본 생성자에 의해 자동으로 (Round 1, Turn 1, Player 0)로 초기화됨
}

void UTurnComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTurnComponent::NextTurn()
{
	int32 PreviousRoundNumber = CurrentTurn.RoundNumber;

	// GameInstance에서 총 플레이어 수 가져오기
	int32 TotalPlayerCount = 4; // 기본값
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(UGameplayStatics::GetGameInstance(World)))
		{
			TotalPlayerCount = GameInstance->GetPlayerStateCount();
		}
	}

	// 현재 턴이 마지막 턴보다 작으면 다음 턴으로
	if (CurrentTurn.TurnNumber < TotalPlayerCount)
	{
		CurrentTurn.TurnNumber++;
	}
	// 현재 턴이 마지막 턴이면 다음 라운드로 이동
	else
	{
		CurrentTurn.RoundNumber++;
		CurrentTurn.TurnNumber = 1;
	}

	// PlayerIndex 업데이트 (TurnNumber - 1)
	UpdatePlayerIndex();

	// 이벤트 브로드캐스트
	OnTurnChanged.Broadcast(CurrentTurn);

	// 라운드가 변경되었으면 라운드 변경 이벤트도 브로드캐스트
	if (PreviousRoundNumber != CurrentTurn.RoundNumber)
	{
		OnRoundChanged.Broadcast(CurrentTurn);
	}
}

void UTurnComponent::InitializeTurn(int32 StartRound)
{
	CurrentTurn.RoundNumber = FMath::Max(StartRound, 1);
	CurrentTurn.TurnNumber = 1;
	UpdatePlayerIndex();
}

void UTurnComponent::NextRound()
{
	// 다음 라운드로 이동
	CurrentTurn.RoundNumber++;
	CurrentTurn.TurnNumber = 1;
	
	// PlayerIndex 업데이트 (TurnNumber - 1)
	UpdatePlayerIndex();

	// 이벤트 브로드캐스트
	OnTurnChanged.Broadcast(CurrentTurn);
	OnRoundChanged.Broadcast(CurrentTurn);
}

bool UTurnComponent::IsValid() const
{
	// RoundNumber는 1 이상이어야 함
	if (CurrentTurn.RoundNumber < 1)
	{
		return false;
	}

	// GameInstance에서 총 플레이어 수 가져오기
	int32 TotalPlayerCount = 4; // 기본값
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(UGameplayStatics::GetGameInstance(World)))
		{
			TotalPlayerCount = GameInstance->GetPlayerStateCount();
		}
	}

	// TurnNumber는 1~TotalPlayerCount 사이여야 함
	if (CurrentTurn.TurnNumber < 1 || CurrentTurn.TurnNumber > TotalPlayerCount)
	{
		return false;
	}

	// PlayerIndex는 0~(TotalPlayerCount-1) 사이여야 함
	if (CurrentTurn.PlayerIndex < 0 || CurrentTurn.PlayerIndex >= TotalPlayerCount)
	{
		return false;
	}

	// PlayerIndex가 TurnNumber - 1과 일치하는지 확인
	if (CurrentTurn.PlayerIndex != (CurrentTurn.TurnNumber - 1))
	{
		return false;
	}

	return true;
}

void UTurnComponent::UpdatePlayerIndex()
{
	// TurnNumber에 따라 PlayerIndex 설정
	// TurnNumber 1 = PlayerIndex 0 (Player 0)
	// TurnNumber 2 = PlayerIndex 1 (AI 1)
	// TurnNumber N = PlayerIndex N-1 (AI N-1)
	CurrentTurn.PlayerIndex = CurrentTurn.TurnNumber - 1;
}

