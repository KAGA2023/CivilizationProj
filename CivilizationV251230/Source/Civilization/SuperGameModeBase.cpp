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
	
	// 모든 플레이어 스테이트 생성 (Player 0 + AI 1~3, 총 4개)
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
	
	// 현재 플레이어가 AI인지 확인
	if (CurrentPlayerIndex >= 1 && CurrentPlayerIndex <= 3) // AI 플레이어
	{
		if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
		{
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

	// 플레이어 턴이거나 AI 턴이 완료된 경우 다음 턴으로 진행
	/*TurnComponent->NextTurn();*/

	// 게임 종료 조건 확인
	/*CheckGameEndConditions();*/
}

void ASuperGameModeBase::EndCurrentPlayerTurn()
{
	if (!TurnComponent || !bIsGameActive)
	{
		return;
	}

	// 현재 플레이어의 PlayerState 가져오기
	int32 CurrentPlayerIndex = TurnComponent->GetCurrentPlayerIndex();
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		if (ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(CurrentPlayerIndex))
		{
			// 플레이어의 턴 종료 처리 (자원 생산 등)
			PlayerState->ProcessTurnResources();
		}

		// 현재 플레이어의 모든 유닛 이동력 회복
		if (UUnitManager* UnitManager = GameInstance->GetUnitManager())
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
	// Player 1 → Player 2 → Player 3 → Player 0 순서로 진행
}

void ASuperGameModeBase::CheckGameEndConditions()
{
	// 게임 종료 조건들 (예시)
	// 여기에 추가적인 승리 조건들을 구현할 수 있습니다
	// - 점수 승리
	// - 지배 승리
	// - 과학 승리
	// - 외교 승리
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
	
	// AI 플레이어인지 확인 (1~3)
	if (CurrentPlayerIndex >= 1 && CurrentPlayerIndex <= 3)
	{
		if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
		{
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
	
	// 국가 RowName 배열 (플레이어 순서대로)
	TArray<FName> CountryNames = { 
		TEXT("England"),  // Player 0
		TEXT("Italy"),    // Player 1
		TEXT("France"),   // Player 2
		TEXT("Germany")   // Player 3
	};
	
	// 모든 PlayerState 생성 (PlayerIndex 0~3: 0=플레이어, 1~3=AI)
	for (int32 i = 0; i <= 3; i++)
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

