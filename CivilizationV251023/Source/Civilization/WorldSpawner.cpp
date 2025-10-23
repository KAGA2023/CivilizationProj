// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldSpawner.h"
#include "SuperGameInstance.h"
#include "Unit/UnitManager.h"

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

