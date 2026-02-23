// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitManager.h"
#include "UnitCharacterBase.h"
#include "UnitVisualizationComponent.h"
#include "../Status/UnitStatusComponent.h"
#include "../Status/UnitStatusStruct.h"
#include "../Combat/UnitCombatComponent.h"
#include "../Combat/UnitCombatStruct.h"
#include "../World/WorldComponent.h"
#include "../World/WorldTileActor.h"
#include "../World/WorldSpawner.h"
#include "../SuperGameInstance.h"
#include "../SuperPlayerState.h"
#include "../Facility/FacilityManager.h"
#include "../AICon/UnitAIController.h"
#include "../AIPlayer/AIPlayerManager.h"
#include "../City/CityComponent.h"
#include "../Widget/UnitWidget/SmallUnitUI.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

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

AUnitCharacterBase* UUnitManager::SpawnUnitAtHex(FVector2D HexPosition, const FName& RowName, int32 PlayerIndex, bool bSkipPlacementCheck)
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

    // 유닛 배치 가능 여부 확인 (로드 시에는 건너뛰기)
    if (!bSkipPlacementCheck && !CanPlaceUnitAtHex(HexPosition))
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

    // 유닛 생성 (Z축으로 180도 회전)
    AUnitCharacterBase* NewUnit = World->SpawnActor<AUnitCharacterBase>(
        AUnitCharacterBase::StaticClass(),
        WorldPosition,
        FRotator(0.0f, 180.0f, 0.0f)
    );

    if (!NewUnit)
    {
        return nullptr;
    }

    // 유닛 초기화
    NewUnit->InitializeUnit(RowName);

    // 플레이어 인덱스 설정
    NewUnit->SetPlayerIndex(PlayerIndex);

    // SmallUnitUI 설정
    if (USmallUnitUI* SmallUI = NewUnit->GetSmallUnitUI())
    {
        // HP 바 설정 (100%)
        SmallUI->SetHPBar(1.0f);

        // 유닛 이미지 설정
        if (UUnitStatusComponent* StatusComp = NewUnit->GetUnitStatusComponent())
        {
            FUnitBaseStat BaseStat = StatusComp->GetBaseStat();
            if (!BaseStat.UnitIcon.IsNull())
            {
                UTexture2D* UnitTexture = BaseStat.UnitIcon.LoadSynchronous();
                SmallUI->SetUnitImg(UnitTexture);
            }
        }

        // 국가 이미지 설정 (PlayerIndex가 유효한 경우)
        if (PlayerIndex >= 0)
        {
            if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
            {
                if (ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(PlayerIndex))
                {
                    if (!PlayerState->CountryLargeImg.IsNull())
                    {
                        UTexture2D* CountryTexture = PlayerState->CountryLargeImg.LoadSynchronous();
                        SmallUI->SetCountryImg(CountryTexture);
                    }
                }
            }
        }
    }

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

FVector2D UUnitManager::FindSpawnLocationNearCity(FVector2D CityHex) const
{
    // 기본 검증
    if (!WorldComponent || !WorldComponent->IsValidHexPosition(CityHex))
    {
        return FVector2D(-1, -1);
    }
    
    // 도시 주변 1칸 범위의 인접 타일 가져오기
    TArray<FVector2D> NeighborHexes = WorldComponent->GetHexNeighbors(CityHex);
    
    // 유효한 소환 위치 필터링
    TArray<FVector2D> ValidLocations;
    
    for (const FVector2D& Hex : NeighborHexes)
    {
        // 조건 1: 유효한 좌표인지 확인
        if (!WorldComponent->IsValidHexPosition(Hex))
        {
            continue;
        }
        
        // 조건 2: 타일이 존재하는지 확인
        UWorldTile* Tile = WorldComponent->GetTileAtHex(Hex);
        if (!Tile)
        {
            continue;
        }
        
        // 조건 3: 통행 가능한지 확인 (바다 아님)
        if (!Tile->IsPassable())
        {
            continue;
        }
        
        // 조건 4: 유닛이 없는지 확인
        if (GetUnitAtHex(Hex) != nullptr)
        {
            continue;
        }
        
        // 모든 조건 통과 → 유효한 소환 위치
        ValidLocations.Add(Hex);
    }
    
    // 유효한 위치가 없으면 실패
    if (ValidLocations.Num() == 0)
    {
        return FVector2D(-1, -1);
    }
    
    // 랜덤 선택
    int32 RandomIndex = FMath::RandRange(0, ValidLocations.Num() - 1);
    return ValidLocations[RandomIndex];
}

TArray<AUnitCharacterBase*> UUnitManager::GetAllUnits() const
{
    return SpawnedUnits;
}

void UUnitManager::DestroyUnit(AUnitCharacterBase* Unit, FVector2D HexPosition)
{
    if (!Unit || !SpawnedUnits.Contains(Unit))
    {
        return;
    }
    
    // 0. 해당 유닛이 현재 선택된 유닛이면 타일 밝기 초기화 및 선택 해제
    if (Unit == CurrentSelectedUnit)
    {
        ClearAllTileBrightness();
        CurrentReachableTiles.Empty();
        ClearSelectedUnit();
    }
    
    // 0-1. 해당 유닛이 하이라이트된 적 유닛 목록에 있으면 제거
    if (HighlightedEnemyUnits.Contains(Unit))
    {
        HighlightedEnemyUnits.Remove(Unit);
    }
    
    // 1. 먼저 HexToUnitMap에서 제거 (안전하게 먼저 처리)
    RemoveUnitFromHex(HexPosition);
    
    // 2. 플레이어 스테이트에서 유닛 제거
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
    
    // 3. SpawnedUnits에서 제거
    SpawnedUnits.Remove(Unit);
    
    // 4. 유닛 Destroy
    Unit->Destroy();
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
    // 유닛 등록 시 그리드(정수) hex를 쓰는 반면, 전투 사망 등에서는 WorldToHex(실수)가 전달될 수 있어
    // 키 불일치로 제거가 안 되는 경우를 막기 위해 정수로 반올림한 hex로 제거
    const FVector2D RoundedHex(FMath::RoundToInt(HexPosition.X), FMath::RoundToInt(HexPosition.Y));
    HexToUnitMap.Remove(RoundedHex);
}

FVector2D UUnitManager::GetHexPositionForUnit(AUnitCharacterBase* Unit) const
{
    if (!Unit)
    {
        return FVector2D::ZeroVector;
    }

    // HexToUnitMap을 순회하여 해당 유닛의 Hex 위치 찾기
    for (const auto& Pair : HexToUnitMap)
    {
        if (Pair.Value == Unit)
        {
            return Pair.Key;
        }
    }

    return FVector2D::ZeroVector;
}

bool UUnitManager::CanPlaceUnitAtHex(FVector2D HexPosition, AUnitCharacterBase* MovingUnit) const
{
    if (!WorldComponent || !WorldComponent->IsValidHexPosition(HexPosition))
    {
        return false;
    }
    
    // 도시 타일인지 확인 (통행불가)
    if (WorldComponent->IsCityAtHex(HexPosition))
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

// 이동 선택 시스템 구현
void UUnitManager::HandleMoveSelection(UWorldTile* ClickedTile)
{
    if (!ClickedTile || !WorldComponent)
    {
        return;
    }
    
    FVector2D HexPos = ClickedTile->GetGridPosition();
    
    // 첫 번째 선택이 없으면 첫 번째로 설정
    if (!HasMoveFirstSelection())
    {
        // 해당 타일에 유닛이 있는지 확인
        AUnitCharacterBase* UnitAtTile = GetUnitAtHex(HexPos);
        if (!UnitAtTile)
        {
            return; // 유닛이 없으면 선택하지 않음
        }
        
        MoveFirstSelectedTile = ClickedTile;
        MoveFirstSelectedTile->SetSelected(true);
    }
    // 첫 번째 선택이 있고 두 번째 선택이 없으면 두 번째로 설정
    else if (!HasMoveSecondSelection())
    {
        // 같은 타일을 선택한 경우 즉시 선택 초기화
        if (MoveFirstSelectedTile && MoveFirstSelectedTile->GetGridPosition() == HexPos)
        {
            ClearMoveSelection(); // 첫 번째 선택까지 모두 초기화
            return;
        }
        
        // 플레이어 0인 경우에만 이동 가능 타일 검증
        AUnitCharacterBase* FirstUnit = GetUnitAtHex(MoveFirstSelectedTile->GetGridPosition());
        if (FirstUnit && FirstUnit->GetPlayerIndex() == 0)
        {
            // 1턴 내 도달 불가능한 타일을 클릭한 경우
            if (!IsReachableTile(HexPos))
            {
                // 선택 취소 (밝기 초기화 및 이동 가능 타일 목록 초기화 포함)
                ClearMoveSelection();
                return;
            }
        }
        
        MoveSecondSelectedTile = ClickedTile;
        MoveSecondSelectedTile->SetSelected(true);
        
        // 유닛 이동 실행
        MoveUnitFromFirstToSecondSelection();
        
        // 선택 초기화는 이동 완료 후 CompleteMovement()에서 처리
    }
}

void UUnitManager::ClearMoveSelection()
{
    // 유닛 선택 해제 (이동 시작 시)
    ClearSelectedUnit();
    
    // 타일 밝기 초기화 및 이동 가능 타일 목록 초기화
    ClearAllTileBrightness();
    CurrentReachableTiles.Empty();
    
    // 기존 선택 해제
    if (MoveFirstSelectedTile)
    {
        MoveFirstSelectedTile->SetSelected(false);
        MoveFirstSelectedTile = nullptr;
    }
    
    if (MoveSecondSelectedTile)
    {
        MoveSecondSelectedTile->SetSelected(false);
        MoveSecondSelectedTile = nullptr;
    }
}

void UUnitManager::MoveUnitFromFirstToSecondSelection()
{
    if (!MoveFirstSelectedTile || !MoveSecondSelectedTile || !WorldComponent)
    {
        ClearMoveSelection(); // 선택 초기화
        return;
    }
    
    FVector2D FirstHexPos = MoveFirstSelectedTile->GetGridPosition();
    FVector2D SecondHexPos = MoveSecondSelectedTile->GetGridPosition();
    
    // 첫 번째 타일의 유닛 가져오기
    AUnitCharacterBase* UnitToMove = GetUnitAtHex(FirstHexPos);
    if (!UnitToMove)
    {
        ClearMoveSelection(); // 선택 초기화
        return;
    }
    
    // 유닛이 이동 가능한 상태인지 확인 (이미 공격했거나 이동력이 없는 경우)
    if (UUnitStatusComponent* StatusComp = UnitToMove->GetUnitStatusComponent())
    {
        if (!StatusComp->CanMove())
        {
            ClearMoveSelection(); // 선택 초기화
            return; // 이동할 수 없는 상태
        }
    }
    
    // 두 번째 타일이 이동 가능한지 확인
    if (!CanPlaceUnitAtHex(SecondHexPos))
    {
        ClearMoveSelection(); // 선택 초기화
        return;
    }
    
    // A* 경로 찾기로 최적 경로 계산
    TArray<FVector2D> Path = FindPath(FirstHexPos, SecondHexPos);
    
    // 경로가 없거나 유효하지 않으면 이동하지 않음
    if (Path.Num() <= 1)
    {
        ClearMoveSelection(); // 선택 초기화
        return;
    }
    
    // 유닛의 남은 이동력 확인 (유닛별 이동력 사용)
    int32 MaxMovementCost = GetUnitRemainingMovement(UnitToMove);
    
    // 이동력 제한이 있는 경로 찾기
    TArray<FVector2D> LimitedPath = FindPathWithMovementCost(FirstHexPos, SecondHexPos, MaxMovementCost);
    
    // 제한된 경로가 없으면 이동하지 않음
    if (LimitedPath.Num() <= 1)
    {
        ClearMoveSelection(); // 선택 초기화
        return;
    }
    
    // 이동 시작 전에 타일 밝기 초기화 및 이동 가능 타일 목록 초기화
    ClearAllTileBrightness();
    CurrentReachableTiles.Empty();
    
    // 이동 시작 시 즉시 유닛 선택 해제 (외곽선 하이라이트 제거)
    if (UnitToMove)
    {
        UnitToMove->SetUnitSelected(false);
    }
    
    // 경로를 따라 유닛 이동 (시각적 애니메이션)
    StartVisualMovement(UnitToMove, LimitedPath);
    // 선택 초기화는 이동 완료 후 CompleteMovement()에서 처리
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

            // 도시 타일은 경로에서 제외 (목표 타일은 제외 - 목표 타일은 CanPlaceUnitAtHex에서 체크)
            if (NeighborHex != EndHex && WorldComponent->IsCityAtHex(NeighborHex))
            {
                continue;
            }

            // 유닛이 있는 타일은 경로에서 제외 (목표 타일은 제외 - 목표 타일은 CanPlaceUnitAtHex에서 체크)
            if (NeighborHex != EndHex && GetUnitAtHex(NeighborHex) != nullptr)
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
    
    // 도시 타일인지 확인 (통행불가)
    if (WorldComponent->IsCityAtHex(HexPosition))
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
    
    // 먼저 이동 가능한지 확인 (층수 차이 검증 포함)
    if (!CanMoveBetweenHexes(FromHex, ToHex))
    {
        return INT32_MAX; // 이동 불가 (층수 차이 2 이상 또는 바다 등)
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

void UUnitManager::RequestBuilderBuildFacility(FVector2D Hex, FName FacilityRowName)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance());
    UFacilityManager* FacilityManager = GameInstance ? GameInstance->GetFacilityManager() : nullptr;
    UWorldComponent* WorldComp = GameInstance ? GameInstance->GetGeneratedWorldComponent() : nullptr;
    if (!FacilityManager || !WorldComp)
    {
        return;
    }

    AUnitCharacterBase* UnitAtTile = GetUnitAtHex(Hex);
    if (UnitAtTile && IsBuilderUnit(UnitAtTile))
    {
        if (UUnitVisualizationComponent* VisComp = UnitAtTile->GetUnitVisualizationComponent())
        {
            VisComp->PlayBuilderActionMontage(FSimpleDelegate::CreateLambda([UnitAtTile, FacilityRowName, Hex, FacilityManager, WorldComp]()
            {
                if (UnitAtTile && UnitAtTile->GetUnitStatusComponent())
                {
                    UnitAtTile->GetUnitStatusComponent()->SetHasAttacked(true);
                }
                FacilityManager->BuildFacility(FacilityRowName, Hex, WorldComp);
            }));
            return;
        }
    }
    FacilityManager->BuildFacility(FacilityRowName, Hex, WorldComp);
}

void UUnitManager::RequestBuilderRepairFacility(FVector2D Hex)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance());
    UFacilityManager* FacilityManager = GameInstance ? GameInstance->GetFacilityManager() : nullptr;
    UWorldComponent* WorldComp = GameInstance ? GameInstance->GetGeneratedWorldComponent() : nullptr;
    if (!FacilityManager || !WorldComp)
    {
        return;
    }

    AUnitCharacterBase* UnitAtTile = GetUnitAtHex(Hex);
    if (UnitAtTile && IsBuilderUnit(UnitAtTile))
    {
        if (UUnitVisualizationComponent* VisComp = UnitAtTile->GetUnitVisualizationComponent())
        {
            VisComp->PlayBuilderActionMontage(FSimpleDelegate::CreateLambda([UnitAtTile, FacilityManager, Hex, WorldComp]()
            {
                FacilityManager->RepairFacility(Hex, WorldComp);
                if (UnitAtTile && UnitAtTile->GetUnitStatusComponent())
                {
                    UnitAtTile->GetUnitStatusComponent()->SetHasAttacked(true);
                }
            }));
            return;
        }
    }
    FacilityManager->RepairFacility(Hex, WorldComp);
}

void UUnitManager::RequestBuilderDestroyFacility(FVector2D Hex)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance());
    UFacilityManager* FacilityManager = GameInstance ? GameInstance->GetFacilityManager() : nullptr;
    UWorldComponent* WorldComp = GameInstance ? GameInstance->GetGeneratedWorldComponent() : nullptr;
    if (!FacilityManager || !WorldComp)
    {
        return;
    }

    AUnitCharacterBase* UnitAtTile = GetUnitAtHex(Hex);
    if (UnitAtTile && IsBuilderUnit(UnitAtTile))
    {
        if (UUnitVisualizationComponent* VisComp = UnitAtTile->GetUnitVisualizationComponent())
        {
            VisComp->PlayBuilderActionMontage(FSimpleDelegate::CreateLambda([UnitAtTile, FacilityManager, Hex, WorldComp]()
            {
                FacilityManager->DestroyFacility(Hex, WorldComp);
                if (UnitAtTile && UnitAtTile->GetUnitStatusComponent())
                {
                    UnitAtTile->GetUnitStatusComponent()->SetHasAttacked(true);
                }
            }));
            return;
        }
    }
    FacilityManager->DestroyFacility(Hex, WorldComp);
}

// 시각적 이동 애니메이션 함수들 구현
void UUnitManager::StartVisualMovement(AUnitCharacterBase* Unit, const TArray<FVector2D>& Path)
{
    if (!Unit || Path.Num() <= 1)
    {
        return;
    }
    
    FVector2D DestinationHex = Path[Path.Num() - 1];
    
    // 이동 시작 직전에 목적지 타일이 여전히 사용 가능한지 최종 확인
    if (!CanPlaceUnitAtHex(DestinationHex, Unit))
    {
        // 목적지가 더 이상 사용 불가능하면 이동 취소
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

    // 최종 위치 등록은 이동 완료 후 OnUnitMovementComplete()에서 수행
    // (이동 중에는 위치 정보가 불일치하지 않도록)
    
    // UnitVisualizationComponent에 이동 명령 전달
    if (UUnitVisualizationComponent* VisComponent = Unit->GetUnitVisualizationComponent())
    {
        // WorldComponent 설정 (없으면)
        if (!VisComponent->GetWorldComponent())
        {
            VisComponent->SetWorldComponent(WorldComponent);
        }
        
        // UnitManager 설정
        VisComponent->SetUnitManager(this);
        
        // 경로를 따라 이동 시작
        VisComponent->StartMovementAlongPath(Path);
    }
}

void UUnitManager::CompleteMovement()
{
    // 선택 초기화
    ClearMoveSelection();
}

void UUnitManager::OnUnitMovementComplete(AUnitCharacterBase* Unit, FVector2D FinalHex)
{
    if (!Unit || !WorldComponent)
    {
        return;
    }
    
    // 이동 완료 후 최종 위치에 유닛 등록
    // (이동 중에는 위치 정보가 불일치하지 않도록 완료 후에만 등록)
    AUnitCharacterBase* UnitAtFinalHex = GetUnitAtHex(FinalHex);
    if (UnitAtFinalHex != Unit)
    {
        // 최종 위치에 유닛 등록
        if (!SetUnitAtHex(FinalHex, Unit))
        {
            // 등록 실패 시 (다른 유닛이 있거나 이동 불가능한 타일)
            // 경고 로그 출력 (필요시)
        }
    }
    
    // 선택 초기화
    ClearMoveSelection();
    
    // ========== AI 플레이어의 유닛이면 AIPlayerManager에 알림 ==========
    int32 UnitPlayerIndex = Unit->GetPlayerIndex();
    if (UWorld* World = GetWorld())
    {
        if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            // 총 플레이어 수 가져오기
            int32 TotalPlayerCount = GameInstance->GetPlayerStateCount();
            
            // AI 플레이어인지 확인 (1 ~ TotalPlayerCount-1)
            if (UnitPlayerIndex >= 1 && UnitPlayerIndex < TotalPlayerCount)
            {
                if (UAIPlayerManager* AIPlayerManager = GameInstance->GetAIPlayerManager())
                {
                    AIPlayerManager->OnUnitMovementFinished(UnitPlayerIndex);
                }
            }
        }
    }
}

// 전투 유닛 판단 함수
bool UUnitManager::IsCombatUnit(AUnitCharacterBase* Unit) const
{
    if (!Unit)
    {
        return false;
    }

    UUnitStatusComponent* StatusComp = Unit->GetUnitStatusComponent();
    if (!StatusComp)
    {
        return false;
    }

    // BaseStat에서 CanAttack 확인 (전투 가능한 유닛)
    FUnitBaseStat BaseStat = StatusComp->GetBaseStat();
    return BaseStat.CanAttack;
}

// 전투 선택 시스템
void UUnitManager::HandleCombatSelection(UWorldTile* ClickedTile)
{
    if (!ClickedTile || !WorldComponent)
    {
        return;
    }
    
    FVector2D HexPos = ClickedTile->GetGridPosition();
    
    // 첫 번째 선택이 없으면 첫 번째로 설정
    if (!HasCombatFirstSelection())
    {
        // 해당 타일에 유닛이 있는지 확인
        AUnitCharacterBase* UnitAtTile = GetUnitAtHex(HexPos);
        if (!UnitAtTile)
        {
            return; // 유닛이 없으면 선택하지 않음
        }
        
        // 전투 유닛인지 확인
        if (!IsCombatUnit(UnitAtTile))
        {
            return; // 전투 유닛이 아니면 선택하지 않음
        }
        
        // 공격 가능한 상태인지 확인
        if (UUnitStatusComponent* StatusComp = UnitAtTile->GetUnitStatusComponent())
        {
            if (!StatusComp->CanAttack())
            {
                return; // 공격할 수 없는 상태
            }
            
            // Range = 0 체크 (전투 불가)
            if (StatusComp->GetRange() == 0)
            {
                return; // Range = 0은 전투 불가
            }
        }
        
        CombatFirstSelectedTile = ClickedTile;
        CombatFirstSelectedTile->SetSelected(true);
    }
    // 첫 번째 선택이 있고 두 번째 선택이 없으면 두 번째로 설정
    else if (!HasCombatSecondSelection())
    {
        // 같은 타일을 선택한 경우 즉시 선택 초기화
        if (CombatFirstSelectedTile && CombatFirstSelectedTile->GetGridPosition() == HexPos)
        {
            ClearCombatSelection(); // 첫 번째 선택까지 모두 초기화
            return;
        }
        
        // 해당 타일에 유닛이 있는지 확인
        AUnitCharacterBase* UnitAtTile = GetUnitAtHex(HexPos);
        // 도시 타일인지 확인
        bool bIsCityTile = WorldComponent->IsCityAtHex(HexPos);
        
        // 유닛도 없고 도시도 아니면 선택하지 않음
        if (!UnitAtTile && !bIsCityTile)
        {
            return;
        }
        
        // 사거리 검증
        AUnitCharacterBase* Attacker = GetUnitAtHex(CombatFirstSelectedTile->GetGridPosition());
        if (Attacker && Attacker->GetUnitStatusComponent())
        {
            int32 BaseAttackRange = Attacker->GetUnitStatusComponent()->GetRange();
            int32 AttackRange = BaseAttackRange; // Range 보너스 적용 전 기본값
            int32 HexDistance = WorldComponent->GetHexDistance(CombatFirstSelectedTile->GetGridPosition(), HexPos);
            
            // 원거리 유닛인 경우 Range 보너스 적용
            if (BaseAttackRange > 1)
            {
                // UnitCombatComponent를 통해 Range 보너스 계산
                if (UUnitCombatComponent* CombatComp = Attacker->GetUnitCombatComponent())
                {
                    // Range 보너스 계산 (평지: +0, 언덕: +1, 산: +2)
                    int32 RangeBonus = CombatComp->CalculateRangeBonus(CombatFirstSelectedTile->GetGridPosition());
                    AttackRange += RangeBonus;
                }
            }
            
            // 사거리 밖이면 선택 초기화
            if (HexDistance > AttackRange)
            {
                ClearCombatSelection(); // 첫 번째 선택도 초기화
                return;
            }
            
            // Range == 1인 근접 공격일 때만 층수 차이 체크
            if (BaseAttackRange == 1)
            {
                // 기존 GetFloorLevel() 함수 재활용
                UWorldTile* AttackerTile = CombatFirstSelectedTile;
                UWorldTile* DefenderTile = WorldComponent->GetTileAtHex(HexPos);
                
                if (AttackerTile && DefenderTile)
                {
                    int32 AttackerFloor = GetFloorLevel(AttackerTile->GetLandType());
                    int32 DefenderFloor = GetFloorLevel(DefenderTile->GetLandType());
                    int32 FloorDifference = FMath::Abs(AttackerFloor - DefenderFloor);
                    
                    // 층수 차이가 2 이상이면 선택 초기화
                    if (FloorDifference >= 2)
                    {
                        ClearCombatSelection(); // 첫 번째 선택도 초기화
                        return;
                    }
                }
            }
        }
        
        CombatSecondSelectedTile = ClickedTile;
        CombatSecondSelectedTile->SetSelected(true);
        
        // 전투 실행
        ExecuteCombatBetweenSelectedUnits();
        
        // 선택 초기화
        ClearCombatSelection();
    }
}

// 전투 선택 초기화
void UUnitManager::ClearCombatSelection()
{
    // 유닛 선택 해제 (전투 시작 시)
    ClearSelectedUnit();
    
    if (CombatFirstSelectedTile)
    {
        CombatFirstSelectedTile->SetSelected(false);
        CombatFirstSelectedTile = nullptr;
    }
    
    if (CombatSecondSelectedTile)
    {
        CombatSecondSelectedTile->SetSelected(false);
        CombatSecondSelectedTile = nullptr;
    }
}

// 전투 실행 함수
void UUnitManager::ExecuteCombatBetweenSelectedUnits()
{
    if (!CombatFirstSelectedTile || !CombatSecondSelectedTile || !WorldComponent)
    {
        return;
    }
    
    FVector2D FirstHexPos = CombatFirstSelectedTile->GetGridPosition();
    FVector2D SecondHexPos = CombatSecondSelectedTile->GetGridPosition();
    
    // 첫 번째 타일의 유닛 가져오기 (공격자)
    AUnitCharacterBase* Attacker = GetUnitAtHex(FirstHexPos);
    if (!Attacker)
    {
        return;
    }
    
    // 전투 시작 전에 타일 밝기 초기화
    ClearAllTileBrightness();
    
    // 두 번째 타일이 도시 타일인지 확인
    bool bIsCityTile = WorldComponent->IsCityAtHex(SecondHexPos);
    
    // 도시 공격인 경우
    if (bIsCityTile)
    {
        // 도시 소유 플레이어 찾기
        UCityComponent* CityComponent = nullptr;
        if (UWorld* World = GetWorld())
        {
            if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
            {
                // 모든 플레이어의 도시 좌표 확인
                int32 TotalPlayerCount = SuperGameInst->GetPlayerStateCount();
                for (int32 i = 0; i < TotalPlayerCount; i++)
                {
                    if (ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(i))
                    {
                        if (PlayerState->HasCity() && PlayerState->GetCityCoordinate() == SecondHexPos)
                        {
                            CityComponent = PlayerState->GetCityComponent();
                            break;
                        }
                    }
                }
            }
        }
        
        if (!CityComponent)
        {
            return;
        }
        
        // 전투 컴포넌트 가져오기 (공격자의 컴포넌트 사용)
        UUnitCombatComponent* CombatComp = Attacker->GetUnitCombatComponent();
        if (!CombatComp)
        {
            return;
        }
        
        // 거리 계산
        int32 HexDistance = WorldComponent->GetHexDistance(FirstHexPos, SecondHexPos);
        
        // 도시 공격 가능 여부 확인
        if (!CombatComp->CanExecuteCombatAgainstCity(Attacker, CityComponent, FirstHexPos, SecondHexPos))
        {
            return;
        }
        
        // 도시 공격 계산 즉시 실행 (데미지 적용 포함)
        FCombatResult CombatResult = CombatComp->ExecuteCombatAgainstCity(Attacker, CityComponent, HexDistance, FirstHexPos, SecondHexPos);
        
        // 전투 시작 시점에 UI 닫기 위해 델리게이트 브로드캐스트
        OnCombatExecuted.Broadcast();
        
        // 공격자의 UnitVisualizationComponent 가져오기
        UUnitVisualizationComponent* AttackerVisComp = Attacker->GetUnitVisualizationComponent();
        if (AttackerVisComp)
        {
            // WorldComponent 설정 (없으면)
            if (!AttackerVisComp->GetWorldComponent())
            {
                AttackerVisComp->SetWorldComponent(WorldComponent);
            }
            
            // UnitManager 설정
            AttackerVisComp->SetUnitManager(this);
            
            // 도시 공격 시각화 시작
            AttackerVisComp->StartCombatVisualizationAgainstCity(Attacker, CityComponent, SecondHexPos, CombatResult);
        }
        else
        {
            // UnitVisualizationComponent가 없으면 즉시 결과 처리 (폴백)
            OnCombatVisualizationComplete(Attacker, nullptr, CombatResult, FirstHexPos, SecondHexPos);
        }
    }
    // 유닛 vs 유닛 전투인 경우
    else
    {
        // 두 번째 타일의 유닛 가져오기 (방어자)
        AUnitCharacterBase* Defender = GetUnitAtHex(SecondHexPos);
        if (!Defender)
        {
            return;
        }
        
        // 전투 컴포넌트 가져오기 (공격자의 컴포넌트 사용)
        UUnitCombatComponent* CombatComp = Attacker->GetUnitCombatComponent();
        if (!CombatComp)
        {
            return;
        }
        
        // 거리 계산
        int32 HexDistance = WorldComponent->GetHexDistance(FirstHexPos, SecondHexPos);
        
        // 전투 가능 여부 확인 (Hex 좌표 포함하여 사거리/층수 검증)
        if (!CombatComp->CanExecuteCombat(Attacker, Defender, FirstHexPos, SecondHexPos))
        {
            return;
        }
        
        // 전투 계산 즉시 실행 (데미지 적용 포함)
        FCombatResult CombatResult = CombatComp->ExecuteCombat(Attacker, Defender, HexDistance, FirstHexPos, SecondHexPos);
        
        // 전투 시작 시점에 UI 닫기 위해 델리게이트 브로드캐스트
        OnCombatExecuted.Broadcast();
        
        // 공격자의 UnitVisualizationComponent 가져오기
        UUnitVisualizationComponent* AttackerVisComp = Attacker->GetUnitVisualizationComponent();
        if (AttackerVisComp)
        {
            // WorldComponent 설정 (없으면)
            if (!AttackerVisComp->GetWorldComponent())
            {
                AttackerVisComp->SetWorldComponent(WorldComponent);
            }
            
            // UnitManager 설정
            AttackerVisComp->SetUnitManager(this);
            
            // 전투 시각화 시작
            AttackerVisComp->StartCombatVisualization(Attacker, Defender, CombatResult);
        }
        else
        {
            // UnitVisualizationComponent가 없으면 즉시 결과 처리 (폴백)
            OnCombatVisualizationComplete(Attacker, Defender, CombatResult, FirstHexPos, SecondHexPos);
        }
    }
}

// 전투 시각화 완료 콜백
void UUnitManager::OnCombatVisualizationComplete(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender, const FCombatResult& CombatResult, FVector2D AttackerHex, FVector2D DefenderHex)
{
    // Attacker는 반드시 있어야 함
    if (!Attacker)
    {
        return;
    }
    
    // Defender는 죽어서 Destroy되었을 수 있으므로 nullptr 체크는 하지 않음
    // 단, Defender가 유효할 때만 일부 처리를 수행
    
    // 유닛이 죽었으면 제거
    // 주의: Death 몽타주가 끝난 후에 destroy되도록 UnitVisualizationComponent에서 처리됨
    // 여기서는 즉시 destroy하지 않음
    // 공격자는 반격으로 죽을 수 없으므로 공격자 사망 처리는 불필요
    // 방어자 사망은 UnitVisualizationComponent의 OnCombatNotify_Death()에서 처리됨
    
    // ========== 전투 후 HP바 갱신 ==========
    // 공격자 HP바 갱신
    UpdateUnitHPBar(Attacker);
    
    // 방어자 HP바 갱신 (살아있는 경우만)
    if (IsValid(Defender))
    {
        UpdateUnitHPBar(Defender);
    }
    
    // ========== AI 플레이어의 유닛이면 AIPlayerManager에 알림 ==========
    int32 AttackerPlayerIndex = Attacker->GetPlayerIndex();
    if (UWorld* World = GetWorld())
    {
        if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            // 총 플레이어 수 가져오기
            int32 TotalPlayerCount = GameInstance->GetPlayerStateCount();
            
            // AI 플레이어인지 확인 (1 ~ TotalPlayerCount-1)
            if (AttackerPlayerIndex >= 1 && AttackerPlayerIndex < TotalPlayerCount)
            {
                if (UAIPlayerManager* AIPlayerManager = GameInstance->GetAIPlayerManager())
                {
                    AIPlayerManager->OnCombatActionFinished(AttackerPlayerIndex);
                }
            }
        }
    }
}

// 유닛 HP바 갱신 함수
void UUnitManager::UpdateUnitHPBar(AUnitCharacterBase* Unit)
{
    if (!Unit)
    {
        return;
    }
    
    // UnitStatusComponent에서 체력 정보 가져오기
    UUnitStatusComponent* StatusComp = Unit->GetUnitStatusComponent();
    if (!StatusComp)
    {
        return;
    }
    
    // SmallUnitUI 가져오기
    USmallUnitUI* SmallUI = Unit->GetSmallUnitUI();
    if (!SmallUI)
    {
        return;
    }
    
    // HP 퍼센트 계산
    int32 CurrentHP = StatusComp->GetCurrentHealth();
    int32 MaxHP = StatusComp->GetMaxHealth();
    
    if (MaxHP > 0)
    {
        float HPPercent = (float)CurrentHP / (float)MaxHP;
        SmallUI->SetHPBar(HPPercent);
    }
}

// 유닛 선택 설정
void UUnitManager::SetSelectedUnit(AUnitCharacterBase* Unit)
{
    // 이전 선택 해제
    if (CurrentSelectedUnit && CurrentSelectedUnit != Unit)
    {
        CurrentSelectedUnit->SetUnitSelected(false);
    }
    
    // 새로운 유닛 선택
    CurrentSelectedUnit = Unit;
    
    if (CurrentSelectedUnit)
    {
        CurrentSelectedUnit->SetUnitSelected(true); // 노란색 외곽선
        
        // 공격 가능한 적 유닛에 빨간색 외곽선 표시
        ShowAttackableEnemies(CurrentSelectedUnit);
    }
}

// 유닛 선택 해제
void UUnitManager::ClearSelectedUnit()
{
    if (CurrentSelectedUnit)
    {
        CurrentSelectedUnit->SetUnitSelected(false);
        CurrentSelectedUnit = nullptr;
    }
    
    // 공격 가능한 적 유닛 외곽선 제거
    ClearAttackableEnemies();
}

// 유닛이 1턴 안에 갈 수 있는 모든 타일 계산 (BFS)
TArray<FVector2D> UUnitManager::CalculateMovementRange(AUnitCharacterBase* Unit) const
{
    TArray<FVector2D> ReachableTiles;
    
    if (!Unit || !WorldComponent)
    {
        return ReachableTiles;
    }
    
    // 유닛의 현재 위치와 이동력 가져오기
    FVector2D StartHex = GetHexPositionForUnit(Unit);
    int32 MaxMovementCost = GetUnitRemainingMovement(Unit);
    
    if (MaxMovementCost <= 0)
    {
        return ReachableTiles; // 이동력 없으면 빈 배열 반환
    }
    
    // BFS로 모든 도달 가능한 타일 탐색
    TMap<FVector2D, int32> VisitedWithCost; // 타일과 그곳까지의 누적 비용
    TArray<FVector2D> Queue;
    
    Queue.Add(StartHex);
    VisitedWithCost.Add(StartHex, 0);
    
    while (Queue.Num() > 0)
    {
        FVector2D Current = Queue[0];
        Queue.RemoveAt(0);
        
        int32 CurrentCost = VisitedWithCost[Current];
        
        // 인접한 6개 타일 확인
        TArray<FVector2D> Neighbors = WorldComponent->GetHexNeighbors(Current);
        
        for (FVector2D Neighbor : Neighbors)
        {
            // 이동 가능한지 확인
            if (!CanMoveToHex(Neighbor))
            {
                continue; // 이동 불가능한 타일은 건너뛰기
            }
            
            // 이동 비용 계산 (층수 차이 포함)
            int32 MoveCost = GetMovementCostBetweenHexesWithFloor(Current, Neighbor);
            
            // 이동 불가능한 경우 (층수 차이 2 이상)
            if (MoveCost == INT32_MAX)
            {
                continue; // 층수 차이가 너무 커서 이동 불가
            }
            
            int32 NewCost = CurrentCost + MoveCost;
            
            // 이동력 범위 내인지 확인
            if (NewCost > MaxMovementCost)
            {
                continue; // 이동력 초과하면 건너뛰기
            }
            
            // 이미 방문했는지 확인 (더 적은 비용으로 방문했으면 건너뛰기)
            if (VisitedWithCost.Contains(Neighbor))
            {
                if (VisitedWithCost[Neighbor] <= NewCost)
                {
                    continue; // 이미 더 효율적인 경로로 방문함
                }
            }
            
            // 새로운 타일 추가
            VisitedWithCost.Add(Neighbor, NewCost);
            Queue.Add(Neighbor);
            
            // 시작 위치가 아닌 경우에만 도달 가능 타일로 추가
            if (Neighbor != StartHex)
            {
                ReachableTiles.Add(Neighbor);
            }
        }
    }
    
    return ReachableTiles;
}

// 이동 가능 타일 밝게, 불가능 타일 어둡게
void UUnitManager::ShowMovementRangeWithBrightness(AUnitCharacterBase* Unit)
{
    // 이전 밝기 제거
    ClearAllTileBrightness();
    
    if (!Unit || !WorldComponent)
    {
        return;
    }
    
    // 이동 가능한 타일 계산
    TArray<FVector2D> ReachableTiles;
    
    // 유닛이 이동 가능한 상태인지 확인 (공격했거나 이동력이 없으면 이동 불가)
    if (UUnitStatusComponent* StatusComp = Unit->GetUnitStatusComponent())
    {
        if (StatusComp->CanMove())
        {
            // 이동 가능한 상태: 정상적으로 이동 범위 계산
            ReachableTiles = CalculateMovementRange(Unit);
        }
        // else: 이동 불가능한 상태 (공격했거나 이동력 없음): ReachableTiles는 빈 배열로 유지 (모든 타일 어둡게)
    }
    
    // WorldSpawner를 통해 모든 타일 액터 가져오기
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    AWorldSpawner* WorldSpawner = Cast<AWorldSpawner>(UGameplayStatics::GetActorOfClass(World, AWorldSpawner::StaticClass()));
    if (!WorldSpawner)
    {
        return;
    }
    
    // 이동 가능한 타일 집합으로 변환 (빠른 검색용)
    TSet<FVector2D> ReachableSet;
    for (const FVector2D& Tile : ReachableTiles)
    {
        ReachableSet.Add(Tile);
    }
    
    // 모든 타일에 대해 밝기 설정
    TArray<UWorldTile*> AllTiles = WorldComponent->GetAllTiles();
    
    for (UWorldTile* Tile : AllTiles)
    {
        if (!Tile)
        {
            continue;
        }
        
        FVector2D TilePos = Tile->GetGridPosition();
        AWorldTileActor* TileActor = WorldSpawner->GetTileActorAtHex(TilePos);
        
        if (!TileActor)
        {
            continue;
        }
        
        // 이동 가능한 타일인지 확인
        if (ReachableSet.Contains(TilePos))
        {
            // 이동 가능: 밝게 (Stencil 1)
            TileActor->SetTileBrightness(1);
        }
        else
        {
            // 이동 불가능: 어둡게 (Stencil 2)
            TileActor->SetTileBrightness(2);
        }
        
        // 추적 목록에 추가
        BrightnessAffectedTiles.Add(TileActor);
    }
}

// 모든 타일 밝기 초기화
void UUnitManager::ClearAllTileBrightness()
{
    // 모든 타일을 보통 밝기로 복원
    for (AWorldTileActor* TileActor : BrightnessAffectedTiles)
    {
        if (TileActor)
        {
            TileActor->ResetBrightness(); // Stencil 0
        }
    }
    
    // 추적 목록 비우기
    BrightnessAffectedTiles.Empty();
}

// 이동 가능 타일 계산 및 저장 (플레이어 전용)
void UUnitManager::CalculateAndStoreReachableTiles(AUnitCharacterBase* Unit)
{
    // 기존 목록 초기화
    CurrentReachableTiles.Empty();
    
    if (!Unit)
    {
        return;
    }
    
    // 이동 가능 타일 계산 (기존 CalculateMovementRange 사용)
    CurrentReachableTiles = CalculateMovementRange(Unit);
}

// 특정 타일이 이동 가능한지 확인
bool UUnitManager::IsReachableTile(FVector2D TilePos) const
{
    return CurrentReachableTiles.Contains(TilePos);
}

// ========== 공격 범위 시스템 구현 ==========
TArray<AUnitCharacterBase*> UUnitManager::CalculateAttackableEnemies(AUnitCharacterBase* Unit) const
{
    TArray<AUnitCharacterBase*> AttackableEnemies;
    
    if (!Unit || !WorldComponent)
    {
        return AttackableEnemies;
    }
    
    // 유닛의 StatusComponent 가져오기
    UUnitStatusComponent* StatusComp = Unit->GetUnitStatusComponent();
    if (!StatusComp)
    {
        return AttackableEnemies;
    }
    
    // 전투 유닛인지 확인 (CanAttack == false면 공격 불가)
    if (!IsCombatUnit(Unit))
    {
        return AttackableEnemies; // 건설자 등은 공격 불가
    }
    
    // 공격 가능 상태인지 확인
    if (!StatusComp->CanAttack())
    {
        return AttackableEnemies; // 이미 공격했거나 이동력이 없는 경우
    }
    
    // 기본 사거리 가져오기
    int32 BaseAttackRange = StatusComp->GetRange();
    
    // Range = 0인 경우 전투 불가
    if (BaseAttackRange == 0)
    {
        return AttackableEnemies;
    }
    
    // 원거리 유닛인 경우 Range 보너스 계산
    int32 AttackRange = BaseAttackRange;
    if (BaseAttackRange > 1)
    {
        // UnitCombatComponent를 통해 Range 보너스 계산
        if (UUnitCombatComponent* CombatComp = Unit->GetUnitCombatComponent())
        {
            FVector2D UnitHex = GetHexPositionForUnit(Unit);
            int32 RangeBonus = CombatComp->CalculateRangeBonus(UnitHex);
            AttackRange += RangeBonus;
        }
    }
    
    // 유닛의 현재 위치 가져오기
    FVector2D UnitHex = GetHexPositionForUnit(Unit);
    int32 UnitPlayerIndex = Unit->GetPlayerIndex();
    
    // 모든 유닛 순회
    for (AUnitCharacterBase* OtherUnit : SpawnedUnits)
    {
        if (!OtherUnit || OtherUnit == Unit)
        {
            continue; // 자기 자신은 제외
        }
        
        // 아군인지 확인 (같은 플레이어 인덱스면 아군)
        if (OtherUnit->GetPlayerIndex() == UnitPlayerIndex)
        {
            continue; // 아군은 제외
        }
        
        // 적 유닛의 위치 가져오기
        FVector2D EnemyHex = GetHexPositionForUnit(OtherUnit);
        
        // 거리 계산
        int32 HexDistance = WorldComponent->GetHexDistance(UnitHex, EnemyHex);
        
        // 사거리 밖이면 제외
        if (HexDistance > AttackRange)
        {
            continue;
        }
        
        // Range == 1인 근접 공격일 때만 층수 차이 체크
        if (BaseAttackRange == 1)
        {
            UWorldTile* UnitTile = WorldComponent->GetTileAtHex(UnitHex);
            UWorldTile* EnemyTile = WorldComponent->GetTileAtHex(EnemyHex);
            
            if (UnitTile && EnemyTile)
            {
                int32 UnitFloor = GetFloorLevel(UnitTile->GetLandType());
                int32 EnemyFloor = GetFloorLevel(EnemyTile->GetLandType());
                int32 FloorDifference = FMath::Abs(UnitFloor - EnemyFloor);
                
                // 층수 차이가 2 이상이면 제외
                if (FloorDifference >= 2)
                {
                    continue;
                }
            }
        }
        
        // 모든 조건을 만족하면 공격 가능한 적으로 추가
        AttackableEnemies.Add(OtherUnit);
    }
    
    return AttackableEnemies;
}

void UUnitManager::ShowAttackableEnemies(AUnitCharacterBase* Unit)
{
    // 이전 하이라이트 제거
    ClearAttackableEnemies();
    
    if (!Unit)
    {
        return;
    }
    
    // 공격 가능한 적 유닛 계산
    TArray<AUnitCharacterBase*> AttackableEnemies = CalculateAttackableEnemies(Unit);
    
    // 각 적 유닛에 빨간색 외곽선 표시
    for (AUnitCharacterBase* EnemyUnit : AttackableEnemies)
    {
        if (EnemyUnit)
        {
            EnemyUnit->SetEnemyOutline(true); // Stencil 4 (빨간색)
            HighlightedEnemyUnits.Add(EnemyUnit); // 추적 목록에 추가
        }
    }
}

void UUnitManager::ClearAttackableEnemies()
{
    // 모든 하이라이트된 적 유닛의 외곽선 제거
    for (AUnitCharacterBase* EnemyUnit : HighlightedEnemyUnits)
    {
        if (EnemyUnit)
        {
            EnemyUnit->SetEnemyOutline(false); // Stencil 0
        }
    }
    
    // 추적 목록 비우기
    HighlightedEnemyUnits.Empty();
}

