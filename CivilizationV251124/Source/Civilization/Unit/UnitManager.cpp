// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitManager.h"
#include "UnitCharacterBase.h"
#include "../Status/UnitStatusComponent.h"
#include "../World/WorldComponent.h"
#include "../SuperGameInstance.h"
#include "../SuperPlayerState.h"
#include "../AICon/UnitAIController.h"
#include "Engine/Engine.h"

UUnitManager::UUnitManager()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UUnitManager::BeginPlay()
{
    Super::BeginPlay();
}

void UUnitManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

AUnitCharacterBase* UUnitManager::SpawnUnitAtHex(FVector2D HexPosition, const FName& RowName, int32 PlayerIndex)
{
    // WorldComponent가 설정되어 있는지 확인
    if (!WorldComponent)
    {
        return nullptr;
    }

    // 타일 존재 확인
    UWorldTile* TargetTile = WorldComponent->GetTileAtHex(HexPosition);
    if (!TargetTile)
    {
        return nullptr;
    }

    // 유닛 배치 가능 여부 확인
    if (!CanPlaceUnitAtHex(HexPosition))
    {
        return nullptr;
    }

    // 월드 좌표 계산
    FVector WorldPosition = WorldComponent->HexToWorld(HexPosition);
    
    // 타일의 지형 타입에 따라 Z축 높이 설정
    if (TargetTile)
    {
        switch (TargetTile->GetLandType())
        {
        case ELandType::Plains:
            WorldPosition.Z = 183.0f; // 평지
            break;
        case ELandType::Hills:
            WorldPosition.Z = 256.0f; // 언덕
            break;
        case ELandType::Mountains:
            WorldPosition.Z = 329.0f; // 산
            break;
        default:
            WorldPosition.Z = 183.0f; // 기본값 (평지)
            break;
        }
    }
    else
    {
        WorldPosition.Z = 183.0f; // 타일이 없으면 기본값
    }
    
    // 유효한 World 가져오기
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // 유닛 생성
    AUnitCharacterBase* NewUnit = World->SpawnActor<AUnitCharacterBase>(
        AUnitCharacterBase::StaticClass(),
        WorldPosition,
        FRotator::ZeroRotator
    );

    if (!NewUnit)
    {
        return nullptr;
    }

    // 유닛 초기화
    NewUnit->InitializeUnit(RowName);

    // 플레이어 인덱스 설정
    NewUnit->SetPlayerIndex(PlayerIndex);

    // 유닛 위치 등록
    SetUnitAtHex(HexPosition, NewUnit);

    // 소환된 유닛 목록에 추가
    SpawnedUnits.Add(NewUnit);

    // 플레이어 스테이트에 유닛 추가 (PlayerIndex가 유효한 경우)
    if (PlayerIndex >= 0)
    {
        if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            if (ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(PlayerIndex))
            {
                PlayerState->AddOwnedUnit(NewUnit);
            }
        }
    }

    return NewUnit;
}

TArray<AUnitCharacterBase*> UUnitManager::GetAllUnits() const
{
    return SpawnedUnits;
}

void UUnitManager::RemoveUnit(AUnitCharacterBase* Unit)
{
    if (Unit && SpawnedUnits.Contains(Unit))
    {
        // 플레이어 스테이트에서 유닛 제거
        int32 UnitPlayerIndex = Unit->GetPlayerIndex();
        if (UnitPlayerIndex >= 0)
        {
            if (UWorld* World = GetWorld())
            {
                if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
                {
                    if (ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(UnitPlayerIndex))
                    {
                        PlayerState->RemoveOwnedUnit(Unit);
                    }
                }
            }
        }

        // 유닛의 현재 위치를 찾아서 제거
        if (WorldComponent)
        {
            // 유닛의 월드 위치를 육각형 좌표로 변환
            FVector2D HexPos = WorldComponent->WorldToHex(Unit->GetActorLocation());
            RemoveUnitFromHex(HexPos);
        }
        
        // 행동 중인 유닛 목록에서도 제거
        ActingUnits.Remove(Unit);
        
        SpawnedUnits.Remove(Unit);
        Unit->Destroy();
    }
}

void UUnitManager::ClearAllUnits()
{
    // 모든 플레이어 스테이트에서 유닛 제거
    if (UWorld* World = GetWorld())
    {
        if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            for (AUnitCharacterBase* Unit : SpawnedUnits)
            {
                if (Unit)
                {
                    int32 UnitPlayerIndex = Unit->GetPlayerIndex();
                    if (UnitPlayerIndex >= 0)
                    {
                        if (ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(UnitPlayerIndex))
                        {
                            PlayerState->RemoveOwnedUnit(Unit);
                        }
                    }
                }
            }
        }
    }

    // 모든 유닛 위치 제거
    if (WorldComponent)
    {
        for (AUnitCharacterBase* Unit : SpawnedUnits)
        {
            if (Unit)
            {
                // 유닛의 월드 위치를 육각형 좌표로 변환
                FVector2D HexPos = WorldComponent->WorldToHex(Unit->GetActorLocation());
                RemoveUnitFromHex(HexPos);
            }
        }
    }
    
    for (AUnitCharacterBase* Unit : SpawnedUnits)
    {
        if (Unit)
        {
            Unit->Destroy();
        }
    }
    SpawnedUnits.Empty();
    HexToUnitMap.Empty();
    ActingUnits.Empty(); // 행동 중인 유닛 목록도 초기화
}

void UUnitManager::SetWorldComponent(UWorldComponent* InWorldComponent)
{
    WorldComponent = InWorldComponent;
}

// 유닛 위치 관리 함수들 구현
AUnitCharacterBase* UUnitManager::GetUnitAtHex(FVector2D HexPosition) const
{
    if (AUnitCharacterBase* const* UnitPtr = HexToUnitMap.Find(HexPosition))
    {
        return *UnitPtr;
    }
    return nullptr;
}

bool UUnitManager::SetUnitAtHex(FVector2D HexPosition, AUnitCharacterBase* Unit)
{
    if (!WorldComponent || !WorldComponent->IsValidHexPosition(HexPosition))
    {
        return false;
    }
    
    // 해당 위치에 이미 유닛이 있는지 확인
    if (GetUnitAtHex(HexPosition) != nullptr)
    {
        return false; // 이미 유닛이 있음
    }
    
    // 타일이 존재하고 이동 가능한지 확인
    UWorldTile* Tile = WorldComponent->GetTileAtHex(HexPosition);
    if (!Tile || !Tile->IsPassable())
    {
        return false; // 이동 불가능한 타일
    }
    
    HexToUnitMap.Add(HexPosition, Unit);
    return true;
}

void UUnitManager::RemoveUnitFromHex(FVector2D HexPosition)
{
    HexToUnitMap.Remove(HexPosition);
}

bool UUnitManager::CanPlaceUnitAtHex(FVector2D HexPosition) const
{
    if (!WorldComponent || !WorldComponent->IsValidHexPosition(HexPosition))
    {
        return false;
    }
    
    // 이미 유닛이 있는지 확인
    if (GetUnitAtHex(HexPosition) != nullptr)
    {
        return false;
    }
    
    // 타일이 존재하고 이동 가능한지 확인
    UWorldTile* Tile = WorldComponent->GetTileAtHex(HexPosition);
    if (!Tile || !Tile->IsPassable())
    {
        return false;
    }
    
    return true;
}

// 유닛 선택 및 이동 시스템 구현
void UUnitManager::HandleTwoTileClick(UWorldTile* ClickedTile)
{
    if (!ClickedTile || !WorldComponent)
    {
        return;
    }
    
    FVector2D HexPos = ClickedTile->GetGridPosition();
    
    // 첫 번째 선택이 없으면 첫 번째로 설정
    if (!HasFirstSelection())
    {
        // 해당 타일에 유닛이 있는지 확인
        AUnitCharacterBase* UnitAtTile = GetUnitAtHex(HexPos);
        if (!UnitAtTile)
        {
            return; // 유닛이 없으면 선택하지 않음
        }
        
        FirstSelectedTile = ClickedTile;
        FirstSelectedTile->SetSelected(true);
    }
    // 첫 번째 선택이 있고 두 번째 선택이 없으면 두 번째로 설정
    else if (!HasSecondSelection())
    {
        SecondSelectedTile = ClickedTile;
        SecondSelectedTile->SetSelected(true);
        
        // 유닛 이동 실행
        MoveUnitFromFirstToSecondSelection();
        
        // 선택 초기화는 이동 완료 후 CompleteMovement()에서 처리
    }
}

void UUnitManager::ClearSelection()
{
    // 기존 선택 해제
    if (FirstSelectedTile)
    {
        FirstSelectedTile->SetSelected(false);
        FirstSelectedTile = nullptr;
    }
    
    if (SecondSelectedTile)
    {
        SecondSelectedTile->SetSelected(false);
        SecondSelectedTile = nullptr;
    }
}

void UUnitManager::MoveUnitFromFirstToSecondSelection()
{
    if (!FirstSelectedTile || !SecondSelectedTile || !WorldComponent)
    {
        ClearSelection(); // 선택 초기화
        return;
    }
    
    FVector2D FirstHexPos = FirstSelectedTile->GetGridPosition();
    FVector2D SecondHexPos = SecondSelectedTile->GetGridPosition();
    
    // 첫 번째 타일의 유닛 가져오기
    AUnitCharacterBase* UnitToMove = GetUnitAtHex(FirstHexPos);
    if (!UnitToMove)
    {
        ClearSelection(); // 선택 초기화
        return;
    }
    
    // 유닛이 이동 가능한 상태인지 확인 (이미 공격했거나 이동력이 없는 경우)
    if (UUnitStatusComponent* StatusComp = UnitToMove->GetUnitStatusComponent())
    {
        if (!StatusComp->CanMove())
        {
            ClearSelection(); // 선택 초기화
            return; // 이동할 수 없는 상태
        }
    }
    
    // 두 번째 타일이 이동 가능한지 확인
    if (!CanPlaceUnitAtHex(SecondHexPos))
    {
        ClearSelection(); // 선택 초기화
        return;
    }
    
    // A* 경로 찾기로 최적 경로 계산
    TArray<FVector2D> Path = FindPath(FirstHexPos, SecondHexPos);
    
    // 경로가 없거나 유효하지 않으면 이동하지 않음
    if (Path.Num() <= 1)
    {
        ClearSelection(); // 선택 초기화
        return;
    }
    
    // 유닛의 남은 이동력 확인 (유닛별 이동력 사용)
    int32 MaxMovementCost = GetUnitRemainingMovement(UnitToMove);
    
    // 이동력 제한이 있는 경로 찾기
    TArray<FVector2D> LimitedPath = FindPathWithMovementCost(FirstHexPos, SecondHexPos, MaxMovementCost);
    
    // 제한된 경로가 없으면 이동하지 않음
    if (LimitedPath.Num() <= 1)
    {
        ClearSelection(); // 선택 초기화
        return;
    }
    
    // 경로를 따라 유닛 이동 (시각적 애니메이션)
    StartVisualMovement(UnitToMove, LimitedPath);
    // 선택 초기화는 이동 완료 후 CompleteMovement()에서 처리
}

void UUnitManager::MoveUnitAlongPath(AUnitCharacterBase* Unit, const TArray<FVector2D>& Path)
{
    if (!Unit || Path.Num() <= 1)
    {
        return;
    }
    
    // 시작점에서 유닛 제거
    FVector2D StartHex = Path[0];
    RemoveUnitFromHex(StartHex);
    
    // 경로의 마지막 지점으로 유닛 이동
    FVector2D EndHex = Path[Path.Num() - 1];
    
    // 목표 위치에 유닛 배치
    SetUnitAtHex(EndHex, Unit);
    
    // 유닛의 월드 위치 업데이트
    FVector NewWorldPosition = WorldComponent->HexToWorld(EndHex);
    
    // 타일의 지형 타입에 따라 Z축 높이 설정
    UWorldTile* TargetTile = WorldComponent->GetTileAtHex(EndHex);
    if (TargetTile)
    {
        switch (TargetTile->GetLandType())
        {
        case ELandType::Plains:
            NewWorldPosition.Z = 183.0f; // 평지
            break;
        case ELandType::Hills:
            NewWorldPosition.Z = 256.0f; // 언덕
            break;
        case ELandType::Mountains:
            NewWorldPosition.Z = 329.0f; // 산
            break;
        default:
            NewWorldPosition.Z = 183.0f; // 기본값 (평지)
            break;
        }
    }
    
    // 유닛의 실제 월드 위치 변경
    Unit->SetActorLocation(NewWorldPosition);
}

// 경로 찾기 및 이동 시스템 구현
TArray<FVector2D> UUnitManager::FindPath(FVector2D StartHex, FVector2D EndHex) const
{
    TArray<FVector2D> Path;

    // 시작점과 끝점이 같으면 시작점만 반환
    if (StartHex == EndHex)
    {
        Path.Add(StartHex);
        return Path;
    }

    // 유효하지 않은 좌표 확인
    if (!WorldComponent || !WorldComponent->IsValidHexPosition(StartHex) || !WorldComponent->IsValidHexPosition(EndHex))
    {
        return Path; // 빈 경로 반환
    }

    // 목표점에 도달할 수 없는지 확인
    if (!CanMoveToHex(EndHex))
    {
        return Path; // 빈 경로 반환
    }

    // A* 알고리즘 구현
    TMap<FVector2D, FAStarNode> OpenSet; // 오픈 리스트 (우선순위 큐 대신 맵 사용)
    TMap<FVector2D, FAStarNode> ClosedSet; // 클로즈드 리스트
    TMap<FVector2D, FAStarNode> CameFrom; // 경로 추적용

    // 시작 노드 초기화
    int32 StartHeuristic = CalculateHeuristic(StartHex, EndHex);
    FAStarNode StartNode(StartHex, 0, StartHeuristic, StartHex, true); // ParentHex를 StartHex로 설정 (시작점 표시)
    OpenSet.Add(StartHex, StartNode);
    CameFrom.Add(StartHex, StartNode); // 시작 노드를 CameFrom에 추가

    int32 IterationCount = 0;
    while (OpenSet.Num() > 0)
    {
        // 오픈 리스트에서 F값이 가장 작은 노드 찾기
        FVector2D CurrentHex = FVector2D::ZeroVector;
        int32 LowestFCost = INT32_MAX;

        for (const auto& Pair : OpenSet)
        {
            if (Pair.Value.FCost < LowestFCost)
            {
                LowestFCost = Pair.Value.FCost;
                CurrentHex = Pair.Key;
            }
        }

        // 현재 노드를 클로즈드 리스트로 이동
        FAStarNode CurrentNode = OpenSet[CurrentHex];
        OpenSet.Remove(CurrentHex);
        ClosedSet.Add(CurrentHex, CurrentNode);

        // 목표에 도달했는지 확인
        if (CurrentHex == EndHex)
        {
            Path = ReconstructPath(CameFrom, CurrentHex);
            break;
        }

        // 인접한 육각형들 확인
        TArray<FVector2D> Neighbors = WorldComponent->GetHexNeighbors(CurrentHex);

        for (const FVector2D& NeighborHex : Neighbors)
        {
            // 유효하지 않은 위치이거나 층수 이동이 불가능한 곳은 건너뛰기
            if (!WorldComponent->IsValidHexPosition(NeighborHex) || !CanMoveBetweenHexes(CurrentHex, NeighborHex))
            {
                continue;
            }

            // 이미 클로즈드 리스트에 있으면 건너뛰기
            if (ClosedSet.Contains(NeighborHex))
            {
                continue;
            }

            // 현재 노드에서 이웃까지의 이동 비용 계산
            int32 MovementCost = GetMovementCostBetweenHexes(CurrentHex, NeighborHex);
            int32 TentativeGCost = CurrentNode.GCost + MovementCost;

            // 이웃이 오픈 리스트에 있는지 확인
            bool bInOpenSet = OpenSet.Contains(NeighborHex);

            // 더 나은 경로를 찾았거나 이웃이 오픈 리스트에 없으면
            if (!bInOpenSet || TentativeGCost < OpenSet[NeighborHex].GCost)
            {
                // 이웃 노드 생성
                int32 Heuristic = CalculateHeuristic(NeighborHex, EndHex);
                FAStarNode NeighborNode(NeighborHex, TentativeGCost, Heuristic, CurrentHex, true);

                // 오픈 리스트에 추가 또는 업데이트
                OpenSet.Add(NeighborHex, NeighborNode);
                CameFrom.Add(NeighborHex, NeighborNode);
            }
        }

        IterationCount++;

        // 너무 많은 반복을 방지 (안전장치)
        if (IterationCount > 10000)
        {
            break;
        }
    }

    return Path;
}

TArray<FVector2D> UUnitManager::FindPathWithMovementCost(FVector2D StartHex, FVector2D EndHex, int32 MaxMovementCost) const
{
    // A* 알고리즘으로 경로를 찾은 후, 최대 이동 비용을 초과하는지 확인
    TArray<FVector2D> Path = FindPath(StartHex, EndHex);

    if (Path.Num() <= 1)
    {
        return Path; // 경로가 없거나 시작점만 있는 경우
    }

    // 경로의 총 이동 비용 계산
    int32 TotalMovementCost = 0;
    for (int32 i = 0; i < Path.Num() - 1; i++)
    {
        TotalMovementCost += GetMovementCostBetweenHexes(Path[i], Path[i + 1]);

        // 최대 이동 비용을 초과하면 여기서 경로를 잘라냄
        if (TotalMovementCost > MaxMovementCost)
        {
            Path.SetNum(i + 1); // 현재까지의 경로만 유지
            break;
        }
    }

    return Path;
}

bool UUnitManager::CanMoveToHex(FVector2D HexPosition) const
{
    if (!WorldComponent)
    {
        return false;
    }
    
    UWorldTile* Tile = WorldComponent->GetTileAtHex(HexPosition);
    if (!Tile)
    {
        return false;
    }

    return Tile->IsPassable();
}

int32 UUnitManager::CalculateHeuristic(FVector2D StartHex, FVector2D EndHex) const
{
    if (!WorldComponent)
    {
        return 0;
    }
    
    // 육각형 거리를 휴리스틱으로 사용 (일반적으로 1의 이동 비용을 가정)
    int32 HexDistance = WorldComponent->GetHexDistance(StartHex, EndHex);
    return HexDistance;
}

TArray<FVector2D> UUnitManager::ReconstructPath(const TMap<FVector2D, FAStarNode>& CameFrom, FVector2D Current) const
{
    TArray<FVector2D> Path;

    // 시작점부터 목표점까지의 경로 재구성
    TArray<FVector2D> ReversedPath;
    ReversedPath.Add(Current);

    // 목표점에서 시작점까지 역순으로 경로 수집
    while (CameFrom.Contains(Current))
    {
        const FAStarNode& CurrentNode = CameFrom[Current];
        FVector2D ParentHex = CurrentNode.ParentHex;
        
        // 시작점에 도달했는지 확인 (부모가 자기 자신인 경우가 시작점)
        if (ParentHex == Current)
        {
            // 시작점에 도달했으므로 종료
            break;
        }
        
        ReversedPath.Add(ParentHex);
        Current = ParentHex;
    }

    // 역순으로 수집된 경로를 올바른 순서로 뒤집기
    for (int32 i = ReversedPath.Num() - 1; i >= 0; i--)
    {
        Path.Add(ReversedPath[i]);
    }

    return Path;
}

int32 UUnitManager::GetMovementCostBetweenHexes(FVector2D FromHex, FVector2D ToHex) const
{
    // 층수 이동 가능성 먼저 체크
    if (!CanMoveBetweenHexes(FromHex, ToHex))
    {
        return INT32_MAX;
    }
    
    // 층수를 고려한 이동비용 계산
    return GetMovementCostBetweenHexesWithFloor(FromHex, ToHex);
}

// 층수 시스템 관련 함수들 구현
int32 UUnitManager::GetFloorLevel(ELandType LandType) const
{
    switch (LandType)
    {
    case ELandType::Plains:
        return 1; // 1층
    case ELandType::Hills:
        return 2; // 2층
    case ELandType::Mountains:
        return 3; // 3층
    default:
        return 1; // 기본값은 1층
    }
}

bool UUnitManager::CanMoveBetweenHexes(FVector2D FromHex, FVector2D ToHex) const
{
    if (!WorldComponent)
    {
        return false;
    }
    
    UWorldTile* FromTile = WorldComponent->GetTileAtHex(FromHex);
    UWorldTile* ToTile = WorldComponent->GetTileAtHex(ToHex);
    
    if (!FromTile || !ToTile)
    {
        return false; // 타일이 없으면 이동 불가
    }
    
    // 바다로는 이동 불가
    if (ToTile->GetTerrainType() == ETerrainType::Ocean)
    {
        return false;
    }
    
    // 바다에서 출발도 불가
    if (FromTile->GetTerrainType() == ETerrainType::Ocean)
    {
        return false;
    }
    
    // 층수 차이 계산
    int32 FromFloor = GetFloorLevel(FromTile->GetLandType());
    int32 ToFloor = GetFloorLevel(ToTile->GetLandType());
    int32 FloorDifference = FMath::Abs(ToFloor - FromFloor);
    
    // 2층 이상 차이나면 이동 불가
    if (FloorDifference >= 2)
    {
        return false;
    }
    
    return true;
}

int32 UUnitManager::GetMovementCostBetweenHexesWithFloor(FVector2D FromHex, FVector2D ToHex) const
{
    if (!WorldComponent)
    {
        return INT32_MAX;
    }
    
    UWorldTile* FromTile = WorldComponent->GetTileAtHex(FromHex);
    UWorldTile* ToTile = WorldComponent->GetTileAtHex(ToHex);
    
    if (!FromTile || !ToTile)
    {
        return INT32_MAX; // 타일이 없으면 이동 불가
    }
    
    // 기본 이동 비용 (도착 타일의 총 이동 비용)
    int32 BaseMovementCost = ToTile->GetTotalMovementCost();
    
    // 층수 차이 계산
    int32 FromFloor = GetFloorLevel(FromTile->GetLandType());
    int32 ToFloor = GetFloorLevel(ToTile->GetLandType());
    int32 FloorDifference = ToFloor - FromFloor;
    
    int32 TotalMovementCost = BaseMovementCost;
    
    if (FloorDifference > 0)
    {
        // 올라가는 경우: 기본비용 + 층수차이
        TotalMovementCost += FloorDifference;
    }
    else if (FloorDifference < 0)
    {
        // 내려가는 경우: 기본비용만 (추가 비용 없음)
        TotalMovementCost = BaseMovementCost;
    }
    else
    {
        // 같은 층: 기본비용만
        TotalMovementCost = BaseMovementCost;
    }
    return TotalMovementCost;
}

// 유닛 이동력 관리 함수들 구현
int32 UUnitManager::GetUnitRemainingMovement(AUnitCharacterBase* Unit) const
{
    if (!Unit)
    {
        return 0;
    }
    
    // UnitStatusComponent에서 남은 이동력을 가져옴
    if (UUnitStatusComponent* StatusComp = Unit->GetUnitStatusComponent())
    {
        // CurrentStat의 RemainingMovementPoints는 이번 턴에 사용 가능한 남은 이동력
        // 경로 탐색 시 실제로 사용할 수 있는 이동력으로 제한해야 함
        return StatusComp->GetCurrentStat().RemainingMovementPoints;
    }
    
    // 컴포넌트가 없으면 기본값 반환
    return 0;
}

// 건설자 판단 함수 구현
bool UUnitManager::IsBuilderUnit(AUnitCharacterBase* Unit) const
{
    if (!Unit)
    {
        return false;
    }

    // UnitStatusComponent 가져오기
    UUnitStatusComponent* StatusComp = Unit->GetUnitStatusComponent();
    if (!StatusComp)
    {
        return false;
    }

    // BaseStat에서 CanBuildFacilities 확인
    FUnitBaseStat BaseStat = StatusComp->GetBaseStat();
    return BaseStat.CanBuildFacilities;
}

// 시각적 이동 애니메이션 함수들 구현
void UUnitManager::StartVisualMovement(AUnitCharacterBase* Unit, const TArray<FVector2D>& Path)
{
    if (!Unit || Path.Num() <= 1)
    {
        return;
    }
    
    // 전체 경로의 이동 비용 계산 및 소비
    // FindPathWithMovementCost()에서 이미 이동력 제한으로 경로를 잘랐으므로,
    // 이 경로는 유닛이 갈 수 있는 최대 거리임
    if (UUnitStatusComponent* StatusComp = Unit->GetUnitStatusComponent())
    {
        int32 TotalCost = 0;
        
        // 경로의 실제 이동 비용 계산 (타일별 비용 합산)
        // 경로는 이미 검증되었으므로 INT32_MAX 체크 불필요
        for (int32 i = 0; i < Path.Num() - 1; i++)
        {
            int32 TileCost = GetMovementCostBetweenHexes(Path[i], Path[i + 1]);
            TotalCost += TileCost;
        }
        
        // 실제 이동력 소비 (이번 턴의 남은 이동력에서 차감)
        StatusComp->ConsumeMovement(TotalCost);
    }
    
    // 시작점에서 유닛 제거
    FVector2D StartHex = Path[0];
    RemoveUnitFromHex(StartHex);
    
    // 최종 위치 계산
    FVector2D FinalHex = Path[Path.Num() - 1];
    
    // 최종 위치에 유닛 등록 (바로 다음!)
    SetUnitAtHex(FinalHex, Unit);
    
    // 행동 중인 유닛 목록에 추가
    if (!ActingUnits.Contains(Unit))
    {
        ActingUnits.Add(Unit);
    }
    
    // AIController에 이동 명령 전달
    if (AUnitAIController* AIController = Cast<AUnitAIController>(Unit->GetController()))
    {
        // WorldComponent 설정 (없으면)
        if (!AIController->GetWorldComponent())
        {
            AIController->SetWorldComponent(WorldComponent);
        }
        
        // UnitManager 설정
        AIController->SetUnitManager(this);
        
        // 경로를 따라 이동 시작
        AIController->StartMovementAlongPath(Path);
    }
}

void UUnitManager::CompleteMovement()
{
    // 선택 초기화
    ClearSelection();
}

void UUnitManager::OnUnitMovementComplete(AUnitCharacterBase* Unit, FVector2D FinalHex)
{
    if (!Unit || !WorldComponent)
    {
        return;
    }
    
    // 최종 위치에 유닛이 이미 등록되어 있는지 확인
    // (StartVisualMovement()에서 이미 등록했으므로 확인만)
    AUnitCharacterBase* UnitAtFinalHex = GetUnitAtHex(FinalHex);
    if (UnitAtFinalHex != Unit)
    {
        // 만약 다른 유닛이 있다면 다시 등록 시도
        SetUnitAtHex(FinalHex, Unit);
    }
    // 이미 등록되어 있으면 그대로 진행
    
    // 행동 중인 유닛 목록에서 제거
    ActingUnits.Remove(Unit);
    
    // 선택 초기화
    ClearSelection();
}

bool UUnitManager::IsAnyUnitActing() const
{
    // 행동 중인 유닛 목록이 비어있지 않으면 행동 중
    return ActingUnits.Num() > 0;
}
