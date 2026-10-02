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
		TSet<FVector2D> TilesToPillage;
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

			TilesToPillage.Add(TileCoord);
		}

		for (const FVector2D& Coord : TilesToPillage)
		{
			FacilityManager->SetFacilityPillaged(Coord, true, WorldComponent);
		}
	}
	// ========== 약탈 처리 끝 ==========

	// 현재 플레이어의 PlayerState 가져오기
	if (ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(CurrentPlayerIndex))
	{
		// 플레이어의 턴 종료 처리 (자원 생산 등)
		PlayerState->ProcessTurnResources();
	}

	// 현재 플레이어의 모든 유닛 이동력 회복
	if (UnitManager)
	{
		TArray<AUnitCharacterBase*> AllUnits = UnitManager->GetAllUnits();
		for (AUnitCharacterBase* Unit : AllUnits)
		{
			if (Unit && Unit->GetPlayerIndex() == CurrentPlayerIndex)
			{
				if (UUnitStatusComponent* StatusComp = Unit->GetUnitStatusComponent())
				{
					StatusComp->ResetTurn();
				}
			}
		}
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
	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}
	
	// 플레이어 0 생존 확인
	ASuperPlayerState* Player0 = GameInstance->GetPlayerState(0);
	if (!Player0 || !Player0->IsAlive())
	{
		return; // 플레이어 이미 패배 (OnPlayerDefeated_Human에서 처리됨)
	}
	
	// 플레이어 0 제외 모두 패배했는가?
	bool bAllOthersDefeated = true;
	int32 TotalPlayerCount = GameInstance->GetPlayerStateCount();
	
	for (int32 i = 1; i < TotalPlayerCount; i++)
	{
		ASuperPlayerState* OtherState = GameInstance->GetPlayerState(i);
		if (OtherState && OtherState->IsAlive())
		{
			bAllOthersDefeated = false;
			break;
		}
	}
	
	if (bAllOthersDefeated)
	{
		// 승리!
		OnPlayerVictory();
	}
}

void ASuperGameModeBase::OnPlayerVictory()
{
	// 플레이어 0(플레이어)의 승리 델리게이트 브로드캐스트
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		if (ASuperPlayerState* Player0 = GameInstance->GetPlayerState(0))
		{
			Player0->OnPlayerVictoryDelegate.Broadcast();
		}
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

