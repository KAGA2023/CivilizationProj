// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AIPlayerStruct.h"
#include "AIPlayerManager.generated.h"

class ASuperPlayerState;

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
	void ProcessUnitMovementState(int32 PlayerIndex);

	// ================= 상태 전환 =================

	// 다음 상태로 전환
	void TransitionToNextState(int32 PlayerIndex);
	// ================= 헬퍼 함수 =================

	// AI 플레이어 가져오기 (포인터 반환)
	FAIPlayerStruct* GetAIPlayerPtr(int32 PlayerIndex);

	// AI 플레이어 새 턴 초기화
	void ResetAIPlayerForNewTurn(FAIPlayerStruct& AIPlayer);
};

