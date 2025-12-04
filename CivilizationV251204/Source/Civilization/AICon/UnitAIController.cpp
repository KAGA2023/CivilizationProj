// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../Unit/UnitCharacterBase.h"
#include "../World/WorldComponent.h"
#include "../Unit/UnitManager.h"
#include "../SuperGameInstance.h"
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
        Blackboard->SetValueAsBool(KeyIsMoving, false);
        Blackboard->SetValueAsBool(KeyMovementComplete, false);
        Blackboard->SetValueAsVector(KeyTargetHexPosition, FVector::ZeroVector);
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


