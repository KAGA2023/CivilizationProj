// Fill out your copyright notice in the Description page of Project Settings.

#include "FacilityManager.h"
#include "Engine/DataTable.h"
#include "UObject/SoftObjectPath.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Actor.h"
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
                        // 유닛 완전 제거 (HexToUnitMap에서 먼저 제거 후 Destroy)
                        UnitManager->DestroyUnit(UnitAtTile, TileCoordinate);
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
	// 타일 약탈 시에만 소수점 좌표 반올림 (FVector2D TMap 조회 실패 방지)
	TileCoordinate = FVector2D(
		static_cast<float>(FMath::RoundToInt(TileCoordinate.X)),
		static_cast<float>(FMath::RoundToInt(TileCoordinate.Y)));

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

	// 약탈 상태 설정 (액터 + BuiltFacilities 동기화, 세이브 시 복원용)
	FacilityActor->SetPillaged(bIsPillaged);
	FacilityData->bIsPillaged = bIsPillaged;

	// 약탈 상태가 변경된 경우에만 처리 (모디파이어는 유지·산출은 SuperPlayerState에서 약탈 타일 제외로 처리)
		if (bWasPillaged != bIsPillaged)
		{
			// 연기 파티클: 약탈 시 스폰, 수리 시 제거
			if (bIsPillaged)
			{
				SpawnPillagedSmokeAtLocation(FacilityActor->GetActorLocation(), TileCoordinate);
				// 약탈 순간 Destroy 사운드 재생
				if (!PillagedDestroySound)
				{
					PillagedDestroySound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Civilization/Sound/Destroy.Destroy"));
				}
				if (PillagedDestroySound)
				{
					UGameplayStatics::PlaySound2D(this, PillagedDestroySound);
				}
			}
			else
			{
				DestroyPillagedSmokeAtTile(TileCoordinate);
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
	}

	// 델리게이트 브로드캐스트 (해당 타일의 시설 상태가 변경됨 = 약탈 상태 변경)
	OnFacilityChanged.Broadcast(TileCoordinate);

	return true;
}

bool UFacilityManager::CanRepairFacilityAtTile(FVector2D TileCoordinate, int32 PlayerIndex, UWorldComponent* WorldComponent) const
{
	if (!WorldComponent)
	{
		return false;
	}
	if (!HasFacilityAtTile(TileCoordinate))
	{
		return false;
	}
	UWorldTile* Tile = WorldComponent->GetTileAtHex(TileCoordinate);
	if (!Tile || Tile->GetOwnerPlayerID() != PlayerIndex)
	{
		return false;
	}
	AFacilityActor* FacilityActor = GetFacilityActorAtHex(TileCoordinate);
	return (FacilityActor && FacilityActor->bIsPillaged);
}

bool UFacilityManager::RepairFacility(FVector2D TileCoordinate, UWorldComponent* WorldComponent)
{
	return SetFacilityPillaged(TileCoordinate, false, WorldComponent);
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

	// 시설 메시 선택 로직 (숲+자원 → 숲만 → 자원만 → 없음 순서)
	UStaticMesh* MeshToUse = nullptr;
	UStaticMesh* PillagedMeshToUse = nullptr;

	const bool bHasForest = Tile->HasForest();
	EBonusResource BonusResource = Tile->GetBonusResource();
	EStrategicResource StrategicResource = Tile->GetStrategicResource();
	ELuxuryResource LuxuryResource = Tile->GetLuxuryResource();
	const bool bHasResource = (StrategicResource != EStrategicResource::None) || (LuxuryResource != ELuxuryResource::None) || (BonusResource != EBonusResource::None);

	// 1) 숲+자원 타일: 자원 데이터의 ForestFacilityMesh / ForestFacilityPillagedMesh
	if (bHasForest && bHasResource)
	{
		if (StrategicResource != EStrategicResource::None)
		{
			UDataTable* StrategicResourceDataTable = WorldComponent->GetStrategicResourceDataTable();
			if (StrategicResourceDataTable)
			{
				FString EnumString = UEnum::GetValueAsString(StrategicResource);
				FName ResourceRowName = FName(*EnumString.RightChop(EnumString.Find(TEXT("::")) + 2));
				FStrategicResourceData* ResourceData = StrategicResourceDataTable->FindRow<FStrategicResourceData>(ResourceRowName, TEXT("SpawnFacilityAtHex"));
				if (ResourceData)
				{
					MeshToUse = ResourceData->ForestFacilityMesh ? ResourceData->ForestFacilityMesh : ResourceData->FacilityMesh;
					PillagedMeshToUse = ResourceData->ForestFacilityPillagedMesh ? ResourceData->ForestFacilityPillagedMesh : ResourceData->PillagedMesh;
				}
			}
		}
		else if (LuxuryResource != ELuxuryResource::None)
		{
			UDataTable* LuxuryResourceDataTable = WorldComponent->GetLuxuryResourceDataTable();
			if (LuxuryResourceDataTable)
			{
				FString EnumString = UEnum::GetValueAsString(LuxuryResource);
				FName ResourceRowName = FName(*EnumString.RightChop(EnumString.Find(TEXT("::")) + 2));
				FLuxuryResourceData* ResourceData = LuxuryResourceDataTable->FindRow<FLuxuryResourceData>(ResourceRowName, TEXT("SpawnFacilityAtHex"));
				if (ResourceData)
				{
					MeshToUse = ResourceData->ForestFacilityMesh ? ResourceData->ForestFacilityMesh : ResourceData->FacilityMesh;
					PillagedMeshToUse = ResourceData->ForestFacilityPillagedMesh ? ResourceData->ForestFacilityPillagedMesh : ResourceData->PillagedMesh;
				}
			}
		}
		else if (BonusResource != EBonusResource::None)
		{
			UDataTable* BonusResourceDataTable = WorldComponent->GetBonusResourceDataTable();
			if (BonusResourceDataTable)
			{
				FString EnumString = UEnum::GetValueAsString(BonusResource);
				FName ResourceRowName = FName(*EnumString.RightChop(EnumString.Find(TEXT("::")) + 2));
				FBonusResourceData* ResourceData = BonusResourceDataTable->FindRow<FBonusResourceData>(ResourceRowName, TEXT("SpawnFacilityAtHex"));
				if (ResourceData)
				{
					MeshToUse = ResourceData->ForestFacilityMesh ? ResourceData->ForestFacilityMesh : ResourceData->FacilityMesh;
					PillagedMeshToUse = ResourceData->ForestFacilityPillagedMesh ? ResourceData->ForestFacilityPillagedMesh : ResourceData->PillagedMesh;
				}
			}
		}
	}
	// 2) 숲만 있는 타일: FFacilityData의 ForestFacilityMesh / ForestFacilityPillagedMesh
	else if (bHasForest)
	{
		MeshToUse = FacilityData->ForestFacilityMesh ? FacilityData->ForestFacilityMesh : FacilityData->FacilityMesh;
		PillagedMeshToUse = FacilityData->ForestFacilityPillagedMesh ? FacilityData->ForestFacilityPillagedMesh : FacilityData->PillagedMesh;
	}
	// 3) 자원만 있는 타일: 자원 데이터의 FacilityMesh / PillagedMesh
	else if (StrategicResource != EStrategicResource::None)
	{
		UDataTable* StrategicResourceDataTable = WorldComponent->GetStrategicResourceDataTable();
		if (StrategicResourceDataTable)
		{
			FString EnumString = UEnum::GetValueAsString(StrategicResource);
			FName ResourceRowName = FName(*EnumString.RightChop(EnumString.Find(TEXT("::")) + 2));
			FStrategicResourceData* ResourceData = StrategicResourceDataTable->FindRow<FStrategicResourceData>(ResourceRowName, TEXT("SpawnFacilityAtHex"));
			if (ResourceData)
			{
				if (ResourceData->FacilityMesh) MeshToUse = ResourceData->FacilityMesh;
				if (ResourceData->PillagedMesh) PillagedMeshToUse = ResourceData->PillagedMesh;
			}
		}
	}
	else if (LuxuryResource != ELuxuryResource::None)
	{
		UDataTable* LuxuryResourceDataTable = WorldComponent->GetLuxuryResourceDataTable();
		if (LuxuryResourceDataTable)
		{
			FString EnumString = UEnum::GetValueAsString(LuxuryResource);
			FName ResourceRowName = FName(*EnumString.RightChop(EnumString.Find(TEXT("::")) + 2));
			FLuxuryResourceData* ResourceData = LuxuryResourceDataTable->FindRow<FLuxuryResourceData>(ResourceRowName, TEXT("SpawnFacilityAtHex"));
			if (ResourceData)
			{
				if (ResourceData->FacilityMesh) MeshToUse = ResourceData->FacilityMesh;
				if (ResourceData->PillagedMesh) PillagedMeshToUse = ResourceData->PillagedMesh;
			}
		}
	}
	else if (BonusResource != EBonusResource::None)
	{
		UDataTable* BonusResourceDataTable = WorldComponent->GetBonusResourceDataTable();
		if (BonusResourceDataTable)
		{
			FString EnumString = UEnum::GetValueAsString(BonusResource);
			FName ResourceRowName = FName(*EnumString.RightChop(EnumString.Find(TEXT("::")) + 2));
			FBonusResourceData* ResourceData = BonusResourceDataTable->FindRow<FBonusResourceData>(ResourceRowName, TEXT("SpawnFacilityAtHex"));
			if (ResourceData)
			{
				if (ResourceData->FacilityMesh) MeshToUse = ResourceData->FacilityMesh;
				if (ResourceData->PillagedMesh) PillagedMeshToUse = ResourceData->PillagedMesh;
			}
		}
	}

	// 폴백: FacilityData의 기본 메시
	if (!MeshToUse)
	{
		MeshToUse = FacilityData->FacilityMesh;
	}
	if (!PillagedMeshToUse)
	{
		PillagedMeshToUse = FacilityData->PillagedMesh;
	}

	// 시설 메시 설정
	NewFacilityActor->SetFacilityMesh(MeshToUse, PillagedMeshToUse);

	// 약탈 상태 설정
	if (FacilityData->bIsPillaged)
	{
		NewFacilityActor->SetPillaged(true);
		SpawnPillagedSmokeAtLocation(NewFacilityActor->GetActorLocation(), TileCoordinate);
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
	DestroyPillagedSmokeAtTile(TileCoordinate);
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
	// 모든 약탈 연기 파티클 제거 (소유 액터 파괴)
	for (auto& Pair : PillagedSmokeComponents)
	{
		for (UParticleSystemComponent* PSC : Pair.Value)
		{
			if (IsValid(PSC))
			{
				if (AActor* Owner = PSC->GetOwner())
				{
					Owner->Destroy();
				}
				else
				{
					PSC->DestroyComponent();
				}
			}
		}
	}
	PillagedSmokeComponents.Empty();

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

void UFacilityManager::DestroyPillagedSmokeAtTile(FVector2D TileCoordinate)
{
	if (TArray<UParticleSystemComponent*>* Found = PillagedSmokeComponents.Find(TileCoordinate))
	{
		for (UParticleSystemComponent* PSC : *Found)
		{
			if (IsValid(PSC))
			{
				if (AActor* Owner = PSC->GetOwner())
				{
					Owner->Destroy();
				}
				else
				{
					PSC->DestroyComponent();
				}
			}
		}
		PillagedSmokeComponents.Remove(TileCoordinate);
	}
}

void UFacilityManager::SpawnPillagedSmokeAtLocation(FVector WorldLocation, FVector2D TileCoordinate)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	if (!PillagedSmokeTemplate)
	{
		PillagedSmokeTemplate = LoadObject<UParticleSystem>(nullptr, TEXT("/Game/Civilization/Particle/P_SmokeCiv.P_SmokeCiv"));
	}
	if (!PillagedSmokeTemplate)
	{
		return;
	}

	const FVector Scale(0.5f, 0.5f, 0.5f);
	// 중점 기준 X -100, Y -100, Y +100 위치에 3개 스폰
	const FVector Offsets[] = {
		FVector(-100.0f, 0.0f, 0.0f),
		FVector(0.0f, -100.0f, 0.0f),
		FVector(0.0f, 100.0f, 0.0f)
	};

	TArray<UParticleSystemComponent*> Components;
	for (const FVector& Offset : Offsets)
	{
		UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
			World, PillagedSmokeTemplate, WorldLocation + Offset, FRotator::ZeroRotator, false);
		if (PSC)
		{
			PSC->SetWorldScale3D(Scale);
			Components.Add(PSC);
		}
	}
	if (Components.Num() > 0)
	{
		PillagedSmokeComponents.Add(TileCoordinate, MoveTemp(Components));
	}
}

void UFacilityManager::SpawnBuildSteamAtTile(FVector2D TileCoordinate, UWorldComponent* WorldComponent)
{
	if (!WorldComponent)
	{
		return;
	}
	UWorldTile* Tile = WorldComponent->GetTileAtHex(TileCoordinate);
	if (!Tile)
	{
		return;
	}
	FVector WorldPosition = WorldComponent->HexToWorld(TileCoordinate);
	switch (Tile->GetLandType())
	{
	case ELandType::Plains:   WorldPosition.Z = 73.0f;  break;
	case ELandType::Hills:    WorldPosition.Z = 146.0f; break;
	case ELandType::Mountains: WorldPosition.Z = 219.0f; break;
	default:                  WorldPosition.Z = 73.0f;  break;
	}
	SpawnBuildSteamAtLocation(WorldPosition);
}

void UFacilityManager::SpawnBuildSteamAtLocation(FVector WorldLocation)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	if (!BuildSteamTemplate)
	{
		BuildSteamTemplate = LoadObject<UParticleSystem>(nullptr, TEXT("/Game/Civilization/Particle/P_Steam_LitCiv.P_Steam_LitCiv"));
	}
	if (!BuildSteamTemplate)
	{
		return;
	}

	const FVector Scale(1.0f, 1.0f, 1.0f);
	// X축 -100 위치에 스팀 파티클 5개 스폰, bAutoDestroy true로 재생 후 자동 소멸 (Emitter Loops 8)
	const FVector SpawnLocation = WorldLocation + FVector(-100.0f, 0.0f, 0.0f);
	for (int32 i = 0; i < 5; ++i)
	{
		UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
			World, BuildSteamTemplate, SpawnLocation, FRotator::ZeroRotator, true);
		if (PSC)
		{
			PSC->SetWorldScale3D(Scale);
		}
	}
}

