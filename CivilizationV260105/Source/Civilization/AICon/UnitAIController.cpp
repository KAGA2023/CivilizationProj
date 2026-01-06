// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Unit/UnitCharacterBase.h"
#include "../World/WorldComponent.h"
#include "../Unit/UnitManager.h"
#include "../SuperGameInstance.h"
#include "../Combat/UnitCombatStruct.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

AUnitAIController::AUnitAIController()
{
    // BehaviorTree와 Blackboard 자동 로드
    static ConstructorHelpers::FObjectFinder<UBehaviorTree>
        Tree(TEXT("/Game/Civilization/AI/BT_Unit.BT_Unit"));
    if (Tree.Succeeded())
    {
        BTData = Tree.Object;
    }

    static ConstructorHelpers::FObjectFinder<UBlackboardData>
        Data(TEXT("/Game/Civilization/AI/BB_Unit.BB_Unit"));
    if (Data.Succeeded())
    {
        BBData = Data.Object;
    }
}

void AUnitAIController::BeginPlay()
{
    Super::BeginPlay();

    // WorldComponent 자동 찾기
    if (!WorldComponent)
    {
        if (UWorld* World = GetWorld())
        {
            if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
            {
                WorldComponent = GameInstance->GetGeneratedWorldComponent();
            }
        }
    }
}

void AUnitAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Blackboard와 BehaviorTree 초기화
    InitBlackboardAndBT();

    // WorldComponent가 없으면 다시 찾기
    if (!WorldComponent)
    {
        if (UWorld* World = GetWorld())
        {
            if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
            {
                WorldComponent = GameInstance->GetGeneratedWorldComponent();
            }
        }
    }
}

void AUnitAIController::InitBlackboardAndBT()
{
    if (!ensure(BBData && BTData))
    {
        return;
    }

    UBlackboardComponent* OutBB = nullptr;
    UseBlackboard(BBData, OutBB);
    RunBehaviorTree(BTData);

    // Blackboard 초기값 설정
    if (Blackboard)
    {
        // 이동 관련 초기값
        Blackboard->SetValueAsBool(KeyIsMoving, false);
        Blackboard->SetValueAsBool(KeyMovementComplete, false);
        Blackboard->SetValueAsVector(KeyTargetHexPosition, FVector::ZeroVector);
        
        // 전투 관련 초기값
        Blackboard->SetValueAsBool(KeyIsInCombat, false);
        Blackboard->SetValueAsBool(KeyCombatComplete, false);
        Blackboard->SetValueAsBool(KeyIsRangedAttack, false);
        
        // 전투 시각화 관련 초기값
        Blackboard->SetValueAsBool(KeyIsAttacker, false);
        Blackboard->SetValueAsBool(KeyDefenderRotateReady, false);
        Blackboard->SetValueAsBool(KeyAttackerMoveReady, false);
        Blackboard->SetValueAsBool(KeyAttackerAnimationStarted, false);
        Blackboard->SetValueAsBool(KeyAttackerAnimationFinished, false);
        Blackboard->SetValueAsBool(KeyDefenderHitFinished, false);
        Blackboard->SetValueAsBool(KeyCanCounterAttack, false);
        Blackboard->SetValueAsBool(KeyDefenderAnimationStarted, false);
        Blackboard->SetValueAsBool(KeyDefenderAnimationFinished, false);
        Blackboard->SetValueAsBool(KeyAttackerHitFinished, false);
    }
}

void AUnitAIController::SetWorldComponent(UWorldComponent* InWorldComponent)
{
    WorldComponent = InWorldComponent;
}

void AUnitAIController::SetUnitManager(UUnitManager* InUnitManager)
{
    UnitManager = InUnitManager;
}

void AUnitAIController::StartMovementAlongPath(const TArray<FVector2D>& Path)
{
    if (Path.Num() <= 1)
    {
        return;
    }

    // 내부 변수에 경로 저장
    CurrentMovementPath = Path;
    CurrentPathIndex = 1; // 첫 번째는 시작점이므로 1부터

    // Blackboard에 다음 목표 헥스 설정
    if (Blackboard && CurrentMovementPath.Num() > 1)
    {
        FVector2D NextHex = CurrentMovementPath[CurrentPathIndex];
        Blackboard->SetValueAsVector(KeyTargetHexPosition, FVector(NextHex.X, NextHex.Y, 0));
        Blackboard->SetValueAsBool(KeyIsMoving, true);
        Blackboard->SetValueAsBool(KeyMovementComplete, false);
    }
}

void AUnitAIController::MoveToNextHexInPath()
{
    // 인덱스 증가 (현재 헥스에 도착했으므로 다음 헥스로)
    CurrentPathIndex++;
    
    // 경로가 끝났는지 확인
    if (CurrentPathIndex >= CurrentMovementPath.Num())
    {
        // CompleteMovement는 호출하지 않음 (Task에서 처리)
        return;
    }

    // 다음 헥스 좌표 가져오기
    FVector2D NextHex = CurrentMovementPath[CurrentPathIndex];

    // Blackboard에 목표 설정
    if (Blackboard)
    {
        Blackboard->SetValueAsVector(KeyTargetHexPosition, FVector(NextHex.X, NextHex.Y, 0));
    }
}

void AUnitAIController::CompleteMovement()
{
    // Blackboard 업데이트
    if (Blackboard)
    {
        Blackboard->SetValueAsBool(KeyIsMoving, false);
        Blackboard->SetValueAsBool(KeyMovementComplete, true);
    }

    // UnitManager에 이동 완료 알림
    AUnitCharacterBase* Unit = Cast<AUnitCharacterBase>(GetPawn());
    if (Unit && UnitManager && CurrentMovementPath.Num() > 0)
    {
        FVector2D FinalHex = CurrentMovementPath[CurrentMovementPath.Num() - 1];
        UnitManager->OnUnitMovementComplete(Unit, FinalHex);
    }

    // 경로 초기화
    CurrentMovementPath.Empty();
    CurrentPathIndex = 0;
}

FVector2D AUnitAIController::GetCurrentHexPosition() const
{
    if (!WorldComponent)
    {
        return FVector2D::ZeroVector;
    }

    AUnitCharacterBase* Unit = Cast<AUnitCharacterBase>(GetPawn());
    if (!Unit)
    {
        return FVector2D::ZeroVector;
    }

    FVector WorldPosition = Unit->GetActorLocation();
    return WorldComponent->WorldToHex(WorldPosition);
}

FVector AUnitAIController::HexToWorld(FVector2D HexPosition) const
{
    if (!WorldComponent)
    {
        return FVector::ZeroVector;
    }

    return WorldComponent->HexToWorld(HexPosition);
}

FVector2D AUnitAIController::WorldToHex(FVector WorldPosition) const
{
    if (!WorldComponent)
    {
        return FVector2D::ZeroVector;
    }

    return WorldComponent->WorldToHex(WorldPosition);
}

void AUnitAIController::StartCombatVisualization(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender, const FCombatResult& CombatResult)
{
    // 유효성 검사
    if (!Attacker || !Defender)
    {
        return;
    }

    // 이미 전투 중이면 무시
    if (bIsInCombat)
    {
        return;
    }

    // ========== 사거리 정보 계산 ==========
    int32 CombatRange = 0;
    bool bIsRangedAttack = false;
    
    if (Attacker && Attacker->GetUnitStatusComponent())
    {
        // 기본 사거리 가져오기
        CombatRange = Attacker->GetUnitStatusComponent()->GetRange();
        
        // 원거리 공격 여부 판단 (Range > 1)
        bIsRangedAttack = (CombatRange > 1);
    }

    // 공격자 원래 위치 계산 (복귀용 - 근접 공격만 사용)
    FVector2D AttackerOriginalHexPosition = FVector2D::ZeroVector;
    if (WorldComponent && Attacker)
    {
        FVector AttackerWorldPos = Attacker->GetActorLocation();
        AttackerOriginalHexPosition = WorldComponent->WorldToHex(AttackerWorldPos);
    }

    // ========== 전투 시각화 데이터 구조체 초기화 ==========
    CurrentCombatData = FCombatVisualizationData(
        Attacker,
        Defender,
        CombatResult,
        AttackerOriginalHexPosition,
        CombatRange,
        bIsRangedAttack
    );

    bIsInCombat = true;

    // ========== 콜리전 이벤트 델리게이트 바인딩 ==========
    // 현재 Pawn이 공격자인지 방어자인지 확인
    AUnitCharacterBase* CurrentUnit = Cast<AUnitCharacterBase>(GetPawn());
    if (CurrentUnit)
    {
        // 피격 이벤트 델리게이트 바인딩
        CurrentUnit->OnHitCollisionReceived.AddDynamic(this, &AUnitAIController::OnHitCollisionReceived);
    }

    // Blackboard 업데이트
    if (Blackboard)
    {
        // 기본 전투 상태 설정
        Blackboard->SetValueAsBool(KeyIsInCombat, true);
        Blackboard->SetValueAsBool(KeyCombatComplete, false);
        
        // ========== 원거리 공격 여부 Blackboard에 저장 ==========
        Blackboard->SetValueAsBool(KeyIsRangedAttack, CurrentCombatData.bIsRangedAttack);
        
        // ========== 공격자/방어자 구분 설정 ==========
        // 현재 Pawn이 공격자인지 방어자인지 확인 (위에서 선언한 CurrentUnit 재사용)
        if (CurrentUnit)
        {
            bool bIsAttacker = (CurrentUnit == Attacker);
            Blackboard->SetValueAsBool(KeyIsAttacker, bIsAttacker);
        }
        
        // ========== 전투 시각화 관련 초기값 설정 ==========
        Blackboard->SetValueAsBool(KeyDefenderRotateReady, false);
        Blackboard->SetValueAsBool(KeyAttackerMoveReady, false);
        Blackboard->SetValueAsBool(KeyAttackerAnimationStarted, false);
        Blackboard->SetValueAsBool(KeyAttackerAnimationFinished, false);
        Blackboard->SetValueAsBool(KeyDefenderHitFinished, false);
        Blackboard->SetValueAsBool(KeyCanCounterAttack, false);
        Blackboard->SetValueAsBool(KeyDefenderAnimationStarted, false);
        Blackboard->SetValueAsBool(KeyDefenderAnimationFinished, false);
        Blackboard->SetValueAsBool(KeyAttackerHitFinished, false);
    }
}

void AUnitAIController::CompleteCombatVisualization()
{
    // UnitManager에 전투 완료 알림
    if (UnitManager && CurrentCombatData.Attacker && CurrentCombatData.Defender && WorldComponent)
    {
        // Hex 좌표 계산
        FVector2D AttackerHex = WorldComponent->WorldToHex(CurrentCombatData.Attacker->GetActorLocation());
        FVector2D DefenderHex = WorldComponent->WorldToHex(CurrentCombatData.Defender->GetActorLocation());
        
        // UnitManager에 완료 알림
        UnitManager->OnCombatVisualizationComplete(
            CurrentCombatData.Attacker, 
            CurrentCombatData.Defender, 
            CurrentCombatData.CombatResult, 
            AttackerHex, 
            DefenderHex
        );
    }

    // 전투 상태 초기화
    bIsInCombat = false;

    // Blackboard 업데이트
    if (Blackboard)
    {
        // 기본 전투 상태 초기화
        Blackboard->SetValueAsBool(KeyIsInCombat, false);
        Blackboard->SetValueAsBool(KeyCombatComplete, true);
        
        // ========== 원거리 공격 정보 초기화 ==========
        Blackboard->SetValueAsBool(KeyIsRangedAttack, false);
        
        // ========== 전투 시각화 관련 초기화 ==========
        Blackboard->SetValueAsBool(KeyIsAttacker, false);
        Blackboard->SetValueAsBool(KeyDefenderRotateReady, false);
        Blackboard->SetValueAsBool(KeyAttackerMoveReady, false);
        Blackboard->SetValueAsBool(KeyAttackerAnimationStarted, false);
        Blackboard->SetValueAsBool(KeyAttackerAnimationFinished, false);
        Blackboard->SetValueAsBool(KeyDefenderHitFinished, false);
        Blackboard->SetValueAsBool(KeyCanCounterAttack, false);
        Blackboard->SetValueAsBool(KeyDefenderAnimationStarted, false);
        Blackboard->SetValueAsBool(KeyDefenderAnimationFinished, false);
        Blackboard->SetValueAsBool(KeyAttackerHitFinished, false);
    }

    // ========== 콜리전 이벤트 델리게이트 언바인딩 ==========
    AUnitCharacterBase* CurrentUnit = Cast<AUnitCharacterBase>(GetPawn());
    if (CurrentUnit)
    {
        CurrentUnit->OnHitCollisionReceived.RemoveDynamic(this, &AUnitAIController::OnHitCollisionReceived);
    }

    // ========== 전투 시각화 데이터 구조체 초기화 ==========
    CurrentCombatData.Reset();
}

// ========== 콜리전 이벤트 핸들러 구현 ==========
void AUnitAIController::OnHitCollisionReceived()
{
    // 현재 Pawn 가져오기
    AUnitCharacterBase* CurrentUnit = Cast<AUnitCharacterBase>(GetPawn());
    if (!CurrentUnit)
    {
        return;
    }

    // 전투 중인지 확인
    if (!bIsInCombat)
    {
        return;
    }

    // 전투 데이터 확인 (현재 유닛이 전투에 참여 중인지)
    // 공격/반격/원거리 모든 상황을 하나의 함수에서 처리
    bool bIsValidCombat = false;
    
    // 1. 공격 상황: 현재 유닛이 방어자
    if (CurrentCombatData.Defender == CurrentUnit)
    {
        bIsValidCombat = true;
    }
    // 2. 반격 상황: 현재 유닛이 공격자 (반격 피격)
    else if (CurrentCombatData.Attacker == CurrentUnit)
    {
        bIsValidCombat = true;
    }
    
    if (!bIsValidCombat)
    {
        return;  // 전투에 참여 중이 아님
    }

    // 피격 애니메이션 재생 (모든 상황에서 동일)
    if (const FUnitData* UnitData = CurrentUnit->GetUnitData())
    {
        if (UAnimMontage* HitMontage = UnitData->HitMontage)
        {
            if (USkeletalMeshComponent* Mesh = CurrentUnit->GetMesh())
            {
                if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
                {
                    AnimInstance->Montage_Play(HitMontage, 1.0f);
                }
            }
        }
    }

    // Blackboard 업데이트는 애니메이션 완료 시 별도 Task에서 처리
    // (DefenderHitFinished 또는 AttackerHitFinished)
}


