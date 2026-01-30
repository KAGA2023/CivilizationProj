// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../World/WorldStruct.h"
#include "../Combat/UnitCombatStruct.h"
#include "UnitVisualizationComponent.generated.h"

class UWorldComponent;
class UUnitManager;
class AUnitCharacterBase;
class UAnimMontage;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CIVILIZATION_API UUnitVisualizationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UUnitVisualizationComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ================= 설정 함수 =================

    // WorldComponent 설정
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void SetWorldComponent(UWorldComponent* InWorldComponent);

    // WorldComponent 가져오기
    UFUNCTION(BlueprintCallable, Category = "Setup")
    UWorldComponent* GetWorldComponent() const { return WorldComponent; }

    // UnitManager 설정
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void SetUnitManager(UUnitManager* InUnitManager);

    // ================= 이동 관련 함수 =================

    // 경로를 따라 이동 시작
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartMovementAlongPath(const TArray<FVector2D>& Path);

    // 이동 중지
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopMovement();

    // 이동 중인지 확인
    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool IsMoving() const { return bIsMoving && CurrentMovementPath.Num() > 0 && CurrentPathIndex < CurrentMovementPath.Num(); }

    // 현재 이동 경로 가져오기
    UFUNCTION(BlueprintCallable, Category = "Movement")
    TArray<FVector2D> GetCurrentMovementPath() const { return CurrentMovementPath; }

    // 현재 경로 인덱스 가져오기
    UFUNCTION(BlueprintCallable, Category = "Movement")
    int32 GetCurrentPathIndex() const { return CurrentPathIndex; }

    // ================= 헥스 좌표 관련 함수 =================

    // 현재 헥스 위치 가져오기
    UFUNCTION(BlueprintCallable, Category = "Hex Position")
    FVector2D GetCurrentHexPosition() const;

    // 헥스 좌표를 월드 좌표로 변환
    UFUNCTION(BlueprintCallable, Category = "Hex Position")
    FVector HexToWorld(FVector2D HexPosition) const;

    // 월드 좌표를 헥스 좌표로 변환
    UFUNCTION(BlueprintCallable, Category = "Hex Position")
    FVector2D WorldToHex(FVector WorldPosition) const;

    // ================= 전투 시각화 관련 함수 =================

    // 근거리 전투 시각화 시작
    UFUNCTION(BlueprintCallable, Category = "Combat Visualization")
    void StartCombatVisualization(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender, const FCombatResult& CombatResult);

    // 도시 공격 전투 시각화 시작
    UFUNCTION(BlueprintCallable, Category = "Combat Visualization")
    void StartCombatVisualizationAgainstCity(AUnitCharacterBase* Attacker, class UCityComponent* CityComponent, FVector2D CityHex, const FCombatResult& CombatResult);

    // 전투 중인지 확인
    UFUNCTION(BlueprintCallable, Category = "Combat Visualization")
    bool IsInCombat() const { return bIsInCombat; }

    // 전투 중지
    UFUNCTION(BlueprintCallable, Category = "Combat Visualization")
    void StopCombatVisualization();

    // 노티파이 콜백 (AnimInstance에서 호출될 함수들)
    UFUNCTION(BlueprintCallable, Category = "Combat Visualization")
    void OnCombatNotify_Hit(); // 공격/반격 노티파이 (상태에 따라 자동 처리)
    
    UFUNCTION(BlueprintCallable, Category = "Combat Visualization")
    void OnCombatNotify_Death(); // 사망 노티파이 (Death 몽타주에서 호출)

private:
    // ================= 내부 변수 =================

    // WorldComponent 참조
    UPROPERTY()
    UWorldComponent* WorldComponent = nullptr;

    // UnitManager 참조
    UPROPERTY()
    UUnitManager* UnitManager = nullptr;

    // 이동 경로 관리
    TArray<FVector2D> CurrentMovementPath;
    int32 CurrentPathIndex = 0; // 현재 목표 타일 인덱스
    bool bIsMoving = false;

    // 현재 목표 타일 (월드 좌표)
    FVector CurrentTargetWorldPosition = FVector::ZeroVector;

    // 이동 관련 설정
    UPROPERTY(EditDefaultsOnly, Category = "Movement Settings")
    float MovementSpeed = 500.0f; // 이동 속도

    UPROPERTY(EditDefaultsOnly, Category = "Movement Settings")
    float ArrivalDistance = 10.0f; // 도착 판정 거리

    // 점프 관련 설정
    UPROPERTY(EditDefaultsOnly, Category = "Jump Settings")
    float JumpStartDistance = 300.0f; // 점프 시작 거리 (목표로부터 이 거리 이내)

    UPROPERTY(EditDefaultsOnly, Category = "Jump Settings")
    float JumpMinDistance = 20.0f; // 점프 최소 거리 (목표로부터 이 거리 이상)

    // 마지막으로 점프한 목표 타일 (중복 점프 방지)
    FVector2D LastJumpedTargetHex = FVector2D(-1, -1);

    // ================= 전투 시각화 전용 이동 =================
    
    // 전투 시각화용 이동 시작 (월드 좌표 직접 지정)
    void StartCombatMovement(const FVector& TargetWorldPosition);
    
    // 전투 시각화용 이동 업데이트
    void UpdateCombatMovement(float DeltaTime);
    
    // 전투 시각화용 이동 중지
    void StopCombatMovement();
    
    // 전투 시각화용 이동 중인지 확인
    bool IsCombatMoving() const { return bIsCombatMoving; }
    
    // 전투 시각화용 이동 목표 도착 확인
    bool HasReachedCombatTarget() const;

    // 전투 시각화용 점프 체크 (공격자/방어자 타일 층수 비교)
    void CheckAndExecuteCombatJump(FVector2D FromHex, FVector2D ToHex);

    // 전투 시각화용 이동 상태
    bool bIsCombatMoving = false;
    FVector CombatTargetWorldPosition = FVector::ZeroVector;

    // ================= 전투 시각화 관련 변수 =================

    // 전투 상태 enum
    enum class ECombatVisualizationState : uint8
    {
        None,               // 전투 없음
        MovingToDefender,    // 공격자가 방어자 타일로 이동 중 (근거리)
        RotatingDefender,    // 방어자가 공격자를 바라보도록 회전 중 (근거리)
        AttackerAttack,      // 공격자 공격 몽타주 재생 중 (근거리)
        DefenderHit,         // 방어자 피격 몽타주 재생 중 (근거리)
        DefenderDeath,       // 방어자 사망 몽타주 재생 중 (근거리)
        DefenderCounter,     // 방어자 반격 몽타주 재생 중 (근거리)
        AttackerHit,         // 공격자 피격 몽타주 재생 중 (근거리)
        ReturningToOrigin,   // 공격자 원래 위치로 복귀 중 (근거리)
        // 원거리 전투 상태
        RotatingAttacker,    // 공격자가 방어자를 바라보도록 회전 중 (원거리)
        RotatingDefender_Ranged,  // 방어자가 공격자를 바라보도록 회전 중 (원거리)
        AttackerAttack_Ranged,    // 공격자 공격 몽타주 재생 중 (원거리)
        DefenderHit_Ranged,  // 방어자 피격 몽타주 재생 중 (원거리)
        DefenderDeath_Ranged, // 방어자 사망 몽타주 재생 중 (원거리)
        ReturningToOrigin_Ranged, // 공격자/방어자 원래 위치로 복귀 중 (원거리)
        // 도시 공격 전투 상태 (근거리/원거리 구분 없음)
        RotatingToCity,        // 공격자가 도시를 바라보도록 회전 중
        AttackingCity,         // 공격자 공격 몽타주 재생 중 (AttackMontage)
        ReturningFromCity      // 공격자 원래 위치로 복귀 중
    };

    // 전투 상태
    ECombatVisualizationState CombatState = ECombatVisualizationState::None;
    bool bIsInCombat = false;
    
    // 원거리 전투 여부
    bool bIsRangedCombat = false;

    // 도시 공격 여부
    bool bIsCityCombat = false;

    // 도시 위치 정보 (도시 공격 전용)
    FVector2D CityHexPosition = FVector2D::ZeroVector;
    FVector CityWorldPosition = FVector::ZeroVector;

    // 전투 관련 참조
    UPROPERTY()
    TWeakObjectPtr<AUnitCharacterBase> CombatAttacker = nullptr;

    UPROPERTY()
    TWeakObjectPtr<AUnitCharacterBase> CombatDefender = nullptr;

    // 전투 결과
    FCombatResult CurrentCombatResult;

    // 공격자 원래 위치 (복귀용)
    FVector2D AttackerOriginalHexPosition = FVector2D::ZeroVector;
    FVector AttackerOriginalWorldPosition = FVector::ZeroVector;

    // 방어자 원래 위치 (복귀용)
    FVector2D DefenderOriginalHexPosition = FVector2D::ZeroVector;
    FVector DefenderOriginalWorldPosition = FVector::ZeroVector;

    // 방어자 타일 위치 (전투 중 공격자가 이동할 위치)
    FVector2D DefenderHexPosition = FVector2D::ZeroVector;
    FVector DefenderWorldPosition = FVector::ZeroVector;

    // 복귀 완료 추적 (원거리 전투 전용, 공격자 컴포넌트에서만 사용)
    bool bAttackerReturned = false;
    bool bDefenderReturned = false;

    // 회전 관련 설정
    UPROPERTY(EditDefaultsOnly, Category = "Combat Settings")
    float RotationSpeed = 360.0f; // 회전 속도 (도/초)

    UPROPERTY(EditDefaultsOnly, Category = "Combat Settings")
    float RotationTolerance = 5.0f; // 회전 완료 판정 각도

    // 몽타주 참조 (블루프린트에서 설정)
    UPROPERTY(EditDefaultsOnly, Category = "Combat Montages")
    UAnimMontage* AttackMontage = nullptr; // 공격/반격 몽타주 (공격과 반격이 동일)

    UPROPERTY(EditDefaultsOnly, Category = "Combat Montages")
    UAnimMontage* HitMontage = nullptr; // 피격 몽타주

    UPROPERTY(EditDefaultsOnly, Category = "Combat Montages")
    UAnimMontage* DeathMontage = nullptr; // 사망 몽타주

    // 현재 재생 중인 몽타주
    UPROPERTY()
    UAnimMontage* CurrentPlayingMontage = nullptr;

    // ================= 내부 함수 =================

    // 현재 목표 타일로 이동 처리
    void UpdateMovement(float DeltaTime);

    // 목표 타일 도착 체크
    bool HasReachedTarget() const;

    // 다음 타일로 이동
    void MoveToNextTile();

    // 이동 완료 처리
    void CompleteMovement();

    // 점프 조건 체크 및 실행
    void CheckAndExecuteJump();

    // ================= 전투 시각화 내부 함수 =================

    // 전투 상태 업데이트 (Tick에서 호출)
    void UpdateCombatVisualization(float DeltaTime);

    // 공격자 방어자 타일로 이동 시작
    void StartMovingToDefender();

    // 방어자 회전 처리
    void UpdateDefenderRotation(float DeltaTime);

    // 공격자 회전 처리 (원거리)
    void UpdateAttackerRotation(float DeltaTime);

    // 방어자 회전 처리 (원거리)
    void UpdateDefenderRotation_Ranged(float DeltaTime);

    // 공격자 공격 몽타주 재생
    void PlayAttackerAttackMontage();

    // 공격자 공격 몽타주 재생 (원거리)
    void PlayAttackerAttackMontage_Ranged();

    // 방어자 피격/사망 몽타주 재생
    void PlayDefenderHitOrDeathMontage();

    // 방어자 피격/사망 몽타주 재생 (원거리)
    void PlayDefenderHitOrDeathMontage_Ranged();

    // 방어자 반격 몽타주 재생
    void PlayDefenderCounterMontage();

    // 공격자 피격/사망 몽타주 재생
    void PlayAttackerHitOrDeathMontage();

    // 공격자 원래 위치로 복귀
    void StartReturningToOrigin();

    // 공격자/방어자 원래 위치로 복귀 (원거리)
    void StartReturningToOrigin_Ranged();

    // 도시 공격 관련 함수
    // 공격자가 도시를 바라보도록 회전 처리
    void UpdateAttackerRotationToCity(float DeltaTime);

    // 도시 공격 몽타주 재생 (AttackMontage 사용)
    void PlayAttackerAttackMontageAgainstCity();

    // 도시 공격 후 원래 위치로 복귀 시작
    void StartReturningFromCity();

    // 도시 공격 전투 완료 처리
    void CompleteCityCombatVisualization();

    // 근거리 전투 완료 처리 (CivilizationShort 방식: 즉시 완료 알림)
    void CompleteMeleeCombatVisualization();

    // 원거리 전투 완료 처리 (CivilizationRanged 방식: 복귀 완료 확인 후 알림)
    void CompleteRangedCombatVisualization();

    // 원거리 전투 복귀 완료 확인 후 알림 (원거리 전용)
    void CheckAndNotifyRangedCombatComplete();

    // 몽타주 재생 (공통 함수)
    void PlayMontage(UAnimMontage* Montage, AUnitCharacterBase* TargetUnit);

    // 몽타주 종료 콜백
    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    // 몽타주 종료 후 처리 함수들
    void OnDefenderHitOrDeathMontageEnded();
    void OnAttackerHitOrDeathMontageEnded();

    // 유닛 회전 처리 (공통 함수)
    void RotateUnitToFaceTarget(AUnitCharacterBase* Unit, const FVector& TargetLocation, float DeltaTime);
};

