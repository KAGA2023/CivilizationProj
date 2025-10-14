// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldSpawner.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

AWorldSpawner::AWorldSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	WorldComponent = nullptr;
	TileActorClass = AWorldTileActor::StaticClass();
	TileMesh = nullptr;
	TileBaseMaterial = nullptr;
	bIsSpawning = false;
	CurrentSpawnIndex = 0;
}

void AWorldSpawner::BeginPlay()
{
	Super::BeginPlay();
}

void AWorldSpawner::SetWorldComponent(UWorldComponent* NewWorldComponent)
{
	WorldComponent = NewWorldComponent;
}

void AWorldSpawner::SpawnAllTiles()
{
	if (!WorldComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[WorldSpawner] WorldComponent is null!"));
		return;
	}

	if (bIsSpawning)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WorldSpawner] Already spawning tiles!"));
		return;
	}

	// 기존 타일들 제거
	ClearAllTiles();

	// 모든 타일 가져오기
	TArray<UWorldTile*> AllTiles = WorldComponent->GetAllTiles();
	if (AllTiles.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[WorldSpawner] No tiles to spawn!"));
		OnTileSpawnCompleted.Broadcast();
		return;
	}

	// 비동기 스폰 준비
	bIsSpawning = true;
	TilesToSpawn = AllTiles;
	CurrentSpawnIndex = 0;

	UE_LOG(LogTemp, Log, TEXT("[WorldSpawner] Starting to spawn %d tiles..."), TilesToSpawn.Num());

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
		
		UE_LOG(LogTemp, Log, TEXT("[WorldSpawner] Tile spawning completed! Total: %d"), TileActors.Num());
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

	// 진행률 브로드캐스트
	OnTileSpawnProgress.Broadcast(CurrentSpawnIndex, TilesToSpawn.Num());
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
		// 메시 설정
		if (TileMesh && TileActor->TileMesh)
		{
			TileActor->TileMesh->SetStaticMesh(TileMesh);
		}

		// 머티리얼 설정
		if (TileBaseMaterial)
		{
			TileActor->BaseMaterial = TileBaseMaterial;
		}

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

