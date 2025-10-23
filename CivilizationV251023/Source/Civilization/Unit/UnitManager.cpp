// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitManager.h"
#include "UnitCharacterBase.h"
#include "../WorldComponent.h"
#include "../SuperGameInstance.h"
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

AUnitCharacterBase* UUnitManager::SpawnUnitAtHex(FVector2D HexPosition, const FName& RowName)
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

    // WorldComponent를 통한 유닛 배치 가능 여부 확인
    if (WorldComponent && !WorldComponent->CanPlaceUnitAtHex(HexPosition))
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
            WorldPosition.Z = 73.0f; // 평지
            break;
        case ELandType::Hills:
            WorldPosition.Z = 146.0f; // 언덕
            break;
        case ELandType::Mountains:
            WorldPosition.Z = 219.0f; // 산
            break;
        default:
            WorldPosition.Z = 73.0f; // 기본값 (평지)
            break;
        }
    }
    else
    {
        WorldPosition.Z = 73.0f; // 타일이 없으면 기본값
    }
    
    // 유효한 World 가져오기
    UWorld* World = GetValidWorld();
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

    // WorldComponent에 유닛 위치 등록
    if (WorldComponent)
    {
        WorldComponent->SetUnitAtHex(HexPosition, NewUnit);
    }

    // 소환된 유닛 목록에 추가
    SpawnedUnits.Add(NewUnit);


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
        // WorldComponent에서 유닛 위치 제거
        if (WorldComponent)
        {
            // 유닛의 현재 위치를 찾아서 제거
            for (auto& Pair : SpawnedUnits)
            {
                if (Pair == Unit)
                {
                    // 유닛의 월드 위치를 육각형 좌표로 변환
                    FVector2D HexPos = WorldComponent->WorldToHex(Unit->GetActorLocation());
                    WorldComponent->RemoveUnitFromHex(HexPos);
                    break;
                }
            }
        }
        
        SpawnedUnits.Remove(Unit);
        Unit->Destroy();
    }
}

void UUnitManager::ClearAllUnits()
{
    // WorldComponent에서 모든 유닛 위치 제거
    if (WorldComponent)
    {
        for (AUnitCharacterBase* Unit : SpawnedUnits)
        {
            if (Unit)
            {
                // 유닛의 월드 위치를 육각형 좌표로 변환
                FVector2D HexPos = WorldComponent->WorldToHex(Unit->GetActorLocation());
                WorldComponent->RemoveUnitFromHex(HexPos);
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
}

void UUnitManager::SetWorldComponent(UWorldComponent* InWorldComponent)
{
    WorldComponent = InWorldComponent;
}

UWorld* UUnitManager::GetValidWorld() const
{
    // Owner를 통해 World 접근 시도
    if (AActor* OwnerActor = GetOwner())
    {
        if (UWorld* World = OwnerActor->GetWorld())
        {
            return World;
        }
    }

    // GEngine을 통해 World 접근 시도
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        if (Context.World())
        {
            return Context.World();
        }
    }

    return nullptr;
}

USuperGameInstance* UUnitManager::GetSuperGameInstance() const
{
    UWorld* World = GetValidWorld();
    if (World)
    {
        return Cast<USuperGameInstance>(World->GetGameInstance());
    }
    return nullptr;
}
