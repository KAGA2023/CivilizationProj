// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldSpawner.h"
#include "../SuperGameInstance.h"
#include "../Unit/UnitManager.h"
#include "../City/CityActor.h"
#include "../SuperPlayerState.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AWorldSpawner::AWorldSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	WorldComponent = nullptr;
	TileActorClass = AWorldTileActor::StaticClass();
	bIsSpawning = false;
	CurrentSpawnIndex = 0;
}

void AWorldSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	// GameInstance에서 WorldComponent 가져오기 및 UnitManager 생성
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			WorldComponent = SuperGameInst->GetGeneratedWorldComponent();
			
			// UnitManager 생성 및 설정
			if (WorldComponent)
			{
				UUnitManager* UnitManager = NewObject<UUnitManager>(this);
				if (UnitManager)
				{
					UnitManager->SetWorldComponent(WorldComponent);
					SuperGameInst->SetUnitManager(UnitManager);
				}
			}
			
			// LoadingTilesUI에서 SpawnAllTiles()를 호출할 때까지 대기
			// 자동 스폰 제거 - LoadingTilesUI가 제어
		}
	}
}

void AWorldSpawner::SpawnAllTiles()
{
	if (!WorldComponent)
	{
		return;
	}

	if (bIsSpawning)
	{
		return;
	}

	// 기존 타일들 제거
	ClearAllTiles();

	// 모든 타일 가져오기
	TArray<UWorldTile*> AllTiles = WorldComponent->GetAllTiles();
	if (AllTiles.Num() == 0)
	{
		OnTileSpawnCompleted.Broadcast();
		return;
	}

	// 비동기 스폰 준비
	bIsSpawning = true;
	TilesToSpawn = AllTiles;
	CurrentSpawnIndex = 0;

	// 타이머로 비동기 스폰 시작 (매 프레임 여러 개씩 스폰)
	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AWorldSpawner::ProcessAsyncSpawn,
		0.016f, // ~60fps
		true
	);
}

void AWorldSpawner::ProcessAsyncSpawn()
{
	if (!bIsSpawning || CurrentSpawnIndex >= TilesToSpawn.Num())
	{
		// 스폰 완료
		bIsSpawning = false;
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
		
		OnTileSpawnCompleted.Broadcast();
		return;
	}

	// 한 프레임에 여러 개 스폰 (성능 조절 가능)
	int32 SpawnPerFrame = 50; // 프레임당 50개씩 스폰
	int32 SpawnCount = 0;

	while (CurrentSpawnIndex < TilesToSpawn.Num() && SpawnCount < SpawnPerFrame)
	{
		UWorldTile* TileData = TilesToSpawn[CurrentSpawnIndex];
		if (TileData)
		{
			SpawnTileActor(TileData);
		}

		CurrentSpawnIndex++;
		SpawnCount++;
	}
}

AWorldTileActor* AWorldSpawner::SpawnTileActor(UWorldTile* TileData)
{
	if (!TileData || !GetWorld())
	{
		return nullptr;
	}

	// 스폰 파라미터 설정
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 타일 액터 스폰
	FVector SpawnLocation = TileData->GetWorldPosition();
	FRotator SpawnRotation = FRotator::ZeroRotator;

	AWorldTileActor* TileActor = GetWorld()->SpawnActor<AWorldTileActor>(
		TileActorClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (TileActor)
	{
		// 타일 데이터 설정
		TileActor->SetTileData(TileData);

		// 맵에 추가
		FVector2D HexPos = TileData->GetGridPosition();
		TileActors.Add(HexPos, TileActor);
	}

	return TileActor;
}

void AWorldSpawner::UpdateTileVisual(FVector2D HexPosition)
{
	if (AWorldTileActor* TileActor = GetTileActorAtHex(HexPosition))
	{
		TileActor->UpdateVisual();
	}
}

AWorldTileActor* AWorldSpawner::GetTileActorAtHex(FVector2D HexPosition) const
{
	if (AWorldTileActor* const* FoundActor = TileActors.Find(HexPosition))
	{
		return *FoundActor;
	}
	return nullptr;
}

void AWorldSpawner::ClearAllTiles()
{
	// 모든 타일 액터 파괴
	for (auto& Pair : TileActors)
	{
		if (Pair.Value)
		{
			Pair.Value->Destroy();
		}
	}

	TileActors.Empty();
}

void AWorldSpawner::SpawnAllCities()
{
	if (!WorldComponent)
	{
		return;
	}

	// 기존 도시 제거
	ClearAllCities();

	TArray<FVector2D> CityHexes = WorldComponent->GetStartingCityHexes();
	for (const FVector2D& Hex : CityHexes)
	{
		SpawnCityActorAtHex(Hex);
	}
}

ACityActor* AWorldSpawner::SpawnCityActorAtHex(FVector2D Hex)
{
	if (!WorldComponent || !GetWorld())
	{
		return nullptr;
	}

	if (!CityActorClass)
	{
		CityActorClass = ACityActor::StaticClass();
	}

	FVector SpawnLocation = WorldComponent->HexToWorld(Hex);

	// 지형 타입에 따라 Z 오프셋 적용
	if (UWorldTile* Tile = WorldComponent->GetTileAtHex(Hex))
	{
		float ZOffset = 73.0f; // Plains 기본값
		ELandType LandType = Tile->GetLandType();
		if (LandType == ELandType::Hills)
		{
			ZOffset = 146.0f;
		}
		else if (LandType == ELandType::Mountains)
		{
			ZOffset = 219.0f;
		}
		SpawnLocation.Z += ZOffset;
	}
	FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACityActor* City = GetWorld()->SpawnActor<ACityActor>(CityActorClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (City)
	{
		CityActors.Add(Hex, City);
	}
	return City;
}

ACityActor* AWorldSpawner::GetCityActorAtHex(FVector2D Hex) const
{
	if (ACityActor* const* Found = CityActors.Find(Hex))
	{
		return *Found;
	}
	return nullptr;
}

void AWorldSpawner::ClearAllCities()
{
	for (auto& Pair : CityActors)
	{
		if (Pair.Value)
		{
			Pair.Value->Destroy();
		}
	}
	CityActors.Empty();
}

void AWorldSpawner::AssignCitiesToPlayers()
{
	// GameInstance에서 PlayerStates 가져오기
	USuperGameInstance* GameInstance = nullptr;
	if (UWorld* World = GetWorld())
	{
		GameInstance = Cast<USuperGameInstance>(World->GetGameInstance());
	}
	
	if (!GameInstance || !WorldComponent)
	{
		return;
	}
	
	// 도시 좌표 가져오기
	TArray<FVector2D> CityHexes = WorldComponent->GetStartingCityHexes();
	
	// 각 플레이어에게 도시 배정 (순서대로)
	for (int32 i = 0; i < CityHexes.Num(); i++)
	{
		// CityActor 가져오기
		ACityActor* City = GetCityActorAtHex(CityHexes[i]);
		if (!City || !City->CityComponent)
		{
			continue;
		}
		
		// PlayerState 가져오기
		ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(i);
		if (!PlayerState)
		{
			continue;
		}
		
		// 도시 배정
		PlayerState->SetCityComponent(City->CityComponent);
		PlayerState->SetCityCoordinate(CityHexes[i]);
		
		// 도시 중심 반경 1칸의 타일들을 소유로 설정 (초기 도시 영역)
		TArray<FVector2D> InitialTiles = WorldComponent->GetHexesInRadius(CityHexes[i], 1);
		for (const FVector2D& TileCoord : InitialTiles)
		{
			PlayerState->AddOwnedTile(TileCoord, WorldComponent);
		}
	}
}

