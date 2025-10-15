// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldComponent.h"
#include "WorldTileActor.h"
#include "WorldSpawner.generated.h"

// 타일 스폰 완료 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTileSpawnCompleted);

/**
 * 월드 타일 액터들을 스폰하고 관리하는 액터
 */
UCLASS()
class CIVILIZATION_API AWorldSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AWorldSpawner();

protected:
	virtual void BeginPlay() override;

public:	
	// WorldComponent 참조
	UPROPERTY(BlueprintReadWrite, Category = "World")
	UWorldComponent* WorldComponent;

	// 스폰된 타일 액터들 (육각형 좌표 → 액터)
	UPROPERTY(BlueprintReadOnly, Category = "World")
	TMap<FVector2D, AWorldTileActor*> TileActors;

	// 타일 액터 클래스 (블루프린트에서 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
	TSubclassOf<AWorldTileActor> TileActorClass;

	// 스폰 완료 이벤트
	UPROPERTY(BlueprintAssignable, Category = "World Events")
	FOnTileSpawnCompleted OnTileSpawnCompleted;

	// 모든 타일 액터 스폰
	UFUNCTION(BlueprintCallable, Category = "World")
	void SpawnAllTiles();

	// 특정 타일 액터 스폰
	UFUNCTION(BlueprintCallable, Category = "World")
	AWorldTileActor* SpawnTileActor(UWorldTile* TileData);

	// 특정 타일 외형 업데이트
	UFUNCTION(BlueprintCallable, Category = "World")
	void UpdateTileVisual(FVector2D HexPosition);

	// 특정 타일 액터 가져오기
	UFUNCTION(BlueprintCallable, Category = "World")
	AWorldTileActor* GetTileActorAtHex(FVector2D HexPosition) const;

	// 모든 타일 액터 제거
	UFUNCTION(BlueprintCallable, Category = "World")
	void ClearAllTiles();

protected:
	// 스폰 중인지 여부
	UPROPERTY(BlueprintReadOnly, Category = "World")
	bool bIsSpawning;

	// 비동기 스폰용 타이머
	FTimerHandle SpawnTimerHandle;
	TArray<UWorldTile*> TilesToSpawn;
	int32 CurrentSpawnIndex;

	// 비동기 스폰 처리
	void ProcessAsyncSpawn();
};

