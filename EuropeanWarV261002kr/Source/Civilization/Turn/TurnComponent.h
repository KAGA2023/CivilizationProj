// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurnStruct.h"
#include "TurnComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CIVILIZATION_API UTurnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTurnComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ========== 턴 조회 ==========

	// 현재 라운드·턴·슬롯 정보를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	FTurnStruct GetCurrentTurn() const { return CurrentTurn; }

	// 현재 라운드 번호를 반환합니다. 1부터 시작합니다.
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	int32 GetCurrentRoundNumber() const { return CurrentTurn.RoundNumber; }

	// 라운드 안에서의 턴 번호를 반환합니다. 1~N입니다.
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	int32 GetCurrentTurnNumber() const { return CurrentTurn.TurnNumber; }

	// 지금 턴인 문명 슬롯을 반환합니다. 0=호스트/싱글, 1=멀티 참가자, 나머지는 AI.
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	int32 GetCurrentPlayerIndex() const { return CurrentTurn.PlayerIndex; }

	// ========== 턴 진행 ==========

	// 다음 슬롯으로 턴을 넘깁니다. 마지막이면 다음 라운드로 갑니다.
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	void NextTurn();

	// 지정한 라운드의 턴 1, 슬롯 0으로 초기화합니다. 게임 시작·로드에 씁니다.
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	void InitializeTurn(int32 StartRound = 1);

	// 남은 턴을 건너뛰고 다음 라운드 턴 1로 갑니다.
	UFUNCTION(BlueprintCallable, Category = "Round Management")
	void NextRound();

	// 라운드·턴·슬롯 값이 서로 맞는지 확인합니다.
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	bool IsValid() const;

	// ========== 복제 ==========

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ========== 이벤트 ==========

	// 턴이 바뀔 때 브로드캐스트하는 델리게이트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnChanged, FTurnStruct, NewTurn);

	// 라운드가 바뀔 때 브로드캐스트하는 델리게이트
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundChanged, FTurnStruct, NewTurn);

	// 턴이 바뀔 때 브로드캐스트합니다.
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTurnChanged OnTurnChanged;

	// 라운드가 바뀔 때 브로드캐스트합니다.
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRoundChanged OnRoundChanged;

protected:
	// ========== 상태 ==========

	// 현재 라운드·턴·슬롯. SuperGameState에 붙으면 참가자에게 복제됩니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentTurn, Category = "Turn Info")
	FTurnStruct CurrentTurn;

	// 복제된 턴이 바뀌면 참가자 쪽에 턴·라운드 이벤트를 올립니다.
	UFUNCTION()
	void OnRep_CurrentTurn(FTurnStruct PreviousTurn);

	// TurnNumber에서 PlayerIndex를 맞춥니다. (TurnNumber - 1)
	void UpdatePlayerIndex();
};
