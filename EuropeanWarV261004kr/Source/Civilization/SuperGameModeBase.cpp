#include "SuperGameModeBase.h"
#include "SuperGameInstance.h"
#include "SuperGameState.h"
#include "SuperGameController.h"
#include "SuperCameraPawn.h"
#include "SuperPlayerState.h"
#include "Unit/UnitManager.h"
#include "Unit/UnitCharacterBase.h"
#include "Status/UnitStatusComponent.h"
#include "Diplomacy/DiplomacyManager.h"
#include "AIPlayer/AIPlayerManager.h"
#include "World/WorldComponent.h"
#include "World/WorldStruct.h"
#include "Facility/FacilityManager.h"

ASuperGameModeBase::ASuperGameModeBase()
{
	// PlayerController, Pawn, PlayerState, GameState 클래스 설정
	PlayerControllerClass = ASuperGameController::StaticClass();
	DefaultPawnClass = ASuperCameraPawn::StaticClass();
	PlayerStateClass = ASuperPlayerState::StaticClass();
	GameStateClass = ASuperGameState::StaticClass();
	
	// 기본값 초기화
	bIsGameActive = false;
	bIsGamePaused = false;
	bVictoryDeclared = false;
	bDeferRemoteVictoryNotify = false;
	VictoryWinnerIndex = INDEX_NONE;
}

UTurnComponent* ASuperGameModeBase::GetTurnComponent() const
{
	if (const ASuperGameState* InGameState = GetGameState<ASuperGameState>())
	{
		return InGameState->GetTurnComponent();
	}

	return nullptr;
}

void ASuperGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (UTurnComponent* Turn = GetTurnComponent())
	{
		Turn->OnRoundChanged.AddDynamic(this, &ASuperGameModeBase::HandleRoundChanged);
		Turn->OnTurnChanged.AddDynamic(this, &ASuperGameModeBase::HandleTurnChanged);
	}
	
	CreateAllPlayerStates();
	InitializeGame();
}

void ASuperGameModeBase::InitializeGame()
{
	// 게임 상태 초기화
	bIsGameActive = true;
	bIsGamePaused = false;

	if (UTurnComponent* Turn = GetTurnComponent())
	{
		Turn->InitializeTurn(1);
	}
}

void ASuperGameModeBase::StartNewGame()
{
	if (bIsGameActive)
	{
		EndGame();
	}

	InitializeGame();
}

void ASuperGameModeBase::EndGame()
{
	bIsGameActive = false;
	bIsGamePaused = false;

	// 월드 컴포넌트 정리 (메모리 해제)
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		GameInstance->ClearGeneratedWorldComponent();
	}
}

void ASuperGameModeBase::PauseGame()
{
	if (bIsGameActive && !bIsGamePaused)
	{
		bIsGamePaused = true;
	}
}

void ASuperGameModeBase::ResumeGame()
{
	if (bIsGameActive && bIsGamePaused)
	{
		bIsGamePaused = false;
	}
}

void ASuperGameModeBase::NextTurn()
{
	UTurnComponent* Turn = GetTurnComponent();
	if (!bIsGameActive || bIsGamePaused || !Turn)
	{
		return;
	}

	// 현재 플레이어의 턴 종료 처리
	EndCurrentPlayerTurn();

	int32 CurrentPlayerIndex = Turn->GetCurrentPlayerIndex();

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	if (GameInstance && GameInstance->IsHumanPlayerIndex(CurrentPlayerIndex))
	{
		Turn->NextTurn();
		CheckGameEndConditions();
		return;
	}
	
	if (GameInstance)
	{
		int32 TotalPlayerCount = GameInstance->GetPlayerStateCount();
		
		if (CurrentPlayerIndex >= 0 && CurrentPlayerIndex < TotalPlayerCount)
		{
			// ========== 패배한 AI 플레이어 체크 ==========
			ASuperPlayerState* CurrentAIState = GameInstance->GetPlayerState(CurrentPlayerIndex);
			if (CurrentAIState && !CurrentAIState->IsAlive())
			{
				// 패배한 AI는 즉시 턴 건너뛰기
				Turn->NextTurn();
				CheckGameEndConditions();
				return;
			}
			
			if (UAIPlayerManager* AIPlayerManager = GameInstance->GetAIPlayerManager())
			{
				// AI 턴이 완료되었는지 확인
				if (AIPlayerManager->IsAITurnComplete(CurrentPlayerIndex))
				{
					// AI 턴 종료
					AIPlayerManager->EndAITurn(CurrentPlayerIndex);
					
					// 다음 턴으로 진행
					Turn->NextTurn();
					
					// 게임 종료 조건 확인
					CheckGameEndConditions();
					return;
				}
				else
				{
					// AI 턴이 아직 완료되지 않았으면 상태 머신 업데이트
					// (비동기 작업이 완료되어 다음 상태로 진행할 수 있는 경우)
					if (!AIPlayerManager->HasPendingAsyncWork(CurrentPlayerIndex))
					{
						AIPlayerManager->UpdateStateMachine(CurrentPlayerIndex);
					}
					// 비동기 작업이 있으면 콜백에서 자동으로 UpdateStateMachine이 호출됨
					return; // NextTurn()은 나중에 콜백에서 호출
				}
			}
		}
	}
}

void ASuperGameModeBase::EndCurrentPlayerTurn()
{
	UTurnComponent* Turn = GetTurnComponent();
	if (!Turn || !bIsGameActive)
	{
		return;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	int32 CurrentPlayerIndex = Turn->GetCurrentPlayerIndex();

	// ========== 턴 종료 시 약탈 처리 (전쟁 중인 상대의 시설 타일 위에 전투 유닛이 있으면 약탈) ==========
	UUnitManager* UnitManager = GameInstance->GetUnitManager();
	UFacilityManager* FacilityManager = GameInstance->GetFacilityManager();
	UWorldComponent* WorldComponent = GameInstance->GetGeneratedWorldComponent();
	UDiplomacyManager* DiplomacyManager = GameInstance->GetDiplomacyManager();

	if (UnitManager && FacilityManager && WorldComponent && DiplomacyManager)
	{
		TArray<FVector2D> TilesToPillage;
		TArray<AUnitCharacterBase*> AllUnits = UnitManager->GetAllUnits();

		for (AUnitCharacterBase* Unit : AllUnits)
		{
			if (!Unit || Unit->GetPlayerIndex() != CurrentPlayerIndex)
			{
				continue;
			}
			UUnitStatusComponent* StatusComp = Unit->GetUnitStatusComponent();
			if (!StatusComp || !StatusComp->CanAttack())
			{
				continue;
			}

			FVector2D TileCoord = UnitManager->GetHexPositionForUnit(Unit);
			UWorldTile* Tile = WorldComponent->GetTileAtHex(TileCoord);
			if (!Tile)
			{
				continue;
			}

			int32 TileOwnerIndex = Tile->GetOwnerPlayerID();
			if (TileOwnerIndex == CurrentPlayerIndex || TileOwnerIndex < 0)
			{
				continue;
			}
			if (!DiplomacyManager->IsAtWar(CurrentPlayerIndex, TileOwnerIndex))
			{
				continue;
			}
			if (!FacilityManager->HasFacilityAtTile(TileCoord))
			{
				continue;
			}

			if (!TilesToPillage.Contains(TileCoord))
			{
				TilesToPillage.Add(TileCoord);
			}
		}

		for (const FVector2D& Coord : TilesToPillage)
		{
			FacilityManager->SetFacilityPillaged(Coord, true, WorldComponent);
		}

		if (TilesToPillage.Num() > 0)
		{
			NotifyRemoteFacilitiesPillaged(TilesToPillage);
		}
	}
	// ========== 약탈 처리 끝 ==========

	// 현재 플레이어의 PlayerState 가져오기
	if (ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(CurrentPlayerIndex))
	{
		// 플레이어의 턴 종료 처리 (자원 생산 등)
		PlayerState->ProcessTurnResources();
	}

	// 현재 플레이어의 모든 유닛 이동력 회복. 참가자 화면에도 같은 슬롯을 되돌립니다.
	if (UnitManager)
	{
		UnitManager->ResetPlayerUnitTurn(CurrentPlayerIndex);
		NotifyRemoteUnitTurnReset(CurrentPlayerIndex);
	}
}

ASuperPlayerState* ASuperGameModeBase::GetPlayerStateIfTurn(int32 RequestingPlayerIndex) const
{
	UTurnComponent* Turn = GetTurnComponent();
	if (!bIsGameActive || bIsGamePaused || !Turn)
	{
		return nullptr;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	if (!GameInstance || !GameInstance->IsHumanPlayerIndex(RequestingPlayerIndex))
	{
		return nullptr;
	}

	if (Turn->GetCurrentPlayerIndex() != RequestingPlayerIndex)
	{
		return nullptr;
	}

	return GameInstance->GetPlayerState(RequestingPlayerIndex);
}

bool ASuperGameModeBase::RequestEndPlayerTurn(int32 RequestingPlayerIndex)
{
	if (!GetPlayerStateIfTurn(RequestingPlayerIndex))
	{
		return false;
	}

	EndCurrentPlayerTurn();
	if (UTurnComponent* Turn = GetTurnComponent())
	{
		Turn->NextTurn();
	}
	CheckGameEndConditions();
	return true;
}

bool ASuperGameModeBase::RequestStartBuildingProduction(int32 RequestingPlayerIndex, FName BuildingRowName)
{
	ASuperPlayerState* PlayerState = GetPlayerStateIfTurn(RequestingPlayerIndex);
	if (!PlayerState || BuildingRowName.IsNone())
	{
		return false;
	}

	return PlayerState->StartBuildingProduction(BuildingRowName);
}

bool ASuperGameModeBase::RequestStartUnitProduction(int32 RequestingPlayerIndex, FName UnitName)
{
	ASuperPlayerState* PlayerState = GetPlayerStateIfTurn(RequestingPlayerIndex);
	if (!PlayerState || UnitName.IsNone())
	{
		return false;
	}

	return PlayerState->StartUnitProduction(UnitName);
}

bool ASuperGameModeBase::RequestStartTechResearch(int32 RequestingPlayerIndex, FName TechRowName)
{
	ASuperPlayerState* PlayerState = GetPlayerStateIfTurn(RequestingPlayerIndex);
	if (!PlayerState || TechRowName.IsNone())
	{
		return false;
	}

	return PlayerState->StartTechResearch(TechRowName);
}

bool ASuperGameModeBase::RequestPurchaseBuilding(int32 RequestingPlayerIndex, FName BuildingRowName)
{
	ASuperPlayerState* PlayerState = GetPlayerStateIfTurn(RequestingPlayerIndex);
	if (!PlayerState || BuildingRowName.IsNone())
	{
		return false;
	}

	return PlayerState->PurchaseBuildingWithGold(BuildingRowName);
}

bool ASuperGameModeBase::RequestPurchaseUnit(int32 RequestingPlayerIndex, FName UnitName)
{
	ASuperPlayerState* PlayerState = GetPlayerStateIfTurn(RequestingPlayerIndex);
	if (!PlayerState || UnitName.IsNone())
	{
		return false;
	}

	return PlayerState->PurchaseUnitWithGold(UnitName);
}

void ASuperGameModeBase::NotifyRemoteUnitSpawned(int32 PlayerIndex, FName UnitName, FVector2D Hex)
{
	if (PlayerIndex < 0 || UnitName.IsNone() || !GetWorld())
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASuperGameController* Controller = Cast<ASuperGameController>(It->Get());
		if (Controller && !Controller->IsLocalController())
		{
			Controller->ClientApplySpawnedUnit(PlayerIndex, UnitName, Hex);
		}
	}
}

bool ASuperGameModeBase::RequestMoveUnit(int32 RequestingPlayerIndex, FVector2D FromHex, FVector2D ToHex)
{
	if (!GetPlayerStateIfTurn(RequestingPlayerIndex))
	{
		return false;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	UUnitManager* UnitManager = GameInstance ? GameInstance->GetUnitManager() : nullptr;
	if (!UnitManager)
	{
		return false;
	}

	AUnitCharacterBase* Unit = UnitManager->GetUnitAtHex(FromHex);
	if (!Unit || Unit->GetPlayerIndex() != RequestingPlayerIndex)
	{
		return false;
	}

	if (!UnitManager->MoveUnitFromHexToHex(FromHex, ToHex))
	{
		return false;
	}

	NotifyRemoteUnitMoved(FromHex, ToHex);
	return true;
}

bool ASuperGameModeBase::RequestCombat(int32 RequestingPlayerIndex, FVector2D AttackerHex, FVector2D TargetHex)
{
	if (!GetPlayerStateIfTurn(RequestingPlayerIndex))
	{
		return false;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	UUnitManager* UnitManager = GameInstance ? GameInstance->GetUnitManager() : nullptr;
	if (!UnitManager)
	{
		return false;
	}

	AUnitCharacterBase* Attacker = UnitManager->GetUnitAtHex(AttackerHex);
	if (!Attacker || Attacker->GetPlayerIndex() != RequestingPlayerIndex)
	{
		return false;
	}

	const bool bWasVictoryDeclared = bVictoryDeclared;
	bDeferRemoteVictoryNotify = true;
	const bool bCombatSucceeded = UnitManager->CombatFromHexToHex(AttackerHex, TargetHex, true);
	bDeferRemoteVictoryNotify = false;
	if (!bCombatSucceeded)
	{
		return false;
	}

	NotifyRemoteCombat(AttackerHex, TargetHex);
	if (!bWasVictoryDeclared && bVictoryDeclared)
	{
		NotifyRemoteVictory(VictoryWinnerIndex);
	}
	return true;
}

void ASuperGameModeBase::NotifyRemoteUnitMoved(FVector2D FromHex, FVector2D ToHex)
{
	if (!GetWorld())
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASuperGameController* Controller = Cast<ASuperGameController>(It->Get());
		if (Controller && !Controller->IsLocalController())
		{
			Controller->ClientApplyMoveUnit(FromHex, ToHex);
		}
	}
}

void ASuperGameModeBase::NotifyRemoteCombat(FVector2D AttackerHex, FVector2D TargetHex)
{
	if (!GetWorld())
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASuperGameController* Controller = Cast<ASuperGameController>(It->Get());
		if (Controller && !Controller->IsLocalController())
		{
			Controller->ClientApplyCombat(AttackerHex, TargetHex);
		}
	}
}

static bool IsOwnedBuilderAtHex(UUnitManager* UnitManager, int32 PlayerIndex, FVector2D Hex)
{
	if (!UnitManager)
	{
		return false;
	}

	AUnitCharacterBase* Unit = UnitManager->GetUnitAtHex(Hex);
	return Unit && Unit->GetPlayerIndex() == PlayerIndex && UnitManager->IsBuilderUnit(Unit);
}

bool ASuperGameModeBase::RequestBuildFacility(int32 RequestingPlayerIndex, FVector2D Hex, FName FacilityRowName)
{
	if (!GetPlayerStateIfTurn(RequestingPlayerIndex) || FacilityRowName.IsNone())
	{
		return false;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	UUnitManager* UnitManager = GameInstance ? GameInstance->GetUnitManager() : nullptr;
	if (!IsOwnedBuilderAtHex(UnitManager, RequestingPlayerIndex, Hex))
	{
		return false;
	}

	UnitManager->RequestBuilderBuildFacility(Hex, FacilityRowName);
	NotifyRemoteBuildFacility(Hex, FacilityRowName);
	return true;
}

bool ASuperGameModeBase::RequestRepairFacility(int32 RequestingPlayerIndex, FVector2D Hex)
{
	if (!GetPlayerStateIfTurn(RequestingPlayerIndex))
	{
		return false;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	UUnitManager* UnitManager = GameInstance ? GameInstance->GetUnitManager() : nullptr;
	if (!IsOwnedBuilderAtHex(UnitManager, RequestingPlayerIndex, Hex))
	{
		return false;
	}

	UnitManager->RequestBuilderRepairFacility(Hex);
	NotifyRemoteRepairFacility(Hex);
	return true;
}

bool ASuperGameModeBase::RequestDestroyFacility(int32 RequestingPlayerIndex, FVector2D Hex)
{
	if (!GetPlayerStateIfTurn(RequestingPlayerIndex))
	{
		return false;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	UUnitManager* UnitManager = GameInstance ? GameInstance->GetUnitManager() : nullptr;
	if (!IsOwnedBuilderAtHex(UnitManager, RequestingPlayerIndex, Hex))
	{
		return false;
	}

	UnitManager->RequestBuilderDestroyFacility(Hex);
	NotifyRemoteDestroyFacility(Hex);
	return true;
}

bool ASuperGameModeBase::RequestPurchaseTile(int32 RequestingPlayerIndex, FVector2D Hex)
{
	ASuperPlayerState* PlayerState = GetPlayerStateIfTurn(RequestingPlayerIndex);
	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	UWorldComponent* WorldComponent = GameInstance ? GameInstance->GetGeneratedWorldComponent() : nullptr;
	if (!PlayerState || !WorldComponent)
	{
		return false;
	}

	if (!PlayerState->PurchaseTile(Hex, WorldComponent))
	{
		return false;
	}

	NotifyRemotePurchaseTile(RequestingPlayerIndex, Hex);
	return true;
}

bool ASuperGameModeBase::RequestDiplomacyAction(int32 RequestingPlayerIndex, EDiplomacyActionType ActionType, int32 TargetPlayerIndex)
{
	if (!GetPlayerStateIfTurn(RequestingPlayerIndex) || ActionType == EDiplomacyActionType::None)
	{
		return false;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	UDiplomacyManager* DiplomacyManager = GameInstance ? GameInstance->GetDiplomacyManager() : nullptr;
	if (!DiplomacyManager)
	{
		return false;
	}

	FDiplomacyAction Action;
	Action.Action = ActionType;
	Action.FromPlayerId = RequestingPlayerIndex;
	Action.ToPlayerId = TargetPlayerIndex;
	const int32 ActionId = DiplomacyManager->IssueAction(Action);
	if (ActionId == -1)
	{
		return false;
	}

	NotifyRemoteDiplomacyAction(RequestingPlayerIndex, TargetPlayerIndex, ActionType, ActionId);
	return true;
}

void ASuperGameModeBase::NotifyRemoteBuildFacility(FVector2D Hex, FName FacilityRowName)
{
	if (!GetWorld())
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASuperGameController* Controller = Cast<ASuperGameController>(It->Get());
		if (Controller && !Controller->IsLocalController())
		{
			Controller->ClientApplyBuildFacility(Hex, FacilityRowName);
		}
	}
}

void ASuperGameModeBase::NotifyRemoteRepairFacility(FVector2D Hex)
{
	if (!GetWorld())
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASuperGameController* Controller = Cast<ASuperGameController>(It->Get());
		if (Controller && !Controller->IsLocalController())
		{
			Controller->ClientApplyRepairFacility(Hex);
		}
	}
}

void ASuperGameModeBase::NotifyRemoteDestroyFacility(FVector2D Hex)
{
	if (!GetWorld())
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASuperGameController* Controller = Cast<ASuperGameController>(It->Get());
		if (Controller && !Controller->IsLocalController())
		{
			Controller->ClientApplyDestroyFacility(Hex);
		}
	}
}

void ASuperGameModeBase::NotifyRemotePurchaseTile(int32 PlayerIndex, FVector2D Hex)
{
	if (PlayerIndex < 0 || !GetWorld())
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASuperGameController* Controller = Cast<ASuperGameController>(It->Get());
		if (Controller && !Controller->IsLocalController())
		{
			Controller->ClientApplyPurchaseTile(PlayerIndex, Hex);
		}
	}
}

void ASuperGameModeBase::NotifyRemoteDiplomacyAction(int32 FromPlayerIndex, int32 TargetPlayerIndex, EDiplomacyActionType ActionType, int32 ActionId)
{
	if (!GetWorld())
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASuperGameController* Controller = Cast<ASuperGameController>(It->Get());
		if (Controller && !Controller->IsLocalController())
		{
			Controller->ClientApplyDiplomacyAction(FromPlayerIndex, TargetPlayerIndex, ActionType, ActionId);
		}
	}
}

void ASuperGameModeBase::NotifyRemoteFacilitiesPillaged(const TArray<FVector2D>& Hexes)
{
	if (Hexes.Num() == 0 || !GetWorld())
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASuperGameController* Controller = Cast<ASuperGameController>(It->Get());
		if (Controller && !Controller->IsLocalController())
		{
			Controller->ClientApplyPillagedFacilities(Hexes);
		}
	}
}

void ASuperGameModeBase::NotifyRemoteVictory(int32 WinnerIndex)
{
	if (WinnerIndex < 0 || !GetWorld())
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASuperGameController* Controller = Cast<ASuperGameController>(It->Get());
		if (Controller && !Controller->IsLocalController())
		{
			Controller->ClientApplyVictory(WinnerIndex);
		}
	}
}

void ASuperGameModeBase::NotifyRemoteUnitTurnReset(int32 PlayerIndex)
{
	if (PlayerIndex < 0 || !GetWorld())
	{
		return;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASuperGameController* Controller = Cast<ASuperGameController>(It->Get());
		if (Controller && !Controller->IsLocalController())
		{
			Controller->ClientApplyResetUnitTurn(PlayerIndex);
		}
	}
}

void ASuperGameModeBase::SetCountryNames(const TArray<FName>& InCountryNames)
{
	CountryNames = InCountryNames;
	
	// 최대 8개로 제한
	if (CountryNames.Num() > 8)
	{
		CountryNames.SetNum(8);
	}
}

void ASuperGameModeBase::CheckGameEndConditions()
{
	if (bVictoryDeclared)
	{
		return;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	// 1대1: 사람이 한 명 패배하고 살아 있는 사람이 한 명이면 그 슬롯이 승리합니다.
	if (GameInstance->IsInMultiplayerSession())
	{
		int32 WinnerIndex = INDEX_NONE;
		int32 LivingHumans = 0;
		bool bAnyHumanDefeated = false;
		const int32 TotalPlayerCount = GameInstance->GetPlayerStateCount();
		for (int32 i = 0; i < TotalPlayerCount; ++i)
		{
			if (!GameInstance->IsHumanPlayerIndex(i))
			{
				continue;
			}

			ASuperPlayerState* State = GameInstance->GetPlayerState(i);
			if (State && State->IsAlive())
			{
				++LivingHumans;
				WinnerIndex = i;
			}
			else if (State && !State->IsAlive())
			{
				bAnyHumanDefeated = true;
			}
		}

		if (bAnyHumanDefeated && LivingHumans == 1)
		{
			OnPlayerVictory(WinnerIndex);
		}
		return;
	}

	// 싱글: 플레이어 0이 살아 있고 나머지 슬롯이 모두 패배면 플레이어 0이 승리합니다.
	ASuperPlayerState* Player0 = GameInstance->GetPlayerState(0);
	if (!Player0 || !Player0->IsAlive())
	{
		return;
	}

	const int32 TotalPlayerCount = GameInstance->GetPlayerStateCount();
	for (int32 i = 1; i < TotalPlayerCount; ++i)
	{
		ASuperPlayerState* OtherState = GameInstance->GetPlayerState(i);
		if (OtherState && OtherState->IsAlive())
		{
			return;
		}
	}

	OnPlayerVictory(0);
}

void ASuperGameModeBase::OnPlayerVictory(int32 WinnerIndex)
{
	if (bVictoryDeclared)
	{
		return;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	ASuperPlayerState* Winner = GameInstance->GetPlayerState(WinnerIndex);
	if (!Winner)
	{
		return;
	}

	bVictoryDeclared = true;
	VictoryWinnerIndex = WinnerIndex;
	Winner->OnPlayerVictoryDelegate.Broadcast();
	if (!bDeferRemoteVictoryNotify)
	{
		NotifyRemoteVictory(WinnerIndex);
	}
}

void ASuperGameModeBase::HandleRoundChanged(FTurnStruct NewTurn)
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		if (UDiplomacyManager* DiplomacyManager = GameInstance->GetDiplomacyManager())
		{
			DiplomacyManager->OnRoundStarted(NewTurn.RoundNumber);
		}
	}
}

void ASuperGameModeBase::HandleTurnChanged(FTurnStruct NewTurn)
{
	int32 CurrentPlayerIndex = NewTurn.PlayerIndex;
	
	// GameInstance 가져오기
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		// 총 플레이어 수 가져오기
		int32 TotalPlayerCount = GameInstance->GetPlayerStateCount();
		
		if (!GameInstance->IsHumanPlayerIndex(CurrentPlayerIndex) && CurrentPlayerIndex < TotalPlayerCount)
		{
			// ========== 패배한 AI 플레이어 체크 ==========
			ASuperPlayerState* CurrentAIState = GameInstance->GetPlayerState(CurrentPlayerIndex);
			if (CurrentAIState && !CurrentAIState->IsAlive())
			{
				// 패배한 AI는 즉시 다음 턴으로 (턴 시작 안 함)
				if (UTurnComponent* Turn = GetTurnComponent())
				{
					Turn->NextTurn();
				}
				return;
			}
			
			if (UAIPlayerManager* AIPlayerManager = GameInstance->GetAIPlayerManager())
			{
				// AI 턴 시작
				AIPlayerManager->StartAITurn(CurrentPlayerIndex);
				
				// 첫 상태 머신 업데이트 (비동기 작업이 없으면 즉시 진행)
				if (!AIPlayerManager->HasPendingAsyncWork(CurrentPlayerIndex))
				{
					AIPlayerManager->UpdateStateMachine(CurrentPlayerIndex);
				}
			}
		}
	}
}

void ASuperGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

void ASuperGameModeBase::CreateAllPlayerStates()
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		GameInstance->CreateAllPlayerStates();
		CountryNames = GameInstance->GetCountryNames();
	}
}

