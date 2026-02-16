#include "SuperGameModeBase.h"
#include "SuperGameInstance.h"
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
	// PlayerController, Pawn, PlayerState 클래스 설정
	PlayerControllerClass = ASuperGameController::StaticClass();
	DefaultPawnClass = ASuperCameraPawn::StaticClass();
	PlayerStateClass = ASuperPlayerState::StaticClass();
	
	// 기본값 초기화
	bIsGameActive = false;
	bIsGamePaused = false;
	TurnComponent = nullptr;
}

void ASuperGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	// TurnComponent 생성
	TurnComponent = NewObject<UTurnComponent>(this, UTurnComponent::StaticClass());
	if (TurnComponent)
	{
		TurnComponent->RegisterComponent();

		// 라운드 변경 시 외교 매니저에 알림
		TurnComponent->OnRoundChanged.AddDynamic(this, &ASuperGameModeBase::HandleRoundChanged);
		
		// 턴 변경 시 AI 턴 처리
		TurnComponent->OnTurnChanged.AddDynamic(this, &ASuperGameModeBase::HandleTurnChanged);
	}
	
	// 모든 플레이어 스테이트 생성 (Player 0 + AI, 동적)
	CreateAllPlayerStates();
	
	// 게임 초기화
	InitializeGame();
}

void ASuperGameModeBase::InitializeGame()
{
	// 게임 상태 초기화
	bIsGameActive = true;
	bIsGamePaused = false;

	// TurnComponent 초기화 (Round 1, Turn 1 시작)
	if (TurnComponent)
	{
		TurnComponent->InitializeTurn(1);
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

	// TurnComponent 정리
	if (TurnComponent)
	{
		TurnComponent->UnregisterComponent();
		TurnComponent = nullptr;
	}

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
	if (!bIsGameActive || bIsGamePaused || !TurnComponent)
	{
		return;
	}

	// 현재 플레이어의 턴 종료 처리
	EndCurrentPlayerTurn();

	// 현재 플레이어가 플레이어0
	int32 CurrentPlayerIndex = TurnComponent->GetCurrentPlayerIndex();
	
	if (CurrentPlayerIndex == 0) {
		TurnComponent->NextTurn();

		// 게임 종료 조건 확인
		CheckGameEndConditions();
		return;
	}
	
	// 현재 플레이어가 AI인지 확인 (PlayerIndex >= 1)
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		int32 TotalPlayerCount = GameInstance->GetPlayerStateCount();
		
		if (CurrentPlayerIndex >= 1 && CurrentPlayerIndex < TotalPlayerCount) // AI 플레이어
		{
			// ========== 패배한 AI 플레이어 체크 ==========
			ASuperPlayerState* CurrentAIState = GameInstance->GetPlayerState(CurrentPlayerIndex);
			if (CurrentAIState && !CurrentAIState->IsAlive())
			{
				// 패배한 AI는 즉시 턴 건너뛰기
				TurnComponent->NextTurn();
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
					TurnComponent->NextTurn();
					
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
	if (!TurnComponent || !bIsGameActive)
	{
		return;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	int32 CurrentPlayerIndex = TurnComponent->GetCurrentPlayerIndex();

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

void ASuperGameModeBase::RequestEndPlayerTurn()
{
	if (!bIsGameActive || bIsGamePaused || !TurnComponent)
	{
		return;
	}

	// 현재 플레이어가 플레이어 0인지 확인
	int32 CurrentPlayerIndex = TurnComponent->GetCurrentPlayerIndex();
	if (CurrentPlayerIndex != 0)
	{
		// 플레이어 0의 턴이 아니면 무시
		return;
	}

	// 플레이어 0의 턴 종료 처리
	EndCurrentPlayerTurn();

	// 다음 턴으로 진행 (Player 1로 자동 전환)
	TurnComponent->NextTurn();

	// 게임 종료 조건 확인
	CheckGameEndConditions();
	
	// 이후 자동으로 HandleTurnChanged가 호출되어 AI 턴이 처리됨
	// Player 1 → Player 2 → ... → Player N-1 → Player 0 순서로 진행
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
		
		// AI 플레이어인지 확인 (1 ~ TotalPlayerCount-1)
		if (CurrentPlayerIndex >= 1 && CurrentPlayerIndex < TotalPlayerCount)
		{
			// ========== 패배한 AI 플레이어 체크 ==========
			ASuperPlayerState* CurrentAIState = GameInstance->GetPlayerState(CurrentPlayerIndex);
			if (CurrentAIState && !CurrentAIState->IsAlive())
			{
				// 패배한 AI는 즉시 다음 턴으로 (턴 시작 안 함)
				if (TurnComponent)
				{
					TurnComponent->NextTurn();
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
	
	// GameInstance에서 미리 생성한 PlayerState 0을 가져와서 사용
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		// PlayerIndex 0인 PlayerState 가져오기 (실제 플레이어용)
		ASuperPlayerState* PlayerState0 = GameInstance->GetPlayerState(0);
		if (PlayerState0)
		{
			// PlayerController의 PlayerState를 GameInstance의 것으로 교체
			NewPlayer->PlayerState = PlayerState0;
		}
	}
}

void ASuperGameModeBase::CreateAllPlayerStates()
{
	// GameInstance 가져오기
	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}
	
	// 기존 PlayerStates 제거 (중복 생성 방지)
	if (GameInstance->GetPlayerStateCount() > 0)
	{
		GameInstance->ClearAllPlayerStates();
	}
	
	// WorldConfig에서 플레이어 수 가져오기
	FWorldConfig WorldConfig = GameInstance->GetWorldConfig();
	int32 TotalPlayerCount = WorldConfig.PlayerCount;
	
	// 플레이어 수 유효성 체크 (2~8명)
	if (TotalPlayerCount < 2)
	{
		TotalPlayerCount = 2;
	}
	else if (TotalPlayerCount > 8)
	{
		TotalPlayerCount = 8;
	}
	
	// GameInstance에서 국가 배열 가져오기
	if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetGameInstance()))
	{
		TArray<FName> GameInstanceCountryNames = SuperGameInst->GetCountryNames();
		if (GameInstanceCountryNames.Num() > 0)
		{
			CountryNames = GameInstanceCountryNames;
		}
	}
	
	// CountryNames가 비어있으면 기본값으로 설정
	if (CountryNames.Num() == 0)
	{
		CountryNames = { 
			TEXT("England"),	// Player 0
			TEXT("Italy"),		// Player 1
			TEXT("France"),		// Player 2
			TEXT("Germany"),	// Player 3
			TEXT("Spain"),		// Player 4
			TEXT("Portugal"),	// Player 5
			TEXT("Russia"),		// Player 6
			TEXT("Hungary")		// Player 7
		};
	}
	
	// 모든 PlayerState 생성 (PlayerIndex 0 ~ TotalPlayerCount-1: 0=플레이어, 1~N=AI)
	for (int32 i = 0; i < TotalPlayerCount; i++)
	{
		// PlayerState 수동 생성
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		ASuperPlayerState* PlayerState = GetWorld()->SpawnActor<ASuperPlayerState>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (PlayerState)
		{
			// PlayerIndex 설정
			PlayerState->PlayerIndex = i;
			
			// GameInstance에 등록
			GameInstance->AddPlayerState(PlayerState);
			
			// 초기화
			PlayerState->InitializePlayer();
			
			// 국가 RowName 설정 (배열 범위 체크)
			if (i < CountryNames.Num() && CountryNames[i] != NAME_None)
			{
				PlayerState->CountryRowName = CountryNames[i];
				
				// 국가 데이터 로드
				PlayerState->LoadCountryDataFromTable();
			}
		}
	}
}

