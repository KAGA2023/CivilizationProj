// Fill out your copyright notice in the Description page of Project Settings.

#include "FacilityManager.h"
#include "Engine/DataTable.h"
#include "UObject/SoftObjectPath.h"
#include "../World/WorldStruct.h"
#include "../World/WorldComponent.h"
#include "../Unit/UnitManager.h"
#include "../SuperGameInstance.h"
#include "../SuperPlayerState.h"
#include "FacilityActor.h"

UFacilityManager::UFacilityManager()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UFacilityManager::BeginPlay()
{
    Super::BeginPlay();

    // 시설 데이터 테이블 로드
    LoadFacilityDataTable();
}

void UFacilityManager::LoadFacilityDataTable()
{
    if (!FacilityDataTable)
    {
        FSoftObjectPath FacilityDataTablePath(TEXT("/Game/Civilization/Data/DT_FacilityData.DT_FacilityData"));
        FacilityDataTable = Cast<UDataTable>(FacilityDataTablePath.TryLoad());
    }
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

    // ========== 자원 조건 우선 체크 ==========
    bool bHasMatchingResource = false;

    // 전략 자원 조건 확인
    // 비어있으면 호환 가능한 전략 자원이 없는 것으로 처리 (건설 불가능)
    if (FacilityData->CompatibleStrategicResources.Num() == 0)
    {
        // 타일에 전략 자원이 있으면 건설 불가능
        if (Tile->GetStrategicResource() != EStrategicResource::None)
        {
            return false; // 호환 가능한 전략 자원이 없음
        }
    }
    else
    {
        // 전략 자원 조건이 있으면 타일에 전략 자원이 있어야 하고 조건에 포함되어야 함
        if (Tile->GetStrategicResource() != EStrategicResource::None)
        {
            if (FacilityData->CompatibleStrategicResources.Contains(Tile->GetStrategicResource()))
            {
                // 조건에 있는 자원이면 건설 가능
                bHasMatchingResource = true;
            }
            else
            {
                // 조건에 없는 자원이면 건설 불가능
                return false;
            }
        }
    }

    // 사치 자원 조건 확인
    // 비어있으면 호환 가능한 사치 자원이 없는 것으로 처리 (건설 불가능)
    if (FacilityData->CompatibleLuxuryResources.Num() == 0)
    {
        // 타일에 사치 자원이 있으면 건설 불가능
        if (Tile->GetLuxuryResource() != ELuxuryResource::None)
        {
            return false; // 호환 가능한 사치 자원이 없음
        }
    }
    else
    {
        // 사치 자원 조건이 있으면 타일에 사치 자원이 있어야 하고 조건에 포함되어야 함
        if (Tile->GetLuxuryResource() != ELuxuryResource::None)
        {
            if (FacilityData->CompatibleLuxuryResources.Contains(Tile->GetLuxuryResource()))
            {
                // 조건에 있는 자원이면 건설 가능
                bHasMatchingResource = true;
            }
            else
            {
                // 조건에 없는 자원이면 건설 불가능
                return false;
            }
        }
    }

    // 보너스 자원 조건 확인
    // 비어있으면 호환 가능한 보너스 자원이 없는 것으로 처리 (건설 불가능)
    if (FacilityData->CompatibleBonusResources.Num() == 0)
    {
        // 타일에 보너스 자원이 있으면 건설 불가능
        if (Tile->GetBonusResource() != EBonusResource::None)
        {
            return false; // 호환 가능한 보너스 자원이 없음
        }
    }
    else
    {
        // 보너스 자원 조건이 있으면 타일에 보너스 자원이 있어야 하고 조건에 포함되어야 함
        if (Tile->GetBonusResource() != EBonusResource::None)
        {
            if (FacilityData->CompatibleBonusResources.Contains(Tile->GetBonusResource()))
            {
                // 조건에 있는 자원이면 건설 가능
                bHasMatchingResource = true;
            }
            else
            {
                // 조건에 없는 자원이면 건설 불가능
                return false;
            }
        }
    }

    // 자원 조건이 만족되면 무조건 건설 가능 (다른 조건 무시)
    if (bHasMatchingResource)
    {
        return true;
    }

    // ========== 자원 조건이 없을 때만 다른 조건들 체크 ==========
    
    // 지형 타입 조건 확인
    // 비어있으면 호환 가능한 지형이 없는 것으로 처리 (건설 불가능)
    if (FacilityData->CompatibleLandTypes.Num() == 0)
    {
        return false; // 호환 가능한 지형이 없음
    }
    // 지형 조건이 있으면 해당 지형 중 하나여야 함
    if (!FacilityData->CompatibleLandTypes.Contains(Tile->GetLandType()))
    {
        return false;
    }

    // 기후 타입 조건 확인
    // 비어있으면 호환 가능한 기후가 없는 것으로 처리 (건설 불가능)
    if (FacilityData->CompatibleClimates.Num() == 0)
    {
        return false; // 호환 가능한 기후가 없음
    }
    // 기후 조건이 있으면 해당 기후 중 하나여야 함
    if (!FacilityData->CompatibleClimates.Contains(Tile->GetClimateType()))
    {
        return false;
    }

    // 숲 조건 확인 (true: 숲이 있는 타일에만, false: 숲이 없는 타일에만)
    if (FacilityData->bIsForest != Tile->HasForest())
    {
        return false;
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

    // 시설 액터 스폰
    SpawnFacilityAtHex(TileCoordinate, FacilityRowName, WorldComponent);

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

    // 사치 자원 추가 (타일에 사치 자원이 있는 경우)
    ELuxuryResource LuxuryResource = Tile->GetLuxuryResource();
    if (LuxuryResource != ELuxuryResource::None)
    {
        // 타일 소유자 PlayerID 확인
        int32 OwnerPlayerID = Tile->GetOwnerPlayerID();
        if (OwnerPlayerID >= 0)
        {
            // 해당 PlayerID의 PlayerState 찾기
            if (UWorld* World = GetWorld())
            {
                if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
                {
                    if (ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(OwnerPlayerID))
                    {
                        // 사치 자원 추가 (AddLuxuryResource 내부에서 UpdatePopulationLimitFromLuxury도 호출됨)
                        PlayerState->AddLuxuryResource(LuxuryResource, 1);
                    }
                }
            }
        }
    }

    // 전략 자원 추가 (타일에 전략 자원이 있는 경우)
    EStrategicResource StrategicResource = Tile->GetStrategicResource();
    if (StrategicResource != EStrategicResource::None)
    {
        // 타일 소유자 PlayerID 확인
        int32 OwnerPlayerID = Tile->GetOwnerPlayerID();
        if (OwnerPlayerID >= 0)
        {
            // 해당 PlayerID의 PlayerState 찾기
            if (UWorld* World = GetWorld())
            {
                if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
                {
                    if (ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(OwnerPlayerID))
                    {
                        // 전략 자원 추가
                        PlayerState->AddStrategicResource(StrategicResource, 1);
                    }
                }
            }
        }
    }

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

	// 사치 자원 제거 (타일에 사치 자원이 있는 경우)
	ELuxuryResource LuxuryResource = Tile->GetLuxuryResource();
	if (LuxuryResource != ELuxuryResource::None)
	{
		// 타일 소유자 PlayerID 확인
		int32 OwnerPlayerID = Tile->GetOwnerPlayerID();
		if (OwnerPlayerID >= 0)
		{
			// 해당 PlayerID의 PlayerState 찾기
			if (UWorld* World = GetWorld())
			{
				if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
				{
					if (ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(OwnerPlayerID))
					{
						// 사치 자원 제거 (RemoveLuxuryResource 내부에서 UpdatePopulationLimitFromLuxury도 호출됨)
						PlayerState->RemoveLuxuryResource(LuxuryResource, 1);
					}
				}
			}
		}
	}

	// 전략 자원 제거 (타일에 전략 자원이 있는 경우)
	EStrategicResource StrategicResource = Tile->GetStrategicResource();
	if (StrategicResource != EStrategicResource::None)
	{
		// 타일 소유자 PlayerID 확인
		int32 OwnerPlayerID = Tile->GetOwnerPlayerID();
		if (OwnerPlayerID >= 0)
		{
			// 해당 PlayerID의 PlayerState 찾기
			if (UWorld* World = GetWorld())
			{
				if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
				{
					if (ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(OwnerPlayerID))
					{
						// 전략 자원 제거
						PlayerState->RemoveStrategicResource(StrategicResource, 1);
					}
				}
			}
		}
	}

	// 시설 액터 제거
	RemoveFacilityActorAtHex(TileCoordinate);

	// 캐시에서 제거
	BuiltFacilities.Remove(TileCoordinate);

	// 타일 생산량 재계산
	WorldComponent->RecalculateTileYields(Tile);

	// 델리게이트 브로드캐스트 (해당 타일의 시설 상태가 변경됨 = 시설 파괴)
	OnFacilityChanged.Broadcast(TileCoordinate);

	return true;
}

bool UFacilityManager::SetFacilityPillaged(FVector2D TileCoordinate, bool bIsPillaged, UWorldComponent* WorldComponent)
{
	if (!WorldComponent)
	{
		return false;
	}

	// 시설이 있는지 확인
	if (!HasFacilityAtTile(TileCoordinate))
	{
		return false;
	}

	// 타일 가져오기
	UWorldTile* Tile = WorldComponent->GetTileAtHex(TileCoordinate);
	if (!Tile)
	{
		return false;
	}

	// 시설 액터 가져오기
	AFacilityActor* FacilityActor = GetFacilityActorAtHex(TileCoordinate);
	if (!FacilityActor)
	{
		return false;
	}

	// 시설 데이터 가져오기
	FFacilityData* FacilityData = BuiltFacilities.Find(TileCoordinate);
	if (!FacilityData)
	{
		return false;
	}

	// 약탈 상태 변경 전 상태 확인
	bool bWasPillaged = FacilityActor->bIsPillaged;

	// 약탈 상태 설정
	FacilityActor->SetPillaged(bIsPillaged);

	// 약탈 상태가 변경된 경우에만 처리
	if (bWasPillaged != bIsPillaged)
	{
		// 타일 모디파이어 처리
		if (bIsPillaged)
		{
			// 약탈되면 타일 모디파이어 제거
			Tile->RemoveTileModifier(FacilityData->TileModifier);
		}
		else
		{
			// 복구되면 타일 모디파이어 다시 추가
			Tile->AddTileModifier(FacilityData->TileModifier);
		}

		// 타일에 사치 자원이 있는 경우
		ELuxuryResource LuxuryResource = Tile->GetLuxuryResource();
		if (LuxuryResource != ELuxuryResource::None)
		{
			// 타일 소유자 PlayerID 확인
			int32 OwnerPlayerID = Tile->GetOwnerPlayerID();
			if (OwnerPlayerID >= 0)
			{
				// 해당 PlayerID의 PlayerState 찾기
				if (UWorld* World = GetWorld())
				{
					if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
					{
						if (ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(OwnerPlayerID))
						{
							if (bIsPillaged)
							{
								// 약탈되면 사치 자원 제거
								PlayerState->RemoveLuxuryResource(LuxuryResource, 1);
							}
							else
							{
								// 복구되면 사치 자원 추가
								PlayerState->AddLuxuryResource(LuxuryResource, 1);
							}
						}
					}
				}
			}
		}

		// 타일에 전략 자원이 있는 경우
		EStrategicResource StrategicResource = Tile->GetStrategicResource();
		if (StrategicResource != EStrategicResource::None)
		{
			// 타일 소유자 PlayerID 확인
			int32 OwnerPlayerID = Tile->GetOwnerPlayerID();
			if (OwnerPlayerID >= 0)
			{
				// 해당 PlayerID의 PlayerState 찾기
				if (UWorld* World = GetWorld())
				{
					if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
					{
						if (ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(OwnerPlayerID))
						{
							if (bIsPillaged)
							{
								// 약탈되면 전략 자원 제거
								PlayerState->RemoveStrategicResource(StrategicResource, 1);
							}
							else
							{
								// 복구되면 전략 자원 추가
								PlayerState->AddStrategicResource(StrategicResource, 1);
							}
						}
					}
				}
			}
		}

		// 타일 생산량 재계산
		WorldComponent->RecalculateTileYields(Tile);
	}

	// 델리게이트 브로드캐스트 (해당 타일의 시설 상태가 변경됨 = 약탈 상태 변경)
	OnFacilityChanged.Broadcast(TileCoordinate);

	return true;
}

AFacilityActor* UFacilityManager::SpawnFacilityAtHex(FVector2D TileCoordinate, FName FacilityRowName, UWorldComponent* WorldComponent)
{
	if (!WorldComponent || !FacilityDataTable)
	{
		return nullptr;
	}

	// 이미 시설 액터가 있는지 확인
	if (AFacilityActor* ExistingActor = GetFacilityActorAtHex(TileCoordinate))
	{
		// 이미 존재하면 기존 액터 반환
		return ExistingActor;
	}

	// 타일 가져오기
	UWorldTile* Tile = WorldComponent->GetTileAtHex(TileCoordinate);
	if (!Tile)
	{
		return nullptr;
	}

	// 시설 데이터 가져오기
	FFacilityData* FacilityData = FacilityDataTable->FindRow<FFacilityData>(FacilityRowName, TEXT("SpawnFacilityAtHex"));
	if (!FacilityData)
	{
		return nullptr;
	}

	// 월드 좌표 계산
	FVector WorldPosition = WorldComponent->HexToWorld(TileCoordinate);
	
	// 타일의 지형 타입에 따라 Z축 높이 설정
	switch (Tile->GetLandType())
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

	// 유효한 World 가져오기
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// 시설 액터 생성
	AFacilityActor* NewFacilityActor = World->SpawnActor<AFacilityActor>(
		AFacilityActor::StaticClass(),
		WorldPosition,
		FRotator::ZeroRotator
	);

	if (!NewFacilityActor)
	{
		return nullptr;
	}

	// 시설 메시 설정
	NewFacilityActor->SetFacilityMesh(FacilityData->FacilityMesh, FacilityData->PillagedMesh);

	// 약탈 상태 설정
	if (FacilityData->bIsPillaged)
	{
		NewFacilityActor->SetPillaged(true);
	}

	// 맵에 등록
	BuiltFacilityActors.Add(TileCoordinate, NewFacilityActor);

	return NewFacilityActor;
}

AFacilityActor* UFacilityManager::GetFacilityActorAtHex(FVector2D TileCoordinate) const
{
	if (AFacilityActor* const* FoundActor = BuiltFacilityActors.Find(TileCoordinate))
	{
		return *FoundActor;
	}
	return nullptr;
}

void UFacilityManager::RemoveFacilityActorAtHex(FVector2D TileCoordinate)
{
	if (AFacilityActor* FacilityActor = GetFacilityActorAtHex(TileCoordinate))
	{
		// 맵에서 제거
		BuiltFacilityActors.Remove(TileCoordinate);

		// 액터 파괴
		if (IsValid(FacilityActor))
		{
			FacilityActor->Destroy();
		}
	}
}

void UFacilityManager::ClearAllFacilityActors()
{
	// 모든 시설 액터 파괴
	for (auto& Pair : BuiltFacilityActors)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->Destroy();
		}
	}

	// 맵 초기화
	BuiltFacilityActors.Empty();
}

