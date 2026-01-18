// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AIPlayerStruct.h"
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
	// ================= 기본 설정 / 초기화 =================

	UAIPlayerManager();

	// AI 플레이어 데이터 초기화 (게임 시작 시 호출)
	UFUNCTION(BlueprintCallable, Category = "AI Player")
	void Initialize();

	// ================= AI 플레이어 관리 =================

	// AI 플레이어 맵
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Player")
	TMap<int32, FAIPlayerStruct> AIPlayers;

	// AI 플레이어 등록
	UFUNCTION(BlueprintCallable, Category = "AI Player")
	void RegisterAIPlayer(int32 PlayerIndex, class ASuperPlayerState* PlayerState);

	// AI 플레이어 가져오기
	UFUNCTION(BlueprintCallable, Category = "AI Player")
	FAIPlayerStruct GetAIPlayer(int32 PlayerIndex) const;

	// AI 플레이어 유효성 확인
	UFUNCTION(BlueprintCallable, Category = "AI Player")
	bool IsAIPlayerValid(int32 PlayerIndex) const;

	// ================= 턴 관리 =================

	// AI 턴 시작
	UFUNCTION(BlueprintCallable, Category = "AI Turn")
	void StartAITurn(int32 PlayerIndex);

	// AI 턴 완료 여부 확인
	UFUNCTION(BlueprintCallable, Category = "AI Turn")
	bool IsAITurnComplete(int32 PlayerIndex) const;

	// AI 턴 종료
	UFUNCTION(BlueprintCallable, Category = "AI Turn")
	void EndAITurn(int32 PlayerIndex);

	// ================= 비동기 작업 완료 콜백 =================

	// 유닛 이동 완료 콜백 (UnitManager에서 호출)
	UFUNCTION(BlueprintCallable, Category = "AI Async")
	void OnUnitMovementFinished(int32 PlayerIndex);

	// 전투 액션 완료 콜백 (UnitManager에서 호출)
	UFUNCTION(BlueprintCallable, Category = "AI Async")
	void OnCombatActionFinished(int32 PlayerIndex);

	// ================= 상태머신 =================

	// 상태머신 업데이트 (매 틱/프레임 또는 이벤트에서 호출)
	UFUNCTION(BlueprintCallable, Category = "AI State Machine")
	void UpdateStateMachine(int32 PlayerIndex);

	// 비동기 작업 대기 중인지 확인
	UFUNCTION(BlueprintCallable, Category = "AI State Machine")
	bool HasPendingAsyncWork(int32 PlayerIndex) const;

protected:
	// ================= 상태 처리 함수들 (5단계에서 구현) =================

	// 상태별 처리 함수들 (private, 나중에 구현)
	void ProcessDiplomacyState(int32 PlayerIndex);
	void ProcessResearchState(int32 PlayerIndex);
	void ProcessCityProductionState(int32 PlayerIndex);
	void ProcessTilePurchaseState(int32 PlayerIndex);
	void ProcessFacilityState(int32 PlayerIndex);
	void ProcessBuilderMovementState(int32 PlayerIndex);
	void ProcessBuilderBuildState(int32 PlayerIndex);
	void ProcessCombatUnitMovementState(int32 PlayerIndex);
	void ProcessCombatUnitCombatState(int32 PlayerIndex);
	void ProcessUnitMovementState(int32 PlayerIndex);

	// ================= 상태 전환 =================

	// 다음 상태로 전환
	void TransitionToNextState(int32 PlayerIndex);
	// ================= 헬퍼 함수 =================

	// AI 플레이어 가져오기 (포인터 반환, 내부 사용)
	FAIPlayerStruct* GetAIPlayerPtr(int32 PlayerIndex);

	// AI 플레이어 새 턴 초기화
	void ResetAIPlayerForNewTurn(FAIPlayerStruct& AIPlayer);

	// 현재 라운드 번호 가져오기 (TurnComponent에서, 실패 시 기본값 1 반환)
	int32 GetCurrentRound() const;

	// ================= 유닛 이동 처리 함수 =================

	// 병사 유닛 이동 처리 (평화 상태)
	// 전쟁 중이 아닐 때만 병사 유닛을 배회시킴
	// ================= 유닛 이동 헬퍼 함수 =================

	// 유닛의 현재 위치 찾기
	// @param Unit 찾을 유닛
	// @param UnitManager 유닛 매니저
	// @param AllTiles 모든 타일 배열
	// @param OutPosition 찾은 위치를 저장할 변수
	// @return 위치를 찾았으면 true, 실패 시 false
	bool FindUnitPosition(
		AUnitCharacterBase* Unit,
		UUnitManager* UnitManager,
		const TArray<UWorldTile*>& AllTiles,
		FVector2D& OutPosition
	);

	// 유닛을 목표 타일로 이동 시도
	// 경로를 찾고 실제 도착 타일을 예약한 후 이동 시작
	// @param Unit 이동할 유닛
	// @param StartPosition 시작 위치
	// @param TargetTile 목표 타일
	// @param UnitManager 유닛 매니저
	// @param OutPendingMovements 대기 중인 이동 수 (참조로 증가됨)
	// @return 이동 시작 성공 시 true, 실패 시 false
	bool TryMoveUnitToTile(
		AUnitCharacterBase* Unit,
		FVector2D StartPosition,
		FVector2D TargetTile,
		UUnitManager* UnitManager,
		int32& OutPendingMovements
	);

	// 배회 타일 찾기
	// 여러 번 시도하여 유효한 배회 타일을 찾음
	// @param PlayerIndex AI 플레이어 인덱스
	// @param Radius 도시 기준 반경 (기본값: 2)
	// @param MaxAttempts 최대 시도 횟수 (기본값: 10)
	// @return 유효한 배회 타일 좌표, 실패 시 FVector2D(-1, -1)
	FVector2D FindValidWanderTile(
		int32 PlayerIndex,
		int32 Radius = 2,
		int32 MaxAttempts = 10
	);

	// ================= 전쟁 상태 헬퍼 함수 =================

	// 4칸 반경 내 적 유닛 탐지 (거리 가까운 순 정렬)
	// @param CombatUnitPosition 병사 유닛의 현재 위치
	// @param DetectionRange 탐지 범위 (기본값: 4)
	// @param PlayerIndex 현재 AI 플레이어 인덱스
	// @param UnitManager 유닛 매니저
	// @param WorldComponent 월드 컴포넌트
	// @param DiplomacyManager 외교 매니저
	// @return 적 유닛 배열 (거리 가까운 순 정렬), 없으면 빈 배열
	TArray<AUnitCharacterBase*> FindEnemyUnitsInRange(
		FVector2D CombatUnitPosition,
		int32 DetectionRange,
		int32 PlayerIndex,
		UUnitManager* UnitManager,
		UWorldComponent* WorldComponent,
		UDiplomacyManager* DiplomacyManager
	);

	// 가장 가까운 적 도시 찾기
	// @param PlayerIndex 현재 AI 플레이어 인덱스
	// @param PlayerState 현재 플레이어 상태
	// @param GameInstance 게임 인스턴스
	// @param WorldComponent 월드 컴포넌트
	// @param DiplomacyManager 외교 매니저
	// @return 가장 가까운 적 도시 좌표, 없으면 FVector2D(-1, -1)
	FVector2D FindClosestEnemyCity(
		int32 PlayerIndex,
		ASuperPlayerState* PlayerState,
		USuperGameInstance* GameInstance,
		UWorldComponent* WorldComponent,
		UDiplomacyManager* DiplomacyManager
	);

	// 사거리 내 적 도시 찾기 (거리 가까운 순 정렬)
	// @param CombatUnitPosition 전투 유닛의 현재 위치
	// @param AttackRange 공격 사거리
	// @param PlayerIndex 현재 AI 플레이어 인덱스
	// @param WorldComponent 월드 컴포넌트
	// @param DiplomacyManager 외교 매니저
	// @param GameInstance 게임 인스턴스
	// @return 적 도시 좌표 배열 (거리 가까운 순 정렬), 없으면 빈 배열
	TArray<FVector2D> FindEnemyCitiesInRange(
		FVector2D CombatUnitPosition,
		int32 AttackRange,
		int32 PlayerIndex,
		UWorldComponent* WorldComponent,
		UDiplomacyManager* DiplomacyManager,
		USuperGameInstance* GameInstance
	);

};

