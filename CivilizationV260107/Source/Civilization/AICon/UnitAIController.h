// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "../Combat/UnitCombatStruct.h"
#include "UnitAIController.generated.h"

class UWorldComponent;
class UUnitManager;
class AUnitCharacterBase;

UCLASS()
class CIVILIZATION_API AUnitAIController : public AAIController
{
    GENERATED_BODY()

public:
    AUnitAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

    // Blackboard와 BehaviorTree 초기화
    void InitBlackboardAndBT();

private:
    // BehaviorTree와 Blackboard 데이터
    UPROPERTY()
    TObjectPtr<class UBehaviorTree> BTData;

    UPROPERTY()
    TObjectPtr<class UBlackboardData> BBData;

    // WorldComponent 참조
    UPROPERTY()
    TObjectPtr<UWorldComponent> WorldComponent = nullptr;

    // UnitManager 참조 (선택적)
    UPROPERTY()
    TObjectPtr<UUnitManager> UnitManager = nullptr;

    // 내부 이동 경로 관리 (Blackboard는 배열을 지원하지 않으므로 내부 변수로 관리)
    TArray<FVector2D> CurrentMovementPath;
    int32 CurrentPathIndex = 0;

    // 전투 관련 내부 변수
    UPROPERTY()
    TObjectPtr<AUnitCharacterBase> CombatAttacker = nullptr;  // 공격자 참조

    UPROPERTY()
    TObjectPtr<AUnitCharacterBase> CombatDefender = nullptr; // 방어자 참조

    UPROPERTY()
    struct FCombatResult CurrentCombatResult;  // 현재 전투 결과

    bool bIsInCombat = false;  // 현재 전투 중인지 여부

    FVector2D AttackerOriginalHexPosition = FVector2D::ZeroVector;  // 공격자 원래 위치 (복귀용)

    // 전투 사거리 정보
    int32 CombatRange = 0;  // 현재 전투의 사거리 저장
    bool bIsRangedAttack = false;  // 원거리 공격 여부 (Range > 1)

    // Blackboard 키 이름 상수
    UPROPERTY(EditDefaultsOnly, Category = "BBKeys")
    FName KeyTargetHexPosition = TEXT("TargetHexPosition");

    UPROPERTY(EditDefaultsOnly, Category = "BBKeys")
    FName KeyMovementComplete = TEXT("MovementComplete");

    UPROPERTY(EditDefaultsOnly, Category = "BBKeys")
    FName KeyIsMoving = TEXT("IsMoving");

    UPROPERTY(EditDefaultsOnly, Category = "BBKeys")
    FName KeyIsInCombat = TEXT("IsInCombat");

    UPROPERTY(EditDefaultsOnly, Category = "BBKeys")
    FName KeyCombatComplete = TEXT("CombatComplete");

    UPROPERTY(EditDefaultsOnly, Category = "BBKeys")
    FName KeyIsRangedAttack = TEXT("IsRangedAttack");

public:
    // WorldComponent 설정
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void SetWorldComponent(UWorldComponent* InWorldComponent);

    // WorldComponent 가져오기
    UFUNCTION(BlueprintCallable, Category = "Setup")
    UWorldComponent* GetWorldComponent() const { return WorldComponent; }

    // UnitManager 설정
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void SetUnitManager(UUnitManager* InUnitManager);

    // 이동 명령 함수들
    UFUNCTION(BlueprintCallable, Category = "Unit Movement")
    void StartMovementAlongPath(const TArray<FVector2D>& Path);

    // 다음 헥스로 이동 (내부 함수, Task에서 호출)
    UFUNCTION(BlueprintCallable, Category = "Unit Movement")
    void MoveToNextHexInPath();

    // 이동 완료 처리
    UFUNCTION(BlueprintCallable, Category = "Unit Movement")
    void CompleteMovement();

    // 헥스 좌표 관련 함수들
    UFUNCTION(BlueprintCallable, Category = "Hex Position")
    FVector2D GetCurrentHexPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Hex Position")
    FVector HexToWorld(FVector2D HexPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Hex Position")
    FVector2D WorldToHex(FVector WorldPosition) const;

    // 현재 이동 경로 가져오기
    UFUNCTION(BlueprintCallable, Category = "Unit Movement")
    TArray<FVector2D> GetCurrentMovementPath() const { return CurrentMovementPath; }

    // 현재 경로 인덱스 가져오기
    UFUNCTION(BlueprintCallable, Category = "Unit Movement")
    int32 GetCurrentPathIndex() const { return CurrentPathIndex; }

    // 이동 중인지 확인
    UFUNCTION(BlueprintCallable, Category = "Unit Movement")
    bool IsMoving() const { return CurrentMovementPath.Num() > 0 && CurrentPathIndex < CurrentMovementPath.Num(); }

    // 전투 명령 함수들
    UFUNCTION(BlueprintCallable, Category = "Unit Combat")
    void StartCombatVisualization(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender, const FCombatResult& CombatResult);

    // 전투 완료 처리
    UFUNCTION(BlueprintCallable, Category = "Unit Combat")
    void CompleteCombatVisualization();

    // 전투 중인지 확인
    UFUNCTION(BlueprintCallable, Category = "Unit Combat")
    bool IsInCombat() const { return bIsInCombat; }

    // 현재 전투 데이터 가져오기
    UFUNCTION(BlueprintCallable, Category = "Unit Combat")
    AUnitCharacterBase* GetCombatAttacker() const { return CombatAttacker; }

    UFUNCTION(BlueprintCallable, Category = "Unit Combat")
    AUnitCharacterBase* GetCombatDefender() const { return CombatDefender; }

    UFUNCTION(BlueprintCallable, Category = "Unit Combat")
    FCombatResult GetCurrentCombatResult() const { return CurrentCombatResult; }

    // 공격자 원래 위치 가져오기
    UFUNCTION(BlueprintCallable, Category = "Unit Combat")
    FVector2D GetAttackerOriginalHexPosition() const { return AttackerOriginalHexPosition; }

    // 전투 사거리 정보 접근 함수
    UFUNCTION(BlueprintCallable, Category = "Unit Combat")
    int32 GetCombatRange() const { return CombatRange; }

    UFUNCTION(BlueprintCallable, Category = "Unit Combat")
    bool IsRangedAttack() const { return bIsRangedAttack; }
};

