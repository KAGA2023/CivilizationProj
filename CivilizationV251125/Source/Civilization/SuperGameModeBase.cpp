#include "SuperGameModeBase.h"
#include "SuperGameInstance.h"
#include "SuperGameController.h"
#include "SuperCameraPawn.h"
#include "SuperPlayerState.h"
#include "Unit/UnitManager.h"

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

	// 다음 턴으로 진행
	TurnComponent->NextTurn();

	// 게임 종료 조건 확인
	CheckGameEndConditions();
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
	}
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
		}
	}
}

