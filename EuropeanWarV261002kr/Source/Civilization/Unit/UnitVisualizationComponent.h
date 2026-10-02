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
class USoundBase;
class ARangedProjectileVisual;

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

    // ========== 설정 ==========

    // 월드 컴포넌트를 연결합니다. hex↔월드 변환에 씁니다.
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void SetWorldComponent(UWorldComponent* InWorldComponent);

    // 연결된 월드 컴포넌트를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Setup")
    UWorldComponent* GetWorldComponent() const { return WorldComponent; }

    // 유닛 매니저를 연결합니다. 이동 완료 알림에 씁니다.
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void SetUnitManager(UUnitManager* InUnitManager);

    // ========== 이동 ==========

    // 경로를 따라 시각적 이동을 시작합니다.
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartMovementAlongPath(const TArray<FVector2D>& Path);

    // 시각적 이동을 멈춥니다.
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopMovement();

    // 지금 경로를 따라 이동 중인지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool IsMoving() const { return bIsMoving && CurrentMovementPath.Num() > 0 && CurrentPathIndex < CurrentMovementPath.Num(); }

    // 현재 이동 경로를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Movement")
    TArray<FVector2D> GetCurrentMovementPath() const { return CurrentMovementPath; }

    // 지금 향하는 경로 인덱스를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Movement")
    int32 GetCurrentPathIndex() const { return CurrentPathIndex; }

    // ========== hex 좌표 ==========

    // 유닛의 현재 hex를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Hex Position")
    FVector2D GetCurrentHexPosition() const;

    // hex를 월드 좌표로 바꿉니다.
    UFUNCTION(BlueprintCallable, Category = "Hex Position")
    FVector HexToWorld(FVector2D HexPosition) const;

    // 월드 좌표를 hex로 바꿉니다.
    UFUNCTION(BlueprintCallable, Category = "Hex Position")
    FVector2D WorldToHex(FVector WorldPosition) const;

    // ========== 전투 연출 ==========

    // 근거리·원거리 유닛 전투 연출을 시작합니다.
    UFUNCTION(BlueprintCallable, Category = "Combat Visualization")
    void StartCombatVisualization(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender, const FCombatResult& CombatResult);

    // 도시 공격 연출을 시작합니다.
    UFUNCTION(BlueprintCallable, Category = "Combat Visualization")
    void StartCombatVisualizationAgainstCity(AUnitCharacterBase* Attacker, class UCityComponent* CityComponent, FVector2D CityHex, const FCombatResult& CombatResult);

    // 지금 전투 연출 중인지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Combat Visualization")
    bool IsInCombat() const { return bIsInCombat; }

    // 전투 연출을 멈춥니다.
    UFUNCTION(BlueprintCallable, Category = "Combat Visualization")
    void StopCombatVisualization();

    // 공격/반격 노티파이. 상태에 따라 Hit를 처리합니다.
    UFUNCTION(BlueprintCallable, Category = "Combat Visualization")
    void OnCombatNotify_Hit();
    
    // 사망 몽타주의 사망 노티파이입니다.
    UFUNCTION(BlueprintCallable, Category = "Combat Visualization")
    void OnCombatNotify_Death();

    // 중간 칼 휘두름용입니다. Hit만 재생하고 단계는 넘기지 않습니다.
    UFUNCTION(BlueprintCallable, Category = "Combat Visualization")
    void OnCombatNotify_HitOnly();

    // 공격자 무기 소켓에서 방어자 쪽으로 화살 파티클을 발사합니다.
    void SpawnRangedProjectile();

    // 건설자 공격 몽타주를 한 번 재생하고, 끝나면 InOnCompleted를 호출합니다.
    void PlayBuilderActionMontage(FSimpleDelegate InOnCompleted);

private:
    // ========== 내부 참조 ==========

    // hex↔월드 변환용 월드 컴포넌트
    UPROPERTY()
    UWorldComponent* WorldComponent = nullptr;

    // 이동 완료 알림용 유닛 매니저
    UPROPERTY()
    UUnitManager* UnitManager = nullptr;

    // ========== 이동 상태 ==========

    // 지금 따라가는 hex 경로
    TArray<FVector2D> CurrentMovementPath;

    // 지금 향하는 경로 인덱스
    int32 CurrentPathIndex = 0;

    // 경로 이동 중인지 여부
    bool bIsMoving = false;

    // 지금 향하는 타일의 월드 좌표
    FVector CurrentTargetWorldPosition = FVector::ZeroVector;

    // 이동 속도
    UPROPERTY(EditDefaultsOnly, Category = "Movement Settings")
    float MovementSpeed = 500.0f;

    // 이 거리 안이면 도착으로 봅니다.
    UPROPERTY(EditDefaultsOnly, Category = "Movement Settings")
    float ArrivalDistance = 10.0f;

    // ========== 점프 ==========

    // 목표까지 이 거리 이하면 점프를 시작합니다.
    UPROPERTY(EditDefaultsOnly, Category = "Jump Settings")
    float JumpStartDistance = 300.0f;

    // 목표까지 이 거리 이상일 때만 점프합니다.
    UPROPERTY(EditDefaultsOnly, Category = "Jump Settings")
    float JumpMinDistance = 20.0f;

    // 마지막으로 점프한 목표 hex. 같은 타일에서 중복 점프를 막습니다.
    FVector2D LastJumpedTargetHex = FVector2D(-1, -1);

    // ========== 전투 연출 이동 ==========

    // 월드 좌표로 전투 연출용 이동을 시작합니다.
    void StartCombatMovement(const FVector& TargetWorldPosition);
    
    // 전투 연출용 이동을 갱신합니다.
    void UpdateCombatMovement(float DeltaTime);
    
    // 전투 연출용 이동을 멈춥니다.
    void StopCombatMovement();
    
    // 전투 연출용 이동 중인지 반환합니다.
    bool IsCombatMoving() const { return bIsCombatMoving; }
    
    // 전투 연출 목표에 도착했는지 반환합니다.
    bool HasReachedCombatTarget() const;

    // 공격자·방어자 타일 층수를 비교해 점프합니다.
    void CheckAndExecuteCombatJump(FVector2D FromHex, FVector2D ToHex);

    // 전투 연출용 이동 중인지 여부
    bool bIsCombatMoving = false;

    // 전투 연출 이동 목표
    FVector CombatTargetWorldPosition = FVector::ZeroVector;

    // ========== 전투 연출 상태 ==========

    // 근거리·원거리·도시 공격 연출 단계
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

    // 지금 전투 연출 단계
    ECombatVisualizationState CombatState = ECombatVisualizationState::None;

    // 전투 연출 중인지 여부
    bool bIsInCombat = false;
    
    // 원거리 전투인지 여부
    bool bIsRangedCombat = false;

    // Notify_Hit로 Hit만 재생 중이면 몽타주 종료 때 다음 단계로 가지 않습니다.
    bool bIsHitOnlyMontage = false;

    // 건설자 액션 몽타주 재생 중인지 여부
    bool bIsPlayingBuilderAction = false;

    // 건설자 몽타주가 끝나면 호출할 콜백
    FSimpleDelegate OnBuilderActionMontageCompleted;

    // 도시 공격인지 여부
    bool bIsCityCombat = false;

    // 공격 중인 도시 hex
    FVector2D CityHexPosition = FVector2D::ZeroVector;

    // 공격 중인 도시 월드 좌표
    FVector CityWorldPosition = FVector::ZeroVector;

    // 전투 공격자
    UPROPERTY()
    TWeakObjectPtr<AUnitCharacterBase> CombatAttacker = nullptr;

    // 전투 방어자
    UPROPERTY()
    TWeakObjectPtr<AUnitCharacterBase> CombatDefender = nullptr;

    // 이번 전투 결과
    FCombatResult CurrentCombatResult;

    // 공격자 원래 hex (복귀용)
    FVector2D AttackerOriginalHexPosition = FVector2D::ZeroVector;

    // 공격자 원래 월드 좌표
    FVector AttackerOriginalWorldPosition = FVector::ZeroVector;

    // 방어자 원래 hex (복귀용)
    FVector2D DefenderOriginalHexPosition = FVector2D::ZeroVector;

    // 방어자 원래 월드 좌표
    FVector DefenderOriginalWorldPosition = FVector::ZeroVector;

    // 방어자 타일 hex (공격자가 다가갈 위치)
    FVector2D DefenderHexPosition = FVector2D::ZeroVector;

    // 방어자 타일 월드 좌표
    FVector DefenderWorldPosition = FVector::ZeroVector;

    // 원거리에서 공격자가 복귀를 끝냈는지 여부
    bool bAttackerReturned = false;

    // 원거리에서 방어자가 복귀를 끝냈는지 여부
    bool bDefenderReturned = false;

    // ========== 전투 설정 ==========

    // 초당 회전 각도
    UPROPERTY(EditDefaultsOnly, Category = "Combat Settings")
    float RotationSpeed = 360.0f;

    // 이 각도 안이면 회전을 끝냅니다.
    UPROPERTY(EditDefaultsOnly, Category = "Combat Settings")
    float RotationTolerance = 5.0f;

    // 공격/반격 몽타주. 둘 다 같은 클립을 씁니다.
    UPROPERTY(EditDefaultsOnly, Category = "Combat Montages")
    UAnimMontage* AttackMontage = nullptr;

    // 피격 몽타주
    UPROPERTY(EditDefaultsOnly, Category = "Combat Montages")
    UAnimMontage* HitMontage = nullptr;

    // 사망 몽타주
    UPROPERTY(EditDefaultsOnly, Category = "Combat Montages")
    UAnimMontage* DeathMontage = nullptr;

    // 지금 재생 중인 몽타주
    UPROPERTY()
    UAnimMontage* CurrentPlayingMontage = nullptr;

    // 도시 공격 Notify_Combat 때 재생할 Destroy 사운드
    UPROPERTY()
    USoundBase* CityAttackDestroySound = nullptr;

    // 원거리 발사체 클래스. 비우면 C++ 기본 클래스를 씁니다.
    UPROPERTY(EditDefaultsOnly, Category = "Combat Visualization")
    TSubclassOf<ARangedProjectileVisual> RangedProjectileClass;

    // 원거리 발사체 초당 속도
    UPROPERTY(EditDefaultsOnly, Category = "Combat Visualization", meta = (ClampMin = "100.0"))
    float RangedProjectileSpeed = 3000.0f;

    // ========== 이동 내부 ==========

    // 현재 목표 타일로 이동합니다.
    void UpdateMovement(float DeltaTime);

    // 목표 타일에 도착했는지 반환합니다.
    bool HasReachedTarget() const;

    // 경로의 다음 타일로 넘어갑니다.
    void MoveToNextTile();

    // 경로 이동을 끝냅니다.
    void CompleteMovement();

    // 층수 차이에 맞춰 점프합니다.
    void CheckAndExecuteJump();

    // ========== 전투 연출 내부 ==========

    // Tick에서 전투 연출 단계를 진행합니다.
    void UpdateCombatVisualization(float DeltaTime);

    // 공격자를 방어자 타일로 보냅니다.
    void StartMovingToDefender();

    // 방어자가 공격자를 바라보게 돌립니다.
    void UpdateDefenderRotation(float DeltaTime);

    // 원거리 공격자가 방어자를 바라보게 돌립니다.
    void UpdateAttackerRotation(float DeltaTime);

    // 원거리 방어자가 공격자를 바라보게 돌립니다.
    void UpdateDefenderRotation_Ranged(float DeltaTime);

    // 근거리 공격 몽타주를 재생합니다.
    void PlayAttackerAttackMontage();

    // 원거리 공격 몽타주를 재생합니다.
    void PlayAttackerAttackMontage_Ranged();

    // 방어자 피격 또는 사망 몽타주를 재생합니다.
    void PlayDefenderHitOrDeathMontage();

    // 연타용으로 방어자 Hit만 재생합니다.
    void PlayDefenderHitMontageOnly();

    // 원거리 연타용으로 방어자 Hit만 재생합니다.
    void PlayDefenderHitMontageOnly_Ranged();

    // 원거리 방어자 피격 또는 사망 몽타주를 재생합니다.
    void PlayDefenderHitOrDeathMontage_Ranged();

    // 방어자 반격 몽타주를 재생합니다.
    void PlayDefenderCounterMontage();

    // 공격자 피격 또는 사망 몽타주를 재생합니다.
    void PlayAttackerHitOrDeathMontage();

    // 근거리 공격자를 원래 위치로 되돌립니다.
    void StartReturningToOrigin();

    // 원거리 공격자·방어자를 원래 위치로 되돌립니다.
    void StartReturningToOrigin_Ranged();

    // 공격자가 도시를 바라보게 돌립니다.
    void UpdateAttackerRotationToCity(float DeltaTime);

    // 도시 공격 몽타주를 재생합니다.
    void PlayAttackerAttackMontageAgainstCity();

    // 도시 공격 후 원래 위치로 돌아갑니다.
    void StartReturningFromCity();

    // 도시 공격 연출을 끝냅니다.
    void CompleteCityCombatVisualization();

    // 근거리 전투 연출을 바로 끝냅니다.
    void CompleteMeleeCombatVisualization();

    // 원거리 복귀가 끝나면 전투 연출을 끝냅니다.
    void CompleteRangedCombatVisualization();

    // 원거리 양쪽 복귀가 끝났는지 확인하고 알립니다.
    void CheckAndNotifyRangedCombatComplete();

    // 대상 유닛에 몽타주를 재생합니다.
    void PlayMontage(UAnimMontage* Montage, AUnitCharacterBase* TargetUnit);

    // 몽타주가 끝나면 다음 단계로 넘깁니다.
    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    // 방어자 피격/사망 몽타주가 끝난 뒤 처리합니다.
    void OnDefenderHitOrDeathMontageEnded();

    // 공격자 피격/사망 몽타주가 끝난 뒤 처리합니다.
    void OnAttackerHitOrDeathMontageEnded();

    // 유닛이 목표를 바라보게 돌립니다.
    void RotateUnitToFaceTarget(AUnitCharacterBase* Unit, const FVector& TargetLocation, float DeltaTime);
};
