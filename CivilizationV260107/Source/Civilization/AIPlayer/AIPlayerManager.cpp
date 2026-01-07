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
			// AI 플레이어는 PlayerIndex 1~3
			for (int32 PlayerIndex = 1; PlayerIndex <= 3; PlayerIndex++)
			{
				if (ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(PlayerIndex))
				{
					RegisterAIPlayer(PlayerIndex, PlayerState);
				}
			}

			//// ========== 디버깅용: AI 플레이어 1, 2, 3이 서로에게 전쟁 선포 ==========
			//UDiplomacyManager* DiplomacyManager = GameInstance->GetDiplomacyManager();
			//if (DiplomacyManager)
			//{
			//	// TurnComponent에서 현재 라운드 가져오기
			//	int32 CurrentRound = 1; // 기본값
			//	if (ASuperGameModeBase* SuperGameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
			//	{
			//		if (UTurnComponent* TurnComponent = SuperGameMode->GetTurnComponent())
			//		{
			//			CurrentRound = TurnComponent->GetCurrentRoundNumber();
			//		}
			//	}

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
	AIPlayer.TargetFacilityTile = FVector2D(-1, -1); // 명시적으로 초기값 설정

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
	AIPlayer.TargetFacilityTile = FVector2D(-1, -1); // 명시적으로 초기값 설정
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

		case EAITurnState::ProcessingUnitMovement:
			ProcessUnitMovementState(PlayerIndex);
			break;

		case EAITurnState::WaitingForAsync:
			// 비동기 작업 대기 중
			// 모든 비동기 작업이 완료되면 다음 상태로 전환
			TransitionToNextState(PlayerIndex);
			// 전환 후 즉시 상태 머신 업데이트 (재귀 호출)
			UpdateStateMachine(PlayerIndex);
			break;

		case EAITurnState::TurnComplete:
			// 턴 완료 상태에서는 SuperGameModeBase에 NextTurn 호출 요청
			if (UWorld* World = GetWorld())
			{
				if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
				{
					GameMode->NextTurn();
				}
			}
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
	
	// 턴이 활성화되어 있고, 비동기 작업이 없으면 상태 머신 업데이트
	if (AIPlayer->bIsTurnActive && !HasPendingAsyncWork(PlayerIndex))
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
	
	// 턴이 활성화되어 있고, 비동기 작업이 없으면 상태 머신 업데이트
	if (AIPlayer->bIsTurnActive && !HasPendingAsyncWork(PlayerIndex))
	{
		UpdateStateMachine(PlayerIndex);
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

	// DiplomacyManager 가져오기
	UDiplomacyManager* DiplomacyManager = nullptr;
	int32 CurrentRound = 1;
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			DiplomacyManager = GameInstance->GetDiplomacyManager();

			// 현재 라운드 가져오기
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
	}

	if (!DiplomacyManager)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

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
					// 호감도 >= 0이면 즉시 수락
					if (Attitude >= 0)
					{
						bAccepted = true;
					}
					// 전쟁 지속이 15라운드 이상이면 25% 확률로 수락
					else
					{
						FDiplomacyPairKey PairKey(PlayerIndex, FromPlayerId);
						if (const FDiplomacyPairState* PairState = DiplomacyManager->PairStates.Find(PairKey))
						{
							if (PairState->LastWarRound > 0)
							{
								int32 WarDuration = CurrentRound - PairState->LastWarRound;
								if (WarDuration >= 15)
								{
									// 25% 확률로 수락
									float RandomChance = FMath::RandRange(0.0f, 1.0f);
									if (RandomChance <= 0.25f)
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
	// 플레이어 인덱스: 0=Player, 1~3=AI
	for (int32 OtherPlayerIndex = 0; OtherPlayerIndex < 4; OtherPlayerIndex++)
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

		// 2. 호감도가 50 이상 → OfferAlliance
		// 플레이어(0)에게는 응답이 필요한 액션 발행하지 않음
		if (Attitude >= 50 && OtherPlayerIndex != 0)
		{
			// 이미 동맹 상태가 아니면 동맹 제안
			if (CurrentStatus != EDiplomacyStatusType::Alliance)
			{
				FDiplomacyAction AllianceAction;
				AllianceAction.Action = EDiplomacyActionType::OfferAlliance;
				AllianceAction.FromPlayerId = PlayerIndex;
				AllianceAction.ToPlayerId = OtherPlayerIndex;
				DiplomacyManager->IssueAction(AllianceAction);
				continue; // 한 플레이어당 하나의 액션만 수행
			}
		}

		// 3. 전쟁 중 상태가 15라운드 지나면 OfferPeace
		// 플레이어(0)에게는 응답이 필요한 액션 발행하지 않음
		if (CurrentStatus == EDiplomacyStatusType::War && OtherPlayerIndex != 0)
		{
			// PairStates에서 LastWarRound 확인
			FDiplomacyPairKey PairKey(PlayerIndex, OtherPlayerIndex);
			if (const FDiplomacyPairState* PairState = DiplomacyManager->PairStates.Find(PairKey))
			{
				if (PairState->LastWarRound > 0)
				{
					int32 WarDuration = CurrentRound - PairState->LastWarRound;
					if (WarDuration >= 15)
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

	// 4. 매 라운드 10% 확률로 랜덤 플레이어에게 Denounce 또는 SendGift
	float RandomChance = FMath::RandRange(0.0f, 1.0f);
	if (RandomChance <= 0.1f) // 10% 확률
	{
		// 랜덤 플레이어 선택
		TArray<int32> AvailablePlayers;
		for (int32 i = 0; i < 4; i++)
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

	// 이미 목표 타일이 설정되어 있고, 그 타일에 시설이 건설되지 않았으면 갱신하지 않음
	// TargetFacilityTile이 초기값(-1, -1)이 아닌지 확인 (헥스 좌표계에서는 음수 좌표가 정상이므로 초기값과 비교)
	bool bHasTargetFacilityTile = (AIPlayer->TargetFacilityTile != FVector2D(-1, -1));
	
		if (bHasTargetFacilityTile)
		{
			// 목표 타일이 여전히 유효한지 확인 (시설이 건설되지 않았는지)
			if (!FacilityManager->HasFacilityAtTile(AIPlayer->TargetFacilityTile))
			{
				// 목표 타일이 유효하면 갱신하지 않고 다음 상태로 전환
				TransitionToNextState(PlayerIndex);
				UpdateStateMachine(PlayerIndex);
				return;
			}
		// 시설이 건설되었으면 목표 타일 초기화하고 새로 찾기
		AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
	}

	// 보유 타일 가져오기
	TArray<UWorldTile*> OwnedTiles = PlayerState->GetOwnedTiles(WorldComponent);

	// 시설이 없는 타일 중 산출량이 높은 타일 찾기
	TArray<FTileWithTotalYield> ValidTiles;
	for (UWorldTile* Tile : OwnedTiles)
	{
		if (!Tile)
		{
			continue;
		}

		FVector2D TileCoord = Tile->GetGridPosition();

		// 시설이 이미 건설되어 있는지 확인
		if (FacilityManager->HasFacilityAtTile(TileCoord))
		{
			continue;
		}

		// 총 산출량 계산 (생산량 + 식량 + 과학력 + 골드)
		int32 ProductionYield = Tile->GetTotalProductionYield();
		int32 FoodYield = Tile->GetTotalFoodYield();
		int32 ScienceYield = Tile->GetTotalScienceYield();
		int32 GoldYield = Tile->GetTotalGoldYield();
		int32 TotalYield = ProductionYield + FoodYield + ScienceYield + GoldYield;

		ValidTiles.Add(FTileWithTotalYield(TileCoord, TotalYield));
	}

	// 시설이 없는 타일이 없으면 TargetFacilityTile을 지정하지 않고 넘어감
	if (ValidTiles.Num() == 0)
	{
		TransitionToNextState(PlayerIndex);
		UpdateStateMachine(PlayerIndex);
		return;
	}

	// 총 산출량 기준으로 정렬 (내림차순)
	ValidTiles.Sort(FTileWithTotalYield::CompareDescending);

	// 산출량이 가장 높은 타일 선택
	FVector2D SelectedTile = ValidTiles[0].Coordinate;

	// 목표 타일 위치 저장
	AIPlayer->TargetFacilityTile = SelectedTile;

	// 다음 상태로 전환
	TransitionToNextState(PlayerIndex);
	
	// 상태 전환 후 상태 머신 업데이트 (다음 상태 처리)
	UpdateStateMachine(PlayerIndex);
}

void UAIPlayerManager::ProcessBuilderUnitMovement(
	int32 PlayerIndex,
	FAIPlayerStruct* AIPlayer,
	ASuperPlayerState* PlayerState,
	USuperGameInstance* GameInstance,
	UWorldComponent* WorldComponent,
	UUnitManager* UnitManager,
	const TArray<UWorldTile*>& AllTiles,
	FReservedTiles& ReservedTiles
)
{
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
		return;
	}

	// ========== 건설자 위치 확인 ==========
	FVector2D BuilderPosition = FVector2D(-1, -1);
	if (!FindUnitPosition(BuilderUnit, UnitManager, AllTiles, BuilderPosition))
	{
		return;
	}

	// ========== 목표 시설 타일 확인 ==========
	// 주의: 헥스 좌표계에서는 음수 좌표가 정상이므로, 초기값(-1, -1)과 비교하여 확인
	bool bHasTargetFacilityTile = (AIPlayer->TargetFacilityTile != FVector2D(-1, -1));
	if (!bHasTargetFacilityTile)
	{
		return;
	}

	// ========== 시설 건설 처리 ==========
	// 건설자가 목표 타일에 도착한 경우
	if (BuilderPosition == AIPlayer->TargetFacilityTile)
	{
		// 필요한 매니저 및 컴포넌트 가져오기
		UFacilityManager* FacilityManager = GameInstance->GetFacilityManager();
		if (!FacilityManager || !PlayerState)
		{
			AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
			return;
		}

		// 목표 타일 가져오기
		UWorldTile* TargetTile = WorldComponent->GetTileAtHex(AIPlayer->TargetFacilityTile);
		if (!TargetTile)
		{
			AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
			return;
		}

		// 기술 조건 필터링: 건설 가능한 시설 목록 가져오기
		TArray<FName> AvailableFacilities = PlayerState->GetAvailableFacilities();
		if (AvailableFacilities.Num() == 0)
		{
			AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
			return;
		}

		// 타일 조건 필터링: CanBuildFacilityOnTile()로 건설 가능한 시설 찾기
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
			return;
		}

		// 건설 가능한 시설 중 랜덤 선택
		int32 RandomIndex = FMath::RandRange(0, BuildableFacilities.Num() - 1);
		FName SelectedFacilityRowName = BuildableFacilities[RandomIndex];

		// 시설 건설 시도 (성공/실패와 관계없이 목표 타일 초기화)
		PlayerState->BuildFacility(SelectedFacilityRowName, AIPlayer->TargetFacilityTile);
		AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
		return;
	}

	// ========== 건설자 이동 처리 ==========
	// 이동 불가능한 경우 랜덤 배회
	if (!BuilderUnit->GetUnitStatusComponent()->CanMove())
	{
		FVector2D WanderTargetTile = FindValidWanderTile(PlayerIndex, ReservedTiles, 2, 10);
		if (WanderTargetTile != FVector2D(-1, -1))
		{
			TryMoveUnitToTile(BuilderUnit, BuilderPosition, WanderTargetTile, UnitManager, ReservedTiles, AIPlayer->PendingUnitMovements);
		}

		AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
		return;
	}

	// 이동 가능한 경우 목표 타일로 이동 시도
	// 목표 타일을 임시로 예약 (경로를 찾기 전에)
	// 경로를 찾은 후에는 실제 도착 타일로 업데이트됨
	ReservedTiles.Add(AIPlayer->TargetFacilityTile);

	if (TryMoveUnitToTile(BuilderUnit, BuilderPosition, AIPlayer->TargetFacilityTile, UnitManager, ReservedTiles, AIPlayer->PendingUnitMovements))
	{
		return;
	}

	// 경로가 없으면 예약 해제하고 랜덤 배회
	ReservedTiles.Remove(AIPlayer->TargetFacilityTile);
	
	FVector2D WanderTargetTile = FindValidWanderTile(PlayerIndex, ReservedTiles, 2, 10);
	if (WanderTargetTile != FVector2D(-1, -1))
	{
		TryMoveUnitToTile(BuilderUnit, BuilderPosition, WanderTargetTile, UnitManager, ReservedTiles, AIPlayer->PendingUnitMovements);
	}

	AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
}

void UAIPlayerManager::ProcessCombatUnitsMovement(
	int32 PlayerIndex,
	FAIPlayerStruct* AIPlayer,
	ASuperPlayerState* PlayerState,
	USuperGameInstance* GameInstance,
	UWorldComponent* WorldComponent,
	UUnitManager* UnitManager,
	const TArray<UWorldTile*>& AllTiles,
	const TArray<AUnitCharacterBase*>& AllUnits,
	FReservedTiles& ReservedTiles
)
{
	// ========== 전쟁 상태 확인 ==========
	UDiplomacyManager* DiplomacyManager = GameInstance->GetDiplomacyManager();
	bool bIsAtWar = false;
	if (DiplomacyManager)
	{
		// 모든 다른 플레이어 확인 (0, 1, 2, 3 중 PlayerIndex가 아닌 것들)
		for (int32 OtherPlayerIndex = 0; OtherPlayerIndex < 4; OtherPlayerIndex++)
		{
			if (OtherPlayerIndex != PlayerIndex && 
				DiplomacyManager->IsAtWar(PlayerIndex, OtherPlayerIndex))
			{
				bIsAtWar = true;
				break;
			}
		}
	}

	// ========== 병사 유닛 찾기 ==========
	TArray<AUnitCharacterBase*> CombatUnits;
	for (AUnitCharacterBase* Unit : AllUnits)
	{
		if (Unit && 
			Unit->GetPlayerIndex() == PlayerIndex && 
			UnitManager->IsCombatUnit(Unit))
		{
			CombatUnits.Add(Unit);
		}
	}

	// ========== 각 병사 유닛 처리 ==========
	for (AUnitCharacterBase* CombatUnit : CombatUnits)
	{
		// 현재 위치 확인
		FVector2D CombatUnitPosition = FVector2D(-1, -1);
		if (!FindUnitPosition(CombatUnit, UnitManager, AllTiles, CombatUnitPosition))
		{
			continue;
		}

		// 이동 가능 여부 확인
		if (!CombatUnit->GetUnitStatusComponent()->CanMove())
		{
			continue;
		}

		// 전쟁 중이 아니면 평화 상태 로직 (기존)
		if (!bIsAtWar)
		{
			// 랜덤 배회 타일 선택
			FVector2D WanderTargetTile = FindValidWanderTile(PlayerIndex, ReservedTiles, 2, 10);
			if (WanderTargetTile != FVector2D(-1, -1))
			{
				TryMoveUnitToTile(CombatUnit, CombatUnitPosition, WanderTargetTile, UnitManager, ReservedTiles, AIPlayer->PendingUnitMovements);
			}
			continue;
		}

		// ========== 전쟁 상태 로직 ==========
		// 4칸 반경 내 적 유닛 탐지
		TArray<AUnitCharacterBase*> EnemyUnits = FindEnemyUnitsInRange(
			CombatUnitPosition, 4, PlayerIndex, UnitManager, WorldComponent, DiplomacyManager
		);

		if (EnemyUnits.Num() > 0)
		{
			// 적 유닛이 있으면 전투 우선
			AUnitCharacterBase* TargetEnemy = EnemyUnits[0]; // 가장 가까운 적
			
			// 적 유닛의 위치 찾기
			FVector2D EnemyPosition = FVector2D(-1, -1);
			if (!FindUnitPosition(TargetEnemy, UnitManager, AllTiles, EnemyPosition))
			{
				continue;
			}

			// 이동 및 공격 시도
			TryMoveAndAttackEnemy(
				CombatUnit, CombatUnitPosition, TargetEnemy, EnemyPosition,
				UnitManager, WorldComponent, ReservedTiles,
				AIPlayer->PendingUnitMovements, AIPlayer->PendingCombatActions
			);
		}
		else
		{
			// 적 유닛이 없으면 가장 가까운 적 도시로 이동
			FVector2D EnemyCityCoord = FindClosestEnemyCity(
				PlayerIndex, PlayerState, GameInstance, WorldComponent, DiplomacyManager
			);
			if (EnemyCityCoord != FVector2D(-1, -1))
			{
				TryMoveUnitToTile(CombatUnit, CombatUnitPosition, EnemyCityCoord, UnitManager, ReservedTiles, AIPlayer->PendingUnitMovements);
			}
		}
	}
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

	// 모든 플레이어 확인 (0, 1, 2, 3)
	for (int32 OtherPlayerIndex = 0; OtherPlayerIndex < 4; OtherPlayerIndex++)
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

bool UAIPlayerManager::TryMoveAndAttackEnemy(
	AUnitCharacterBase* CombatUnit,
	FVector2D CombatUnitPosition,
	AUnitCharacterBase* EnemyUnit,
	FVector2D EnemyPosition,
	UUnitManager* UnitManager,
	UWorldComponent* WorldComponent,
	FReservedTiles& ReservedTiles,
	int32& OutPendingMovements,
	int32& OutPendingCombatActions
)
{
	if (!CombatUnit || !EnemyUnit || !UnitManager || !WorldComponent)
	{
		return false;
	}

	// 공격 가능 여부 확인
	UUnitStatusComponent* StatusComp = CombatUnit->GetUnitStatusComponent();
	if (!StatusComp || !StatusComp->CanAttack())
	{
		return false;
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

	// 현재 거리 확인
	int32 CurrentDistance = WorldComponent->GetHexDistance(CombatUnitPosition, EnemyPosition);

	// 사거리 내인지 확인
	if (CurrentDistance <= FinalRange)
	{
		// 바로 공격 가능
		// 전투 가능 여부 확인
		UUnitCombatComponent* CombatComp = CombatUnit->GetUnitCombatComponent();
		if (!CombatComp)
		{
			return false;
		}

		if (!CombatComp->CanExecuteCombat(CombatUnit, EnemyUnit, CombatUnitPosition, EnemyPosition))
		{
			return false;
		}

		// 전투 실행 (UnitManager의 ExecuteCombatBetweenSelectedUnits 로직 재사용)
		// 거리 계산
		int32 HexDistance = WorldComponent->GetHexDistance(CombatUnitPosition, EnemyPosition);

		// 전투 계산 즉시 실행 (데미지 적용 포함)
		FCombatResult CombatResult = CombatComp->ExecuteCombat(CombatUnit, EnemyUnit, HexDistance, CombatUnitPosition, EnemyPosition);

		// 공격자의 UnitVisualizationComponent 가져오기
		UUnitVisualizationComponent* AttackerVisComp = CombatUnit->GetUnitVisualizationComponent();
		if (AttackerVisComp)
		{
			// WorldComponent 설정 (없으면)
			if (!AttackerVisComp->GetWorldComponent())
			{
				AttackerVisComp->SetWorldComponent(WorldComponent);
			}

			// UnitManager 설정
			AttackerVisComp->SetUnitManager(UnitManager);

			// 전투 시각화 시작
			AttackerVisComp->StartCombatVisualization(CombatUnit, EnemyUnit, CombatResult);
		}
		else
		{
			// UnitVisualizationComponent가 없으면 즉시 결과 처리 (폴백)
			UnitManager->OnCombatVisualizationComplete(CombatUnit, EnemyUnit, CombatResult, CombatUnitPosition, EnemyPosition);
		}

		// PendingCombatActions 증가
		OutPendingCombatActions++;
		return true;
	}
	else
	{
		// 이동 후 공격 필요
		// 이동 목표 타일 선택: 사거리 내 타일 중 현재 위치에서 가장 가까운 타일
		TArray<FVector2D> HexesInRange = WorldComponent->GetHexesInRadius(EnemyPosition, FinalRange);

		// 유닛의 남은 이동력 가져오기
		int32 MaxMovementCost = UnitManager->GetUnitRemainingMovement(CombatUnit);

		FVector2D BestTargetTile = FVector2D(-1, -1);
		int32 BestDistance = INT32_MAX;

		// 각 후보 타일 확인
		for (const FVector2D& CandidateTile : HexesInRange)
		{
			// 타일이 이동 가능한지 확인
			if (!UnitManager->CanPlaceUnitAtHex(CandidateTile))
			{
				continue;
			}

			// 경로 찾기
			TArray<FVector2D> Path = UnitManager->FindPathWithMovementCost(CombatUnitPosition, CandidateTile, MaxMovementCost);
			if (Path.Num() <= 1)
			{
				continue; // 경로가 없으면 건너뛰기
			}

			// 현재 위치에서의 거리 계산
			int32 Distance = WorldComponent->GetHexDistance(CombatUnitPosition, CandidateTile);
			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				BestTargetTile = CandidateTile;
			}
		}

		// 이동 목표 타일이 없으면 실패
		if (BestTargetTile == FVector2D(-1, -1))
		{
			return false;
		}

		// 이동 실행
		ReservedTiles.Add(BestTargetTile);
		if (TryMoveUnitToTile(CombatUnit, CombatUnitPosition, BestTargetTile, UnitManager, ReservedTiles, OutPendingMovements))
		{
			// 이동 후 공격은 비동기이므로, 이동 완료 후 처리하거나 다음 턴에 처리
			// 현재는 이동만 실행
			return true;
		}
		else
		{
			// 이동 실패 시 예약 해제
			ReservedTiles.Remove(BestTargetTile);
			return false;
		}
	}
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

	// 공통 데이터 준비
	TArray<UWorldTile*> AllTiles = WorldComponent->GetAllTiles();
	TArray<AUnitCharacterBase*> AllUnits = UnitManager->GetAllUnits();
	
	// 예약된 타일 추적 시스템
	// 이미 이동을 시작한 유닛들의 목표 타일(실제 도착 타일)을 추적하여 중복 이동 방지
	FReservedTiles ReservedTiles;

	// 건설자 유닛 이동 처리
	ProcessBuilderUnitMovement(
		PlayerIndex,
		AIPlayer,
		PlayerState,
		GameInstance,
		WorldComponent,
		UnitManager,
		AllTiles,
		ReservedTiles
	);

	// 병사 유닛 이동 처리
	ProcessCombatUnitsMovement(
		PlayerIndex,
		AIPlayer,
		PlayerState,
		GameInstance,
		WorldComponent,
		UnitManager,
		AllTiles,
		AllUnits,
		ReservedTiles
	);

	// 모든 처리 완료
	// 이동을 시작한 유닛이 있으면 WaitingForAsync로 전환, 없으면 다음 상태로
	if (AIPlayer->PendingUnitMovements > 0)
	{
		AIPlayer->CurrentState = EAITurnState::WaitingForAsync;
		// 비동기 작업이 완료되면 콜백에서 UpdateStateMachine이 호출됨
	}
	else
	{
		TransitionToNextState(PlayerIndex);
		// 상태 전환 후 상태 머신 업데이트 (다음 상태 처리)
		UpdateStateMachine(PlayerIndex);
	}
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
	FReservedTiles& ReservedTiles,
	int32& OutPendingMovements
)
{
	if (!Unit || !UnitManager)
	{
		return false;
	}

	// 유닛의 남은 이동력 가져오기
	int32 MaxMovementCost = UnitManager->GetUnitRemainingMovement(Unit);

	// 경로 찾기
	TArray<FVector2D> Path = UnitManager->FindPathWithMovementCost(
		StartPosition,
		TargetTile,
		MaxMovementCost
	);

	// 경로가 있으면 이동 시작
	if (Path.Num() > 1)
	{
		// 실제 도착 타일 (경로의 마지막 타일 = 실제 도착 타일)
		// 이동력 부족으로 경로가 잘렸을 경우를 대비하여 실제 도착 타일을 사용
		FVector2D DestinationTile = Path[Path.Num() - 1];
		
		// 경로를 찾은 후, 실제 이동 시작 전에 목적지 타일이 여전히 사용 가능한지 최종 확인
		// (경로 탐색 중 다른 유닛이 같은 타일을 예약했을 수 있음)
		// 자기 자신의 예약은 무시하도록 Unit 파라미터 전달
		if (!UnitManager->CanPlaceUnitAtHex(DestinationTile, Unit))
		{
			// 목적지가 더 이상 사용 불가능하면 이동 취소
			// 원래 목표 타일 예약 해제
			if (DestinationTile != TargetTile)
			{
				ReservedTiles.Remove(TargetTile);
			}
			ReservedTiles.Remove(DestinationTile);
			return false;
		}
		
		// 원래 목표 타일과 실제 도착 타일이 다르면 예약 업데이트
		if (DestinationTile != TargetTile)
		{
			ReservedTiles.Remove(TargetTile);
		}
		
		// 실제 도착 타일 예약 (AI 로컬 예약, UnitManager의 전역 예약과는 별개)
		ReservedTiles.Add(DestinationTile);
		
		// 이동 시작 (StartVisualMovement 내부에서도 CanPlaceUnitAtHex 재확인)
		UnitManager->StartVisualMovement(Unit, Path);
		OutPendingMovements++;
		
		return true;
	}

	return false;
}

FVector2D UAIPlayerManager::FindValidWanderTile(
	int32 PlayerIndex,
	const FReservedTiles& ReservedTiles,
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

	// ========== 예약되지 않은 타일 찾기 (반복 시도) ==========
	// 최대 시도 횟수(MaxAttempts)만큼 반복하여 예약되지 않은 배회 타일 찾기
	for (int32 Attempt = 0; Attempt < MaxAttempts; Attempt++)
	{
		// 유효한 타일 중 랜덤 선택
		int32 RandomIndex = FMath::RandRange(0, ValidTiles.Num() - 1);
		FVector2D CandidateTile = ValidTiles[RandomIndex];
		
		// 예약되지 않은 타일이면 반환
		if (!ReservedTiles.Contains(CandidateTile))
		{
			return CandidateTile;
		}
	}

	// 모든 시도 실패 시 무효 좌표 반환
	return FVector2D(-1, -1);
}

