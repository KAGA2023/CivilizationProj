// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DiplomacyStruct.h"
#include "DiplomacyManager.generated.h"

// 외교 액션 관련 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiplomacyActionIssued, const FDiplomacyAction&, Action);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDiplomacyActionResolved, const FDiplomacyAction&, Action, bool, bAccepted);

// 전쟁/평화 상태 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDiplomacyStatusChanged, int32, PlayerA, int32, PlayerB, EDiplomacyStatusType, NewStatus);

// 조약 활성/비활성 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnDiplomacyTreatyChanged, int32, PlayerA, int32, PlayerB, EDiplomacyTreatyType, TreatyType, bool, bActive);

UCLASS(BlueprintType)
class CIVILIZATION_API UDiplomacyManager : public UObject
{
	GENERATED_BODY()

public:
	// ================= 기본 설정 / 초기화 =================

	UDiplomacyManager();

	// 외교 데이터 초기화 (플레이어 수 기준)
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	void Initialize(int32 NumPlayers);

	// ================= 전쟁 / 평화 상태 =================

	// 전쟁/평화 상태 조회
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	EDiplomacyStatusType GetStatus(int32 PlayerA, int32 PlayerB) const;

	// 전쟁 중인지 여부 확인
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	bool IsAtWar(int32 PlayerA, int32 PlayerB) const;

	// 전쟁 선포
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	bool DeclareWar(int32 PlayerA, int32 PlayerB, int32 CurrentRound);

	// 평화 체결
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	bool MakePeace(int32 PlayerA, int32 PlayerB, int32 CurrentRound);

	// ================= 조약 =================

	// 조약 추가 (라운드 기반)
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	bool AddTreaty(int32 PlayerA, int32 PlayerB, EDiplomacyTreatyType TreatyType, int32 StartRound, int32 DurationRounds);

	// 조약 조기 종료 (과거 기록은 남기지 않음)
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	bool CancelTreaty(int32 PlayerA, int32 PlayerB, EDiplomacyTreatyType TreatyType, int32 CurrentRound);

	// 현재 라운드에 활성화된 조약이 있는지 확인
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	bool HasActiveTreaty(int32 PlayerA, int32 PlayerB, EDiplomacyTreatyType TreatyType, int32 CurrentRound) const;

	// 새 라운드 시작 시 호출 (조약 만료 정리 등)
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	void OnRoundStarted(int32 CurrentRound);

	// ================= 델리게이트 =================

	// 액션이 새로 발행되었을 때 (요청/선언/제안)
	UPROPERTY(BlueprintAssignable, Category = "Diplomacy|Events")
	FOnDiplomacyActionIssued OnDiplomacyActionIssued;

	// 액션이 수락/거절 등으로 처리되었을 때
	UPROPERTY(BlueprintAssignable, Category = "Diplomacy|Events")
	FOnDiplomacyActionResolved OnDiplomacyActionResolved;

	// 전쟁/평화 상태가 바뀌었을 때
	UPROPERTY(BlueprintAssignable, Category = "Diplomacy|Events")
	FOnDiplomacyStatusChanged OnDiplomacyStatusChanged;

	// 조약이 활성/비활성으로 바뀌었을 때
	UPROPERTY(BlueprintAssignable, Category = "Diplomacy|Events")
	FOnDiplomacyTreatyChanged OnDiplomacyTreatyChanged;

	// ================= 호감도 =================

	// From -> To 호감도 점수 가져오기
	UFUNCTION(BlueprintCallable, Category = "Diplomacy|Attitude")
	int32 GetAttitude(int32 FromPlayerId, int32 ToPlayerId) const;

	// From -> To 호감도 점수 설정
	UFUNCTION(BlueprintCallable, Category = "Diplomacy|Attitude")
	void SetAttitude(int32 FromPlayerId, int32 ToPlayerId, int32 NewScore);

	// From -> To 호감도 점수 변경
	UFUNCTION(BlueprintCallable, Category = "Diplomacy|Attitude")
	void AddAttitude(int32 FromPlayerId, int32 ToPlayerId, int32 Delta);

	// ================= 외교 액션 =================

	// 외교 액션 발행 (요청/선언/제안 등록)
	// 반환값: 부여된 ActionId
	UFUNCTION(BlueprintCallable, Category = "Diplomacy|Action")
	int32 IssueAction(const FDiplomacyAction& Action);

	// 외교 액션 처리 (수락/거절 등)
	UFUNCTION(BlueprintCallable, Category = "Diplomacy|Action")
	void ResolveAction(int32 ActionId, bool bAccepted);

	// 특정 플레이어에게 도착한 미처리 액션 목록 조회
	UFUNCTION(BlueprintCallable, Category = "Diplomacy|Action")
	TArray<FDiplomacyAction> GetPendingActionsForPlayer(int32 ToPlayerId) const;

	// ================= 내부 데이터 =================

	// 현재 게임의 플레이어 수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Diplomacy")
	int32 PlayerCount = 0;

	// 플레이어 쌍(A<->B)의 공통 상태/조약 데이터 맵
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Diplomacy")
	TMap<FDiplomacyPairKey, FDiplomacyPairState> PairStates;

	// 방향성(From -> To) 호감도 맵 (런타임 전용, UPROPERTY 아님)
	TMap<int32, TMap<int32, int32>> Attitudes;

	// 마지막으로 처리한 라운드 번호 (OnRoundStarted 중복 호출 방지용)
	int32 LastProcessedRound = -1;

	// 현재 라운드 번호 (외교 액션/조약에 사용)
	int32 CurrentRound = 1;

	// 처리 대기 중인 외교 액션들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Diplomacy|Action")
	TArray<FDiplomacyAction> PendingActions;

	// 다음에 부여할 액션 ID
	int32 NextActionId = 1;
};
