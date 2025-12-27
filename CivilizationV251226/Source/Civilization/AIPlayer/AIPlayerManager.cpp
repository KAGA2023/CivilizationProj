// Fill out your copyright notice in the Description page of Project Settings.

#include "AIPlayerManager.h"
#include "../SuperPlayerState.h"
#include "../SuperGameInstance.h"
#include "../SuperGameModeBase.h"
#include "../Turn/TurnComponent.h"
#include "../Research/ResearchComponent.h"
#include "../City/CityComponent.h"
#include "../Status/UnitStatusStruct.h"
#include "../Unit/UnitCharacterBase.h"
#include "../World/WorldComponent.h"
#include "../World/WorldStruct.h"
#include "../Facility/FacilityManager.h"
#include "../Diplomacy/DiplomacyManager.h"
#include "../Diplomacy/DiplomacyStruct.h"
#include "../Unit/UnitManager.h"
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
		return;
	}

	ASuperPlayerState* PlayerState = AIPlayer->PlayerStateRef.Get();
	if (!PlayerState)
	{
		TransitionToNextState(PlayerIndex);
		return;
	}

	// ResearchComponent 가져오기
	UResearchComponent* ResearchComponent = PlayerState->GetResearchComponent();
	if (!ResearchComponent)
	{
		TransitionToNextState(PlayerIndex);
		return;
	}

	// 현재 연구 중인 기술이 있는지 확인
	FResearchCurrentStat CurrentStat = ResearchComponent->GetCurrentStat();
	if (CurrentStat.DevelopingName != NAME_None)
	{
		// 이미 연구 중이면 다음 상태로 전환
		TransitionToNextState(PlayerIndex);
		return;
	}

	// 연구 가능한 기술 목록 가져오기
	TArray<FName> ResearchableTechs = ResearchComponent->GetResearchableTechs();
	
	// 연구 가능한 기술이 없으면 다음 상태로 전환
	if (ResearchableTechs.Num() == 0)
	{
		TransitionToNextState(PlayerIndex);
		return;
	}

	// 랜덤으로 하나 선택
	int32 RandomIndex = FMath::RandRange(0, ResearchableTechs.Num() - 1);
	FName SelectedTech = ResearchableTechs[RandomIndex];

	// 연구 시작
	PlayerState->StartTechResearch(SelectedTech);

	// 다음 상태로 전환
	TransitionToNextState(PlayerIndex);
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
		return;
	}

	ASuperPlayerState* PlayerState = AIPlayer->PlayerStateRef.Get();
	if (!PlayerState)
	{
		TransitionToNextState(PlayerIndex);
		return;
	}

	// CityComponent 가져오기
	UCityComponent* CityComponent = PlayerState->GetCityComponent();
	if (!CityComponent)
	{
		TransitionToNextState(PlayerIndex);
		return;
	}

	// 현재 생산 중인지 확인
	FCityCurrentStat CurrentStat = CityComponent->GetCurrentStat();
	if (CurrentStat.ProductionType != EProductionType::None)
	{
		// 이미 생산 중이면 다음 상태로 전환
		TransitionToNextState(PlayerIndex);
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
}

void UAIPlayerManager::ProcessTilePurchaseState(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer || !AIPlayer->PlayerStateRef.IsValid())
	{
		TransitionToNextState(PlayerIndex);
		return;
	}

	ASuperPlayerState* PlayerState = AIPlayer->PlayerStateRef.Get();
	if (!PlayerState)
	{
		TransitionToNextState(PlayerIndex);
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
}

void UAIPlayerManager::ProcessFacilityState(int32 PlayerIndex)
{
	FAIPlayerStruct* AIPlayer = GetAIPlayerPtr(PlayerIndex);
	if (!AIPlayer || !AIPlayer->PlayerStateRef.IsValid())
	{
		TransitionToNextState(PlayerIndex);
		return;
	}

	ASuperPlayerState* PlayerState = AIPlayer->PlayerStateRef.Get();
	if (!PlayerState)
	{
		TransitionToNextState(PlayerIndex);
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
		return;
	}

	// 이미 목표 타일이 설정되어 있고, 그 타일에 시설이 건설되지 않았으면 갱신하지 않음
	// TargetFacilityTile이 초기값(-1, -1)이 아닌지 확인 (헥스 좌표계에서는 음수 좌표가 정상이므로 초기값과 비교)
	bool bTargetFacilityTileSet = (AIPlayer->TargetFacilityTile != FVector2D(-1, -1));
	
	if (bTargetFacilityTileSet)
	{
		// 목표 타일이 여전히 유효한지 확인 (시설이 건설되지 않았는지)
		if (!FacilityManager->HasFacilityAtTile(AIPlayer->TargetFacilityTile))
		{
			// 목표 타일이 유효하면 갱신하지 않고 다음 상태로 전환
			TransitionToNextState(PlayerIndex);
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
		return;
	}

	ASuperPlayerState* PlayerState = AIPlayer->PlayerStateRef.Get();
	if (!PlayerState)
	{
		TransitionToNextState(PlayerIndex);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		TransitionToNextState(PlayerIndex);
		return;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance());
	if (!GameInstance)
	{
		TransitionToNextState(PlayerIndex);
		return;
	}

	UWorldComponent* WorldComponent = GameInstance->GetGeneratedWorldComponent();
	UUnitManager* UnitManager = GameInstance->GetUnitManager();

	if (!WorldComponent || !UnitManager)
	{
		TransitionToNextState(PlayerIndex);
		return;
	}

	// 1. 건설자 유닛 찾기
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

	// 건설자가 없으면 다음 상태로 전환
	if (!BuilderUnit)
	{
		TransitionToNextState(PlayerIndex);
		return;
	}

	// 건설자의 현재 위치 찾기 (모든 타일을 순회하면서 확인)
	FVector2D BuilderPosition = FVector2D(-1, -1);
	bool bBuilderPositionFound = false;
	TArray<UWorldTile*> AllTiles = WorldComponent->GetAllTiles();
	for (UWorldTile* Tile : AllTiles)
	{
		if (!Tile)
		{
			continue;
		}
		FVector2D TileCoord = Tile->GetGridPosition();
		if (UnitManager->GetUnitAtHex(TileCoord) == BuilderUnit)
		{
			BuilderPosition = TileCoord;
			bBuilderPositionFound = true;
			break;
		}
	}

	if (!bBuilderPositionFound)
	{
		TransitionToNextState(PlayerIndex);
		return;
	}

	// 2. TargetFacilityTile 확인
	// TargetFacilityTile이 초기값(-1, -1)이 아닌지 확인 (헥스 좌표계에서는 음수 좌표가 정상이므로 초기값과 비교)
	bool bTargetFacilityTileSet = (AIPlayer->TargetFacilityTile != FVector2D(-1, -1));
	
	if (bTargetFacilityTileSet)
	{
		
		// 건설자가 TargetFacilityTile에 도달했는지 확인
		if (BuilderPosition == AIPlayer->TargetFacilityTile)
		{
			// 도착했으면 시설 건설 시도
			// 1. 필요한 매니저 및 컴포넌트 가져오기
			UFacilityManager* FacilityManager = GameInstance->GetFacilityManager();

			if (!FacilityManager || !PlayerState)
			{
				AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
				TransitionToNextState(PlayerIndex);
				return;
			}

			// 2. 목표 타일 가져오기
			UWorldTile* TargetTile = WorldComponent->GetTileAtHex(AIPlayer->TargetFacilityTile);
			if (!TargetTile)
			{
				AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
				TransitionToNextState(PlayerIndex);
				return;
			}

			// 3. 기술 조건 필터링: 건설 가능한 시설 목록 가져오기
			TArray<FName> AvailableFacilities = PlayerState->GetAvailableFacilities();
			if (AvailableFacilities.Num() == 0)
			{
				// 건설 가능한 시설이 없으면 초기화하고 다음 상태로
				AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
				TransitionToNextState(PlayerIndex);
				return;
			}

			// 4. 타일 조건 필터링: CanBuildFacilityOnTile()로 건설 가능한 시설 찾기
			TArray<FName> BuildableFacilities;
			for (const FName& FacilityRowName : AvailableFacilities)
			{
				if (FacilityManager->CanBuildFacilityOnTile(FacilityRowName, TargetTile))
				{
					BuildableFacilities.Add(FacilityRowName);
				}
			}

			// 5. 건설 가능한 시설이 없으면 초기화하고 다음 상태로
			if (BuildableFacilities.Num() == 0)
			{
				AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
				TransitionToNextState(PlayerIndex);
				return;
			}

			// 6. 건설 가능한 시설 중 랜덤 선택
			int32 RandomIndex = FMath::RandRange(0, BuildableFacilities.Num() - 1);
			FName SelectedFacilityRowName = BuildableFacilities[RandomIndex];

			// 7. 시설 건설 시도
			bool bBuildSuccess = PlayerState->BuildFacility(SelectedFacilityRowName, AIPlayer->TargetFacilityTile);

			// 8. 성공/실패와 관계없이 TargetFacilityTile 초기화
			AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
			TransitionToNextState(PlayerIndex);
			return;
		}

		// 건설자 이동 가능 여부 확인
		if (!BuilderUnit->GetUnitStatusComponent()->CanMove())
		{
			// 이동 불가능하면 랜덤 배회하고 TargetFacilityTile 초기화
			FVector2D WanderTile = GetRandomWanderTile(PlayerIndex, 4);
			if (WanderTile.X >= 0 && WanderTile.Y >= 0)
			{
				// 배회할 타일로 경로 찾기
				int32 MaxMovementCost = UnitManager->GetUnitRemainingMovement(BuilderUnit);
				TArray<FVector2D> Path = UnitManager->FindPathWithMovementCost(
					BuilderPosition,
					WanderTile,
					MaxMovementCost
				);

				// 경로가 있으면 이동 시작
				if (Path.Num() > 1)
				{
					UnitManager->StartVisualMovement(BuilderUnit, Path);
					AIPlayer->PendingUnitMovements++;
					AIPlayer->CurrentState = EAITurnState::WaitingForAsync;
				}
			}

			// TargetFacilityTile 초기화
			AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
			return;
		}

		// 이동 가능하면 TargetFacilityTile로 이동
		// 건설자의 남은 이동력 가져오기
		int32 MaxMovementCost = UnitManager->GetUnitRemainingMovement(BuilderUnit);

		// 경로 찾기
		TArray<FVector2D> Path = UnitManager->FindPathWithMovementCost(
			BuilderPosition,
			AIPlayer->TargetFacilityTile,
			MaxMovementCost
		);

		// 경로가 있으면 이동 시작
		if (Path.Num() > 1)
		{
			UnitManager->StartVisualMovement(BuilderUnit, Path);
			AIPlayer->PendingUnitMovements++;
			AIPlayer->CurrentState = EAITurnState::WaitingForAsync;
		}
		else
		{
			// 경로가 없으면 랜덤 배회하고 TargetFacilityTile 초기화
			FVector2D WanderTile = GetRandomWanderTile(PlayerIndex, 4);
			if (WanderTile.X >= 0 && WanderTile.Y >= 0)
			{
				// 배회할 타일로 경로 찾기
				TArray<FVector2D> WanderPath = UnitManager->FindPathWithMovementCost(
					BuilderPosition,
					WanderTile,
					MaxMovementCost
				);

				// 경로가 있으면 이동 시작
				if (WanderPath.Num() > 1)
				{
					UnitManager->StartVisualMovement(BuilderUnit, WanderPath);
					AIPlayer->PendingUnitMovements++;
					AIPlayer->CurrentState = EAITurnState::WaitingForAsync;
				}
			}

			// TargetFacilityTile 초기화
			AIPlayer->TargetFacilityTile = FVector2D(-1, -1);
		}
	}
	else
	{
		// TargetFacilityTile이 없으면 다음 상태로 전환
		TransitionToNextState(PlayerIndex);
	}
}

FVector2D UAIPlayerManager::GetRandomWanderTile(int32 PlayerIndex, int32 Radius) const
{
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
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			WorldComponent = GameInstance->GetGeneratedWorldComponent();
		}
	}

	if (!WorldComponent)
	{
		return FVector2D(-1, -1);
	}

	// 도시 좌표 가져오기
	FVector2D CityCoordinate = PlayerState->GetCityCoordinate();
	if (CityCoordinate.X < 0 || CityCoordinate.Y < 0)
	{
		return FVector2D(-1, -1);
	}

	// 도시 기준 반경 내 타일 좌표들 가져오기
	TArray<FVector2D> HexesInRadius = WorldComponent->GetHexesInRadius(CityCoordinate, Radius);

	// GameInstance 가져오기 (다른 플레이어 소유 타일 확인용)
	USuperGameInstance* GameInstance = nullptr;
	if (UWorld* World = GetWorld())
	{
		GameInstance = Cast<USuperGameInstance>(World->GetGameInstance());
	}

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
			if (GameInstance)
			{
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

	// 랜덤으로 타일 선택
	int32 RandomIndex = FMath::RandRange(0, ValidTiles.Num() - 1);
	return ValidTiles[RandomIndex];
}

