#include "SuperGameModeBase.h"
#include "SuperGameInstance.h"
#include "SuperGameController.h"
#include "SuperCameraPawn.h"
#include "SuperPlayerState.h"

ASuperGameModeBase::ASuperGameModeBase()
{
	// PlayerController, Pawn, PlayerState 클래스 설정
	PlayerControllerClass = ASuperGameController::StaticClass();
	DefaultPawnClass = ASuperCameraPawn::StaticClass();
	PlayerStateClass = ASuperPlayerState::StaticClass();
	
	// 기본값 초기화
	bIsGameActive = false;
	bIsGamePaused = false;
	bIsPlayerTurnActive = false;
	CurrentTurn = 1;
	CurrentPlayerIndex = 0;
	TurnTimeLimit = 60.0f; // 60초 턴 제한
	CurrentTurnTime = 0.0f;
	GameSpeed = 1.0f;
	MapSize = FVector2D(50, 50); // 기본 맵 크기 50x50
	MaxPlayers = 4;
	MaxTurns = 500; // 기본 최대 턴 수

	// 기본 플레이어들 추가 (테스트용)
	Players.Add(TEXT("Player 1"));
	Players.Add(TEXT("AI Player 1"));
}

void ASuperGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	
	// 모든 플레이어 스테이트 생성 (AI 3개)
	CreateAIPlayerStates();
	
	// 게임 초기화
	InitializeGame();
}

void ASuperGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsGameActive && !bIsGamePaused)
	{
		UpdateGameTime(DeltaTime);
	}
}

void ASuperGameModeBase::InitializeGame()
{
	// 게임 상태 초기화
	bIsGameActive = true;
	bIsGamePaused = false;
	bIsPlayerTurnActive = true;
	CurrentTurn = 1;
	CurrentPlayerIndex = 0;
	CurrentTurnTime = 0.0f;

	// 게임 시작 이벤트 브로드캐스트
	OnGameStarted.Broadcast();
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
	bIsPlayerTurnActive = false;

	// 월드 컴포넌트 정리 (메모리 해제)
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		GameInstance->ClearGeneratedWorldComponent();
	}

	// 게임 종료 이벤트 브로드캐스트
	OnGameEnded.Broadcast();
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
	if (!bIsGameActive || bIsGamePaused)
	{
		return;
	}

	// 현재 플레이어의 턴 종료
	EndCurrentPlayerTurn();

	// 다음 플레이어로 전환
	SwitchToNextPlayer();

	// 턴이 한 바퀴 돌았으면 새로운 턴 시작
	if (CurrentPlayerIndex == 0)
	{
		CurrentTurn++;
		OnTurnChanged.Broadcast(CurrentTurn);
	}

	// 새로운 플레이어 턴 시작
	bIsPlayerTurnActive = true;
	CurrentTurnTime = 0.0f;
	OnPlayerTurnChanged.Broadcast(CurrentPlayerIndex);

	// 게임 종료 조건 확인
	CheckGameEndConditions();
}

void ASuperGameModeBase::EndCurrentPlayerTurn()
{
	if (bIsPlayerTurnActive)
	{
		bIsPlayerTurnActive = false;
	}
}

void ASuperGameModeBase::AddPlayer(const FString& PlayerName)
{
	if (Players.Num() < MaxPlayers && !PlayerName.IsEmpty())
	{
		Players.Add(PlayerName);
	}
}

void ASuperGameModeBase::RemovePlayer(int32 PlayerIndex)
{
	if (Players.IsValidIndex(PlayerIndex))
	{
		Players.RemoveAt(PlayerIndex);
		
		// 현재 플레이어 인덱스 조정
		if (CurrentPlayerIndex > PlayerIndex)
		{
			CurrentPlayerIndex--;
		}
	}
}

FString ASuperGameModeBase::GetPlayerName(int32 PlayerIndex) const
{
	if (Players.IsValidIndex(PlayerIndex))
	{
		return Players[PlayerIndex];
	}
	return TEXT("Invalid PlayerIndex");
}

void ASuperGameModeBase::SetMapSize(int32 Width, int32 Height)
{
	MapSize.X = FMath::Clamp(Width, 10, 200);  // 최소 10x10, 최대 200x200
	MapSize.Y = FMath::Clamp(Height, 10, 200);
}

void ASuperGameModeBase::SetMaxTurns(int32 NewMaxTurns)
{
	MaxTurns = FMath::Max(NewMaxTurns, 0); // 최소 0 (0이면 무제한)
}

void ASuperGameModeBase::SwitchToNextPlayer()
{
	CurrentPlayerIndex = (CurrentPlayerIndex + 1) % Players.Num();
}

void ASuperGameModeBase::CheckGameEndConditions()
{
	// 게임 종료 조건들 (예시)
	// 1. 한 플레이어만 남았을 때
	if (Players.Num() <= 1)
	{
		EndGame();
		return;
	}

	// 2. 최대 턴 수 도달 (MaxTurns가 0이면 무제한)
	if (MaxTurns > 0 && CurrentTurn > MaxTurns)
	{
		EndGame();
		return;
	}

	// 여기에 추가적인 승리 조건들을 구현할 수 있습니다
	// - 점수 승리
	// - 지배 승리
	// - 과학 승리
	// - 외교 승리
}

void ASuperGameModeBase::UpdateGameTime(float DeltaTime)
{
	if (bIsPlayerTurnActive)
	{
		CurrentTurnTime += DeltaTime;

		// 턴 시간 제한 체크 (옵션)
		if (TurnTimeLimit > 0 && CurrentTurnTime >= TurnTimeLimit)
		{
			NextTurn();
		}
	}
}

void ASuperGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	// PlayerState는 PostLogin 이후에 생성됨
	if (ASuperPlayerState* PlayerState = Cast<ASuperPlayerState>(NewPlayer->PlayerState))
	{
		// PlayerIndex 0으로 설정 (실제 플레이어)
		PlayerState->PlayerIndex = 0;
		
		// GameInstance에 등록
		if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
		{
			GameInstance->AddPlayerState(PlayerState);
		}
	}
}

void ASuperGameModeBase::CreateAIPlayerStates()
{
	// GameInstance 가져오기
	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}
	
	// AI PlayerState 3개 생성 (Player 1~3)
	for (int32 i = 1; i <= 3; i++)
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

