#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SuperGameModeBase.generated.h"

UCLASS(BlueprintType, Blueprintable)
class CIVILIZATION_API ASuperGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASuperGameModeBase();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	// 게임 상태 관리
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void StartNewGame(); // 새 게임을 시작합니다

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void EndGame(); // 현재 게임을 종료합니다

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void PauseGame(); // 게임을 일시정지합니다

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void ResumeGame(); // 일시정지된 게임을 재개합니다

	// 턴 관리 시스템
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	void NextTurn(); // 다음 턴으로 진행합니다

	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	void EndCurrentPlayerTurn(); // 현재 플레이어의 턴을 종료합니다

	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	int32 GetCurrentTurn() const { return CurrentTurn; } // 현재 턴 수를 반환합니다

	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	int32 GetCurrentPlayerIndex() const { return CurrentPlayerIndex; } // 현재 플레이어 인덱스를 반환합니다

	// 플레이어 관리
	UFUNCTION(BlueprintCallable, Category = "Player Management")
	void AddPlayer(const FString& PlayerName); // 새로운 플레이어를 추가합니다

	UFUNCTION(BlueprintCallable, Category = "Player Management")
	void RemovePlayer(int32 PlayerIndex); // 지정된 플레이어를 제거합니다

	UFUNCTION(BlueprintCallable, Category = "Player Management")
	int32 GetPlayerCount() const { return Players.Num(); } // 총 플레이어 수를 반환합니다

	UFUNCTION(BlueprintCallable, Category = "Player Management")
	FString GetPlayerName(int32 PlayerIndex) const; // 지정된 플레이어의 이름을 반환합니다

	// 게임 설정
	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	void SetGameSpeed(float Speed) { GameSpeed = Speed; } // 게임 진행 속도를 설정합니다

	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	float GetGameSpeed() const { return GameSpeed; } // 현재 게임 속도를 반환합니다

	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	void SetMapSize(int32 Width, int32 Height); // 맵 크기를 설정합니다

	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	FVector2D GetMapSize() const { return MapSize; } // 현재 맵 크기를 반환합니다

	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	void SetMaxTurns(int32 NewMaxTurns); // 최대 턴 수를 설정합니다

	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	int32 GetMaxTurns() const { return MaxTurns; } // 현재 최대 턴 수를 반환합니다

	// 이벤트 델리게이트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnChanged, int32, NewTurn); // 새로운 턴이 시작될 때 호출되는 델리게이트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerTurnChanged, int32, PlayerIndex); // 플레이어 턴이 변경될 때 호출되는 델리게이트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStarted); // 게임이 시작될 때 호출되는 델리게이트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameEnded); // 게임이 종료될 때 호출되는 델리게이트

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTurnChanged OnTurnChanged; // 턴 변경 이벤트

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerTurnChanged OnPlayerTurnChanged; // 플레이어 턴 변경 이벤트

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGameStarted OnGameStarted; // 게임 시작 이벤트

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGameEnded OnGameEnded; // 게임 종료 이벤트

protected:
	// 게임 상태 변수들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
	bool bIsGameActive; // 게임이 활성화되어 있는지 여부

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
	bool bIsGamePaused; // 게임이 일시정지 상태인지 여부

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
	bool bIsPlayerTurnActive; // 현재 플레이어의 턴이 활성화되어 있는지 여부

	// 턴 관리 변수들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Management")
	int32 CurrentTurn; // 현재 턴 수

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Management")
	int32 CurrentPlayerIndex; // 현재 플레이어의 인덱스

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Management")
	float TurnTimeLimit; // 턴 시간 제한 (초 단위, 0이면 무제한)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Management")
	float CurrentTurnTime; // 현재 턴의 경과 시간

	// 플레이어 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Management")
	TArray<FString> Players; // 플레이어 이름 목록

	// 게임 설정 변수들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
	float GameSpeed; // 게임 진행 속도 배율 (1.0 = 기본속도)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
	FVector2D MapSize; // 맵 크기 (X=너비, Y=높이)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
	int32 MaxPlayers; // 최대 플레이어 수

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
	int32 MaxTurns; // 최대 턴 수 (0이면 무제한)

	// 내부 함수들
	void InitializeGame(); // 게임을 초기화합니다
	void ProcessTurn(); // 턴을 처리합니다
	void SwitchToNextPlayer(); // 다음 플레이어로 전환합니다
	void CheckGameEndConditions(); // 게임 종료 조건을 확인합니다
	void UpdateGameTime(float DeltaTime); // 게임 시간을 업데이트합니다
};
