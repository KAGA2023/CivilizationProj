// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurnStruct.h"
#include "TurnComponent.generated.h"

/**
 * 라운드 및 턴 관리 컴포넌트
 * 라운드와 각 플레이어의 턴을 함께 관리합니다.
 * 
 * 라운드 및 턴 진행 순서:
 * - Round 1, Turn 1: Player 0 (PlayerIndex 0)
 * - Round 1, Turn 2: AI 1 (PlayerIndex 1)
 * - Round 1, Turn 3: AI 2 (PlayerIndex 2)
 * - Round 1, Turn 4: AI 3 (PlayerIndex 3)
 * - Round 2, Turn 1: Player 0 (PlayerIndex 0)
 * - ...
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CIVILIZATION_API UTurnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTurnComponent();

protected:
	virtual void BeginPlay() override;

public:
	// 현재 턴 정보 가져오기
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	FTurnStruct GetCurrentTurn() const { return CurrentTurn; }

	// 현재 라운드 번호 가져오기
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	int32 GetCurrentRoundNumber() const { return CurrentTurn.RoundNumber; }

	// 현재 턴 번호 가져오기 (라운드 내에서의 턴 번호: 1~4)
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	int32 GetCurrentTurnNumber() const { return CurrentTurn.TurnNumber; }

	// 현재 플레이어 인덱스 가져오기 (0=Player, 1~3=AI)
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	int32 GetCurrentPlayerIndex() const { return CurrentTurn.PlayerIndex; }

	// 다음 턴으로 진행
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	void NextTurn();

	// 턴 초기화 (게임 시작 시 또는 세이브/로드 시 사용)
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	void InitializeTurn(int32 StartRound = 1);

	// 다음 라운드로 진행 (현재 라운드의 모든 턴을 건너뛰고 다음 라운드로)
	UFUNCTION(BlueprintCallable, Category = "Round Management")
	void NextRound();

	// 라운드 및 턴이 유효한지 확인
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	bool IsValid() const;

	// 이벤트 델리게이트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnChanged, FTurnStruct, NewTurn);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundChanged, FTurnStruct, NewTurn);
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTurnChanged OnTurnChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRoundChanged OnRoundChanged;

protected:
	// 현재 턴 정보
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn Info")
	FTurnStruct CurrentTurn;

	// 내부 함수들
	void UpdatePlayerIndex(); // TurnNumber에 따라 PlayerIndex 업데이트
};

