#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Turn/TurnComponent.h"
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
	virtual void PostLogin(APlayerController* NewPlayer) override;

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

	// 턴 관리
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	UTurnComponent* GetTurnComponent() const { return TurnComponent; }

	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	void NextTurn(); // 다음 턴으로 진행합니다

	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	void EndCurrentPlayerTurn(); // 현재 플레이어의 턴을 종료합니다

	// 이벤트 델리게이트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStarted); // 게임이 시작될 때 호출되는 델리게이트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameEnded); // 게임이 종료될 때 호출되는 델리게이트

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

	// 턴 관리 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn Management")
	TObjectPtr<UTurnComponent> TurnComponent;

	// 내부 함수들
	void InitializeGame(); // 게임을 초기화합니다
	void CheckGameEndConditions(); // 게임 종료 조건을 확인합니다
	void CreateAllPlayerStates(); // 모든 플레이어 스테이트 생성 (Player 0 + AI 1~3)
};
