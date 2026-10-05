// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AIPlayerStruct.h"
#include "TimerManager.h"
#include "AIPlayerManager.generated.h"

class ASuperPlayerState;
class USuperGameInstance;
class UWorldComponent;
class UUnitManager;
class UWorldTile;
class AUnitCharacterBase;
class UDiplomacyManager;

UCLASS(BlueprintType)
class CIVILIZATION_API UAIPlayerManager : public UObject
{
	GENERATED_BODY()

public:
	// ========== 기본 설정 / 초기화 ==========

	UAIPlayerManager();

	// AI 슬롯 데이터를 만들고 턴 상태머신을 준비합니다.
	UFUNCTION(BlueprintCallable, Category = "AI Player")
	void Initialize();

	// ========== AI 플레이어 관리 ==========

	// 문명 슬롯 → AI 턴 데이터. 사람 슬롯은 넣지 않습니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Player")
	TMap<int32, FAIPlayerStruct> AIPlayers;

	// 해당 슬롯을 AI로 등록하고 PlayerState를 연결합니다.
	UFUNCTION(BlueprintCallable, Category = "AI Player")
	void RegisterAIPlayer(int32 PlayerIndex, class ASuperPlayerState* PlayerState);

	// 해당 슬롯의 AI 데이터를 복사해 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "AI Player")
	FAIPlayerStruct GetAIPlayer(int32 PlayerIndex) const;

	// 해당 슬롯이 등록된 AI인지 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "AI Player")
	bool IsAIPlayerValid(int32 PlayerIndex) const;

	// ========== 턴 관리 ==========

	// 해당 AI 슬롯의 턴을 시작합니다.
	UFUNCTION(BlueprintCallable, Category = "AI Turn")
	void StartAITurn(int32 PlayerIndex);

	// 해당 AI 슬롯의 턴이 끝났는지 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "AI Turn")
	bool IsAITurnComplete(int32 PlayerIndex) const;

	// 해당 AI 슬롯의 턴을 종료합니다.
	UFUNCTION(BlueprintCallable, Category = "AI Turn")
	void EndAITurn(int32 PlayerIndex);

	// ========== 비동기 작업 완료 콜백 ==========

	// 유닛 이동이 끝나면 UnitManager가 호출합니다.
	UFUNCTION(BlueprintCallable, Category = "AI Async")
	void OnUnitMovementFinished(int32 PlayerIndex);

	// 전투 연출이 끝나면 UnitManager가 호출합니다.
	UFUNCTION(BlueprintCallable, Category = "AI Async")
	void OnCombatActionFinished(int32 PlayerIndex);

	// ========== 상태머신 ==========

	// 해당 AI 슬롯의 상태머신을 한 단계 진행합니다.
	UFUNCTION(BlueprintCallable, Category = "AI State Machine")
	void UpdateStateMachine(int32 PlayerIndex);

	// 이동·전투 등 비동기 작업이 남았는지 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "AI State Machine")
	bool HasPendingAsyncWork(int32 PlayerIndex) const;

	// 다음 턴 전환 1초 대기가 끝나면 호출합니다. 재귀 Broadcast를 막습니다.
	UFUNCTION()
	void OnNextTurnDelayTimerExpired();

protected:
	// 다음 턴으로 넘기기 전 대기 타이머
	FTimerHandle NextTurnDelayTimerHandle;

	// 연속 전투 시 이전 복귀 연출이 정리되도록 전투 시작을 0.3초 늦춥니다.
	FTimerHandle DelayedCombatStartTimerHandle;

	// ========== 상태 처리 ==========

	// 외교 단계를 처리합니다.
	void ProcessDiplomacyState(int32 PlayerIndex);

	// 연구 단계를 처리합니다.
	void ProcessResearchState(int32 PlayerIndex);

	// 도시 생산 단계를 처리합니다.
	void ProcessCityProductionState(int32 PlayerIndex);

	// 타일 구매 단계를 처리합니다.
	void ProcessTilePurchaseState(int32 PlayerIndex);

	// 시설 건설 목표를 고릅니다.
	void ProcessFacilityState(int32 PlayerIndex);

	// 건설자를 목표 타일로 보냅니다.
	void ProcessBuilderMovementState(int32 PlayerIndex);

	// 건설자가 시설을 짓거나 수리합니다.
	void ProcessBuilderBuildState(int32 PlayerIndex);

	// 병사를 전쟁·배회 목표로 보냅니다.
	void ProcessCombatUnitMovementState(int32 PlayerIndex);

	// 병사 전투를 처리합니다.
	void ProcessCombatUnitCombatState(int32 PlayerIndex);

	// 일반 유닛 이동을 처리합니다.
	void ProcessUnitMovementState(int32 PlayerIndex);

	// ========== 상태 전환 ==========

	// 다음 턴 단계로 넘깁니다.
	void TransitionToNextState(int32 PlayerIndex);

	// ========== 헬퍼 ==========

	// 내부에서 쓰는 AI 데이터 포인터를 반환합니다.
	FAIPlayerStruct* GetAIPlayerPtr(int32 PlayerIndex);

	// 새 턴에 맞춰 AI 진행 값을 초기화합니다.
	void ResetAIPlayerForNewTurn(FAIPlayerStruct& AIPlayer);

	// 갇힌 AI 유닛을 지우고 인구를 되돌립니다. 턴 시작 때 호출합니다.
	void RemoveTrappedUnitsForAIPlayer(int32 PlayerIndex);

	// 현재 라운드 번호를 반환합니다. 실패하면 1입니다.
	int32 GetCurrentRound() const;

	// ========== 유닛 이동 ==========

	// 유닛의 현재 hex를 찾습니다.
	bool FindUnitPosition(
		AUnitCharacterBase* Unit,
		UUnitManager* UnitManager,
		const TArray<UWorldTile*>& AllTiles,
		FVector2D& OutPosition
	);

	// 경로를 찾아 목표 타일로 이동을 시작합니다.
	bool TryMoveUnitToTile(
		AUnitCharacterBase* Unit,
		FVector2D StartPosition,
		FVector2D TargetTile,
		UUnitManager* UnitManager,
		int32& OutPendingMovements
	);

	// 경로가 있는 배회 타일을 여러 번 시도해 찾습니다. 실패하면 (-1, -1)입니다.
	FVector2D FindValidWanderTile(
		int32 PlayerIndex,
		int32 Radius = 2,
		int32 MaxAttempts = 10,
		AUnitCharacterBase* Unit = nullptr
	);

	// ========== 전쟁 헬퍼 ==========

	// 사거리 안 적 유닛을 가까운 순으로 찾습니다.
	TArray<AUnitCharacterBase*> FindEnemyUnitsInRange(
		FVector2D CombatUnitPosition,
		int32 DetectionRange,
		int32 PlayerIndex,
		UUnitManager* UnitManager,
		UWorldComponent* WorldComponent,
		UDiplomacyManager* DiplomacyManager
	);

	// 가장 가까운 적 도시 hex를 반환합니다. 없으면 (-1, -1)입니다.
	FVector2D FindClosestEnemyCity(
		int32 PlayerIndex,
		ASuperPlayerState* PlayerState,
		USuperGameInstance* GameInstance,
		UWorldComponent* WorldComponent,
		UDiplomacyManager* DiplomacyManager
	);

	// 사거리 안 적 도시를 가까운 순으로 찾습니다.
	TArray<FVector2D> FindEnemyCitiesInRange(
		FVector2D CombatUnitPosition,
		int32 AttackRange,
		int32 PlayerIndex,
		UWorldComponent* WorldComponent,
		UDiplomacyManager* DiplomacyManager,
		USuperGameInstance* GameInstance
	);

};
