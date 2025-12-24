// Fill out your copyright notice in the Description page of Project Settings.

#include "AIPlayerManager.h"
#include "../SuperPlayerState.h"
#include "../SuperGameInstance.h"
#include "../SuperGameModeBase.h"
#include "../Turn/TurnComponent.h"
#include "Engine/World.h"

UAIPlayerManager::UAIPlayerManager()
{
	// 생성자 초기화
	AIPlayers.Empty();
}

void UAIPlayerManager::Initialize()
{
	// GameInstance에서 모든 AI 플레이어 등록
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			// AI 플레이어는 PlayerIndex 1~3
			for (int32 PlayerIndex = 1; PlayerIndex <= 3; PlayerIndex++)
			{
				if (ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(PlayerIndex))
				{
					RegisterAIPlayer(PlayerIndex, PlayerState);
				}
			}
		}
	}
}

void UAIPlayerManager::RegisterAIPlayer(int32 PlayerIndex, ASuperPlayerState* PlayerState)
{
	// 유효성 체크
	if (PlayerIndex < 1 || PlayerIndex > 3)
	{
		return; // AI 플레이어는 1~3만 가능
	}

	if (!PlayerState)
	{
		return;
	}

	// AI 플레이어 구조체 생성 및 설정
	FAIPlayerStruct AIPlayer;
	AIPlayer.PlayerIndex = PlayerIndex;
	AIPlayer.PlayerStateRef = PlayerState;
	AIPlayer.CurrentState = EAITurnState::Idle;
	AIPlayer.bIsTurnActive = false;
	AIPlayer.CurrentTurnRound = 0;
	AIPlayer.PendingUnitMovements = 0;
	AIPlayer.PendingCombatActions = 0;

	// 맵에 추가 (기존 것이 있으면 교체)
	AIPlayers.Add(PlayerIndex, AIPlayer);
}

FAIPlayerStruct UAIPlayerManager::GetAIPlayer(int32 PlayerIndex) const
{
	if (const FAIPlayerStruct* Found = AIPlayers.Find(PlayerIndex))
	{
		return *Found;
	}
	// 없으면 기본값 반환
	return FAIPlayerStruct();
}

FAIPlayerStruct* UAIPlayerManager::GetAIPlayerPtr(int32 PlayerIndex)
{
	if (FAIPlayerStruct* Found = AIPlayers.Find(PlayerIndex))
	{
		return Found;
	}
	return nullptr;
}

bool UAIPlayerManager::IsAIPlayerValid(int32 PlayerIndex) const
{
	if (const FAIPlayerStruct* AIPlayer = AIPlayers.Find(PlayerIndex))
	{
		// PlayerStateRef 유효성 확인
		return AIPlayer->PlayerStateRef.IsValid();
	}
	return false;
}

void UAIPlayerManager::StartAITurn(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer)
	{
		return;
	}

	// 현재 라운드 정보 가져오기
	int32 CurrentRound = 1;
	if (UWorld* World = GetWorld())
	{
		if (AGameModeBase* GameMode = World->GetAuthGameMode())
		{
			if (ASuperGameModeBase* SuperGameMode = Cast<ASuperGameModeBase>(GameMode))
			{
				if (UTurnComponent* TurnComponent = SuperGameMode->GetTurnComponent())
				{
					CurrentRound = TurnComponent->GetCurrentRoundNumber();
				}
			}
		}
	}

	// 새 턴 초기화
	ResetAIPlayerForNewTurn(*AIPlayer);
	AIPlayer->CurrentTurnRound = CurrentRound;
}

bool UAIPlayerManager::IsAITurnComplete(int32 PlayerIndex) const
{
	if (const FAIPlayerStruct* AIPlayer = AIPlayers.Find(PlayerIndex))
	{
		return AIPlayer->CurrentState == EAITurnState::TurnComplete;
	}
	return false;
}

void UAIPlayerManager::EndAITurn(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer)
	{
		return;
	}

	// 턴 종료 처리
	AIPlayer->bIsTurnActive = false;
	AIPlayer->CurrentState = EAITurnState::Idle;
}

void UAIPlayerManager::ResetAIPlayerForNewTurn(FAIPlayerStruct& AIPlayer)
{
	AIPlayer.CurrentState = EAITurnState::Idle;
	AIPlayer.bIsTurnActive = true;
	AIPlayer.PendingUnitMovements = 0;
	AIPlayer.PendingCombatActions = 0;
}

void UAIPlayerManager::UpdateStateMachine(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer || !AIPlayer->bIsTurnActive)
	{
		return; // AI 플레이어가 없거나 턴이 활성화되지 않음
	}

	// 현재 상태에 따라 처리 함수 호출
	switch (AIPlayer->CurrentState)
	{
		case EAITurnState::Idle:
			// Idle 상태에서는 첫 번째 상태로 전환
			TransitionToNextState(PlayerIndex);
			break;

		case EAITurnState::ProcessingDiplomacy:
			ProcessDiplomacyState(PlayerIndex);
			break;

		case EAITurnState::ProcessingResearch:
			ProcessResearchState(PlayerIndex);
			break;

		case EAITurnState::ProcessingCityProduction:
			ProcessCityProductionState(PlayerIndex);
			break;

		case EAITurnState::ProcessingTilePurchase:
			ProcessTilePurchaseState(PlayerIndex);
			break;

		case EAITurnState::ProcessingFacility:
			ProcessFacilityState(PlayerIndex);
			break;

		case EAITurnState::ProcessingUnitMovement:
			ProcessUnitMovementState(PlayerIndex);
			break;

		case EAITurnState::WaitingForAsync:
			// 비동기 작업 대기 중
			// 모든 비동기 작업이 완료되면 다음 상태로 전환
			if (!HasPendingAsyncWork(PlayerIndex))
			{
				TransitionToNextState(PlayerIndex);
			}
			break;

		case EAITurnState::TurnComplete:
			// 턴 완료 상태에서는 아무것도 하지 않음
			break;

		default:
			break;
	}
}

void UAIPlayerManager::TransitionToNextState(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer)
	{
		return;
	}

	// 상태 전환 순서에 따라 다음 상태로 이동
	switch (AIPlayer->CurrentState)
	{
		case EAITurnState::Idle:
			AIPlayer->CurrentState = EAITurnState::ProcessingDiplomacy;
			break;

		case EAITurnState::ProcessingDiplomacy:
			AIPlayer->CurrentState = EAITurnState::ProcessingResearch;
			break;

		case EAITurnState::ProcessingResearch:
			AIPlayer->CurrentState = EAITurnState::ProcessingCityProduction;
			break;

		case EAITurnState::ProcessingCityProduction:
			AIPlayer->CurrentState = EAITurnState::ProcessingTilePurchase;
			break;

		case EAITurnState::ProcessingTilePurchase:
			AIPlayer->CurrentState = EAITurnState::ProcessingFacility;
			break;

		case EAITurnState::ProcessingFacility:
			AIPlayer->CurrentState = EAITurnState::ProcessingUnitMovement;
			break;

		case EAITurnState::ProcessingUnitMovement:
			// 비동기 작업이 있으면 대기 상태로, 없으면 턴 완료
			if (HasPendingAsyncWork(PlayerIndex))
			{
				AIPlayer->CurrentState = EAITurnState::WaitingForAsync;
			}
			else
			{
				AIPlayer->CurrentState = EAITurnState::TurnComplete;
			}
			break;

		case EAITurnState::WaitingForAsync:
			// 비동기 작업 완료 후 턴 완료
			AIPlayer->CurrentState = EAITurnState::TurnComplete;
			break;

		default:
			break;
	}
}

bool UAIPlayerManager::HasPendingAsyncWork(int32 PlayerIndex) const
{
	if (const FAIPlayerStruct* AIPlayer = AIPlayers.Find(PlayerIndex))
	{
		return AIPlayer->PendingUnitMovements > 0 || AIPlayer->PendingCombatActions > 0;
	}
	return false;
}

// ================= 상태별 처리 함수들 (5단계에서 구현) =================

void UAIPlayerManager::ProcessDiplomacyState(int32 PlayerIndex)
{
	// TODO: 5단계에서 구현
	// 외교 결정 후 즉시 다음 상태로 전환
	TransitionToNextState(PlayerIndex);
}

void UAIPlayerManager::ProcessResearchState(int32 PlayerIndex)
{
	// TODO: 5단계에서 구현
	// 연구 선택 후 즉시 다음 상태로 전환
	TransitionToNextState(PlayerIndex);
}

void UAIPlayerManager::ProcessCityProductionState(int32 PlayerIndex)
{
	// TODO: 5단계에서 구현
	// 도시 생산 결정 후 즉시 다음 상태로 전환
	TransitionToNextState(PlayerIndex);
}

void UAIPlayerManager::ProcessTilePurchaseState(int32 PlayerIndex)
{
	// TODO: 5단계에서 구현
	// 타일 구매 결정 후 즉시 다음 상태로 전환
	TransitionToNextState(PlayerIndex);
}

void UAIPlayerManager::ProcessFacilityState(int32 PlayerIndex)
{
	// TODO: 5단계에서 구현
	// 시설 건설 결정 후 즉시 다음 상태로 전환
	TransitionToNextState(PlayerIndex);
}

void UAIPlayerManager::ProcessUnitMovementState(int32 PlayerIndex)
{
	// TODO: 5단계에서 구현
	// 유닛 이동/전투 결정 후 비동기 작업이 있으면 WaitingForAsync로, 없으면 다음 상태로 전환
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer)
	{
		return;
	}

	// 유닛 이동/전투 결정 후 비동기 작업 확인
	if (HasPendingAsyncWork(PlayerIndex))
	{
		AIPlayer->CurrentState = EAITurnState::WaitingForAsync;
	}
	else
	{
		TransitionToNextState(PlayerIndex);
	}
}

