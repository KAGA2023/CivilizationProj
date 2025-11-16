// Fill out your copyright notice in the Description page of Project Settings.

#include "FacilityManager.h"
#include "Engine/DataTable.h"
#include "UObject/SoftObjectPath.h"
#include "../World/WorldStruct.h"
#include "../World/WorldComponent.h"
#include "../Unit/UnitManager.h"
#include "../SuperGameInstance.h"

UFacilityManager::UFacilityManager()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UFacilityManager::BeginPlay()
{
    Super::BeginPlay();

    // 시설 데이터 테이블 로드
    if (!FacilityDataTable)
    {
        FSoftObjectPath FacilityDataTablePath(TEXT("/Game/Civilization/Data/DT_FacilityData.DT_FacilityData"));
        FacilityDataTable = Cast<UDataTable>(FacilityDataTablePath.TryLoad());
    }
}

void UFacilityManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UFacilityManager::HasFacilityAtTile(FVector2D TileCoordinate) const
{
    return BuiltFacilities.Contains(TileCoordinate);
}

FFacilityData UFacilityManager::GetFacilityAtTile(FVector2D TileCoordinate) const
{
    if (const FFacilityData* FacilityData = BuiltFacilities.Find(TileCoordinate))
    {
        return *FacilityData;
    }
    
    // 시설이 없으면 기본값 반환
    return FFacilityData();
}

bool UFacilityManager::CanBuildFacilityOnTile(FName FacilityRowName, UWorldTile* Tile) const
{
    // 타일이 유효한지 확인
    if (!Tile)
    {
        return false;
    }

    // 데이터 테이블이 로드되었는지 확인
    if (!FacilityDataTable)
    {
        return false;
    }

    // 시설 데이터 가져오기
    FFacilityData* FacilityData = FacilityDataTable->FindRow<FFacilityData>(FacilityRowName, TEXT("CanBuildFacilityOnTile"));
    if (!FacilityData)
    {
        return false;
    }

    // 이미 시설이 있는지 확인
    FVector2D TileCoordinate = Tile->GetGridPosition();
    if (HasFacilityAtTile(TileCoordinate))
    {
        return false;
    }

    // 타일이 땅인지 확인 (바다는 건설 불가)
    if (Tile->GetTerrainType() != ETerrainType::Land)
    {
        return false;
    }

    // 지형 타입 조건 확인
    if (FacilityData->CompatibleLandTypes.Num() > 0)
    {
        if (!FacilityData->CompatibleLandTypes.Contains(Tile->GetLandType()))
        {
            return false;
        }
    }

    // 기후 타입 조건 확인
    if (FacilityData->CompatibleClimates.Num() > 0)
    {
        if (!FacilityData->CompatibleClimates.Contains(Tile->GetClimateType()))
        {
            return false;
        }
    }

    // 숲 필요 여부 확인
    if (FacilityData->bRequiresForest)
    {
        if (!Tile->HasForest())
        {
            return false;
        }
    }

    // 보너스 자원 조건 확인
    if (FacilityData->CompatibleBonusResources.Num() > 0)
    {
        if (Tile->GetBonusResource() == EBonusResource::None || 
            !FacilityData->CompatibleBonusResources.Contains(Tile->GetBonusResource()))
        {
            return false;
        }
    }

    // 사치 자원 조건 확인
    if (FacilityData->CompatibleLuxuryResources.Num() > 0)
    {
        if (Tile->GetLuxuryResource() == ELuxuryResource::None || 
            !FacilityData->CompatibleLuxuryResources.Contains(Tile->GetLuxuryResource()))
        {
            return false;
        }
    }

    // 전략 자원 조건 확인
    if (FacilityData->CompatibleStrategicResources.Num() > 0)
    {
        if (Tile->GetStrategicResource() == EStrategicResource::None || 
            !FacilityData->CompatibleStrategicResources.Contains(Tile->GetStrategicResource()))
        {
            return false;
        }
    }

    // 모든 조건을 만족하면 건설 가능
    return true;
}

bool UFacilityManager::BuildFacility(FName FacilityRowName, FVector2D TileCoordinate, UWorldComponent* WorldComponent)
{
    if (!FacilityDataTable || !WorldComponent)
    {
        return false;
    }

    // 타일 가져오기
    UWorldTile* Tile = WorldComponent->GetTileAtHex(TileCoordinate);
    if (!Tile)
    {
        return false;
    }

    // 타일 조건 재검증
    if (!CanBuildFacilityOnTile(FacilityRowName, Tile))
    {
        return false;
    }

    // 시설 데이터 로드
    FFacilityData* FacilityData = FacilityDataTable->FindRow<FFacilityData>(FacilityRowName, TEXT("BuildFacility"));
    if (!FacilityData)
    {
        return false;
    }

    // 타일 모디파이어 적용
    Tile->AddTileModifier(FacilityData->TileModifier);

    // 캐시: 시설 저장
    BuiltFacilities.Add(TileCoordinate, *FacilityData);

    // 건설자 제거 (해당 타일의 유닛이 건설자라면 제거)
    if (UWorld* World = GetWorld())
    {
        if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            if (UUnitManager* UnitManager = GameInstance->GetUnitManager())
            {
                if (AUnitCharacterBase* UnitAtTile = UnitManager->GetUnitAtHex(TileCoordinate))
                {
                    if (UnitManager->IsBuilderUnit(UnitAtTile))
                    {
                        UnitManager->RemoveUnit(UnitAtTile);
                    }
                }
            }
        }
    }

    // 타일 생산량 재계산 및 알림 (있다면)
    WorldComponent->RecalculateTileYields(Tile);

	// 델리게이트 브로드캐스트 (해당 타일의 시설 상태가 변경됨 = 시설 생성)
	OnFacilityChanged.Broadcast(TileCoordinate);

    return true;
}

bool UFacilityManager::DestroyFacility(FVector2D TileCoordinate, UWorldComponent* WorldComponent)
{
	if (!WorldComponent)
	{
		return false;
	}

	FFacilityData* ExistingFacility = BuiltFacilities.Find(TileCoordinate);
	if (!ExistingFacility)
	{
		// 해당 타일에 시설이 없음
		return false;
	}

	UWorldTile* Tile = WorldComponent->GetTileAtHex(TileCoordinate);
	if (!Tile)
	{
		// 타일이 유효하지 않지만, 캐시는 정리
		BuiltFacilities.Remove(TileCoordinate);
		return false;
	}

	// 타일 모디파이어 제거
	Tile->RemoveTileModifier(ExistingFacility->TileModifier);

	// 캐시에서 제거
	BuiltFacilities.Remove(TileCoordinate);

	// 타일 생산량 재계산
	WorldComponent->RecalculateTileYields(Tile);

	// 델리게이트 브로드캐스트 (해당 타일의 시설 상태가 변경됨 = 시설 파괴)
	OnFacilityChanged.Broadcast(TileCoordinate);

	return true;
}

