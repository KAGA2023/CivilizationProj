// Fill out your copyright notice in the Description page of Project Settings.

#include "AIPlayerManager.h"
#include "../SuperPlayerState.h"
#include "../SuperGameInstance.h"
#include "../SuperGameModeBase.h"
#include "../Turn/TurnComponent.h"
#include "../Research/ResearchComponent.h"
#include "../City/CityComponent.h"
#include "../Status/UnitStatusStruct.h"
#include "../Status/UnitStatusComponent.h"
#include "../Unit/UnitCharacterBase.h"
#include "../World/WorldComponent.h"
#include "../World/WorldStruct.h"
#include "../Facility/FacilityManager.h"
#include "../Diplomacy/DiplomacyManager.h"
#include "../Diplomacy/DiplomacyStruct.h"
#include "../Unit/UnitManager.h"
#include "../Combat/UnitCombatComponent.h"
#include "../Combat/UnitCombatStruct.h"
#include "../Unit/UnitVisualizationComponent.h"
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
			// 총 플레이어 수 가져오기 (PlayerIndex 0 = 플레이어, 1부터 = AI)
			int32 TotalPlayerCount = GameInstance->GetPlayerStateCount();
			
			// AI 플레이어는 PlayerIndex 1부터 마지막까지
			for (int32 PlayerIndex = 1; PlayerIndex < TotalPlayerCount; PlayerIndex++)
			{
				if (ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(PlayerIndex))
				{
					RegisterAIPlayer(PlayerIndex, PlayerState);
					
					// 로드 시 AI 플레이어 상태 초기화 (유닛 처리 상태 리셋)
					FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
					if (AIPlayer)
					{
						// 유닛 처리 관련 상태 초기화
						AIPlayer->bIsProcessingCombatUnit = false;
						AIPlayer->CurrentCombatUnitIndex = 0;
						AIPlayer->CombatUnitsQueue.Empty();
						AIPlayer->PendingUnitMovements = 0;
						AIPlayer->PendingCombatActions = 0;
						AIPlayer->PendingPostAsyncState = EAITurnState::None;
						
						// 로드 시 유닛이 이미 존재할 수 있으므로, PlayerState의 유닛 목록 확인
						// (로드 후 Initialize()가 호출될 때 유닛이 이미 스폰되어 있을 수 있음)
						// 이는 ProcessCombatUnitMovementState()에서 UnitManager->GetAllUnits()로 찾으므로
						// 여기서는 상태만 초기화하면 됨
					}
				}
			}

			//// ========== 디버깅용: AI 플레이어 1, 2, 3이 서로에게 전쟁 선포 ==========
			//UDiplomacyManager* DiplomacyManager = GameInstance->GetDiplomacyManager();
			//if (DiplomacyManager)
			//{
			//	// 현재 라운드 가져오기
			//	int32 CurrentRound = GetCurrentRound();

			//	// AI 플레이어 1, 2, 3이 서로에게 전쟁 선포
			//	// Player 1 -> Player 2
			//	DiplomacyManager->DeclareWar(1, 2, CurrentRound);
			//	// Player 1 -> Player 3
			//	DiplomacyManager->DeclareWar(1, 3, CurrentRound);
			//	// Player 2 -> Player 3
			//	DiplomacyManager->DeclareWar(2, 3, CurrentRound);
			//}
		}
	}
}

void UAIPlayerManager::RegisterAIPlayer(int32 PlayerIndex, ASuperPlayerState* PlayerState)
{
	// PlayerState 유효성 체크
	if (!PlayerState)
	{
		return;
	}

	// PlayerIndex 유효성 체크 (1 이상이어야 함, 0은 플레이어)
	if (PlayerIndex < 1)
	{
		return;
	}

	// GameInstance에서 총 플레이어 수 가져와서 동적 범위 체크
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			int32 TotalPlayerCount = GameInstance->GetPlayerStateCount();
			
			// AI 플레이어는 PlayerIndex 1부터 (TotalPlayerCount - 1)까지
			if (PlayerIndex >= TotalPlayerCount)
			{
				return; // 범위를 벗어난 플레이어 인덱스
			}
		}
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
	AIPlayer.TargetFacilityTile = FVector2D(-1, -1); // 명시적으로 초기값 설정
	AIPlayer.bTargetFacilityIsRepair = false;

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
	int32 CurrentRound = GetCurrentRound();

	// 새 턴 초기화
	ResetAIPlayerForNewTurn(*AIPlayer);
	// 갇힌 유닛 제거 (데이터·월드 Destroy, 인구 반환)
	RemoveTrappedUnitsForAIPlayer(PlayerIndex);
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
	AIPlayer.TargetFacilityTile = FVector2D(-1, -1); // 명시적으로 초기값 설정
	AIPlayer.bTargetFacilityIsRepair = false;
	AIPlayer.PendingPostAsyncState = EAITurnState::None; // 비동기 후 상태 초기화
	AIPlayer.CombatUnitsQueue.Empty();
	AIPlayer.CurrentCombatUnitIndex = 0;
	AIPlayer.bIsProcessingCombatUnit = false;
}

void UAIPlayerManager::RemoveTrappedUnitsForAIPlayer(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer)
	{
		return;
	}

	ASuperPlayerState* PlayerState = AIPlayer->PlayerStateRef.Get();
	if (!PlayerState)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	UUnitManager* UnitManager = GameInstance->GetUnitManager();
	if (!UnitManager)
	{
		return;
	}

	TArray<AUnitCharacterBase*> OwnedUnits = PlayerState->GetOwnedUnits();
	TArray<AUnitCharacterBase*> UnitsToRemove;
	for (AUnitCharacterBase* Unit : OwnedUnits)
	{
		if (!Unit)
		{
			continue;
		}
		if (UnitManager->IsUnitTrapped(Unit))
		{
			UnitsToRemove.Add(Unit);
		}
	}

	for (AUnitCharacterBase* Unit : UnitsToRemove)
	{
		FVector2D Hex = UnitManager->GetHexPositionForUnit(Unit);
		const FVector2D RoundedHex(FMath::RoundToInt(Hex.X), FMath::RoundToInt(Hex.Y));
		UnitManager->DestroyUnit(Unit, RoundedHex);
	}
}

int32 UAIPlayerManager::GetCurrentRound() const
{
	int32 CurrentRound = 1; // 기본값
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
	return CurrentRound;
}

void UAIPlayerManager::UpdateStateMachine(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer || !AIPlayer->bIsTurnActive)
	{
		return; // AI 플레이어가 없거나 턴이 활성화되지 않음
	}

	// 비동기 작업이 있으면 대기
	if (HasPendingAsyncWork(PlayerIndex))
	{
		return;
	}

	// 현재 상태에 따라 처리 함수 호출
	switch (AIPlayer->CurrentState)
	{
		case EAITurnState::Idle:
			// Idle 상태에서는 첫 번째 상태로 전환
			TransitionToNextState(PlayerIndex);
			// 전환 후 즉시 상태 머신 업데이트 (재귀 호출)
			UpdateStateMachine(PlayerIndex);
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

		case EAITurnState::ProcessingBuilderMovement:
			ProcessBuilderMovementState(PlayerIndex);
			break;

		case EAITurnState::ProcessingBuilderBuild:
			ProcessBuilderBuildState(PlayerIndex);
			break;

		case EAITurnState::ProcessingCombatUnitMovement:
			ProcessCombatUnitMovementState(PlayerIndex);
			break;

		case EAITurnState::ProcessingCombatUnitCombat:
			ProcessCombatUnitCombatState(PlayerIndex);
			break;

		case EAITurnState::WaitingForAsync:
			// 비동기 작업 대기 중
			// 모든 비동기 작업이 완료되면 다음 상태로 전환
			TransitionToNextState(PlayerIndex);
			// 전환 후 즉시 상태 머신 업데이트 (재귀 호출)
			UpdateStateMachine(PlayerIndex);
			break;

		case EAITurnState::TurnComplete:
			// 턴 완료 후 0.3초 대기 후 NextTurn 호출 (재귀 Broadcast 방지)
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().ClearTimer(NextTurnDelayTimerHandle);
				World->GetTimerManager().SetTimer(NextTurnDelayTimerHandle, this, &UAIPlayerManager::OnNextTurnDelayTimerExpired, 0.3f, false);
			}
			break;

		default:
			break;
	}
}

void UAIPlayerManager::OnNextTurnDelayTimerExpired()
{
	if (UWorld* World = GetWorld())
	{
		if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
		{
			GameMode->NextTurn();
		}
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
			AIPlayer->CurrentState = EAITurnState::ProcessingBuilderMovement;
			break;

		case EAITurnState::ProcessingBuilderMovement:
			// 비동기 작업이 있으면 대기 상태로, 없으면 건설 상태로
			if (HasPendingAsyncWork(PlayerIndex))
			{
				AIPlayer->PendingPostAsyncState = EAITurnState::ProcessingBuilderBuild;
				AIPlayer->CurrentState = EAITurnState::WaitingForAsync;
			}
			else
			{
				AIPlayer->CurrentState = EAITurnState::ProcessingBuilderBuild;
			}
			break;

		case EAITurnState::ProcessingBuilderBuild:
			AIPlayer->CurrentState = EAITurnState::ProcessingCombatUnitMovement;
			break;

		case EAITurnState::ProcessingCombatUnitMovement:
			// 비동기 작업이 있으면 대기 상태로, 없으면 전투 상태로
			if (HasPendingAsyncWork(PlayerIndex))
			{
				AIPlayer->PendingPostAsyncState = EAITurnState::ProcessingCombatUnitCombat;
				AIPlayer->CurrentState = EAITurnState::WaitingForAsync;
			}
			else
			{
				AIPlayer->CurrentState = EAITurnState::ProcessingCombatUnitCombat;
			}
			break;

		case EAITurnState::ProcessingCombatUnitCombat:
			// 비동기 작업이 있으면 대기 상태로, 없으면 턴 완료
			if (HasPendingAsyncWork(PlayerIndex))
			{
				AIPlayer->PendingPostAsyncState = EAITurnState::TurnComplete;
				AIPlayer->CurrentState = EAITurnState::WaitingForAsync;
			}
			else
			{
				AIPlayer->CurrentState = EAITurnState::TurnComplete;
			}
			break;

		case EAITurnState::WaitingForAsync:
			// 비동기 작업 완료 후 저장된 상태로 이동, 없으면 턴 완료
			if (AIPlayer->PendingPostAsyncState != EAITurnState::None)
			{
				AIPlayer->CurrentState = AIPlayer->PendingPostAsyncState;
				AIPlayer->PendingPostAsyncState = EAITurnState::None;
			}
			else
			{
				AIPlayer->CurrentState = EAITurnState::TurnComplete;
			}
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

// ================= 비동기 작업 완료 콜백 =================

void UAIPlayerManager::OnUnitMovementFinished(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer)
	{
		return;
	}

	// PendingUnitMovements 감소 (0 이하로 내려가지 않도록)
	if (AIPlayer->PendingUnitMovements > 0)
	{
		AIPlayer->PendingUnitMovements--;
	}

	// 병사 유닛 이동 처리 중이면 (이동 대기 중일 때만 처리; 전투 대기 중엔 상태 덮어쓰지 않음)
	const bool bWaitingForMovement = (AIPlayer->CurrentState == EAITurnState::WaitingForAsync && AIPlayer->PendingPostAsyncState == EAITurnState::ProcessingCombatUnitMovement);
	if (AIPlayer->CurrentState == EAITurnState::ProcessingCombatUnitMovement || bWaitingForMovement)
	{
		// 현재 유닛 처리 완료 - 다음 유닛으로 인덱스 증가
		AIPlayer->CurrentCombatUnitIndex++;
		
		// bIsProcessingCombatUnit은 false로 설정하지 않음 (큐 재초기화 방지)
		// 다음 유닛 처리를 위해 상태를 명시적으로 ProcessingCombatUnitMovement로 설정
		AIPlayer->CurrentState = EAITurnState::ProcessingCombatUnitMovement;
		AIPlayer->PendingPostAsyncState = EAITurnState::None;
		
		// UpdateStateMachine()을 호출하면 TransitionToNextState()가 호출되어
		// WaitingForAsync 상태에서 PendingPostAsyncState로 넘어가버릴 수 있음
		// 따라서 직접 ProcessCombatUnitMovementState()를 호출하여 다음 유닛 처리
		ProcessCombatUnitMovementState(PlayerIndex);
		return; // 여기서 종료, ProcessCombatUnitMovementState에서 처리
	}
	// 턴이 활성화되어 있고, 비동기 작업이 없으면 상태 머신 업데이트
	else if (AIPlayer->bIsTurnActive && !HasPendingAsyncWork(PlayerIndex))
	{
		UpdateStateMachine(PlayerIndex);
	}
}

void UAIPlayerManager::OnCombatActionFinished(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer)
	{
		return;
	}

	// PendingCombatActions 감소 (0 이하로 내려가지 않도록)
	if (AIPlayer->PendingCombatActions > 0)
	{
		AIPlayer->PendingCombatActions--;
	}

	// 병사 유닛 전투 처리 중이면
	if (AIPlayer->CurrentState == EAITurnState::ProcessingCombatUnitCombat ||
		AIPlayer->CurrentState == EAITurnState::WaitingForAsync ||
		AIPlayer->PendingPostAsyncState == EAITurnState::ProcessingCombatUnitCombat)
	{
		// 현재 유닛 처리 완료 - 다음 유닛으로 인덱스 증가
		AIPlayer->CurrentCombatUnitIndex++;

		// bIsProcessingCombatUnit은 false로 설정하지 않음 (큐 재초기화 방지)
		// 다음 유닛 처리를 위해 상태를 명시적으로 ProcessingCombatUnitCombat로 설정
		AIPlayer->CurrentState = EAITurnState::ProcessingCombatUnitCombat;
		AIPlayer->PendingPostAsyncState = EAITurnState::None;

		// UpdateStateMachine()을 호출하면 TransitionToNextState()가 호출되어
		// WaitingForAsync 상태에서 PendingPostAsyncState로 넘어가버릴 수 있음
		// 따라서 직접 ProcessCombatUnitCombatState()를 호출하여 다음 유닛 처리
		ProcessCombatUnitCombatState(PlayerIndex);

		// 중요: 모든 전투가 완료되었는지 확인
		// PendingCombatActions가 0이고, 모든 유닛 처리가 완료되었으면 상태 머신 업데이트
		if (AIPlayer->PendingCombatActions == 0 &&
			AIPlayer->CurrentCombatUnitIndex >= AIPlayer->CombatUnitsQueue.Num())
		{
			// 모든 전투 완료
			AIPlayer->bIsProcessingCombatUnit = false;
			AIPlayer->CurrentCombatUnitIndex = 0;
			AIPlayer->CombatUnitsQueue.Empty();

			// 다음 상태로 전환 (ProcessingCombatUnitCombat -> TurnComplete)
			TransitionToNextState(PlayerIndex);
			UpdateStateMachine(PlayerIndex);

			// 턴 완료 시 NextTurn 타이머 직접 예약 (이중 보장)
			if (AIPlayer->CurrentState == EAITurnState::TurnComplete && AIPlayer->bIsTurnActive)
			{
				if (UWorld* World = GetWorld())
				{
					World->GetTimerManager().ClearTimer(NextTurnDelayTimerHandle);
					World->GetTimerManager().SetTimer(NextTurnDelayTimerHandle, this, &UAIPlayerManager::OnNextTurnDelayTimerExpired, 0.3f, false);
				}
			}
		}

		return; // 여기서 종료, ProcessCombatUnitCombatState에서 처리
	}
	// 턴이 활성화되어 있고, 비동기 작업이 없으면 상태 머신 업데이트
	else if (AIPlayer->bIsTurnActive && !HasPendingAsyncWork(PlayerIndex))
	{
		UpdateStateMachine(PlayerIndex);
	}
	else
	{
		// Fix A: State가 잘못 바뀌었어도(예: 12 TurnComplete) 전투 완료였으면 다음 전투/턴 정리 처리
		const bool bHadCombatQueue = (AIPlayer->CombatUnitsQueue.Num() > 0);
		if (AIPlayer->PendingCombatActions == 0 && (bHadCombatQueue || AIPlayer->bIsProcessingCombatUnit))
		{
			AIPlayer->CurrentCombatUnitIndex++;
			AIPlayer->CurrentState = EAITurnState::ProcessingCombatUnitCombat;
			AIPlayer->PendingPostAsyncState = EAITurnState::None;
			ProcessCombatUnitCombatState(PlayerIndex);
			if (AIPlayer->PendingCombatActions == 0 &&
				AIPlayer->CurrentCombatUnitIndex >= AIPlayer->CombatUnitsQueue.Num())
			{
				AIPlayer->bIsProcessingCombatUnit = false;
				AIPlayer->CurrentCombatUnitIndex = 0;
				AIPlayer->CombatUnitsQueue.Empty();
				TransitionToNextState(PlayerIndex);
				UpdateStateMachine(PlayerIndex);
				// 턴 완료 시 NextTurn 타이머 직접 예약
				if (AIPlayer->CurrentState == EAITurnState::TurnComplete && AIPlayer->bIsTurnActive)
				{
					if (UWorld* World = GetWorld())
					{
						World->GetTimerManager().ClearTimer(NextTurnDelayTimerHandle);
						World->GetTimerManager().SetTimer(NextTurnDelayTimerHandle, this, &UAIPlayerManager::OnNextTurnDelayTimerExpired, 0.3f, false);
					}
				}
			}
			return;
		}
	}
}

// ================= 상태별 처리 함수들 =================

void UAIPlayerManager::ProcessDiplomacyState(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer || !AIPlayer->PlayerStateRef.IsValid())
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// DiplomacyManager 및 GameInstance 가져오기
	UDiplomacyManager* DiplomacyManager = nullptr;
	USuperGameInstance* GameInstance = nullptr;
	if (UWorld* World = GetWorld())
	{
		GameInstance = Cast<USuperGameInstance>(World->GetGameInstance());
		if (GameInstance)
		{
			DiplomacyManager = GameInstance->GetDiplomacyManager();
		}
	}

	// 현재 라운드 가져오기
	int32 CurrentRound = GetCurrentRound();

	if (!DiplomacyManager)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// 총 플레이어 수 가져오기
	int32 TotalPlayerCount = GameInstance ? GameInstance->GetPlayerStateCount() : 4; // 기본값 4 (안전 장치)

	// ================= 1단계: 받은 액션 제안들 응답 처리 =================
	
	// 자신에게 도착한 미처리 액션 목록 가져오기
	TArray<FDiplomacyAction> PendingActions = DiplomacyManager->GetPendingActionsForPlayer(PlayerIndex);
	
	// 각 액션에 대해 응답 결정
	for (const FDiplomacyAction& Action : PendingActions)
	{
		int32 FromPlayerId = Action.FromPlayerId;
		
		// 호감도 및 상태 확인
		int32 Attitude = DiplomacyManager->GetAttitude(PlayerIndex, FromPlayerId);
		EDiplomacyStatusType CurrentStatus = DiplomacyManager->GetStatus(PlayerIndex, FromPlayerId);
		
		bool bAccepted = false;
		
		// 액션 타입에 따라 응답 결정
		switch (Action.Action)
		{
			case EDiplomacyActionType::OfferAlliance:
			{
				// 현재 상태가 Peace이고 호감도 >= 50이면 수락
				if (CurrentStatus == EDiplomacyStatusType::Peace && Attitude >= 50)
				{
					bAccepted = true;
				}
				break;
			}
			
			case EDiplomacyActionType::OfferPeace:
			{
				// 현재 상태가 전쟁 중이어야 함
				if (CurrentStatus == EDiplomacyStatusType::War)
				{
					// 상대에 대한 호감도 >= 20이면 수락
					if (Attitude >= 20)
					{
						bAccepted = true;
					}
					// 호감도 < 20이면 전쟁이 10라운드 이상일 때 10% 확률로 수락
					else
					{
						FDiplomacyPairKey PairKey(PlayerIndex, FromPlayerId);
						if (const FDiplomacyPairState* PairState = DiplomacyManager->PairStates.Find(PairKey))
						{
							if (PairState->LastWarRound > 0)
							{
								int32 WarDuration = CurrentRound - PairState->LastWarRound;
								if (WarDuration >= 10)
								{
									float RandomChance = FMath::RandRange(0.0f, 1.0f);
									if (RandomChance <= 0.10f)
									{
										bAccepted = true;
									}
								}
							}
						}
					}
				}
				break;
			}
			
			default:
				// 다른 액션 타입은 거절 (DeclareWar는 즉시 처리되므로 여기 올 일 없음)
				bAccepted = false;
				break;
		}
		
		// 응답 처리
		DiplomacyManager->ResolveAction(Action.ActionId, bAccepted);
	}

	// ================= 2단계: 액션 발행 (기존 로직) =================

	// 모든 다른 플레이어에 대해 외교 결정
	// 플레이어 인덱스: 0=Player, 1~TotalPlayerCount-1=AI
	for (int32 OtherPlayerIndex = 0; OtherPlayerIndex < TotalPlayerCount; OtherPlayerIndex++)
	{
		// 자신은 건너뛰기
		if (OtherPlayerIndex == PlayerIndex)
		{
			continue;
		}

		// 호감도 가져오기
		int32 Attitude = DiplomacyManager->GetAttitude(PlayerIndex, OtherPlayerIndex);
		EDiplomacyStatusType CurrentStatus = DiplomacyManager->GetStatus(PlayerIndex, OtherPlayerIndex);

		// 1. 호감도가 -50 이하 → DeclareWar
		if (Attitude <= -50)
		{
			// 이미 전쟁 중이 아니고, 동맹 상태가 아니면 전쟁 선포
			if (CurrentStatus != EDiplomacyStatusType::War && CurrentStatus != EDiplomacyStatusType::Alliance)
			{
				FDiplomacyAction WarAction;
				WarAction.Action = EDiplomacyActionType::DeclareWar;
				WarAction.FromPlayerId = PlayerIndex;
				WarAction.ToPlayerId = OtherPlayerIndex;
				DiplomacyManager->IssueAction(WarAction);
				continue; // 한 플레이어당 하나의 액션만 수행
			}
		}

		// 2. 호감도가 50 이상이고 전쟁 중이 아니면 → OfferAlliance
		// 플레이어(0)에게는 응답이 필요한 액션 발행하지 않음
		if (Attitude >= 50 && OtherPlayerIndex != 0)
		{
			if (CurrentStatus != EDiplomacyStatusType::Alliance && CurrentStatus != EDiplomacyStatusType::War)
			{
				FDiplomacyAction AllianceAction;
				AllianceAction.Action = EDiplomacyActionType::OfferAlliance;
				AllianceAction.FromPlayerId = PlayerIndex;
				AllianceAction.ToPlayerId = OtherPlayerIndex;
				DiplomacyManager->IssueAction(AllianceAction);
				continue; // 한 플레이어당 하나의 액션만 수행
			}
		}

		// 3. 전쟁 중이고 전쟁이 10라운드 이상 지나면 OfferPeace (플레이어0가 아니면)
		if (CurrentStatus == EDiplomacyStatusType::War && OtherPlayerIndex != 0)
		{
			FDiplomacyPairKey PairKey(PlayerIndex, OtherPlayerIndex);
			if (const FDiplomacyPairState* PairState = DiplomacyManager->PairStates.Find(PairKey))
			{
				if (PairState->LastWarRound > 0)
				{
					int32 WarDuration = CurrentRound - PairState->LastWarRound;
					if (WarDuration >= 10)
					{
						FDiplomacyAction PeaceAction;
						PeaceAction.Action = EDiplomacyActionType::OfferPeace;
						PeaceAction.FromPlayerId = PlayerIndex;
						PeaceAction.ToPlayerId = OtherPlayerIndex;
						DiplomacyManager->IssueAction(PeaceAction);
						continue; // 한 플레이어당 하나의 액션만 수행
					}
				}
			}
		}
	}

	// 4. 50% 확률로 랜덤 한 플레이어에게 Denounce 또는 SendGift
	float RandomChance = FMath::RandRange(0.0f, 1.0f);
	if (RandomChance <= 0.5f) // 50% 확률
	{
		// 랜덤 플레이어 선택
		TArray<int32> AvailablePlayers;
		for (int32 i = 0; i < TotalPlayerCount; i++)
		{
			if (i != PlayerIndex)
			{
				AvailablePlayers.Add(i);
			}
		}

		if (AvailablePlayers.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, AvailablePlayers.Num() - 1);
			int32 TargetPlayer = AvailablePlayers[RandomIndex];

			// Denounce 또는 SendGift 랜덤 선택
			EDiplomacyActionType RandomAction = FMath::RandBool() ? EDiplomacyActionType::Denounce : EDiplomacyActionType::SendGift;

			FDiplomacyAction Action;
			Action.Action = RandomAction;
			Action.FromPlayerId = PlayerIndex;
			Action.ToPlayerId = TargetPlayer;
			DiplomacyManager->IssueAction(Action);
		}
	}

	// 다음 상태로 전환
	TransitionToNextState(PlayerIndex);
	
	// 상태 전환 후 상태 머신 업데이트 (다음 상태 처리)
	UpdateStateMachine(PlayerIndex);
}

void UAIPlayerManager::ProcessResearchState(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer)
	{
		return;
	}

	// PlayerStateRef 유효성 확인
	if (!AIPlayer->PlayerStateRef.IsValid())
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	ASuperPlayerState* PlayerState = AIPlayer->PlayerStateRef.Get();
	if (!PlayerState)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// ResearchComponent 가져오기
	UResearchComponent* ResearchComponent = PlayerState->GetResearchComponent();
	if (!ResearchComponent)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// 현재 연구 중인 기술이 있는지 확인
	FResearchCurrentStat CurrentStat = ResearchComponent->GetCurrentStat();
	if (CurrentStat.DevelopingName != NAME_None)
	{
		// 이미 연구 중이면 다음 상태로 전환
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// 연구 가능한 기술 목록 가져오기
	TArray<FName> ResearchableTechs = ResearchComponent->GetResearchableTechs();
	
	// 연구 가능한 기술이 없으면 다음 상태로 전환
	if (ResearchableTechs.Num() == 0)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// 랜덤으로 하나 선택
	int32 RandomIndex = FMath::RandRange(0, ResearchableTechs.Num() - 1);
	FName SelectedTech = ResearchableTechs[RandomIndex];

	// 연구 시작
	PlayerState->StartTechResearch(SelectedTech);

	// 다음 상태로 전환
	TransitionToNextState(PlayerIndex);
	
	// 상태 전환 후 상태 머신 업데이트 (다음 상태 처리)
	UpdateStateMachine(PlayerIndex);
}

void UAIPlayerManager::ProcessCityProductionState(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer)
	{
		return;
	}

	// PlayerStateRef 유효성 확인
	if (!AIPlayer->PlayerStateRef.IsValid())
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	ASuperPlayerState* PlayerState = AIPlayer->PlayerStateRef.Get();
	if (!PlayerState)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// CityComponent 가져오기
	UCityComponent* CityComponent = PlayerState->GetCityComponent();
	if (!CityComponent)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// 현재 생산 중인지 확인
	FCityCurrentStat CurrentStat = CityComponent->GetCurrentStat();
	if (CurrentStat.ProductionType != EProductionType::None)
	{
		// 이미 생산 중이면 다음 상태로 전환
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// 생산 가능한 유닛 목록 가져오기
	TArray<FName> AvailableUnits = CityComponent->GetAvailableUnits();
	
	// 생산 가능한 건물 목록 가져오기
	TArray<FName> AvailableBuildings = CityComponent->GetAvailableBuildings();

	// 인구수 제한 확인
	int32 CurrentPopulation = PlayerState->GetPopulation();
	int32 LimitPopulation = PlayerState->GetLimitPopulation();
	bool bCanProduceUnit = (CurrentPopulation < LimitPopulation);

	// 건설자 보유 여부 확인
	bool bHasBuilder = false;
	TArray<AUnitCharacterBase*> OwnedUnits = PlayerState->GetOwnedUnits();
	for (AUnitCharacterBase* Unit : OwnedUnits)
	{
		if (Unit && Unit->GetUnitStatusComponent())
		{
			FUnitBaseStat UnitStat = Unit->GetUnitStatusComponent()->GetBaseStat();
			if (UnitStat.CanBuildFacilities)
			{
				bHasBuilder = true;
				break;
			}
		}
	}

	// 건설자 유닛 찾기 (CanBuildFacilities=true)
	TArray<FName> BuilderUnits;
	for (const FName& UnitName : AvailableUnits)
	{
		FUnitBaseStat UnitStat = CityComponent->GetUnitDataFromTable(UnitName);
		if (UnitStat.CanBuildFacilities)
		{
			BuilderUnits.Add(UnitName);
		}
	}

	// 병사 유닛 찾기 (CanAttack=true)
	TArray<FName> CombatUnits;
	for (const FName& UnitName : AvailableUnits)
	{
		FUnitBaseStat UnitStat = CityComponent->GetUnitDataFromTable(UnitName);
		if (UnitStat.CanAttack)
		{
			CombatUnits.Add(UnitName);
		}
	}

	// 마지막 생산 타입에 따라 다음 생산 타입 결정
	// 순서: 건설자 -> 병사 -> 건물 -> 건설자 -> ...
	EAILastProductionType NextProductionType = EAILastProductionType::None;
	
	switch (AIPlayer->LastProductionType)
	{
		case EAILastProductionType::None:
		case EAILastProductionType::Building:
			// None이거나 Building 다음은 Builder
			NextProductionType = EAILastProductionType::Builder;
			break;
		case EAILastProductionType::Builder:
			// Builder 다음은 Combat
			NextProductionType = EAILastProductionType::Combat;
			break;
		case EAILastProductionType::Combat:
			// Combat 다음은 Building
			NextProductionType = EAILastProductionType::Building;
			break;
	}

	// 다음 생산 타입에 따라 생산 결정
	bool bProductionStarted = false;
	
	switch (NextProductionType)
	{
		case EAILastProductionType::Builder:
			// 건설자 생산 (인구수 제한 확인 + 건설자 보유 여부 확인)
			if (BuilderUnits.Num() > 0 && bCanProduceUnit && !bHasBuilder)
			{
				int32 RandomIndex = FMath::RandRange(0, BuilderUnits.Num() - 1);
				FName SelectedBuilder = BuilderUnits[RandomIndex];
				PlayerState->StartUnitProduction(SelectedBuilder);
				AIPlayer->LastProductionType = EAILastProductionType::Builder;
				bProductionStarted = true;
			}
			break;

		case EAILastProductionType::Combat:
			// 병사 생산 (랜덤, 인구수 제한 확인)
			if (CombatUnits.Num() > 0 && bCanProduceUnit)
			{
				int32 RandomIndex = FMath::RandRange(0, CombatUnits.Num() - 1);
				FName SelectedCombat = CombatUnits[RandomIndex];
				PlayerState->StartUnitProduction(SelectedCombat);
				AIPlayer->LastProductionType = EAILastProductionType::Combat;
				bProductionStarted = true;
			}
			break;

		case EAILastProductionType::Building:
			// 건물 생산 (랜덤)
			if (AvailableBuildings.Num() > 0)
			{
				int32 RandomIndex = FMath::RandRange(0, AvailableBuildings.Num() - 1);
				FName SelectedBuilding = AvailableBuildings[RandomIndex];
				PlayerState->StartBuildingProduction(SelectedBuilding);
				AIPlayer->LastProductionType = EAILastProductionType::Building;
				bProductionStarted = true;
			}
			break;

		default:
			break;
	}

	// 원하는 타입을 생산할 수 없으면, 순서대로 다음 타입 시도 (순환)
	// 순서: 건설자 -> 병사 -> 건물 -> 건설자 -> ...
	if (!bProductionStarted)
	{
		// 다음 순서의 타입들을 순환하면서 시도
		EAILastProductionType TryOrder[3] = {
			EAILastProductionType::Builder,
			EAILastProductionType::Combat,
			EAILastProductionType::Building
		};

		// 현재 시도한 타입의 인덱스 찾기
		int32 CurrentIndex = -1;
		for (int32 i = 0; i < 3; i++)
		{
			if (TryOrder[i] == NextProductionType)
			{
				CurrentIndex = i;
				break;
			}
		}

		// 다음 순서부터 시도 (순환)
		for (int32 i = 1; i < 3; i++)
		{
			int32 TryIndex = (CurrentIndex + i) % 3;
			EAILastProductionType TryType = TryOrder[TryIndex];

			switch (TryType)
			{
				case EAILastProductionType::Builder:
					// 건설자 생산 (인구수 제한 확인 + 건설자 보유 여부 확인)
					if (BuilderUnits.Num() > 0 && bCanProduceUnit && !bHasBuilder)
					{
						int32 RandomIndex = FMath::RandRange(0, BuilderUnits.Num() - 1);
						FName SelectedBuilder = BuilderUnits[RandomIndex];
						PlayerState->StartUnitProduction(SelectedBuilder);
						AIPlayer->LastProductionType = EAILastProductionType::Builder;
						bProductionStarted = true;
					}
					break;

				case EAILastProductionType::Combat:
					// 병사 생산 (랜덤, 인구수 제한 확인)
					if (CombatUnits.Num() > 0 && bCanProduceUnit)
					{
						int32 RandomIndex = FMath::RandRange(0, CombatUnits.Num() - 1);
						FName SelectedCombat = CombatUnits[RandomIndex];
						PlayerState->StartUnitProduction(SelectedCombat);
						AIPlayer->LastProductionType = EAILastProductionType::Combat;
						bProductionStarted = true;
					}
					break;

				case EAILastProductionType::Building:
					if (AvailableBuildings.Num() > 0)
					{
						int32 RandomIndex = FMath::RandRange(0, AvailableBuildings.Num() - 1);
						FName SelectedBuilding = AvailableBuildings[RandomIndex];
						PlayerState->StartBuildingProduction(SelectedBuilding);
						AIPlayer->LastProductionType = EAILastProductionType::Building;
						bProductionStarted = true;
					}
					break;

				default:
					break;
			}

			// 성공하면 중단
			if (bProductionStarted)
			{
				break;
			}
		}

		// 모든 타입을 생산할 수 없는 경우
		// LastProductionType은 업데이트하지 않고 다음 상태로 전환
		// (다음 턴에 다시 시도할 수 있도록)
	}

	// 다음 상태로 전환
	TransitionToNextState(PlayerIndex);
	
	// 상태 전환 후 상태 머신 업데이트 (다음 상태 처리)
	UpdateStateMachine(PlayerIndex);
}

void UAIPlayerManager::ProcessTilePurchaseState(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer || !AIPlayer->PlayerStateRef.IsValid())
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	ASuperPlayerState* PlayerState = AIPlayer->PlayerStateRef.Get();
	if (!PlayerState)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// WorldComponent 가져오기
	UWorldComponent* WorldComponent = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			WorldComponent = GameInstance->GetGeneratedWorldComponent();
		}
	}

	if (!WorldComponent)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// 소유한 타일 좌표들 가져오기
	TArray<FVector2D> OwnedTiles = PlayerState->GetOwnedTileCoordinates();

	// 구매 가능한 타일 목록 수집 (인접 타일 중)
	TSet<FVector2D> PurchaseableTileSet;

	// 각 소유 타일의 인접 타일 확인
	for (const FVector2D& OwnedTileCoord : OwnedTiles)
	{
		// 인접 타일 좌표들 가져오기
		TArray<FVector2D> NeighborCoords = WorldComponent->GetHexNeighbors(OwnedTileCoord);

		// 각 인접 타일이 구매 가능한지 확인
		for (const FVector2D& NeighborCoord : NeighborCoords)
		{
			// 이미 확인한 타일은 건너뛰기
			if (PurchaseableTileSet.Contains(NeighborCoord))
			{
				continue;
			}

			// 구매 가능한지 확인
			if (PlayerState->CanPurchaseTile(NeighborCoord, WorldComponent))
			{
				PurchaseableTileSet.Add(NeighborCoord);
			}
		}
	}

	// 바다 타일 제외하고 총 생산량(생산량+식량+과학력+골드) 기준으로 정렬
	TArray<FTileWithTotalYield> ValidTiles;

	for (const FVector2D& TileCoord : PurchaseableTileSet)
	{
		if (UWorldTile* Tile = WorldComponent->GetTileAtHex(TileCoord))
		{
			// 바다 타일 제외
			if (Tile->GetTerrainType() == ETerrainType::Ocean)
			{
				continue;
			}

			// 생산량 + 식량 + 과학력 + 골드 총합 계산
			int32 ProductionYield = Tile->GetProductionYield();
			int32 FoodYield = Tile->GetFoodYield();
			int32 ScienceYield = Tile->GetScienceYield();
			int32 GoldYield = Tile->GetGoldYield();
			int32 TotalYield = ProductionYield + FoodYield + ScienceYield + GoldYield;

			ValidTiles.Add(FTileWithTotalYield(TileCoord, TotalYield));
		}
	}

	// 구매 가능한 타일이 없으면 다음 상태로 전환
	if (ValidTiles.Num() == 0)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// 총 생산량 기준으로 정렬 (내림차순)
	ValidTiles.Sort(FTileWithTotalYield::CompareDescending);

	// 최고 총 생산량 찾기
	int32 MaxTotalYield = ValidTiles[0].TotalYield;

	// 최고 총 생산량을 가진 타일들만 필터링
	TArray<FVector2D> BestTiles;
	for (const FTileWithTotalYield& TileInfo : ValidTiles)
	{
		if (TileInfo.TotalYield == MaxTotalYield)
		{
			BestTiles.Add(TileInfo.Coordinate);
		}
		else
		{
			break; // 정렬되어 있으므로 더 낮은 총 생산량은 건너뛰기
		}
	}

	// 최고 총 생산량 타일 중 랜덤 선택
	if (BestTiles.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, BestTiles.Num() - 1);
		FVector2D SelectedTile = BestTiles[RandomIndex];

		// 타일 구매 시도 (골드가 부족하면 실패하고 다음 라운드에 다시 시도)
		PlayerState->PurchaseTile(SelectedTile, WorldComponent);
	}

	// 다음 상태로 전환 (골드 부족으로 실패해도 다음 라운드에 다시 시도)
	TransitionToNextState(PlayerIndex);
	
	// 상태 전환 후 상태 머신 업데이트 (다음 상태 처리)
	UpdateStateMachine(PlayerIndex);
}

void UAIPlayerManager::ProcessFacilityState(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer || !AIPlayer->PlayerStateRef.IsValid())
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	ASuperPlayerState* PlayerState = AIPlayer->PlayerStateRef.Get();
	if (!PlayerState)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// WorldComponent 가져오기
	UWorldComponent* WorldComponent = nullptr;
	UFacilityManager* FacilityManager = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			WorldComponent = GameInstance->GetGeneratedWorldComponent();
			FacilityManager = GameInstance->GetFacilityManager();
		}
	}

	if (!WorldComponent || !FacilityManager)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// 이미 목표 타일이 설정되어 있으면 유효성 확인 (수리 목표 vs 건설 목표)
	bool bHasTargetFacilityTile = (AIPlayer->TargetFacilityTile != FVector2D(-1, -1));
	if (bHasTargetFacilityTile)
	{
		if (AIPlayer->bTargetFacilityIsRepair)
		{
			// 수리 목표: 타일에 시설이 있고 약탈 상태면 유지
			if (FacilityManager->CanRepairFacilityAtTile(AIPlayer->TargetFacilityTile, PlayerIndex, WorldComponent))
			{
				TransitionToNextState(PlayerIndex);
				UpdateStateMachine(PlayerIndex);
				return;
			}
			AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
			AIPlayer->bTargetFacilityIsRepair = false;
		}
		else
		{
			// 건설 목표: 타일에 시설이 없으면 유지
			if (!FacilityManager->HasFacilityAtTile(AIPlayer->TargetFacilityTile))
			{
				TransitionToNextState(PlayerIndex);
				UpdateStateMachine(PlayerIndex);
				return;
			}
			AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
		}
	}

	// 보유 타일 가져오기
	TArray<UWorldTile*> OwnedTiles = PlayerState->GetOwnedTiles(WorldComponent);

	// 도시 좌표 가져오기 (도시 타일은 시설 후보에서 제외)
	FVector2D CityCoord = FVector2D(-1, -1);
	if (PlayerState->HasCity())
	{
		CityCoord = PlayerState->GetCityCoordinate();
	}

	// 1) 수리 가능한 타일 수집 (우선)
	TArray<FTileWithTotalYield> RepairTiles;
	for (UWorldTile* Tile : OwnedTiles)
	{
		if (!Tile) continue;
		FVector2D TileCoord = Tile->GetGridPosition();
		if (TileCoord == CityCoord) continue;
		if (!FacilityManager->CanRepairFacilityAtTile(TileCoord, PlayerIndex, WorldComponent)) continue;
		int32 TotalYield = Tile->GetTotalProductionYield() + Tile->GetTotalFoodYield()
			+ Tile->GetTotalScienceYield() + Tile->GetTotalGoldYield();
		RepairTiles.Add(FTileWithTotalYield(TileCoord, TotalYield));
	}
	if (RepairTiles.Num() > 0)
	{
		RepairTiles.Sort(FTileWithTotalYield::CompareDescending);
		AIPlayer->TargetFacilityTile = RepairTiles[0].Coordinate;
		AIPlayer->bTargetFacilityIsRepair = true;
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// 2) 시설이 없는 타일 중 건설 후보
	TArray<FTileWithTotalYield> ValidTiles;
	for (UWorldTile* Tile : OwnedTiles)
	{
		if (!Tile) continue;
		FVector2D TileCoord = Tile->GetGridPosition();
		if (TileCoord == CityCoord) continue;
		if (FacilityManager->HasFacilityAtTile(TileCoord)) continue;
		int32 ProductionYield = Tile->GetTotalProductionYield();
		int32 FoodYield = Tile->GetTotalFoodYield();
		int32 ScienceYield = Tile->GetTotalScienceYield();
		int32 GoldYield = Tile->GetTotalGoldYield();
		int32 TotalYield = ProductionYield + FoodYield + ScienceYield + GoldYield;
		ValidTiles.Add(FTileWithTotalYield(TileCoord, TotalYield));
	}
	if (ValidTiles.Num() == 0)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}
	ValidTiles.Sort(FTileWithTotalYield::CompareDescending);
	AIPlayer->TargetFacilityTile = ValidTiles[0].Coordinate;
	AIPlayer->bTargetFacilityIsRepair = false;

	TransitionToNextState(PlayerIndex);
	UpdateStateMachine(PlayerIndex);
}

void UAIPlayerManager::ProcessBuilderMovementState(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer || !AIPlayer->PlayerStateRef.IsValid())
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	ASuperPlayerState* PlayerState = AIPlayer->PlayerStateRef.Get();
	if (!PlayerState)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance());
	if (!GameInstance)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	UWorldComponent* WorldComponent = GameInstance->GetGeneratedWorldComponent();
	UUnitManager* UnitManager = GameInstance->GetUnitManager();

	if (!WorldComponent || !UnitManager)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// 공통 데이터 준비
	TArray<UWorldTile*> AllTiles = WorldComponent->GetAllTiles();

	// ========== 건설자 유닛 찾기 ==========
	TArray<AUnitCharacterBase*> AllUnits = UnitManager->GetAllUnits();
	AUnitCharacterBase* BuilderUnit = nullptr;
	for (AUnitCharacterBase* Unit : AllUnits)
	{
		if (Unit && Unit->GetPlayerIndex() == PlayerIndex && UnitManager->IsBuilderUnit(Unit))
		{
			BuilderUnit = Unit;
			break;
		}
	}

	if (!BuilderUnit)
	{
		// 건설자가 없으면 다음 상태로 전환
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// ========== 건설자 위치 확인 ==========
	FVector2D BuilderPosition = FVector2D(-1, -1);
	if (!FindUnitPosition(BuilderUnit, UnitManager, AllTiles, BuilderPosition))
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// ========== 목표 시설 타일 확인 ==========
	bool bHasTargetFacilityTile = (AIPlayer->TargetFacilityTile != FVector2D(-1, -1));
	if (!bHasTargetFacilityTile)
	{
		// 목표 타일이 없으면 다음 상태로 전환
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// ========== 건설자가 목표 타일에 도착했는지 확인 ==========
	if (BuilderPosition == AIPlayer->TargetFacilityTile)
	{
		// 이미 도착했으면 다음 상태(건설)로 전환
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// ========== 건설자 이동 처리 ==========
	// 이동 불가능한 경우 랜덤 배회
	if (!BuilderUnit->GetUnitStatusComponent()->CanMove())
	{
		FVector2D WanderTargetTile = FindValidWanderTile(PlayerIndex, 2, 10);
		if (WanderTargetTile != FVector2D(-1, -1))
		{
			TryMoveUnitToTile(BuilderUnit, BuilderPosition, WanderTargetTile, UnitManager, AIPlayer->PendingUnitMovements);
		}
		AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
		AIPlayer->bTargetFacilityIsRepair = false;
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// 이동 가능한 경우 목표 타일로 이동 시도
	if (TryMoveUnitToTile(BuilderUnit, BuilderPosition, AIPlayer->TargetFacilityTile, UnitManager, AIPlayer->PendingUnitMovements))
	{
		// 이동 시작 성공 - 비동기 작업이 시작되었으므로 TransitionToNextState에서 WaitingForAsync로 전환됨
		TransitionToNextState(PlayerIndex);
		// UpdateStateMachine은 비동기 작업 완료 후 호출됨
		return;
	}

	// 경로가 없으면 랜덤 배회
	FVector2D WanderTargetTile = FindValidWanderTile(PlayerIndex, 2, 10);
	if (WanderTargetTile != FVector2D(-1, -1))
	{
		TryMoveUnitToTile(BuilderUnit, BuilderPosition, WanderTargetTile, UnitManager, AIPlayer->PendingUnitMovements);
	}
	AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
	AIPlayer->bTargetFacilityIsRepair = false;
	TransitionToNextState(PlayerIndex);
	UpdateStateMachine(PlayerIndex);
}

void UAIPlayerManager::ProcessBuilderBuildState(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer || !AIPlayer->PlayerStateRef.IsValid())
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	ASuperPlayerState* PlayerState = AIPlayer->PlayerStateRef.Get();
	if (!PlayerState)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance());
	if (!GameInstance)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	UWorldComponent* WorldComponent = GameInstance->GetGeneratedWorldComponent();
	UUnitManager* UnitManager = GameInstance->GetUnitManager();
	UFacilityManager* FacilityManager = GameInstance->GetFacilityManager();

	if (!WorldComponent || !UnitManager || !FacilityManager)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// ========== 목표 시설 타일 확인 ==========
	bool bHasTargetFacilityTile = (AIPlayer->TargetFacilityTile != FVector2D(-1, -1));
	if (!bHasTargetFacilityTile)
	{
		// 목표 타일이 없으면 다음 상태로 전환
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// ========== 건설자 유닛 찾기 ==========
	TArray<UWorldTile*> AllTiles = WorldComponent->GetAllTiles();
	TArray<AUnitCharacterBase*> AllUnits = UnitManager->GetAllUnits();
	AUnitCharacterBase* BuilderUnit = nullptr;
	for (AUnitCharacterBase* Unit : AllUnits)
	{
		if (Unit && Unit->GetPlayerIndex() == PlayerIndex && UnitManager->IsBuilderUnit(Unit))
		{
			BuilderUnit = Unit;
			break;
		}
	}

	if (!BuilderUnit)
	{
		// 건설자가 없으면 목표 타일 초기화하고 다음 상태로
		AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// ========== 건설자 위치 확인 ==========
	FVector2D BuilderPosition = FVector2D(-1, -1);
	if (!FindUnitPosition(BuilderUnit, UnitManager, AllTiles, BuilderPosition))
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// ========== 시설 수리/건설 처리 ==========
	if (BuilderPosition == AIPlayer->TargetFacilityTile)
	{
		if (AIPlayer->bTargetFacilityIsRepair)
		{
			// 수리: 약탈 해제 (건설자 소모 없음)
			FacilityManager->RepairFacility(AIPlayer->TargetFacilityTile, WorldComponent);
			AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
			AIPlayer->bTargetFacilityIsRepair = false;
		}
		else
		{
			// 건설
			UWorldTile* TargetTile = WorldComponent->GetTileAtHex(AIPlayer->TargetFacilityTile);
			if (!TargetTile)
			{
				AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
				AIPlayer->bTargetFacilityIsRepair = false;
				TransitionToNextState(PlayerIndex);
				UpdateStateMachine(PlayerIndex);
				return;
			}
			TArray<FName> AvailableFacilities = PlayerState->GetAvailableFacilities();
			if (AvailableFacilities.Num() == 0)
			{
				AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
				AIPlayer->bTargetFacilityIsRepair = false;
				TransitionToNextState(PlayerIndex);
				UpdateStateMachine(PlayerIndex);
				return;
			}
			TArray<FName> BuildableFacilities;
			for (const FName& FacilityRowName : AvailableFacilities)
			{
				if (FacilityManager->CanBuildFacilityOnTile(FacilityRowName, TargetTile))
				{
					BuildableFacilities.Add(FacilityRowName);
				}
			}
			if (BuildableFacilities.Num() == 0)
			{
				AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
				AIPlayer->bTargetFacilityIsRepair = false;
				TransitionToNextState(PlayerIndex);
				UpdateStateMachine(PlayerIndex);
				return;
			}
			int32 RandomIndex = FMath::RandRange(0, BuildableFacilities.Num() - 1);
			FName SelectedFacilityRowName = BuildableFacilities[RandomIndex];
			PlayerState->BuildFacility(SelectedFacilityRowName, AIPlayer->TargetFacilityTile);
			AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
			AIPlayer->bTargetFacilityIsRepair = false;
		}
	}
	else
	{
		// 건설자가 목표 타일에 도착하지 않았으면 목표 타일 유지 (다음 턴에 다시 시도)
	}

	// 다음 상태로 전환
	TransitionToNextState(PlayerIndex);
	UpdateStateMachine(PlayerIndex);
}

void UAIPlayerManager::ProcessCombatUnitMovementState(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer || !AIPlayer->PlayerStateRef.IsValid())
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	ASuperPlayerState* PlayerState = AIPlayer->PlayerStateRef.Get();
	if (!PlayerState)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance());
	if (!GameInstance)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	UWorldComponent* WorldComponent = GameInstance->GetGeneratedWorldComponent();
	UUnitManager* UnitManager = GameInstance->GetUnitManager();
	UDiplomacyManager* DiplomacyManager = GameInstance->GetDiplomacyManager();

	if (!WorldComponent || !UnitManager || !DiplomacyManager)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// 공통 데이터 준비
	TArray<UWorldTile*> AllTiles = WorldComponent->GetAllTiles();
	TArray<AUnitCharacterBase*> AllUnits = UnitManager->GetAllUnits();

	// ========== 전쟁 상태 확인 ==========
	bool bIsAtWar = false;
	// 총 플레이어 수 가져오기
	int32 TotalPlayerCount = GameInstance->GetPlayerStateCount();
	for (int32 OtherPlayerIndex = 0; OtherPlayerIndex < TotalPlayerCount; OtherPlayerIndex++)
	{
		if (OtherPlayerIndex != PlayerIndex && 
			DiplomacyManager->IsAtWar(PlayerIndex, OtherPlayerIndex))
		{
			bIsAtWar = true;
			break;
		}
	}

	// ========== 순차 처리 초기화 ==========
	// 첫 호출이거나 이전 유닛 처리가 완료된 경우
	if (!AIPlayer->bIsProcessingCombatUnit)
	{
		// 큐 초기화 및 인덱스 리셋
		AIPlayer->CombatUnitsQueue.Empty();
		
		// 병사 유닛 찾기 및 큐에 추가 (로드 시 유닛이 이미 존재할 수 있음)
		int32 FoundCombatUnits = 0;
		for (AUnitCharacterBase* Unit : AllUnits)
		{
			if (Unit && IsValid(Unit))
			{
				int32 UnitPlayerIndex = Unit->GetPlayerIndex();
				if (UnitPlayerIndex == PlayerIndex && UnitManager->IsCombatUnit(Unit))
				{
					AIPlayer->CombatUnitsQueue.Add(TWeakObjectPtr<AUnitCharacterBase>(Unit));
					FoundCombatUnits++;
				}
			}
		}
		
		// 로드 시 유닛이 없을 수 있으므로, 큐가 비어있어도 정상적으로 처리
		AIPlayer->CurrentCombatUnitIndex = 0;
		AIPlayer->bIsProcessingCombatUnit = (AIPlayer->CombatUnitsQueue.Num() > 0);
	}

	// ========== 순차적으로 각 병사 유닛 처리 ==========
	while (AIPlayer->CurrentCombatUnitIndex < AIPlayer->CombatUnitsQueue.Num())
	{
		// 현재 처리할 유닛 가져오기
		AUnitCharacterBase* CombatUnit = AIPlayer->CombatUnitsQueue[AIPlayer->CurrentCombatUnitIndex].Get();
		
		// 유닛이 유효하지 않으면 스킵
		if (!CombatUnit || !IsValid(CombatUnit))
		{
			AIPlayer->CurrentCombatUnitIndex++;
			continue;
		}

		// 현재 위치 확인
		FVector2D CombatUnitPosition = FVector2D(-1, -1);
		if (!FindUnitPosition(CombatUnit, UnitManager, AllTiles, CombatUnitPosition))
		{
			AIPlayer->CurrentCombatUnitIndex++;
			continue;
		}

		// 이동 가능 여부 확인
		if (!CombatUnit->GetUnitStatusComponent()->CanMove())
		{
			AIPlayer->CurrentCombatUnitIndex++;
			continue;
		}

		// 전쟁 중이 아니면 평화 상태 로직 (배회)
		if (!bIsAtWar)
		{
			// 랜덤 배회 타일 선택
			FVector2D WanderTargetTile = FindValidWanderTile(PlayerIndex, 2, 10);
			if (WanderTargetTile != FVector2D(-1, -1))
			{
				// 이동 시작 시도
				if (TryMoveUnitToTile(CombatUnit, CombatUnitPosition, WanderTargetTile, UnitManager, AIPlayer->PendingUnitMovements))
				{
					// 이동 시작 성공
					AIPlayer->bIsProcessingCombatUnit = true;
					AIPlayer->PendingPostAsyncState = EAITurnState::ProcessingCombatUnitMovement;
					
					// 비동기 작업 대기 상태로 전환
					TransitionToNextState(PlayerIndex);
					return; // 여기서 종료, 콜백에서 재개
				}
				else
				{
					// 이동 실패 시 다음 유닛으로
					AIPlayer->CurrentCombatUnitIndex++;
					continue;
				}
			}
			else
			{
				// 배회 타일을 찾을 수 없으면 다음 유닛으로
				AIPlayer->CurrentCombatUnitIndex++;
				continue;
			}
		}

		// ========== 전쟁 상태 로직 ==========
		// StatusComp를 먼저 가져오기 (여러 곳에서 사용)
		UUnitStatusComponent* StatusComp = CombatUnit->GetUnitStatusComponent();
		
		// 1단계: 현재 위치에서 이미 공격 가능한지 확인
		// 공격 가능 여부 확인
		if (StatusComp && StatusComp->CanAttack())
		{
			// 사정거리 계산
			int32 BaseRange = StatusComp->GetRange();
			int32 FinalRange = BaseRange;

			// 원거리 유닛(Range > 1)이면 Range 보너스 적용
			if (BaseRange > 1)
			{
				UUnitCombatComponent* CombatComp = CombatUnit->GetUnitCombatComponent();
				if (CombatComp)
				{
					int32 RangeBonus = CombatComp->CalculateRangeBonus(CombatUnitPosition);
					FinalRange += RangeBonus;
				}
			}

			// 현재 위치에서 사정거리 내 적 유닛 탐지
			TArray<AUnitCharacterBase*> EnemyUnitsInRange = FindEnemyUnitsInRange(
				CombatUnitPosition, FinalRange, PlayerIndex, UnitManager, WorldComponent, DiplomacyManager
			);

			// 현재 위치에서 사정거리 내 적 도시 탐지
			TArray<FVector2D> EnemyCitiesInRange = FindEnemyCitiesInRange(
				CombatUnitPosition, FinalRange, PlayerIndex, WorldComponent, DiplomacyManager, GameInstance
			);

			// 이미 공격 가능한 위치에 있으면 이동 건너뛰고 다음 유닛으로 (전투 단계에서 처리)
			if (EnemyUnitsInRange.Num() > 0 || EnemyCitiesInRange.Num() > 0)
			{
				AIPlayer->CurrentCombatUnitIndex++;
				continue;
			}
		}

		// 2단계: 공격 불가능한 위치에 있으면 이동 시도
		// 4칸 반경 내 적 유닛 탐지 (이동 목표 찾기용)
		TArray<AUnitCharacterBase*> EnemyUnits = FindEnemyUnitsInRange(
			CombatUnitPosition, 4, PlayerIndex, UnitManager, WorldComponent, DiplomacyManager
		);

		if (EnemyUnits.Num() > 0)
		{
			// 적 유닛이 있으면 적 유닛의 사정거리 내 공격 가능한 타일로 이동
			AUnitCharacterBase* TargetEnemy = EnemyUnits[0]; // 가장 가까운 적
			
			// 적 유닛의 위치 찾기
			FVector2D EnemyPosition = FVector2D(-1, -1);
			if (!FindUnitPosition(TargetEnemy, UnitManager, AllTiles, EnemyPosition))
			{
				AIPlayer->CurrentCombatUnitIndex++;
				continue;
			}

			// 전투 유닛의 사정거리 계산 (위에서 선언한 StatusComp 재사용)
			int32 BaseRange = 1; // 기본 사정거리
			int32 FinalRange = BaseRange;
			if (StatusComp)
			{
				BaseRange = StatusComp->GetRange();
				FinalRange = BaseRange;
				
				// 원거리 유닛(Range > 1)이면 Range 보너스 적용 (현재 위치 기준)
				if (BaseRange > 1)
				{
					UUnitCombatComponent* CombatComp = CombatUnit->GetUnitCombatComponent();
					if (CombatComp)
					{
						int32 RangeBonus = CombatComp->CalculateRangeBonus(CombatUnitPosition);
						FinalRange += RangeBonus;
					}
				}
			}

			// 적 유닛 주변 사정거리 내 타일들 찾기
			TArray<FVector2D> TilesInAttackRange = WorldComponent->GetHexesInRadius(EnemyPosition, FinalRange);
			
			// 현재 위치에서 이동 가능한 타일 중 가장 가까운 타일 찾기
			FVector2D BestTargetTile = FVector2D(-1, -1);
			int32 MinDistance = INT32_MAX;
			int32 MaxMovementCost = UnitManager->GetUnitRemainingMovement(CombatUnit);
			
			for (const FVector2D& Tile : TilesInAttackRange)
			{
				// 타일이 유효한지 확인
				if (!WorldComponent->IsValidHexPosition(Tile))
				{
					continue;
				}
				
				// 유닛 배치 가능한지 확인
				if (!UnitManager->CanPlaceUnitAtHex(Tile, CombatUnit))
				{
					continue;
				}
				
				// 현재 위치에서 이 타일까지 이동 가능한지 확인 (경로 탐색, 국경선 적용)
				TArray<FVector2D> Path = UnitManager->FindPathWithMovementCost(
					CombatUnitPosition, Tile, MaxMovementCost, CombatUnit->GetPlayerIndex()
				);
				
				if (Path.Num() > 1)
				{
					// 이동 가능한 타일 중 가장 가까운 타일 선택
					int32 Distance = WorldComponent->GetHexDistance(CombatUnitPosition, Tile);
					if (Distance < MinDistance)
					{
						MinDistance = Distance;
						BestTargetTile = Tile;
					}
				}
			}

			// 사정거리 내 공격 가능한 타일로 이동 시도
			if (BestTargetTile != FVector2D(-1, -1))
			{
				if (TryMoveUnitToTile(CombatUnit, CombatUnitPosition, BestTargetTile, UnitManager, AIPlayer->PendingUnitMovements))
				{
					// 이동 시작 성공
					AIPlayer->bIsProcessingCombatUnit = true;
					AIPlayer->PendingPostAsyncState = EAITurnState::ProcessingCombatUnitMovement;
					
					// 비동기 작업 대기 상태로 전환
					TransitionToNextState(PlayerIndex);
					return; // 여기서 종료, 콜백에서 재개
				}
			}
			
			// 사정거리 내 공격 가능한 타일로 이동 실패 시 다음 유닛으로
			AIPlayer->CurrentCombatUnitIndex++;
			continue;
		}
		else
		{
			// 적 유닛이 없으면 가장 가까운 적 도시로 이동
			FVector2D EnemyCityCoord = FindClosestEnemyCity(
				PlayerIndex, PlayerState, GameInstance, WorldComponent, DiplomacyManager
			);
			if (EnemyCityCoord != FVector2D(-1, -1))
			{
				// 적 도시 주변에서 이동 가능한 타일 찾기 (1칸 → 2칸 → 3칸 확장)
				FVector2D BestTargetTile = FVector2D(-1, -1);
				int32 MinDistance = INT32_MAX;
				int32 MaxMovementCost = UnitManager->GetUnitRemainingMovement(CombatUnit);
				
				// 1칸 범위 시도
				TArray<FVector2D> OneRingHexes = WorldComponent->GetHexNeighbors(EnemyCityCoord);
				for (const FVector2D& NeighborHex : OneRingHexes)
				{
					// 유효한 좌표인지 확인
					if (!WorldComponent->IsValidHexPosition(NeighborHex))
					{
						continue;
					}
					
					// 타일이 존재하고 이동 가능한지 확인
					UWorldTile* Tile = WorldComponent->GetTileAtHex(NeighborHex);
					if (!Tile || !Tile->IsPassable())
					{
						continue;
					}
					
					// 유닛 배치 가능한지 확인
					if (!UnitManager->CanPlaceUnitAtHex(NeighborHex, CombatUnit))
					{
						continue;
					}
					
					// 현재 위치에서 이 타일까지 이동 가능한지 확인 (경로 탐색, 국경선 적용)
					TArray<FVector2D> Path = UnitManager->FindPathWithMovementCost(
						CombatUnitPosition, NeighborHex, MaxMovementCost, CombatUnit->GetPlayerIndex()
					);
					
					if (Path.Num() > 1)
					{
						// 이동 가능한 타일 중 가장 가까운 타일 선택
						int32 Distance = WorldComponent->GetHexDistance(CombatUnitPosition, NeighborHex);
						if (Distance < MinDistance)
						{
							MinDistance = Distance;
							BestTargetTile = NeighborHex;
						}
					}
				}
				
				// 1칸 범위에 타일이 없으면 2칸 범위 시도
				if (BestTargetTile == FVector2D(-1, -1))
				{
					TArray<FVector2D> AllHexesInRadius = WorldComponent->GetHexesInRadius(EnemyCityCoord, 2);
					for (const FVector2D& Hex : AllHexesInRadius)
					{
						// 정확히 2칸 거리만 (1칸 제외)
						int32 DistanceFromCity = WorldComponent->GetHexDistance(EnemyCityCoord, Hex);
						if (DistanceFromCity != 2)
						{
							continue;
						}
						
						// 유효한 좌표인지 확인
						if (!WorldComponent->IsValidHexPosition(Hex))
						{
							continue;
						}
						
						// 타일이 존재하고 이동 가능한지 확인
						UWorldTile* Tile = WorldComponent->GetTileAtHex(Hex);
						if (!Tile || !Tile->IsPassable())
						{
							continue;
						}
						
						// 유닛 배치 가능한지 확인
						if (!UnitManager->CanPlaceUnitAtHex(Hex, CombatUnit))
						{
							continue;
						}
						
						// 현재 위치에서 이 타일까지 이동 가능한지 확인 (경로 탐색, 국경선 적용)
						TArray<FVector2D> Path = UnitManager->FindPathWithMovementCost(
							CombatUnitPosition, Hex, MaxMovementCost, CombatUnit->GetPlayerIndex()
						);
						
						if (Path.Num() > 1)
						{
							// 이동 가능한 타일 중 가장 가까운 타일 선택
							int32 Distance = WorldComponent->GetHexDistance(CombatUnitPosition, Hex);
							if (Distance < MinDistance)
							{
								MinDistance = Distance;
								BestTargetTile = Hex;
							}
						}
					}
				}
				
				// 2칸 범위에도 타일이 없으면 3칸 범위 시도
				if (BestTargetTile == FVector2D(-1, -1))
				{
					TArray<FVector2D> AllHexesInRadius = WorldComponent->GetHexesInRadius(EnemyCityCoord, 3);
					for (const FVector2D& Hex : AllHexesInRadius)
					{
						// 정확히 3칸 거리만 (1~2칸 제외)
						int32 DistanceFromCity = WorldComponent->GetHexDistance(EnemyCityCoord, Hex);
						if (DistanceFromCity != 3)
						{
							continue;
						}
						
						// 유효한 좌표인지 확인
						if (!WorldComponent->IsValidHexPosition(Hex))
						{
							continue;
						}
						
						// 타일이 존재하고 이동 가능한지 확인
						UWorldTile* Tile = WorldComponent->GetTileAtHex(Hex);
						if (!Tile || !Tile->IsPassable())
						{
							continue;
						}
						
						// 유닛 배치 가능한지 확인
						if (!UnitManager->CanPlaceUnitAtHex(Hex, CombatUnit))
						{
							continue;
						}
						
						// 현재 위치에서 이 타일까지 이동 가능한지 확인 (경로 탐색, 국경선 적용)
						TArray<FVector2D> Path = UnitManager->FindPathWithMovementCost(
							CombatUnitPosition, Hex, MaxMovementCost, CombatUnit->GetPlayerIndex()
						);
						
						if (Path.Num() > 1)
						{
							// 이동 가능한 타일 중 가장 가까운 타일 선택
							int32 Distance = WorldComponent->GetHexDistance(CombatUnitPosition, Hex);
							if (Distance < MinDistance)
							{
								MinDistance = Distance;
								BestTargetTile = Hex;
							}
						}
					}
				}
				
				// 찾은 타일로 이동 시도
				if (BestTargetTile != FVector2D(-1, -1))
				{
					if (TryMoveUnitToTile(CombatUnit, CombatUnitPosition, BestTargetTile, UnitManager, AIPlayer->PendingUnitMovements))
					{
						// 이동 시작 성공
						AIPlayer->bIsProcessingCombatUnit = true;
						AIPlayer->PendingPostAsyncState = EAITurnState::ProcessingCombatUnitMovement;
						
						// 비동기 작업 대기 상태로 전환
						TransitionToNextState(PlayerIndex);
						return; // 여기서 종료, 콜백에서 재개
					}
				}
				
				// 이동 실패 시 다음 유닛으로
				AIPlayer->CurrentCombatUnitIndex++;
				continue;
			}
			else
			{
				// 적 도시를 찾을 수 없으면 다음 유닛으로
				AIPlayer->CurrentCombatUnitIndex++;
				continue;
			}
		}
	}

	// ========== 모든 유닛 처리 완료 확인 ==========
	// while 루프가 끝났다는 것은 CurrentCombatUnitIndex >= CombatUnitsQueue.Num()이라는 의미
	// 이는 모든 유닛을 처리했다는 의미이므로 다음 상태로 전환
	// 하지만 이동이 시작되어 비동기 대기 중인 경우는 여기서 처리하지 않음 (콜백에서 처리)
	if (AIPlayer->CurrentCombatUnitIndex >= AIPlayer->CombatUnitsQueue.Num())
	{
		// 모든 유닛 처리 완료
		AIPlayer->bIsProcessingCombatUnit = false;
		AIPlayer->CurrentCombatUnitIndex = 0;
		AIPlayer->CombatUnitsQueue.Empty();

		// 다음 상태로 전환
		TransitionToNextState(PlayerIndex);
		if (!HasPendingAsyncWork(PlayerIndex))
		{
			UpdateStateMachine(PlayerIndex);
		}
	}
	// 아직 처리할 유닛이 남아있거나 이동이 시작되어 비동기 대기 중인 경우는 여기서 아무것도 하지 않음
	// (이동 완료 후 OnUnitMovementFinished에서 다시 ProcessCombatUnitMovementState가 호출됨)
}

void UAIPlayerManager::ProcessCombatUnitCombatState(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer || !AIPlayer->PlayerStateRef.IsValid())
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	ASuperPlayerState* PlayerState = AIPlayer->PlayerStateRef.Get();
	if (!PlayerState)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance());
	if (!GameInstance)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	UWorldComponent* WorldComponent = GameInstance->GetGeneratedWorldComponent();
	UUnitManager* UnitManager = GameInstance->GetUnitManager();
	UDiplomacyManager* DiplomacyManager = GameInstance->GetDiplomacyManager();

	if (!WorldComponent || !UnitManager || !DiplomacyManager)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// 공통 데이터 준비
	TArray<UWorldTile*> AllTiles = WorldComponent->GetAllTiles();
	TArray<AUnitCharacterBase*> AllUnits = UnitManager->GetAllUnits();

	// ========== 순차 처리 초기화 ==========
	// 첫 호출이거나 이전 유닛 처리가 완료된 경우
	if (!AIPlayer->bIsProcessingCombatUnit)
	{
		// 큐 초기화 및 인덱스 리셋
		AIPlayer->CombatUnitsQueue.Empty();
		
		// 병사 유닛 찾기 및 큐에 추가 (로드 시 유닛이 이미 존재할 수 있음)
		int32 FoundCombatUnits = 0;
		for (AUnitCharacterBase* Unit : AllUnits)
		{
			if (Unit && IsValid(Unit))
			{
				int32 UnitPlayerIndex = Unit->GetPlayerIndex();
				if (UnitPlayerIndex == PlayerIndex && UnitManager->IsCombatUnit(Unit))
				{
					AIPlayer->CombatUnitsQueue.Add(TWeakObjectPtr<AUnitCharacterBase>(Unit));
					FoundCombatUnits++;
				}
			}
		}
		
		// 로드 시 유닛이 없을 수 있으므로, 큐가 비어있어도 정상적으로 처리
		AIPlayer->CurrentCombatUnitIndex = 0;
		AIPlayer->bIsProcessingCombatUnit = (AIPlayer->CombatUnitsQueue.Num() > 0);
	}

	// ========== 순차적으로 각 병사 유닛 처리 ==========
	while (AIPlayer->CurrentCombatUnitIndex < AIPlayer->CombatUnitsQueue.Num())
	{
		// 현재 처리할 유닛 가져오기
		AUnitCharacterBase* CombatUnit = AIPlayer->CombatUnitsQueue[AIPlayer->CurrentCombatUnitIndex].Get();
		
		// 유닛이 유효하지 않으면 스킵
		if (!CombatUnit || !IsValid(CombatUnit))
		{
			AIPlayer->CurrentCombatUnitIndex++;
			continue;
		}

		// 현재 위치 확인
		FVector2D CombatUnitPosition = FVector2D(-1, -1);
		if (!FindUnitPosition(CombatUnit, UnitManager, AllTiles, CombatUnitPosition))
		{
			AIPlayer->CurrentCombatUnitIndex++;
			continue;
		}

		// 공격 가능 여부 확인
		UUnitStatusComponent* StatusComp = CombatUnit->GetUnitStatusComponent();
		if (!StatusComp || !StatusComp->CanAttack())
		{
			AIPlayer->CurrentCombatUnitIndex++;
			continue;
		}

		// 사거리 계산
		int32 BaseRange = StatusComp->GetRange();
		int32 FinalRange = BaseRange;

		// 원거리 유닛(Range > 1)이면 Range 보너스 적용
		if (BaseRange > 1)
		{
			UUnitCombatComponent* CombatComp = CombatUnit->GetUnitCombatComponent();
			if (CombatComp)
			{
				int32 RangeBonus = CombatComp->CalculateRangeBonus(CombatUnitPosition);
				FinalRange += RangeBonus;
			}
		}

		// 현재 위치에서 사거리 내 적 유닛 탐지
		TArray<AUnitCharacterBase*> EnemyUnits = FindEnemyUnitsInRange(
			CombatUnitPosition, FinalRange, PlayerIndex, UnitManager, WorldComponent, DiplomacyManager
		);

		if (EnemyUnits.Num() > 0)
		{
			// 사거리 내 적 유닛이 있으면 즉시 공격 시도
			AUnitCharacterBase* TargetEnemy = EnemyUnits[0]; // 가장 가까운 적
			
			// 적 유닛의 위치 찾기
			FVector2D EnemyPosition = FVector2D(-1, -1);
			if (!FindUnitPosition(TargetEnemy, UnitManager, AllTiles, EnemyPosition))
			{
				AIPlayer->CurrentCombatUnitIndex++;
				continue;
			}

			// 전투 가능 여부 확인
			UUnitCombatComponent* CombatComp = CombatUnit->GetUnitCombatComponent();
			if (!CombatComp)
			{
				AIPlayer->CurrentCombatUnitIndex++;
				continue;
			}

			if (!CombatComp->CanExecuteCombat(CombatUnit, TargetEnemy, CombatUnitPosition, EnemyPosition))
			{
				AIPlayer->CurrentCombatUnitIndex++;
				continue;
			}

			// 전투 시작
			AIPlayer->bIsProcessingCombatUnit = true;

			// 전투 실행
			int32 HexDistance = WorldComponent->GetHexDistance(CombatUnitPosition, EnemyPosition);
			FCombatResult CombatResult = CombatComp->ExecuteCombat(CombatUnit, TargetEnemy, HexDistance, CombatUnitPosition, EnemyPosition);

			// AI 전투 시각화: 연속 전투 시 이전 복귀 등이 정리되도록 0.3초 지연 후 시작
			UUnitVisualizationComponent* AttackerVisComp = CombatUnit->GetUnitVisualizationComponent();
			if (World)
			{
				World->GetTimerManager().ClearTimer(DelayedCombatStartTimerHandle);
				FTimerDelegate DelayedStartDel;
				DelayedStartDel.BindLambda([this, PlayerIndex, CombatUnit, TargetEnemy, CombatResult, CombatUnitPosition, EnemyPosition, AttackerVisComp, UnitManager, WorldComponent]()
				{
					if (!IsValid(CombatUnit))
					{
						TransitionToNextState(PlayerIndex);
						return;
					}
					if (AttackerVisComp)
					{
						if (!AttackerVisComp->GetWorldComponent())
						{
							AttackerVisComp->SetWorldComponent(WorldComponent);
						}
						AttackerVisComp->SetUnitManager(UnitManager);
						if (IsValid(TargetEnemy))
						{
							AttackerVisComp->StartCombatVisualization(CombatUnit, TargetEnemy, CombatResult);
						}
						else
						{
							UnitManager->OnCombatVisualizationComplete(CombatUnit, TargetEnemy, CombatResult, CombatUnitPosition, EnemyPosition);
						}
					}
					else
					{
						UnitManager->OnCombatVisualizationComplete(CombatUnit, TargetEnemy, CombatResult, CombatUnitPosition, EnemyPosition);
					}
					FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
					if (AIPlayer)
					{
						AIPlayer->PendingCombatActions++;
						AIPlayer->PendingPostAsyncState = EAITurnState::ProcessingCombatUnitCombat;
					}
					TransitionToNextState(PlayerIndex);
				});
				World->GetTimerManager().SetTimer(DelayedCombatStartTimerHandle, DelayedStartDel, 0.3f, false);
			}
			else
			{
				// World 없으면 지연 없이 즉시 실행
				if (AttackerVisComp)
				{
					if (!AttackerVisComp->GetWorldComponent()) AttackerVisComp->SetWorldComponent(WorldComponent);
					AttackerVisComp->SetUnitManager(UnitManager);
					AttackerVisComp->StartCombatVisualization(CombatUnit, TargetEnemy, CombatResult);
				}
				else
				{
					UnitManager->OnCombatVisualizationComplete(CombatUnit, TargetEnemy, CombatResult, CombatUnitPosition, EnemyPosition);
				}
				AIPlayer->PendingCombatActions++;
				AIPlayer->PendingPostAsyncState = EAITurnState::ProcessingCombatUnitCombat;
				TransitionToNextState(PlayerIndex);
			}
			return; // 여기서 종료, 콜백에서 재개
		}
		else
		{
			// 사거리 내 적 유닛이 없으면 적 도시 공격 시도
			TArray<FVector2D> EnemyCities = FindEnemyCitiesInRange(
				CombatUnitPosition, FinalRange, PlayerIndex, WorldComponent, DiplomacyManager, GameInstance
			);

			if (EnemyCities.Num() > 0)
			{
				// 사거리 내 적 도시가 있으면 가장 가까운 도시 공격 시도
				FVector2D TargetCityCoord = EnemyCities[0]; // 가장 가까운 적 도시

				// 도시 컴포넌트 가져오기
				UCityComponent* CityComponent = nullptr;
				int32 TotalPlayerCount = GameInstance->GetPlayerStateCount();
				for (int32 i = 0; i < TotalPlayerCount; i++)
				{
					if (ASuperPlayerState* OtherPlayerState = GameInstance->GetPlayerState(i))
					{
						if (OtherPlayerState->HasCity() && OtherPlayerState->GetCityCoordinate() == TargetCityCoord)
						{
							CityComponent = OtherPlayerState->GetCityComponent();
							break;
						}
					}
				}

				if (!CityComponent)
				{
					AIPlayer->CurrentCombatUnitIndex++;
					continue;
				}

				// 전투 가능 여부 확인
				UUnitCombatComponent* CombatComp = CombatUnit->GetUnitCombatComponent();
				if (!CombatComp)
				{
					AIPlayer->CurrentCombatUnitIndex++;
					continue;
				}

				if (!CombatComp->CanExecuteCombatAgainstCity(CombatUnit, CityComponent, CombatUnitPosition, TargetCityCoord))
				{
					AIPlayer->CurrentCombatUnitIndex++;
					continue;
				}

				// 전투 시작
				AIPlayer->bIsProcessingCombatUnit = true;

				// 전투 실행
				int32 HexDistance = WorldComponent->GetHexDistance(CombatUnitPosition, TargetCityCoord);
				FCombatResult CombatResult = CombatComp->ExecuteCombatAgainstCity(CombatUnit, CityComponent, HexDistance, CombatUnitPosition, TargetCityCoord);

				// AI 도시 공격 시각화: 0.3초 지연 후 시작
				UUnitVisualizationComponent* AttackerVisComp = CombatUnit->GetUnitVisualizationComponent();
				if (World)
				{
					World->GetTimerManager().ClearTimer(DelayedCombatStartTimerHandle);
					FTimerDelegate DelayedStartDel;
					DelayedStartDel.BindLambda([this, PlayerIndex, CombatUnit, CityComponent, TargetCityCoord, CombatResult, CombatUnitPosition, AttackerVisComp, UnitManager, WorldComponent]()
					{
						if (!IsValid(CombatUnit))
						{
							TransitionToNextState(PlayerIndex);
							return;
						}
						if (AttackerVisComp && IsValid(CityComponent))
						{
							if (!AttackerVisComp->GetWorldComponent())
							{
								AttackerVisComp->SetWorldComponent(WorldComponent);
							}
							AttackerVisComp->SetUnitManager(UnitManager);
							AttackerVisComp->StartCombatVisualizationAgainstCity(CombatUnit, CityComponent, TargetCityCoord, CombatResult);
						}
						else
						{
							UnitManager->OnCombatVisualizationComplete(CombatUnit, nullptr, CombatResult, CombatUnitPosition, TargetCityCoord);
						}
						FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
						if (AIPlayer)
						{
							AIPlayer->PendingCombatActions++;
							AIPlayer->PendingPostAsyncState = EAITurnState::ProcessingCombatUnitCombat;
						}
						TransitionToNextState(PlayerIndex);
					});
					World->GetTimerManager().SetTimer(DelayedCombatStartTimerHandle, DelayedStartDel, 0.3f, false);
				}
				else
				{
					if (AttackerVisComp)
					{
						if (!AttackerVisComp->GetWorldComponent()) AttackerVisComp->SetWorldComponent(WorldComponent);
						AttackerVisComp->SetUnitManager(UnitManager);
						AttackerVisComp->StartCombatVisualizationAgainstCity(CombatUnit, CityComponent, TargetCityCoord, CombatResult);
					}
					else
					{
						UnitManager->OnCombatVisualizationComplete(CombatUnit, nullptr, CombatResult, CombatUnitPosition, TargetCityCoord);
					}
					AIPlayer->PendingCombatActions++;
					AIPlayer->PendingPostAsyncState = EAITurnState::ProcessingCombatUnitCombat;
					TransitionToNextState(PlayerIndex);
				}
				return; // 여기서 종료, 콜백에서 재개
			}
			else
			{
				// 사거리 내 공격 가능한 적 유닛도 적 도시도 없으면 다음 유닛으로
				AIPlayer->CurrentCombatUnitIndex++;
				continue;
			}
		}
	}

	// ========== 모든 유닛 처리 완료 확인 ==========
	// while 루프가 끝났다는 것은 CurrentCombatUnitIndex >= CombatUnitsQueue.Num()이라는 의미
	// 이는 모든 유닛을 처리했다는 의미이므로 다음 상태로 전환
	// 하지만 전투가 시작되어 비동기 대기 중인 경우는 여기서 처리하지 않음 (콜백에서 처리)
	if (AIPlayer->CurrentCombatUnitIndex >= AIPlayer->CombatUnitsQueue.Num())
	{
		// 모든 유닛 처리 완료
		AIPlayer->bIsProcessingCombatUnit = false;
		AIPlayer->CurrentCombatUnitIndex = 0;
		AIPlayer->CombatUnitsQueue.Empty();

		// 다음 상태로 전환 (ProcessingCombatUnitCombat -> TurnComplete)
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);

		// 턴 완료 상태인데 타이머가 설정되지 않았을 수 있으므로, 여기서 직접 NextTurn 예약 (이중 보장)
		if (AIPlayer->CurrentState == EAITurnState::TurnComplete && AIPlayer->bIsTurnActive && World)
		{
			World->GetTimerManager().ClearTimer(NextTurnDelayTimerHandle);
			World->GetTimerManager().SetTimer(NextTurnDelayTimerHandle, this, &UAIPlayerManager::OnNextTurnDelayTimerExpired, 0.3f, false);
		}
	}
	// 아직 처리할 유닛이 남아있거나 전투가 시작되어 비동기 대기 중인 경우는 여기서 아무것도 하지 않음
	// (전투 완료 후 OnCombatActionFinished에서 다시 ProcessCombatUnitCombatState가 호출됨)
}

// ================= 전쟁 상태 헬퍼 함수 구현 =================

TArray<AUnitCharacterBase*> UAIPlayerManager::FindEnemyUnitsInRange(
	FVector2D CombatUnitPosition,
	int32 DetectionRange,
	int32 PlayerIndex,
	UUnitManager* UnitManager,
	UWorldComponent* WorldComponent,
	UDiplomacyManager* DiplomacyManager
)
{
	TArray<AUnitCharacterBase*> EnemyUnits;

	if (!UnitManager || !WorldComponent || !DiplomacyManager)
	{
		return EnemyUnits;
	}

	// 반경 내 타일 수집
	TArray<FVector2D> HexesInRadius = WorldComponent->GetHexesInRadius(CombatUnitPosition, DetectionRange);

	// 적 유닛과 거리를 함께 저장할 구조체
	struct FEnemyUnitWithDistance
	{
		AUnitCharacterBase* Unit;
		int32 Distance;
		FVector2D Position;

		FEnemyUnitWithDistance(AUnitCharacterBase* InUnit, int32 InDistance, FVector2D InPosition)
			: Unit(InUnit), Distance(InDistance), Position(InPosition) {}
	};

	TArray<FEnemyUnitWithDistance> EnemyUnitsWithDistance;

	// 각 타일에서 적 유닛 확인
	for (const FVector2D& HexCoord : HexesInRadius)
	{
		AUnitCharacterBase* UnitAtHex = UnitManager->GetUnitAtHex(HexCoord);
		if (!UnitAtHex)
		{
			continue;
		}

		// 적 유닛인지 확인
		int32 EnemyPlayerIndex = UnitAtHex->GetPlayerIndex();
		if (EnemyPlayerIndex == PlayerIndex)
		{
			continue; // 같은 플레이어 유닛은 건너뛰기
		}

		// 전쟁 관계인지 확인
		if (!DiplomacyManager->IsAtWar(PlayerIndex, EnemyPlayerIndex))
		{
			continue; // 전쟁 관계가 아니면 건너뛰기
		}

		// 거리 계산
		int32 Distance = WorldComponent->GetHexDistance(CombatUnitPosition, HexCoord);
		EnemyUnitsWithDistance.Add(FEnemyUnitWithDistance(UnitAtHex, Distance, HexCoord));
	}

	// 거리 가까운 순으로 정렬
	EnemyUnitsWithDistance.Sort([](const FEnemyUnitWithDistance& A, const FEnemyUnitWithDistance& B)
	{
		return A.Distance < B.Distance;
	});

	// 정렬된 유닛만 반환
	for (const FEnemyUnitWithDistance& EnemyData : EnemyUnitsWithDistance)
	{
		EnemyUnits.Add(EnemyData.Unit);
	}

	return EnemyUnits;
}

FVector2D UAIPlayerManager::FindClosestEnemyCity(
	int32 PlayerIndex,
	ASuperPlayerState* PlayerState,
	USuperGameInstance* GameInstance,
	UWorldComponent* WorldComponent,
	UDiplomacyManager* DiplomacyManager
)
{
	if (!PlayerState || !GameInstance || !WorldComponent || !DiplomacyManager)
	{
		return FVector2D(-1, -1);
	}

	// 내 도시 좌표 가져오기
	FVector2D MyCityCoord = PlayerState->GetCityCoordinate();
	if (!PlayerState->HasCity())
	{
		return FVector2D(-1, -1);
	}

	// 가장 가까운 적 도시 정보 저장
	FVector2D ClosestEnemyCityCoord = FVector2D(-1, -1);
	int32 ClosestDistance = INT32_MAX;
	TArray<FVector2D> CitiesWithSameDistance; // 거리가 같은 도시들

	// 모든 플레이어 확인 (동적)
	int32 TotalPlayerCount = GameInstance->GetPlayerStateCount();
	for (int32 OtherPlayerIndex = 0; OtherPlayerIndex < TotalPlayerCount; OtherPlayerIndex++)
	{
		if (OtherPlayerIndex == PlayerIndex)
		{
			continue; // 자신은 건너뛰기
		}

		// 전쟁 관계인지 확인
		if (!DiplomacyManager->IsAtWar(PlayerIndex, OtherPlayerIndex))
		{
			continue; // 전쟁 관계가 아니면 건너뛰기
		}

		// 적 플레이어 상태 가져오기
		ASuperPlayerState* EnemyPlayerState = GameInstance->GetPlayerState(OtherPlayerIndex);
		if (!EnemyPlayerState || !EnemyPlayerState->HasCity())
		{
			continue; // 도시가 없으면 건너뛰기
		}

		// 적 도시 좌표 가져오기
		FVector2D EnemyCityCoord = EnemyPlayerState->GetCityCoordinate();

		// 거리 계산
		int32 Distance = WorldComponent->GetHexDistance(MyCityCoord, EnemyCityCoord);

		// 가장 가까운 도시 업데이트
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestEnemyCityCoord = EnemyCityCoord;
			CitiesWithSameDistance.Empty();
			CitiesWithSameDistance.Add(EnemyCityCoord);
		}
		else if (Distance == ClosestDistance)
		{
			// 거리가 같으면 배열에 추가
			CitiesWithSameDistance.Add(EnemyCityCoord);
		}
	}

	// 적 도시가 없으면 반환
	if (ClosestEnemyCityCoord == FVector2D(-1, -1))
	{
		return FVector2D(-1, -1);
	}

	// 거리가 같은 도시가 여러 개 있으면 랜덤 선택
	if (CitiesWithSameDistance.Num() > 1)
	{
		int32 RandomIndex = FMath::RandRange(0, CitiesWithSameDistance.Num() - 1);
		return CitiesWithSameDistance[RandomIndex];
	}

	return ClosestEnemyCityCoord;
}

TArray<FVector2D> UAIPlayerManager::FindEnemyCitiesInRange(
	FVector2D CombatUnitPosition,
	int32 AttackRange,
	int32 PlayerIndex,
	UWorldComponent* WorldComponent,
	UDiplomacyManager* DiplomacyManager,
	USuperGameInstance* GameInstance
)
{
	TArray<FVector2D> EnemyCities;

	if (!WorldComponent || !DiplomacyManager || !GameInstance)
	{
		return EnemyCities;
	}

	// 반경 내 타일 수집
	TArray<FVector2D> HexesInRadius = WorldComponent->GetHexesInRadius(CombatUnitPosition, AttackRange);

	// 적 도시와 거리를 함께 저장할 구조체
	struct FEnemyCityWithDistance
	{
		FVector2D CityCoord;
		int32 Distance;

		FEnemyCityWithDistance(FVector2D InCityCoord, int32 InDistance)
			: CityCoord(InCityCoord), Distance(InDistance) {}
	};

	TArray<FEnemyCityWithDistance> EnemyCitiesWithDistance;

	// 각 타일에서 적 도시 확인
	for (const FVector2D& HexCoord : HexesInRadius)
	{
		// 도시가 있는지 확인
		if (!WorldComponent->IsCityAtHex(HexCoord))
		{
			continue;
		}

		// 도시 소유 플레이어 찾기
		int32 CityOwnerIndex = -1;
		int32 TotalPlayerCount = GameInstance->GetPlayerStateCount();
		for (int32 i = 0; i < TotalPlayerCount; i++)
		{
			if (ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(i))
			{
				if (PlayerState->HasCity() && PlayerState->GetCityCoordinate() == HexCoord)
				{
					CityOwnerIndex = i;
					break;
				}
			}
		}

		// 도시 소유자를 찾을 수 없으면 건너뛰기
		if (CityOwnerIndex == -1)
		{
			continue;
		}

		// 자신의 도시는 건너뛰기
		if (CityOwnerIndex == PlayerIndex)
		{
			continue;
		}

		// 전쟁 관계인지 확인
		if (!DiplomacyManager->IsAtWar(PlayerIndex, CityOwnerIndex))
		{
			continue; // 전쟁 관계가 아니면 건너뛰기
		}

		// 거리 계산
		int32 Distance = WorldComponent->GetHexDistance(CombatUnitPosition, HexCoord);
		EnemyCitiesWithDistance.Add(FEnemyCityWithDistance(HexCoord, Distance));
	}

	// 거리 가까운 순으로 정렬
	EnemyCitiesWithDistance.Sort([](const FEnemyCityWithDistance& A, const FEnemyCityWithDistance& B)
	{
		return A.Distance < B.Distance;
	});

	// 정렬된 도시만 반환
	for (const FEnemyCityWithDistance& CityData : EnemyCitiesWithDistance)
	{
		EnemyCities.Add(CityData.CityCoord);
	}

	return EnemyCities;
}

void UAIPlayerManager::ProcessUnitMovementState(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer)
	{
		return;
	}

	// 필요한 매니저들 가져오기
	if (!AIPlayer->PlayerStateRef.IsValid())
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	ASuperPlayerState* PlayerState = AIPlayer->PlayerStateRef.Get();
	if (!PlayerState)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance());
	if (!GameInstance)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	UWorldComponent* WorldComponent = GameInstance->GetGeneratedWorldComponent();
	UUnitManager* UnitManager = GameInstance->GetUnitManager();

	if (!WorldComponent || !UnitManager)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// ProcessUnitMovementState는 이제 사용되지 않음
	// 병사 유닛 이동 및 전투는 ProcessingCombatUnitMovement와 ProcessingCombatUnitCombat로 분리됨
	// 다음 상태로 전환
	TransitionToNextState(PlayerIndex);
	UpdateStateMachine(PlayerIndex);
}

// ================= 유닛 이동 헬퍼 함수 구현 =================

bool UAIPlayerManager::FindUnitPosition(
	AUnitCharacterBase* Unit,
	UUnitManager* UnitManager,
	const TArray<UWorldTile*>& AllTiles,
	FVector2D& OutPosition
)
{
	if (!Unit || !UnitManager)
	{
		return false;
	}

	// 모든 타일을 순회하면서 유닛 위치 찾기
	for (UWorldTile* Tile : AllTiles)
	{
		if (!Tile)
		{
			continue;
		}
		FVector2D TileCoord = Tile->GetGridPosition();
		if (UnitManager->GetUnitAtHex(TileCoord) == Unit)
		{
			OutPosition = TileCoord;
			return true;
		}
	}

	return false;
}

bool UAIPlayerManager::TryMoveUnitToTile(
	AUnitCharacterBase* Unit,
	FVector2D StartPosition,
	FVector2D TargetTile,
	UUnitManager* UnitManager,
	int32& OutPendingMovements
)
{
	if (!Unit || !UnitManager)
	{
		return false;
	}

	// 유닛의 남은 이동력 가져오기
	int32 MaxMovementCost = UnitManager->GetUnitRemainingMovement(Unit);

	// 경로 찾기 (국경선 적용)
	TArray<FVector2D> Path = UnitManager->FindPathWithMovementCost(
		StartPosition,
		TargetTile,
		MaxMovementCost,
		Unit->GetPlayerIndex()
	);

	// 경로가 있으면 이동 시작
	if (Path.Num() > 1)
	{
		// 실제 도착 타일 (경로의 마지막 타일 = 실제 도착 타일)
		// 이동력 부족으로 경로가 잘렸을 경우를 대비하여 실제 도착 타일을 사용
		FVector2D DestinationTile = Path[Path.Num() - 1];
		
		// 경로를 찾은 후, 실제 이동 시작 전에 목적지 타일이 여전히 사용 가능한지 최종 확인
		// 자기 자신의 예약은 무시하도록 Unit 파라미터 전달
		if (!UnitManager->CanPlaceUnitAtHex(DestinationTile, Unit))
		{
			// 목적지가 더 이상 사용 불가능하면 이동 취소
			return false;
		}
		
		// AI 이동은 시각화 없이 즉시 적용
		UnitManager->StartMovementImmediate(Unit, Path);
		OutPendingMovements++;
		
		return true;
	}

	return false;
}

FVector2D UAIPlayerManager::FindValidWanderTile(
	int32 PlayerIndex,
	int32 Radius,
	int32 MaxAttempts
)
{
	// ========== 초기 설정 (한 번만 수행) ==========
	// AI 플레이어 가져오기
	const FAIPlayerStruct* AIPlayer = AIPlayers.Find(PlayerIndex);
	if (!AIPlayer || !AIPlayer->PlayerStateRef.IsValid())
	{
		return FVector2D(-1, -1);
	}

	ASuperPlayerState* PlayerState = AIPlayer->PlayerStateRef.Get();
	if (!PlayerState)
	{
		return FVector2D(-1, -1);
	}

	// WorldComponent 가져오기
	UWorldComponent* WorldComponent = nullptr;
	USuperGameInstance* GameInstance = nullptr;
	if (UWorld* World = GetWorld())
	{
		GameInstance = Cast<USuperGameInstance>(World->GetGameInstance());
		if (GameInstance)
		{
			WorldComponent = GameInstance->GetGeneratedWorldComponent();
		}
	}

	if (!WorldComponent || !GameInstance)
	{
		return FVector2D(-1, -1);
	}

	// 내가 소유한 모든 타일 가져오기
	TArray<UWorldTile*> OwnedTiles = PlayerState->GetOwnedTiles(WorldComponent);
	if (OwnedTiles.Num() == 0)
	{
		return FVector2D(-1, -1);
	}

	// 각 소유 타일 기준으로 반경 내 타일 수집 (중복 제거를 위해 TSet 사용)
	TSet<FVector2D> HexesInRadiusSet;
	for (UWorldTile* OwnedTile : OwnedTiles)
	{
		if (!OwnedTile)
		{
			continue;
		}

		FVector2D OwnedTileCoord = OwnedTile->GetGridPosition();
		TArray<FVector2D> HexesAroundOwnedTile = WorldComponent->GetHexesInRadius(OwnedTileCoord, Radius);

		// TSet에 추가하여 자동으로 중복 제거
		for (const FVector2D& HexCoord : HexesAroundOwnedTile)
		{
			HexesInRadiusSet.Add(HexCoord);
		}
	}

	// TSet을 TArray로 변환
	TArray<FVector2D> HexesInRadius = HexesInRadiusSet.Array();

	// 바다 타일 및 다른 플레이어 소유 타일 제외하고 유효한 타일만 필터링
	TArray<FVector2D> ValidTiles;
	for (const FVector2D& HexCoord : HexesInRadius)
	{
		if (UWorldTile* Tile = WorldComponent->GetTileAtHex(HexCoord))
		{
			// 바다 타일 제외
			if (Tile->GetTerrainType() == ETerrainType::Ocean)
			{
				continue;
			}

			// 다른 플레이어가 소유한 타일인지 확인
			bool bIsOwnedByOtherPlayer = false;
			for (int32 i = 0; i < GameInstance->GetPlayerStateCount(); i++)
			{
				if (i == PlayerIndex)
				{
					continue; // 현재 플레이어는 건너뛰기
				}

				if (ASuperPlayerState* OtherPlayerState = GameInstance->GetPlayerState(i))
				{
					if (OtherPlayerState->IsTileOwned(HexCoord))
					{
						bIsOwnedByOtherPlayer = true;
						break;
					}
				}
			}

			// 다른 플레이어가 소유하지 않은 타일만 추가
			if (!bIsOwnedByOtherPlayer)
			{
				ValidTiles.Add(HexCoord);
			}
		}
	}

	// 유효한 타일이 없으면 반환
	if (ValidTiles.Num() == 0)
	{
		return FVector2D(-1, -1);
	}

	// ========== 배회 타일 찾기 (반복 시도) ==========
	// 최대 시도 횟수(MaxAttempts)만큼 반복하여 배회 타일 찾기
	for (int32 Attempt = 0; Attempt < MaxAttempts; Attempt++)
	{
		// 유효한 타일 중 랜덤 선택
		int32 RandomIndex = FMath::RandRange(0, ValidTiles.Num() - 1);
		FVector2D CandidateTile = ValidTiles[RandomIndex];
		
		// 유효한 타일이면 반환
		return CandidateTile;
	}

	// 모든 시도 실패 시 무효 좌표 반환
	return FVector2D(-1, -1);
}

