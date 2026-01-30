// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitVisualizationComponent.h"
#include "../Unit/UnitCharacterBase.h"
#include "../World/WorldComponent.h"
#include "../Unit/UnitManager.h"
#include "../Combat/UnitCombatStruct.h"
#include "../City/CityComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UUnitVisualizationComponent::UUnitVisualizationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bIsMoving = false;
    CurrentPathIndex = 0;
    MovementSpeed = 500.0f;
    ArrivalDistance = 10.0f;
    JumpStartDistance = 300.0f;
    JumpMinDistance = 20.0f;
    LastJumpedTargetHex = FVector2D(-1, -1);
}

void UUnitVisualizationComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UUnitVisualizationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 이동 중이면 이동 업데이트
    if (IsMoving())
    {
        UpdateMovement(DeltaTime);
    }

    // 전투 시각화용 이동 중이면 이동 업데이트
    if (bIsCombatMoving)
    {
        UpdateCombatMovement(DeltaTime);
    }

    // 전투 시각화 중이면 전투 업데이트
    if (bIsInCombat)
    {
        UpdateCombatVisualization(DeltaTime);
    }
}

void UUnitVisualizationComponent::SetWorldComponent(UWorldComponent* InWorldComponent)
{
    WorldComponent = InWorldComponent;
}

void UUnitVisualizationComponent::SetUnitManager(UUnitManager* InUnitManager)
{
    UnitManager = InUnitManager;
}

void UUnitVisualizationComponent::StartMovementAlongPath(const TArray<FVector2D>& Path)
{
    if (Path.Num() <= 1)
    {
        return;
    }

    // 기존 이동 중지
    StopMovement();

    // 경로 저장
    CurrentMovementPath = Path;
    CurrentPathIndex = 1; // 첫 번째는 시작점이므로 1부터
    bIsMoving = true;
    LastJumpedTargetHex = FVector2D(-1, -1); // 점프 기록 초기화

    // 첫 번째 목표 타일 설정
    if (CurrentPathIndex < CurrentMovementPath.Num())
    {
        FVector2D TargetHex = CurrentMovementPath[CurrentPathIndex];
        CurrentTargetWorldPosition = HexToWorld(TargetHex);
    }
}

void UUnitVisualizationComponent::StopMovement()
{
    bIsMoving = false;
    CurrentMovementPath.Empty();
    CurrentPathIndex = 0;

    // Character의 이동 중지
    AUnitCharacterBase* Unit = Cast<AUnitCharacterBase>(GetOwner());
    if (Unit)
    {
        if (UCharacterMovementComponent* MoveComp = Unit->GetCharacterMovement())
        {
            MoveComp->StopMovementImmediately();
        }
    }
}

FVector2D UUnitVisualizationComponent::GetCurrentHexPosition() const
{
    if (!WorldComponent)
    {
        return FVector2D::ZeroVector;
    }

    AUnitCharacterBase* Unit = Cast<AUnitCharacterBase>(GetOwner());
    if (!Unit)
    {
        return FVector2D::ZeroVector;
    }

    FVector WorldPosition = Unit->GetActorLocation();
    return WorldComponent->WorldToHex(WorldPosition);
}

FVector UUnitVisualizationComponent::HexToWorld(FVector2D HexPosition) const
{
    if (!WorldComponent)
    {
        return FVector::ZeroVector;
    }

    return WorldComponent->HexToWorld(HexPosition);
}

FVector2D UUnitVisualizationComponent::WorldToHex(FVector WorldPosition) const
{
    if (!WorldComponent)
    {
        return FVector2D::ZeroVector;
    }

    return WorldComponent->WorldToHex(WorldPosition);
}

void UUnitVisualizationComponent::UpdateMovement(float DeltaTime)
{
    AUnitCharacterBase* Unit = Cast<AUnitCharacterBase>(GetOwner());
    if (!Unit || !WorldComponent)
    {
        StopMovement();
        return;
    }

    // 현재 위치와 목표 위치
    FVector CurrentPosition = Unit->GetActorLocation();
    FVector TargetPosition = CurrentTargetWorldPosition;

    // 목표까지의 방향 벡터
    FVector Direction = (TargetPosition - CurrentPosition).GetSafeNormal();
    
    // Z축은 무시 (평면 이동만)
    Direction.Z = 0.0f;
    Direction.Normalize();

    // 점프 체크 및 실행
    CheckAndExecuteJump();

    // 이동 입력 적용
    if (UCharacterMovementComponent* MoveComp = Unit->GetCharacterMovement())
    {
        // 이동 속도 설정
        MoveComp->MaxWalkSpeed = MovementSpeed;

        // 이동 입력 적용
        Unit->AddMovementInput(Direction, 1.0f);
    }

    // 목표 도착 체크
    if (HasReachedTarget())
    {
        MoveToNextTile();
    }
}

bool UUnitVisualizationComponent::HasReachedTarget() const
{
    AUnitCharacterBase* Unit = Cast<AUnitCharacterBase>(GetOwner());
    if (!Unit)
    {
        return false;
    }

    FVector CurrentPosition = Unit->GetActorLocation();
    FVector TargetPosition = CurrentTargetWorldPosition;

    // Z축은 무시하고 평면 거리만 계산
    FVector2D CurrentPos2D(CurrentPosition.X, CurrentPosition.Y);
    FVector2D TargetPos2D(TargetPosition.X, TargetPosition.Y);
    
    float Distance = FVector2D::Distance(CurrentPos2D, TargetPos2D);

    return Distance <= ArrivalDistance;
}

void UUnitVisualizationComponent::MoveToNextTile()
{
    // 인덱스 증가 (현재 타일 도착했으므로 다음 타일로)
    CurrentPathIndex++;

    // 경로가 끝났는지 확인
    if (CurrentPathIndex >= CurrentMovementPath.Num())
    {
        // 이동 완료
        CompleteMovement();
        return;
    }

    // 다음 타일로 이동
    FVector2D NextHex = CurrentMovementPath[CurrentPathIndex];
    CurrentTargetWorldPosition = HexToWorld(NextHex);
    LastJumpedTargetHex = FVector2D(-1, -1); // 다음 타일로 넘어가면 점프 기록 초기화
}

void UUnitVisualizationComponent::CompleteMovement()
{
    // 이동 중지
    bIsMoving = false;

    // Character의 이동 중지
    AUnitCharacterBase* Unit = Cast<AUnitCharacterBase>(GetOwner());
    if (Unit)
    {
        if (UCharacterMovementComponent* MoveComp = Unit->GetCharacterMovement())
        {
            MoveComp->StopMovementImmediately();
        }
    }

    // UnitManager에 이동 완료 알림
    if (Unit && UnitManager && CurrentMovementPath.Num() > 0)
    {
        FVector2D FinalHex = CurrentMovementPath[CurrentMovementPath.Num() - 1];
        UnitManager->OnUnitMovementComplete(Unit, FinalHex);
    }

    // 경로 초기화
    CurrentMovementPath.Empty();
    CurrentPathIndex = 0;
    LastJumpedTargetHex = FVector2D(-1, -1);
}

void UUnitVisualizationComponent::CheckAndExecuteJump()
{
    AUnitCharacterBase* Unit = Cast<AUnitCharacterBase>(GetOwner());
    if (!Unit || !WorldComponent)
    {
        return;
    }

    // 현재 목표 타일 (경로가 있으면 경로에서, 없으면 월드 위치에서 가져오기)
    FVector2D CurrentTargetHex = FVector2D::ZeroVector;
    if (CurrentMovementPath.Num() > 0 && CurrentPathIndex < CurrentMovementPath.Num())
    {
        CurrentTargetHex = CurrentMovementPath[CurrentPathIndex];
    }
    else
    {
        // 경로가 없으면 월드 위치에서 헥스 좌표 가져오기 (전투 시각화용)
        CurrentTargetHex = WorldComponent->WorldToHex(CurrentTargetWorldPosition);
    }

    // 현재 위치와 목표 위치
    FVector CurrentPosition = Unit->GetActorLocation();
    FVector TargetPosition = CurrentTargetWorldPosition;

    // 평면 거리 계산 (Z축 무시)
    FVector2D CurrentPos2D(CurrentPosition.X, CurrentPosition.Y);
    FVector2D TargetPos2D(TargetPosition.X, TargetPosition.Y);
    float Distance = FVector2D::Distance(CurrentPos2D, TargetPos2D);

    // 점프 조건 체크
    // 1. 목표로부터 JumpStartDistance 이내
    // 2. 목표로부터 JumpMinDistance 이상 떨어져 있음
    // 3. 이 목표 타일에 대해 아직 점프하지 않았음
    if (Distance <= JumpStartDistance && Distance > JumpMinDistance && LastJumpedTargetHex != CurrentTargetHex)
    {
        // 현재 타일을 경로에서 가져오기 (정확한 타일 정보를 위해)
        FVector2D CurrentHex = FVector2D::ZeroVector;
        if (CurrentMovementPath.Num() > 0 && CurrentPathIndex > 0)
        {
            // CurrentPathIndex는 다음 타일을 가리키므로, 현재 타일은 CurrentPathIndex - 1
            int32 CurrentTileIndex = CurrentPathIndex - 1;
            if (CurrentTileIndex >= 0 && CurrentTileIndex < CurrentMovementPath.Num())
            {
                CurrentHex = CurrentMovementPath[CurrentTileIndex];
            }
            else
            {
                // 인덱스가 범위를 벗어나면 첫 번째 타일 사용
                CurrentHex = CurrentMovementPath[0];
            }
        }
        else
        {
            // 경로가 없으면 월드 위치에서 가져오기 (폴백)
            CurrentHex = GetCurrentHexPosition();
        }

        UWorldTile* CurrentTile = WorldComponent->GetTileAtHex(CurrentHex);
        UWorldTile* TargetTile = WorldComponent->GetTileAtHex(CurrentTargetHex);

        if (CurrentTile && TargetTile)
        {
            ELandType CurrentLandType = CurrentTile->GetLandType();
            ELandType TargetLandType = TargetTile->GetLandType();

            // 점프 조건: 평지 → 언덕, 언덕 → 산
            bool bShouldJump = false;
            if (CurrentLandType == ELandType::Plains && TargetLandType == ELandType::Hills)
            {
                bShouldJump = true;
            }
            else if (CurrentLandType == ELandType::Hills && TargetLandType == ELandType::Mountains)
            {
                bShouldJump = true;
            }

            // 점프 실행
            if (bShouldJump)
            {
                if (UCharacterMovementComponent* MoveComp = Unit->GetCharacterMovement())
                {
                    // 점프 가능 여부 확인
                    if (!MoveComp->IsFalling() && MoveComp->CanAttemptJump())
                    {
                        Unit->Jump();
                        LastJumpedTargetHex = CurrentTargetHex; // 점프 기록
                    }
                }
            }
        }
    }
}

// ================= 전투 시각화 전용 이동 =================

void UUnitVisualizationComponent::StartCombatMovement(const FVector& TargetWorldPosition)
{
    AUnitCharacterBase* Unit = Cast<AUnitCharacterBase>(GetOwner());
    if (!Unit)
    {
        return;
    }

    // 기존 전투 이동 중지
    StopCombatMovement();

    // 목표 위치 설정 및 이동 시작
    CombatTargetWorldPosition = TargetWorldPosition;

    // 점프 로직(일반 이동용 CheckAndExecuteJump)을 재사용하기 위해
    // 전투 이동의 목표도 CurrentTargetWorldPosition에 반영
    CurrentTargetWorldPosition = TargetWorldPosition;

    // 전투 이동 시작 시 점프 기록 초기화 (같은 타일에서도 다시 점프 가능하도록)
    LastJumpedTargetHex = FVector2D(-1, -1);

    bIsCombatMoving = true;
}

void UUnitVisualizationComponent::UpdateCombatMovement(float DeltaTime)
{
    AUnitCharacterBase* Unit = Cast<AUnitCharacterBase>(GetOwner());
    if (!Unit)
    {
        StopCombatMovement();
        return;
    }

    // 현재 위치와 목표 위치
    FVector CurrentPosition = Unit->GetActorLocation();
    FVector TargetPosition = CombatTargetWorldPosition;

    // 목표까지의 방향 벡터
    FVector Direction = (TargetPosition - CurrentPosition).GetSafeNormal();
    
    // Z축은 무시 (평면 이동만)
    Direction.Z = 0.0f;
    Direction.Normalize();

    // 전투 이동 중에는 공격자/방어자 타일 비교로 점프 체크
    if (bIsInCombat && CombatAttacker.IsValid() && CombatDefender.IsValid())
    {
        AUnitCharacterBase* OwnerUnit = Cast<AUnitCharacterBase>(GetOwner());
        AUnitCharacterBase* Attacker = CombatAttacker.Get();
        
        // 공격자인 경우
        if (OwnerUnit == Attacker)
        {
            // 복귀 중인지 확인
            bool bIsReturning = (CombatState == ECombatVisualizationState::ReturningToOrigin || 
                                 CombatState == ECombatVisualizationState::ReturningToOrigin_Ranged);
            
            // 거리가 가까워졌을 때 점프 시도 (일반 이동의 거리 조건 참고)
            float Distance = FVector2D::Distance(
                FVector2D(CurrentPosition.X, CurrentPosition.Y),
                FVector2D(TargetPosition.X, TargetPosition.Y)
            );
            
            // 목표에 가까워졌을 때만 점프 체크 (중복 점프 방지)
            if (Distance <= JumpStartDistance && Distance > JumpMinDistance)
            {
                if (bIsReturning)
                {
                    // 복귀 중: 현재 위치와 원래 위치 비교
                    FVector2D CurrentHex = WorldComponent->WorldToHex(CurrentPosition);
                    FVector2D OriginHex = AttackerOriginalHexPosition;
                    CheckAndExecuteCombatJump(CurrentHex, OriginHex);
                }
                else
                {
                    // 방어자로 이동 중: 공격자 원래 위치와 방어자 위치 비교
                    FVector2D AttackerHex = AttackerOriginalHexPosition;
                    FVector2D DefenderHex = DefenderHexPosition;
                    CheckAndExecuteCombatJump(AttackerHex, DefenderHex);
                }
            }
        }
        else
        {
            // 일반 이동 로직 사용
            CheckAndExecuteJump();
        }
    }
    else
    {
        // 일반 이동 로직 사용
        CheckAndExecuteJump();
    }

    // 이동 입력 적용
    if (UCharacterMovementComponent* MoveComp = Unit->GetCharacterMovement())
    {
        // 이동 속도 설정
        MoveComp->MaxWalkSpeed = MovementSpeed;

        // 이동 입력 적용
        Unit->AddMovementInput(Direction, 1.0f);
    }

    // 목표 도착 체크
    if (HasReachedCombatTarget())
    {
        StopCombatMovement();
    }
}

void UUnitVisualizationComponent::StopCombatMovement()
{
    bIsCombatMoving = false;
    CombatTargetWorldPosition = FVector::ZeroVector;

    // Character의 이동 중지
    AUnitCharacterBase* Unit = Cast<AUnitCharacterBase>(GetOwner());
    if (Unit)
    {
        if (UCharacterMovementComponent* MoveComp = Unit->GetCharacterMovement())
        {
            MoveComp->StopMovementImmediately();
        }
    }
}

bool UUnitVisualizationComponent::HasReachedCombatTarget() const
{
    AUnitCharacterBase* Unit = Cast<AUnitCharacterBase>(GetOwner());
    if (!Unit)
    {
        return true;
    }

    FVector CurrentPosition = Unit->GetActorLocation();
    FVector TargetPosition = CombatTargetWorldPosition;

    // Z축은 무시하고 평면 거리만 계산
    FVector2D CurrentPos2D(CurrentPosition.X, CurrentPosition.Y);
    FVector2D TargetPos2D(TargetPosition.X, TargetPosition.Y);
    
    float Distance = FVector2D::Distance(CurrentPos2D, TargetPos2D);

    return Distance <= ArrivalDistance;
}

// ================= 전투 시각화 구현 =================

void UUnitVisualizationComponent::StartCombatVisualization(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender, const FCombatResult& CombatResult)
{
    if (!Attacker || !Defender || !WorldComponent)
    {
        return;
    }

    // 이 컴포넌트의 소유자가 공격자인지 확인
    // 이 함수는 공격자의 컴포넌트에서 호출되어야 함
    AUnitCharacterBase* OwnerUnit = Cast<AUnitCharacterBase>(GetOwner());
    if (OwnerUnit != Attacker)
    {
        // 이 컴포넌트가 공격자의 것이 아니면 무시
        return;
    }

    // ========== 0. 근거리/원거리 전투 구분 ==========
    // 공격자의 Range 확인 (Range > 1이면 원거리 전투)
    bIsRangedCombat = false;
    if (UUnitStatusComponent* AttackerStatus = Attacker->GetUnitStatusComponent())
    {
        int32 AttackerRange = AttackerStatus->GetRange();
        if (AttackerRange > 1)
        {
            bIsRangedCombat = true;
        }
    }

    // ========== 1. 공격자/방어자 참조 저장 ==========
    CombatAttacker = Attacker;
    CombatDefender = Defender;
    CurrentCombatResult = CombatResult;
    bIsInCombat = true;
    
    // 근거리/원거리에 따라 초기 상태 설정
    if (bIsRangedCombat)
    {
        CombatState = ECombatVisualizationState::RotatingAttacker; // 원거리: 이동 없이 회전부터
    }
    else
    {
        CombatState = ECombatVisualizationState::MovingToDefender; // 근거리: 이동부터
    }

    // ========== 2. 위치 정보 저장 ==========
    // 공격자 원래 위치 저장 (복귀용)
    FVector AttackerWorldPos = Attacker->GetActorLocation();
    AttackerOriginalWorldPosition = AttackerWorldPos;
    AttackerOriginalHexPosition = WorldComponent->WorldToHex(AttackerWorldPos);

    // 방어자 원래 위치 저장 (복귀용)
    FVector DefenderWorldPos = Defender->GetActorLocation();
    DefenderOriginalWorldPosition = DefenderWorldPos;
    DefenderOriginalHexPosition = WorldComponent->WorldToHex(DefenderWorldPos);

    // 방어자 타일 위치 저장 (전투 중 공격자가 이동할 위치)
    DefenderWorldPosition = DefenderWorldPos;
    DefenderHexPosition = WorldComponent->WorldToHex(DefenderWorldPos);

    // ========== 3. 공격자 컴포넌트 초기화 ==========
    // 몽타주 델리게이트 바인딩 (공격자)
    // 주의: 기존 바인딩이 있을 수 있으므로 먼저 제거 후 추가
    if (USkeletalMeshComponent* MeshComp = Attacker->GetMesh())
    {
        if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
        {
            AnimInstance->OnMontageEnded.RemoveAll(this);
            AnimInstance->OnMontageEnded.AddDynamic(this, &UUnitVisualizationComponent::OnMontageEnded);
        }
    }

    // ========== 4. 방어자 컴포넌트 초기화 ==========
    if (UUnitVisualizationComponent* DefenderVisComp = Defender->GetUnitVisualizationComponent())
    {
        // WorldComponent 설정 (없으면)
        if (!DefenderVisComp->GetWorldComponent())
        {
            DefenderVisComp->SetWorldComponent(WorldComponent);
        }

        // UnitManager 설정 (없으면)
        if (UnitManager)
        {
            DefenderVisComp->SetUnitManager(UnitManager);
        }

        // 전투 데이터 설정
        DefenderVisComp->bIsRangedCombat = bIsRangedCombat; // 원거리 전투 여부 전달
        if (bIsRangedCombat)
        {
            DefenderVisComp->CombatState = ECombatVisualizationState::RotatingDefender_Ranged; // 원거리: 방어자 회전
        }
        else
        {
            DefenderVisComp->CombatState = ECombatVisualizationState::RotatingDefender; // 근거리: 방어자 회전
        }
        DefenderVisComp->CombatAttacker = Attacker;
        DefenderVisComp->CombatDefender = Defender;
        DefenderVisComp->CurrentCombatResult = CombatResult;
        DefenderVisComp->bIsInCombat = true;
        DefenderVisComp->DefenderHexPosition = DefenderHexPosition;
        DefenderVisComp->DefenderWorldPosition = DefenderWorldPosition;
        DefenderVisComp->DefenderOriginalHexPosition = DefenderOriginalHexPosition;
        DefenderVisComp->DefenderOriginalWorldPosition = DefenderOriginalWorldPosition;
        DefenderVisComp->AttackerOriginalHexPosition = AttackerOriginalHexPosition;
        DefenderVisComp->AttackerOriginalWorldPosition = AttackerOriginalWorldPosition;

        // 방어자 몽타주 델리게이트 바인딩
        // 주의: 기존 바인딩이 있을 수 있으므로 먼저 제거 후 추가
        if (USkeletalMeshComponent* DefenderMeshComp = Defender->GetMesh())
        {
            if (UAnimInstance* DefenderAnimInstance = DefenderMeshComp->GetAnimInstance())
            {
                DefenderAnimInstance->OnMontageEnded.RemoveAll(DefenderVisComp);
                DefenderAnimInstance->OnMontageEnded.AddDynamic(DefenderVisComp, &UUnitVisualizationComponent::OnMontageEnded);
            }
        }
    }

    // 근거리/원거리에 따라 시작 함수 호출
    if (bIsRangedCombat)
    {
        // 원거리: 이동 없이 회전부터 시작 (UpdateCombatVisualization에서 처리)
    }
    else
    {
        // 근거리: 이동 시작
        StartMovingToDefender();
    }
}

void UUnitVisualizationComponent::StopCombatVisualization()
{
    if (!bIsInCombat)
    {
        return;
    }

    AUnitCharacterBase* OwnerUnit = Cast<AUnitCharacterBase>(GetOwner());
    if (!OwnerUnit)
    {
        return;
    }

    // 몽타주 델리게이트 해제
    if (USkeletalMeshComponent* MeshComp = OwnerUnit->GetMesh())
    {
        if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
        {
            AnimInstance->OnMontageEnded.RemoveAll(this);
        }
    }

    // 현재 재생 중인 몽타주 중지
    if (CurrentPlayingMontage)
    {
        if (USkeletalMeshComponent* MeshComp = OwnerUnit->GetMesh())
        {
            if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
            {
                AnimInstance->Montage_Stop(0.1f, CurrentPlayingMontage);
            }
        }
    }

    // 전투 상태 초기화
    bIsInCombat = false;
    bIsRangedCombat = false;
    bIsCityCombat = false;
    CombatState = ECombatVisualizationState::None;
    CombatAttacker = nullptr;
    CombatDefender = nullptr;
    CurrentPlayingMontage = nullptr;
    CityHexPosition = FVector2D::ZeroVector;
    CityWorldPosition = FVector::ZeroVector;
}

void UUnitVisualizationComponent::UpdateCombatVisualization(float DeltaTime)
{
    AUnitCharacterBase* OwnerUnit = Cast<AUnitCharacterBase>(GetOwner());
    if (!OwnerUnit || !WorldComponent)
    {
        StopCombatVisualization();
        return;
    }

    switch (CombatState)
    {
        case ECombatVisualizationState::MovingToDefender:
        {
            // 공격자가 방어자 타일로 이동 중
            // 이동 완료는 HasReachedCombatTarget()에서 체크되어 StopCombatMovement() 호출
            // 여기서는 이동 완료 후 상태 전환만 처리
            if (!bIsCombatMoving)
            {
                // 이동 완료 - 공격 애니메이션 시작
                CombatState = ECombatVisualizationState::AttackerAttack;
                PlayAttackerAttackMontage();
            }
            break;
        }

        case ECombatVisualizationState::RotatingDefender:
        {
            // 방어자 회전 처리
            if (CombatAttacker.IsValid())
            {
                UpdateDefenderRotation(DeltaTime);
            }
            break;
        }

        case ECombatVisualizationState::AttackerAttack:
        case ECombatVisualizationState::DefenderHit:
        case ECombatVisualizationState::DefenderDeath:
        case ECombatVisualizationState::DefenderCounter:
        case ECombatVisualizationState::AttackerHit:
        {
            // 몽타주 재생 중 - OnMontageEnded에서 처리
            break;
        }

        case ECombatVisualizationState::ReturningToOrigin:
        {
            // 공격자 또는 방어자 복귀 중 (근거리)
            // 이동 완료는 HasReachedTarget()에서 체크되어 MoveToNextTile() 호출
            if (!bIsMoving)
            {
                // 복귀 완료 처리
                // 공격자인지 방어자인지 확인
                bool bIsAttacker = CombatAttacker.IsValid() && OwnerUnit == CombatAttacker.Get();
                bool bIsDefender = CombatDefender.IsValid() && OwnerUnit == CombatDefender.Get();

                if (bIsAttacker)
                {
                    // 근거리 전투: 공격자 복귀 완료 시 즉시 전투 완료 (CivilizationShort 방식)
                    CompleteMeleeCombatVisualization();
                }
                else if (bIsDefender)
                {
                    // 방어자 복귀 완료 - 전투 상태 초기화만 수행 (알림은 공격자에서 처리)
                    StopCombatVisualization();
                }
            }
            break;
        }

        // ================= 원거리 전투 상태 =================
        case ECombatVisualizationState::RotatingAttacker:
        {
            // 공격자 회전 처리 (원거리)
            if (CombatDefender.IsValid())
            {
                UpdateAttackerRotation(DeltaTime);
            }
            break;
        }

        case ECombatVisualizationState::RotatingDefender_Ranged:
        {
            // 방어자 회전 처리 (원거리)
            if (CombatAttacker.IsValid())
            {
                UpdateDefenderRotation_Ranged(DeltaTime);
            }
            break;
        }

        case ECombatVisualizationState::AttackerAttack_Ranged:
        case ECombatVisualizationState::DefenderHit_Ranged:
        case ECombatVisualizationState::DefenderDeath_Ranged:
        {
            // 몽타주 재생 중 (원거리) - OnMontageEnded에서 처리
            break;
        }

        case ECombatVisualizationState::ReturningToOrigin_Ranged:
        {
            // 공격자 또는 방어자 복귀 중 (원거리)
            // 이동 완료는 HasReachedCombatTarget()에서 체크되어 StopCombatMovement() 호출
            if (!bIsCombatMoving)
            {
                // 복귀 완료 처리
                // 공격자인지 방어자인지 확인
                bool bIsAttacker = CombatAttacker.IsValid() && OwnerUnit == CombatAttacker.Get();
                bool bIsDefender = CombatDefender.IsValid() && OwnerUnit == CombatDefender.Get();

                if (bIsAttacker)
                {
                    // 원거리 전투: 복귀 완료 플래그 설정 후 확인 (CivilizationRanged 방식)
                    bAttackerReturned = true;
                    CheckAndNotifyRangedCombatComplete();
                }
                else if (bIsDefender)
                {
                    // 방어자 복귀 완료 - 공격자 컴포넌트에 알림
                    if (CombatAttacker.IsValid())
                    {
                        if (UUnitVisualizationComponent* AttackerVisComp = CombatAttacker->GetUnitVisualizationComponent())
                        {
                            AttackerVisComp->bDefenderReturned = true;
                            AttackerVisComp->CheckAndNotifyRangedCombatComplete();
                        }
                    }
                    // 방어자 자신은 전투 상태 초기화
                    StopCombatVisualization();
                }
            }
            break;
        }

        // ================= 도시 공격 전투 상태 =================
        case ECombatVisualizationState::RotatingToCity:
        {
            // 공격자가 도시를 바라보도록 회전 처리
            UpdateAttackerRotationToCity(DeltaTime);
            break;
        }

        case ECombatVisualizationState::AttackingCity:
        {
            // 공격 몽타주 재생 중 - OnMontageEnded에서 처리
            break;
        }

        case ECombatVisualizationState::ReturningFromCity:
        {
            // 공격자 복귀 중
            // 이동 완료는 HasReachedCombatTarget()에서 체크되어 StopCombatMovement() 호출
            if (!bIsCombatMoving)
            {
                // 복귀 완료 - 전투 완료 처리
                CompleteCityCombatVisualization();
            }
            break;
        }

        default:
            break;
    }
}

void UUnitVisualizationComponent::StartMovingToDefender()
{
    AUnitCharacterBase* OwnerUnit = Cast<AUnitCharacterBase>(GetOwner());
    if (!OwnerUnit || !WorldComponent || !CombatDefender.IsValid())
    {
        return;
    }

    // 공격자와 방어자의 실제 월드 좌표 가져오기
    FVector AttackerWorldPos = AttackerOriginalWorldPosition;
    FVector DefenderWorldPos = DefenderWorldPosition;

    // 방어자로부터 공격자 방향 벡터 계산
    FVector DirectionToAttacker = (AttackerWorldPos - DefenderWorldPos).GetSafeNormal();
    
    // 방어자로부터 20.0f 떨어진 목표 위치 계산
    FVector TargetWorldPosition = DefenderWorldPos + DirectionToAttacker * 20.0f;
    
    // 전투 시각화 전용 이동 시작
    StartCombatMovement(TargetWorldPosition);
}

void UUnitVisualizationComponent::CheckAndExecuteCombatJump(FVector2D FromHex, FVector2D ToHex)
{
    if (!WorldComponent)
    {
        return;
    }

    AUnitCharacterBase* Unit = Cast<AUnitCharacterBase>(GetOwner());
    if (!Unit)
    {
        return;
    }

    // 헥스 좌표를 정수로 반올림 (GetTileAtHex는 정수 좌표를 기대함)
    FVector2D FromHexRounded = FVector2D(FMath::RoundToInt(FromHex.X), FMath::RoundToInt(FromHex.Y));
    FVector2D ToHexRounded = FVector2D(FMath::RoundToInt(ToHex.X), FMath::RoundToInt(ToHex.Y));

    // 중복 점프 방지 (이미 이 타일로 점프했으면 다시 점프하지 않음)
    if (LastJumpedTargetHex == ToHexRounded)
    {
        return;
    }

    UWorldTile* CurrentTile = WorldComponent->GetTileAtHex(FromHexRounded);
    UWorldTile* TargetTile  = WorldComponent->GetTileAtHex(ToHexRounded);

    if (!CurrentTile || !TargetTile)
    {
        return;
    }

    ELandType CurrentLandType = CurrentTile->GetLandType();
    ELandType TargetLandType  = TargetTile->GetLandType();

    // 점프 조건: 평지 → 언덕, 언덕 → 산 (일반 이동 로직과 동일)
    bool bShouldJump = false;
    if (CurrentLandType == ELandType::Plains && TargetLandType == ELandType::Hills)
    {
        bShouldJump = true;
    }
    else if (CurrentLandType == ELandType::Hills && TargetLandType == ELandType::Mountains)
    {
        bShouldJump = true;
    }

    if (!bShouldJump)
    {
        return;
    }

    if (UCharacterMovementComponent* MoveComp = Unit->GetCharacterMovement())
    {
        if (!MoveComp->IsFalling() && MoveComp->CanAttemptJump())
        {
            Unit->Jump();
            LastJumpedTargetHex = ToHexRounded; // 점프 기록
        }
    }
}

void UUnitVisualizationComponent::UpdateDefenderRotation(float DeltaTime)
{
    AUnitCharacterBase* Defender = Cast<AUnitCharacterBase>(GetOwner());
    if (!Defender || !CombatAttacker.IsValid())
    {
        return;
    }

    // 공격자 위치
    FVector AttackerLocation = CombatAttacker->GetActorLocation();
    
    // 회전 처리
    RotateUnitToFaceTarget(Defender, AttackerLocation, DeltaTime);

    // 회전 완료 체크
    FVector DefenderLocation = Defender->GetActorLocation();
    FVector Direction = (AttackerLocation - DefenderLocation).GetSafeNormal();
    Direction.Z = 0.0f;
    Direction.Normalize();

    FRotator TargetRotation = Direction.Rotation();
    FRotator CurrentRotation = Defender->GetActorRotation();
    float YawDifference = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetRotation.Yaw));

    if (YawDifference <= RotationTolerance)
    {
        Defender->SetActorRotation(TargetRotation);
        // 회전 완료 - 상태는 그대로 유지 (공격 애니메이션 시작을 기다림)
    }
}

void UUnitVisualizationComponent::PlayAttackerAttackMontage()
{
    AUnitCharacterBase* Attacker = Cast<AUnitCharacterBase>(GetOwner());
    if (!Attacker)
    {
        return;
    }

    // UnitData에서 AttackMontage 가져오기
    UAnimMontage* MontageToPlay = nullptr;
    if (const FUnitData* UnitData = Attacker->GetUnitData())
    {
        MontageToPlay = UnitData->AttackMontage;
    }
    else
    {
        MontageToPlay = AttackMontage;
    }

    if (!MontageToPlay)
    {
        // 몽타주가 없으면 바로 다음 단계로 (노티파이 호출)
        OnCombatNotify_Hit();
        return;
    }

    PlayMontage(MontageToPlay, Attacker);
}

void UUnitVisualizationComponent::PlayDefenderHitOrDeathMontage()
{
    if (!CombatDefender.IsValid())
    {
        return;
    }

    AUnitCharacterBase* Defender = CombatDefender.Get();
    
    UAnimMontage* MontageToPlay = nullptr;
    
    // 방어자가 죽었으면 사망 몽타주, 아니면 피격 몽타주
    if (!CurrentCombatResult.bDefenderAlive)
    {
        // UnitData에서 DeathMontage 가져오기
        if (const FUnitData* UnitData = Defender->GetUnitData())
        {
            MontageToPlay = UnitData->DeathMontage;
        }
        else
        {
            MontageToPlay = DeathMontage; // 폴백
        }
    }
    else
    {
        // UnitData에서 HitMontage 가져오기
        if (const FUnitData* UnitData = Defender->GetUnitData())
        {
            MontageToPlay = UnitData->HitMontage;
        }
        else
        {
            MontageToPlay = HitMontage; // 폴백
        }
    }

    if (!MontageToPlay)
    {
        // 몽타주가 없으면 바로 다음 단계로
        OnDefenderHitOrDeathMontageEnded();
        return;
    }

    // 상태 설정
    if (!CurrentCombatResult.bDefenderAlive)
    {
        CombatState = ECombatVisualizationState::DefenderDeath;
    }
    else
    {
        CombatState = ECombatVisualizationState::DefenderHit;
    }

    // 방어자 피격/사망 몽타주 재생
    if (UUnitVisualizationComponent* DefenderVisComp = Defender->GetUnitVisualizationComponent())
    {
        DefenderVisComp->CombatState = CombatState;
        DefenderVisComp->PlayMontage(MontageToPlay, Defender);
    }
}

void UUnitVisualizationComponent::PlayDefenderCounterMontage()
{
    if (!CombatDefender.IsValid())
    {
        return;
    }

    AUnitCharacterBase* Defender = CombatDefender.Get();

    // UnitData에서 AttackMontage 가져오기 (반격도 공격 몽타주 사용)
    UAnimMontage* MontageToPlay = nullptr;
    if (const FUnitData* UnitData = Defender->GetUnitData())
    {
        MontageToPlay = UnitData->AttackMontage;
    }
    else
    {
        MontageToPlay = AttackMontage; // 폴백
    }

    if (!MontageToPlay)
    {
        // 반격 몽타주가 없으면 바로 다음 단계로 (노티파이 호출)
        OnCombatNotify_Hit();
        return;
    }

    // 방어자의 컴포넌트에서 반격 몽타주 재생 (AttackMontage 사용)
    if (UUnitVisualizationComponent* DefenderVisComp = Defender->GetUnitVisualizationComponent())
    {
        DefenderVisComp->CombatState = ECombatVisualizationState::DefenderCounter;
        DefenderVisComp->PlayMontage(MontageToPlay, Defender);
    }
}

void UUnitVisualizationComponent::PlayAttackerHitOrDeathMontage()
{
    AUnitCharacterBase* Attacker = Cast<AUnitCharacterBase>(GetOwner());
    if (!Attacker)
    {
        return;
    }

    // 반격으로 공격자는 죽을 수 없으므로 항상 피격 몽타주만 재생
    UAnimMontage* MontageToPlay = nullptr;
    
    // UnitData에서 HitMontage 가져오기
    if (const FUnitData* UnitData = Attacker->GetUnitData())
    {
        MontageToPlay = UnitData->HitMontage;
    }
    else
    {
        MontageToPlay = HitMontage; // 폴백
    }

    if (!MontageToPlay)
    {
        // 몽타주가 없으면 바로 다음 단계로
        OnAttackerHitOrDeathMontageEnded();
        return;
    }

    // 상태 설정 (항상 피격 상태)
    CombatState = ECombatVisualizationState::AttackerHit;

    // 공격자 피격 몽타주 재생
    PlayMontage(MontageToPlay, Attacker);
}

void UUnitVisualizationComponent::StartReturningToOrigin()
{
    // 이 함수는 공격자 또는 방어자의 컴포넌트에서 호출될 수 있음
    AUnitCharacterBase* OwnerUnit = Cast<AUnitCharacterBase>(GetOwner());
    if (!OwnerUnit || !WorldComponent)
    {
        return;
    }

    // 공격자인지 방어자인지 확인
    bool bIsAttacker = CombatAttacker.IsValid() && OwnerUnit == CombatAttacker.Get();
    bool bIsDefender = CombatDefender.IsValid() && OwnerUnit == CombatDefender.Get();

    if (!bIsAttacker && !bIsDefender)
    {
        return;
    }

    // 공격자인 경우
    if (bIsAttacker)
    {
        // 반격으로 공격자는 죽을 수 없으므로 항상 복귀 수행
        // 현재 위치(방어자 근처)와 원래 위치 비교해서 점프 체크
        FVector2D CurrentHex = WorldComponent->WorldToHex(OwnerUnit->GetActorLocation());
        FVector2D OriginHex = AttackerOriginalHexPosition;
        CheckAndExecuteCombatJump(CurrentHex, OriginHex);

        // 원래 위치의 월드 좌표 가져오기
        FVector OriginWorldPosition = AttackerOriginalWorldPosition;

        // 복귀 상태 설정 및 전투 시각화 전용 이동 시작
        CombatState = ECombatVisualizationState::ReturningToOrigin;
        StartCombatMovement(OriginWorldPosition);
    }
    // 방어자인 경우
    else if (bIsDefender)
    {
        // 방어자가 살아있지 않으면 복귀하지 않음
        if (!CurrentCombatResult.bDefenderAlive)
        {
            StopCombatVisualization();
            return;
        }

        // 원래 위치의 월드 좌표 가져오기
        FVector OriginWorldPosition = DefenderOriginalWorldPosition;

        // 복귀 상태 설정 및 전투 시각화 전용 이동 시작
        CombatState = ECombatVisualizationState::ReturningToOrigin;
        StartCombatMovement(OriginWorldPosition);
    }
}

// ================= 근거리 전투 완료 처리 (CivilizationShort 방식) =================
void UUnitVisualizationComponent::CompleteMeleeCombatVisualization()
{
    AUnitCharacterBase* Attacker = Cast<AUnitCharacterBase>(GetOwner());
    if (!Attacker || !CombatAttacker.IsValid() || Attacker != CombatAttacker.Get())
    {
        return;
    }

    // 방어자가 살아있으면 원래 위치로 복귀 시작 (복귀 완료를 기다리지 않음)
    if (CombatDefender.IsValid() && CurrentCombatResult.bDefenderAlive)
    {
        AUnitCharacterBase* Defender = CombatDefender.Get();
        if (UUnitVisualizationComponent* DefenderVisComp = Defender->GetUnitVisualizationComponent())
        {
            // 로드 시 WorldComponent가 설정되지 않았을 수 있으므로 확인 및 설정
            if (!DefenderVisComp->GetWorldComponent() && WorldComponent)
            {
                DefenderVisComp->SetWorldComponent(WorldComponent);
            }
            
            // UnitManager도 설정 (없으면)
            if (!DefenderVisComp->UnitManager && UnitManager)
            {
                DefenderVisComp->SetUnitManager(UnitManager);
            }
            
            if (WorldComponent)
            {
                FVector2D DefenderCurrentHex = WorldComponent->WorldToHex(Defender->GetActorLocation());
                if (DefenderCurrentHex != DefenderVisComp->DefenderOriginalHexPosition)
                {
                    // 방어자 복귀 시작 (근거리)
                    DefenderVisComp->StartReturningToOrigin();
                    // 주의: 공격자는 방어자 복귀를 기다리지 않고 즉시 완료 알림
                }
                else
                {
                    // 이미 원래 위치에 있으면 즉시 초기화
                    DefenderVisComp->StopCombatVisualization();
                }
            }
        }
    }
    else if (CombatDefender.IsValid())
    {
        // 방어자가 죽었으면 즉시 초기화
        if (UUnitVisualizationComponent* DefenderVisComp = CombatDefender->GetUnitVisualizationComponent())
        {
            DefenderVisComp->StopCombatVisualization();
        }
    }

    // 근거리 전투: 즉시 전투 완료 알림 (방어자 복귀를 기다리지 않음)
    if (UnitManager && CombatAttacker.IsValid())
    {
        FVector2D AttackerHex = AttackerOriginalHexPosition;
        FVector2D DefenderHex = DefenderOriginalHexPosition;
        
        AUnitCharacterBase* DefenderPtr = nullptr;
        if (CombatDefender.IsValid())
        {
            DefenderPtr = CombatDefender.Get();
        }
        
        UnitManager->OnCombatVisualizationComplete(CombatAttacker.Get(), DefenderPtr, CurrentCombatResult, AttackerHex, DefenderHex);
    }

    // 공격자 자신 초기화
    StopCombatVisualization();
}

// ================= 원거리 전투 완료 처리 (CivilizationRanged 방식) =================
void UUnitVisualizationComponent::CompleteRangedCombatVisualization()
{
    AUnitCharacterBase* Attacker = Cast<AUnitCharacterBase>(GetOwner());
    if (!Attacker || !CombatAttacker.IsValid() || Attacker != CombatAttacker.Get())
    {
        return;
    }

    // 복귀 완료 플래그 초기화 (원거리 전투용)
    bAttackerReturned = false;
    bDefenderReturned = false;

    // 방어자가 살아있으면 원래 위치로 복귀 시작 (원거리는 제자리이므로 복귀 없을 수도 있음)
    if (CombatDefender.IsValid() && CurrentCombatResult.bDefenderAlive)
    {
        AUnitCharacterBase* Defender = CombatDefender.Get();
        if (UUnitVisualizationComponent* DefenderVisComp = Defender->GetUnitVisualizationComponent())
        {
            // 로드 시 WorldComponent가 설정되지 않았을 수 있으므로 확인 및 설정
            if (!DefenderVisComp->GetWorldComponent() && WorldComponent)
            {
                DefenderVisComp->SetWorldComponent(WorldComponent);
            }
            
            // UnitManager도 설정 (없으면)
            if (!DefenderVisComp->UnitManager && UnitManager)
            {
                DefenderVisComp->SetUnitManager(UnitManager);
            }
            
            if (WorldComponent)
            {
                FVector2D DefenderCurrentHex = WorldComponent->WorldToHex(Defender->GetActorLocation());
                if (DefenderCurrentHex != DefenderVisComp->DefenderOriginalHexPosition)
                {
                    // 방어자 복귀 시작 (원거리)
                    DefenderVisComp->StartReturningToOrigin_Ranged();
                    bDefenderReturned = false; // 복귀 필요
                }
                else
                {
                    // 이미 원래 위치에 있으면 복귀 완료로 처리
                    DefenderVisComp->StopCombatVisualization();
                    bDefenderReturned = true; // 복귀 불필요
                }
            }
        }
    }
    else if (CombatDefender.IsValid())
    {
        // 방어자가 죽었으면 복귀 완료로 처리
        if (UUnitVisualizationComponent* DefenderVisComp = CombatDefender->GetUnitVisualizationComponent())
        {
            DefenderVisComp->StopCombatVisualization();
        }
        bDefenderReturned = true; // 방어자가 죽었으므로 복귀 불필요
    }
    else
    {
        bDefenderReturned = true;
    }

    // 원거리 전투: 공격자 복귀 시작 (제자리 공격이므로 복귀 없을 수도 있음)
    if (WorldComponent)
    {
        FVector2D AttackerCurrentHex = WorldComponent->WorldToHex(Attacker->GetActorLocation());
        if (AttackerCurrentHex != AttackerOriginalHexPosition)
        {
            // 공격자가 원래 위치에 있지 않으면 복귀 시작 (원거리)
            StartReturningToOrigin_Ranged();
            bAttackerReturned = false; // 복귀 필요
        }
        else
        {
            // 이미 원래 위치에 있으면 복귀 완료로 처리
            bAttackerReturned = true;
            // 모든 복귀 완료 확인 후 전투 완료 알림
            CheckAndNotifyRangedCombatComplete();
        }
    }
    else
    {
        // WorldComponent가 없으면 복귀 완료로 처리 (폴백)
        bAttackerReturned = true;
        CheckAndNotifyRangedCombatComplete();
    }
}

// ================= 원거리 전투 복귀 완료 확인 후 알림 (CivilizationRanged 방식) =================
void UUnitVisualizationComponent::CheckAndNotifyRangedCombatComplete()
{
    // 공격자 컴포넌트에서만 호출되어야 함
    AUnitCharacterBase* OwnerUnit = Cast<AUnitCharacterBase>(GetOwner());
    if (!OwnerUnit || !CombatAttacker.IsValid() || OwnerUnit != CombatAttacker.Get())
    {
        return;
    }

    // 모든 복귀 완료 확인 (원거리 전투)
    // 방어자가 죽었을 때는 공격자 복귀 완료만으로 전투 완료
    bool bAllReturned = bAttackerReturned && (bDefenderReturned || !CurrentCombatResult.bDefenderAlive);

    if (bAllReturned)
    {
        // UnitManager에 전투 완료 알림
        // 주의: 방어자가 Destroy되었어도 전투 완료 알림은 반드시 보내야 함 (PendingCombatActions 감소를 위해)
        if (UnitManager && CombatAttacker.IsValid())
        {
            FVector2D AttackerHex = AttackerOriginalHexPosition;
            FVector2D DefenderHex = DefenderOriginalHexPosition;
            
            // 방어자 참조 가져오기 (Destroy되었어도 참조는 유효할 수 있음)
            AUnitCharacterBase* DefenderPtr = nullptr;
            if (CombatDefender.IsValid())
            {
                DefenderPtr = CombatDefender.Get();
            }
            
            // 방어자가 Destroy되었어도 전투 완료 알림은 보내야 함
            UnitManager->OnCombatVisualizationComplete(CombatAttacker.Get(), DefenderPtr, CurrentCombatResult, AttackerHex, DefenderHex);
        }

        // 공격자 자신 초기화
        StopCombatVisualization();
    }
}

void UUnitVisualizationComponent::PlayMontage(UAnimMontage* Montage, AUnitCharacterBase* TargetUnit)
{
    if (!Montage || !TargetUnit)
    {
        return;
    }

    if (USkeletalMeshComponent* MeshComp = TargetUnit->GetMesh())
    {
        if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
        {
            // 현재 재생 중인 몽타주 저장
            CurrentPlayingMontage = Montage;
            
            // 몽타주 재생
            AnimInstance->Montage_Play(Montage, 1.0f);
        }
    }
}

void UUnitVisualizationComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (!Montage || Montage != CurrentPlayingMontage)
    {
        return;
    }

    CurrentPlayingMontage = nullptr;

    // 현재 상태에 따라 다음 단계로 진행
    switch (CombatState)
    {
        case ECombatVisualizationState::AttackerAttack:
        {
            // 공격 애니메이션 종료
            // 피격/사망은 노티파이에서 이미 처리됨
            // 반격 체크 → 반격 있으면 반격 몽타주, 없으면 복귀/완료 (5단계에서 구현)
            OnDefenderHitOrDeathMontageEnded();
            break;
        }

        case ECombatVisualizationState::DefenderHit:
        case ECombatVisualizationState::DefenderDeath:
        {
            // 방어자 피격/사망 애니메이션 종료 (5단계에서 구현)
            OnDefenderHitOrDeathMontageEnded();
            break;
        }

        case ECombatVisualizationState::DefenderCounter:
        {
            // 방어자 반격 애니메이션 종료 - 공격자 피격/사망 애니메이션 재생
            // 주의: 이 함수는 방어자의 컴포넌트에서 호출되므로, 공격자의 컴포넌트를 찾아서 호출해야 함
            if (CombatAttacker.IsValid())
            {
                AUnitCharacterBase* Attacker = CombatAttacker.Get();
                if (UUnitVisualizationComponent* AttackerVisComp = Attacker->GetUnitVisualizationComponent())
                {
                    AttackerVisComp->PlayAttackerHitOrDeathMontage();
                }
            }
            break;
        }

        case ECombatVisualizationState::AttackerHit:
        {
            // 공격자 피격 애니메이션 종료 - 복귀 (근거리)
            // 반격으로 공격자는 죽을 수 없으므로 항상 복귀
            StartReturningToOrigin();
            break;
        }

        // ================= 원거리 전투 몽타주 종료 처리 =================
        case ECombatVisualizationState::AttackerAttack_Ranged:
        {
            // 공격자 공격 애니메이션 종료 (원거리)
            // 노티파이에서 이미 방어자 피격/사망 애니메이션이 시작되었으므로 여기서는 아무것도 하지 않음
            break;
        }

        case ECombatVisualizationState::DefenderHit_Ranged:
        case ECombatVisualizationState::DefenderDeath_Ranged:
        {
            // 방어자 피격/사망 애니메이션 종료 (원거리)
            // 원거리 전투 완료 처리 (CivilizationRanged 방식)
            if (CombatAttacker.IsValid())
            {
                AUnitCharacterBase* Attacker = CombatAttacker.Get();
                if (UUnitVisualizationComponent* AttackerVisComp = Attacker->GetUnitVisualizationComponent())
                {
                    // 원거리 전투 완료 함수 호출
                    AttackerVisComp->CompleteRangedCombatVisualization();
                }
            }
            // 방어자 상태 초기화
            StopCombatVisualization();
            break;
        }

        // ================= 도시 공격 몽타주 종료 처리 =================
        case ECombatVisualizationState::AttackingCity:
        {
            // 도시 공격 몽타주 종료 - 복귀 시작
            StartReturningFromCity();
            break;
        }

        default:
            break;
    }
}

void UUnitVisualizationComponent::OnCombatNotify_Hit()
{
    // 노티파이 호출 - 현재 상태에 따라 피격/사망 애니메이션 재생
    // AttackerAttack 상태: 방어자 피격/사망 (근거리)
    // AttackerAttack_Ranged 상태: 방어자 피격/사망 (원거리)
    // DefenderCounter 상태: 공격자 피격/사망 (근거리 반격)
    switch (CombatState)
    {
        case ECombatVisualizationState::AttackerAttack:
        {
            // 공격 노티파이 - 방어자 피격/사망 애니메이션 재생 (근거리)
            PlayDefenderHitOrDeathMontage();
            break;
        }

        case ECombatVisualizationState::AttackerAttack_Ranged:
        {
            // 공격 노티파이 - 방어자 피격/사망 애니메이션 재생 (원거리)
            PlayDefenderHitOrDeathMontage_Ranged();
            break;
        }

        case ECombatVisualizationState::DefenderCounter:
        {
            // 반격 노티파이 - 공격자 피격/사망 애니메이션 재생 (근거리 반격)
            // 주의: 이 함수는 방어자의 컴포넌트에서 호출되므로, 공격자의 컴포넌트를 찾아서 호출해야 함
            if (CombatAttacker.IsValid())
            {
                AUnitCharacterBase* Attacker = CombatAttacker.Get();
                if (UUnitVisualizationComponent* AttackerVisComp = Attacker->GetUnitVisualizationComponent())
                {
                    AttackerVisComp->PlayAttackerHitOrDeathMontage();
                }
            }
            break;
        }

        default:
            // 다른 상태에서는 무시
            break;
    }
}

void UUnitVisualizationComponent::OnDefenderHitOrDeathMontageEnded()
{
    // 이 함수는 방어자의 컴포넌트에서 호출됨
    AUnitCharacterBase* OwnerUnit = Cast<AUnitCharacterBase>(GetOwner());
    if (!OwnerUnit)
    {
        return;
    }

    // 이 컴포넌트의 소유자가 방어자인지 확인
    if (!CombatDefender.IsValid() || OwnerUnit != CombatDefender.Get())
    {
        return;
    }

    // 방어자가 죽었으면 사망 처리 (Death 노티파이에서 이미 처리됨)
    // 여기서는 몽타주 종료 후 정리만 수행
    if (!CurrentCombatResult.bDefenderAlive)
    {
        // 방어자가 죽었으면 반격 없음 - 공격자 복귀
        // 주의: 이 함수는 방어자의 컴포넌트에서 호출되므로, 공격자의 컴포넌트를 찾아서 호출해야 함
        // 반격으로 공격자는 죽을 수 없으므로 항상 살아있음
        if (CombatAttacker.IsValid())
        {
            AUnitCharacterBase* Attacker = CombatAttacker.Get();
            if (UUnitVisualizationComponent* AttackerVisComp = Attacker->GetUnitVisualizationComponent())
            {
                AttackerVisComp->StartReturningToOrigin();
            }
        }
        return;
    }

    // 방어자가 살아있으면 반격 체크
    if (CurrentCombatResult.DefenderDamageDealt > 0)
    {
        // 반격 있음 - 방어자 반격 애니메이션 재생
        PlayDefenderCounterMontage();
    }
    else
    {
        // 반격 없음 - 공격자 복귀
        // 주의: 이 함수는 방어자의 컴포넌트에서 호출되므로, 공격자의 컴포넌트를 찾아서 호출해야 함
        // 반격으로 공격자는 죽을 수 없으므로 항상 살아있음
        if (CombatAttacker.IsValid())
        {
            AUnitCharacterBase* Attacker = CombatAttacker.Get();
            if (UUnitVisualizationComponent* AttackerVisComp = Attacker->GetUnitVisualizationComponent())
            {
                AttackerVisComp->StartReturningToOrigin();
            }
        }
    }
}

void UUnitVisualizationComponent::OnAttackerHitOrDeathMontageEnded()
{
    // 이 함수는 공격자의 컴포넌트에서만 호출되어야 함
    AUnitCharacterBase* OwnerUnit = Cast<AUnitCharacterBase>(GetOwner());
    if (!OwnerUnit)
    {
        return;
    }

    // 이 컴포넌트의 소유자가 공격자인지 확인
    if (!CombatAttacker.IsValid() || OwnerUnit != CombatAttacker.Get())
    {
        return;
    }

    // 반격으로 공격자는 죽을 수 없으므로 항상 복귀
    StartReturningToOrigin();
}

void UUnitVisualizationComponent::OnCombatNotify_Death()
{
    // 사망 노티파이 - Death 몽타주에서 호출됨
    // 반격으로 공격자는 죽을 수 없으므로 방어자만 destroy 처리
    AUnitCharacterBase* OwnerUnit = Cast<AUnitCharacterBase>(GetOwner());
    if (!OwnerUnit)
    {
        return;
    }

    bool bIsDefender = CombatDefender.IsValid() && OwnerUnit == CombatDefender.Get();

    if (bIsDefender && !CurrentCombatResult.bDefenderAlive)
    {
        // 방어자가 죽었으면 destroy 처리
        if (UnitManager)
        {
            UnitManager->DestroyUnit(OwnerUnit, DefenderHexPosition);
        }
        
        // 방어자가 죽었으면 반격 없음 - 공격자 복귀 시작
        // 주의: destroy 전에 공격자 복귀를 시작해야 함 (destroy 후에는 컴포넌트가 유효하지 않을 수 있음)
        // 반격으로 공격자는 죽을 수 없으므로 항상 살아있음
        if (CombatAttacker.IsValid())
        {
            AUnitCharacterBase* Attacker = CombatAttacker.Get();
            if (UUnitVisualizationComponent* AttackerVisComp = Attacker->GetUnitVisualizationComponent())
            {
                // 근거리/원거리에 따라 다른 처리
                if (AttackerVisComp->bIsRangedCombat)
                {
                    // 원거리 전투 완료 (원거리는 복귀 불필요하므로 즉시 완료)
                    AttackerVisComp->CompleteRangedCombatVisualization();
                }
                else
                {
                    // 근거리 전투: 공격자 복귀 시작 (복귀 완료 후 CompleteMeleeCombatVisualization 호출됨)
                    AttackerVisComp->StartReturningToOrigin();
                }
            }
        }
    }
}

void UUnitVisualizationComponent::RotateUnitToFaceTarget(AUnitCharacterBase* Unit, const FVector& TargetLocation, float DeltaTime)
{
    if (!Unit)
    {
        return;
    }

    FVector UnitLocation = Unit->GetActorLocation();
    FVector Direction = (TargetLocation - UnitLocation).GetSafeNormal();
    Direction.Z = 0.0f;
    Direction.Normalize();

    if (Direction.SizeSquared() < 0.01f)
    {
        return;
    }

    FRotator TargetRotation = Direction.Rotation();
    FRotator CurrentRotation = Unit->GetActorRotation();
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationSpeed / 90.0f);
    Unit->SetActorRotation(NewRotation);
}

// ================= 원거리 전투 함수 구현 =================

void UUnitVisualizationComponent::UpdateAttackerRotation(float DeltaTime)
{
    AUnitCharacterBase* Attacker = Cast<AUnitCharacterBase>(GetOwner());
    if (!Attacker || !CombatDefender.IsValid())
    {
        return;
    }

    // 방어자 위치
    FVector DefenderLocation = CombatDefender->GetActorLocation();
    
    // 회전 처리
    RotateUnitToFaceTarget(Attacker, DefenderLocation, DeltaTime);

    // 회전 완료 체크
    FVector AttackerLocation = Attacker->GetActorLocation();
    FVector Direction = (DefenderLocation - AttackerLocation).GetSafeNormal();
    Direction.Z = 0.0f;
    Direction.Normalize();

    FRotator TargetRotation = Direction.Rotation();
    FRotator CurrentRotation = Attacker->GetActorRotation();
    float YawDifference = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetRotation.Yaw));

    if (YawDifference <= RotationTolerance)
    {
        Attacker->SetActorRotation(TargetRotation);
        // 회전 완료 - 공격 애니메이션 시작
        // 방어자도 회전 완료되었는지 확인 (UpdateCombatVisualization에서 처리)
        // 여기서는 상태만 변경
        CombatState = ECombatVisualizationState::AttackerAttack_Ranged;
        PlayAttackerAttackMontage_Ranged();
    }
}

void UUnitVisualizationComponent::UpdateDefenderRotation_Ranged(float DeltaTime)
{
    AUnitCharacterBase* Defender = Cast<AUnitCharacterBase>(GetOwner());
    if (!Defender || !CombatAttacker.IsValid())
    {
        return;
    }

    // 공격자 위치
    FVector AttackerLocation = CombatAttacker->GetActorLocation();
    
    // 회전 처리
    RotateUnitToFaceTarget(Defender, AttackerLocation, DeltaTime);

    // 회전 완료 체크
    FVector DefenderLocation = Defender->GetActorLocation();
    FVector Direction = (AttackerLocation - DefenderLocation).GetSafeNormal();
    Direction.Z = 0.0f;
    Direction.Normalize();

    FRotator TargetRotation = Direction.Rotation();
    FRotator CurrentRotation = Defender->GetActorRotation();
    float YawDifference = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetRotation.Yaw));

    if (YawDifference <= RotationTolerance)
    {
        Defender->SetActorRotation(TargetRotation);
        // 회전 완료 - 상태는 그대로 유지 (공격 애니메이션 시작을 기다림)
    }
}

void UUnitVisualizationComponent::PlayAttackerAttackMontage_Ranged()
{
    AUnitCharacterBase* Attacker = Cast<AUnitCharacterBase>(GetOwner());
    if (!Attacker)
    {
        return;
    }

    // UnitData에서 AttackMontage 가져오기
    UAnimMontage* MontageToPlay = nullptr;
    if (const FUnitData* UnitData = Attacker->GetUnitData())
    {
        MontageToPlay = UnitData->AttackMontage;
    }
    else
    {
        MontageToPlay = AttackMontage;
    }

    if (!MontageToPlay)
    {
        // 몽타주가 없으면 바로 다음 단계로 (노티파이 호출)
        OnCombatNotify_Hit();
        return;
    }

    PlayMontage(MontageToPlay, Attacker);
}

void UUnitVisualizationComponent::PlayDefenderHitOrDeathMontage_Ranged()
{
    if (!CombatDefender.IsValid())
    {
        return;
    }

    AUnitCharacterBase* Defender = CombatDefender.Get();
    
    UAnimMontage* MontageToPlay = nullptr;
    
    // 방어자가 죽었으면 사망 몽타주, 아니면 피격 몽타주
    if (!CurrentCombatResult.bDefenderAlive)
    {
        // UnitData에서 DeathMontage 가져오기
        if (const FUnitData* UnitData = Defender->GetUnitData())
        {
            MontageToPlay = UnitData->DeathMontage;
        }
        else
        {
            MontageToPlay = DeathMontage; // 폴백
        }
        
        // 방어자의 컴포넌트에서 사망 몽타주 재생
        if (UUnitVisualizationComponent* DefenderVisComp = Defender->GetUnitVisualizationComponent())
        {
            DefenderVisComp->CombatState = ECombatVisualizationState::DefenderDeath_Ranged;
            DefenderVisComp->PlayMontage(MontageToPlay, Defender);
        }
    }
    else
    {
        // UnitData에서 HitMontage 가져오기
        if (const FUnitData* UnitData = Defender->GetUnitData())
        {
            MontageToPlay = UnitData->HitMontage;
        }
        else
        {
            MontageToPlay = HitMontage; // 폴백
        }
        
        // 방어자의 컴포넌트에서 피격 몽타주 재생
        if (UUnitVisualizationComponent* DefenderVisComp = Defender->GetUnitVisualizationComponent())
        {
            DefenderVisComp->CombatState = ECombatVisualizationState::DefenderHit_Ranged;
            DefenderVisComp->PlayMontage(MontageToPlay, Defender);
        }
    }
}

void UUnitVisualizationComponent::StartReturningToOrigin_Ranged()
{
    // 이 함수는 공격자 또는 방어자의 컴포넌트에서 호출될 수 있음
    AUnitCharacterBase* OwnerUnit = Cast<AUnitCharacterBase>(GetOwner());
    if (!OwnerUnit || !WorldComponent)
    {
        return;
    }

    // 공격자인지 방어자인지 확인
    bool bIsAttacker = CombatAttacker.IsValid() && OwnerUnit == CombatAttacker.Get();
    bool bIsDefender = CombatDefender.IsValid() && OwnerUnit == CombatDefender.Get();

    if (!bIsAttacker && !bIsDefender)
    {
        return;
    }

    // 공격자인 경우
    if (bIsAttacker)
    {
        // 원거리 전투는 제자리 복귀이므로 점프 체크 불필요
        // 원래 위치의 월드 좌표 가져오기
        FVector OriginWorldPosition = AttackerOriginalWorldPosition;

        // 복귀 상태 설정 및 전투 시각화 전용 이동 시작
        CombatState = ECombatVisualizationState::ReturningToOrigin_Ranged;
        StartCombatMovement(OriginWorldPosition);
    }
    // 방어자인 경우
    else if (bIsDefender)
    {
        // 방어자가 살아있지 않으면 복귀하지 않음
        if (!CurrentCombatResult.bDefenderAlive)
        {
            StopCombatVisualization();
            return;
        }

        // 원래 위치의 월드 좌표 가져오기
        FVector OriginWorldPosition = DefenderOriginalWorldPosition;

        // 복귀 상태 설정 및 전투 시각화 전용 이동 시작
        CombatState = ECombatVisualizationState::ReturningToOrigin_Ranged;
        StartCombatMovement(OriginWorldPosition);
    }
}

// ================= 도시 공격 전투 시각화 구현 =================

void UUnitVisualizationComponent::StartCombatVisualizationAgainstCity(AUnitCharacterBase* Attacker, UCityComponent* CityComponent, FVector2D CityHex, const FCombatResult& CombatResult)
{
    if (!Attacker || !CityComponent || !WorldComponent)
    {
        return;
    }

    // 이 컴포넌트의 소유자가 공격자인지 확인
    // 이 함수는 공격자의 컴포넌트에서 호출되어야 함
    AUnitCharacterBase* OwnerUnit = Cast<AUnitCharacterBase>(GetOwner());
    if (OwnerUnit != Attacker)
    {
        // 이 컴포넌트가 공격자의 것이 아니면 무시
        return;
    }

    // ========== 1. 전투 데이터 저장 ==========
    CombatAttacker = Attacker;
    CombatDefender = nullptr; // 도시는 유닛이 아니므로 nullptr
    CurrentCombatResult = CombatResult;
    bIsInCombat = true;
    bIsCityCombat = true;
    bIsRangedCombat = false; // 도시 공격은 근거리/원거리 구분 없음

    // ========== 2. 위치 정보 저장 ==========
    // 공격자 원래 위치 저장 (복귀용)
    FVector AttackerWorldPos = Attacker->GetActorLocation();
    AttackerOriginalWorldPosition = AttackerWorldPos;
    AttackerOriginalHexPosition = WorldComponent->WorldToHex(AttackerWorldPos);

    // 도시 위치 저장
    CityHexPosition = CityHex;
    CityWorldPosition = WorldComponent->HexToWorld(CityHex);

    // ========== 3. 공격자 컴포넌트 초기화 ==========
    // 몽타주 델리게이트 바인딩 (공격자)
    // 주의: 기존 바인딩이 있을 수 있으므로 먼저 제거 후 추가
    if (USkeletalMeshComponent* MeshComp = Attacker->GetMesh())
    {
        if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
        {
            AnimInstance->OnMontageEnded.RemoveAll(this);
            AnimInstance->OnMontageEnded.AddDynamic(this, &UUnitVisualizationComponent::OnMontageEnded);
        }
    }

    // ========== 4. 초기 상태 설정 ==========
    CombatState = ECombatVisualizationState::RotatingToCity;
}

void UUnitVisualizationComponent::UpdateAttackerRotationToCity(float DeltaTime)
{
    AUnitCharacterBase* Attacker = Cast<AUnitCharacterBase>(GetOwner());
    if (!Attacker)
    {
        return;
    }

    // 도시 위치
    FVector CityLocation = CityWorldPosition;
    
    // 회전 처리
    RotateUnitToFaceTarget(Attacker, CityLocation, DeltaTime);

    // 회전 완료 체크
    FVector AttackerLocation = Attacker->GetActorLocation();
    FVector Direction = (CityLocation - AttackerLocation).GetSafeNormal();
    Direction.Z = 0.0f;
    Direction.Normalize();

    FRotator TargetRotation = Direction.Rotation();
    FRotator CurrentRotation = Attacker->GetActorRotation();
    float YawDifference = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetRotation.Yaw));

    if (YawDifference <= RotationTolerance)
    {
        Attacker->SetActorRotation(TargetRotation);
        // 회전 완료 - 공격 애니메이션 시작
        CombatState = ECombatVisualizationState::AttackingCity;
        PlayAttackerAttackMontageAgainstCity();
    }
}

void UUnitVisualizationComponent::PlayAttackerAttackMontageAgainstCity()
{
    AUnitCharacterBase* Attacker = Cast<AUnitCharacterBase>(GetOwner());
    if (!Attacker)
    {
        return;
    }

    // UnitData에서 AttackMontage 가져오기
    UAnimMontage* MontageToPlay = nullptr;
    if (const FUnitData* UnitData = Attacker->GetUnitData())
    {
        MontageToPlay = UnitData->AttackMontage;
    }
    else
    {
        MontageToPlay = AttackMontage;
    }

    if (!MontageToPlay)
    {
        // 몽타주가 없으면 바로 복귀 시작
        StartReturningFromCity();
        return;
    }

    PlayMontage(MontageToPlay, Attacker);
}

void UUnitVisualizationComponent::StartReturningFromCity()
{
    AUnitCharacterBase* OwnerUnit = Cast<AUnitCharacterBase>(GetOwner());
    if (!OwnerUnit || !WorldComponent)
    {
        return;
    }

    // 공격자인지 확인
    if (!CombatAttacker.IsValid() || OwnerUnit != CombatAttacker.Get())
    {
        return;
    }

    // 원래 위치의 월드 좌표 가져오기
    FVector OriginWorldPosition = AttackerOriginalWorldPosition;

    // 복귀 상태 설정 및 전투 시각화 전용 이동 시작
    CombatState = ECombatVisualizationState::ReturningFromCity;
    StartCombatMovement(OriginWorldPosition);
}

void UUnitVisualizationComponent::CompleteCityCombatVisualization()
{
    AUnitCharacterBase* Attacker = Cast<AUnitCharacterBase>(GetOwner());
    if (!Attacker || !CombatAttacker.IsValid() || Attacker != CombatAttacker.Get())
    {
        return;
    }

    // UnitManager에 전투 완료 알림
    if (UnitManager && CombatAttacker.IsValid())
    {
        FVector2D AttackerHex = AttackerOriginalHexPosition;
        FVector2D CityHex = CityHexPosition;
        
        // 도시는 유닛이 아니므로 nullptr 전달
        UnitManager->OnCombatVisualizationComplete(CombatAttacker.Get(), nullptr, CurrentCombatResult, AttackerHex, CityHex);
    }

    // 공격자 자신 초기화
    StopCombatVisualization();
}
