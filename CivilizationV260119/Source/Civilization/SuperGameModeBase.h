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
	void NextTurn(); // 다음 턴으로 진행합니다(AI 턴 처리용)

	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	void EndCurrentPlayerTurn(); // 현재 플레이어의 턴을 종료합니다

	// 플레이어 0의 턴 종료 요청 (UI에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	void RequestEndPlayerTurn(); // 플레이어 0의 턴을 종료하고 다음 턴으로 진행합니다(UI에서 호출)

	// 국가 배열 설정 (UI에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void SetCountryNames(const TArray<FName>& InCountryNames);

protected:
	// 게임 상태 변수들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
	bool bIsGameActive; // 게임이 활성화되어 있는지 여부

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
	bool bIsGamePaused; // 게임이 일시정지 상태인지 여부

	// 턴 관리 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn Management")
	TObjectPtr<UTurnComponent> TurnComponent;

	// 국가 RowName 배열 (최대 8명까지 지원)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
	TArray<FName> CountryNames;

	// 내부 함수들
	void InitializeGame(); // 게임을 초기화합니다
	void CheckGameEndConditions(); // 게임 종료 조건을 확인합니다
	void CreateAllPlayerStates(); // 모든 플레이어 스테이트 생성 (Player 0 + AI, 동적)

	// 라운드 변경 이벤트 핸들러
	UFUNCTION()
	void HandleRoundChanged(FTurnStruct NewTurn);
	
	// 턴 변경 이벤트 핸들러 (AI 턴 처리용)
	UFUNCTION()
	void HandleTurnChanged(FTurnStruct NewTurn);
};
