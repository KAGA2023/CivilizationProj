// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DiplomacyStruct.h"
#include "DiplomacyManager.generated.h"

// 외교 액션이 새로 발행되었을 때 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiplomacyActionIssued, const FDiplomacyAction&, Action);

// 외교 액션이 수락·거절로 처리되었을 때 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDiplomacyActionResolved, const FDiplomacyAction&, Action, bool, bAccepted);

// 전쟁·평화·동맹 상태가 바뀌었을 때 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDiplomacyStatusChanged, int32, PlayerA, int32, PlayerB, EDiplomacyStatusType, NewStatus);

UCLASS(BlueprintType)
class CIVILIZATION_API UDiplomacyManager : public UObject
{
	GENERATED_BODY()

public:
	// ========== 기본 설정 / 초기화 ==========

	UDiplomacyManager();

	// 플레이어 수 기준으로 쌍 상태와 호감도를 초기화합니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	void Initialize(int32 NumPlayers);

	// ========== 전쟁 / 평화 상태 ==========

	// 두 슬롯 사이의 전쟁·평화·동맹 상태를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	EDiplomacyStatusType GetStatus(int32 PlayerA, int32 PlayerB) const;

	// 두 슬롯이 전쟁 중인지 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	bool IsAtWar(int32 PlayerA, int32 PlayerB) const;

	// 전쟁을 선포합니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	bool DeclareWar(int32 PlayerA, int32 PlayerB, int32 CurrentRound);

	// 평화를 체결합니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	bool MakePeace(int32 PlayerA, int32 PlayerB, int32 CurrentRound);

	// 동맹을 체결합니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	bool MakeAlliance(int32 PlayerA, int32 PlayerB, int32 CurrentRound);

	// 새 라운드 시작 시 호출합니다. 동맹 만료 등을 정리합니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	void OnRoundStarted(int32 CurrentRound);

	// ========== 델리게이트 ==========

	// 액션이 새로 발행되었을 때 (요청/선언/제안)
	UPROPERTY(BlueprintAssignable, Category = "Diplomacy|Events")
	FOnDiplomacyActionIssued OnDiplomacyActionIssued;

	// 액션이 수락/거절 등으로 처리되었을 때
	UPROPERTY(BlueprintAssignable, Category = "Diplomacy|Events")
	FOnDiplomacyActionResolved OnDiplomacyActionResolved;

	// 전쟁/평화 상태가 바뀌었을 때
	UPROPERTY(BlueprintAssignable, Category = "Diplomacy|Events")
	FOnDiplomacyStatusChanged OnDiplomacyStatusChanged;

	// ========== 호감도 ==========

	// From -> To 호감도 점수를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy|Attitude")
	int32 GetAttitude(int32 FromPlayerId, int32 ToPlayerId) const;

	// From -> To 호감도 점수를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy|Attitude")
	void SetAttitude(int32 FromPlayerId, int32 ToPlayerId, int32 NewScore);

	// From -> To 호감도 점수를 더합니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy|Attitude")
	void AddAttitude(int32 FromPlayerId, int32 ToPlayerId, int32 Delta);

	// ========== 외교 액션 ==========

	// 외교 액션을 등록하고 부여된 ActionId를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy|Action")
	int32 IssueAction(const FDiplomacyAction& Action);

	// 외교 액션을 수락하거나 거절합니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy|Action")
	void ResolveAction(int32 ActionId, bool bAccepted);

	// 특정 슬롯에 도착한 미처리 액션 목록을 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy|Action")
	TArray<FDiplomacyAction> GetPendingActionsForPlayer(int32 ToPlayerId) const;

	// ========== 패배한 플레이어 처리 ==========

	// 패배한 슬롯의 모든 외교 관계를 정리합니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	void CleanupPlayerDiplomacy(int32 DefeatedPlayerId);

	// ========== 내부 데이터 ==========

	// 현재 게임의 플레이어 수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Diplomacy")
	int32 PlayerCount = 0;

	// 플레이어 쌍(A<->B)의 공통 상태 맵
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Diplomacy")
	TMap<FDiplomacyPairKey, FDiplomacyPairState> PairStates;

	// 방향성(From -> To) 호감도 맵. 런타임 전용이며 UPROPERTY가 아닙니다.
	TMap<int32, TMap<int32, int32>> Attitudes;

	// 마지막으로 처리한 라운드 번호. OnRoundStarted 중복 호출을 막습니다.
	int32 LastProcessedRound = -1;

	// 현재 라운드 번호 캐시. 외교 액션·조약에 씁니다.
	int32 CachedCurrentRound = 1;

	// 처리 대기 중인 외교 액션들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Diplomacy|Action")
	TArray<FDiplomacyAction> PendingActions;

	// 다음에 부여할 액션 ID
	int32 NextActionId = 1;
};
