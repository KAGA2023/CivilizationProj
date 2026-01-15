// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldComponent.h"
#include "WorldTileActor.h"
#include "../City/CityActor.h"
#include "WorldSpawner.generated.h"

class UUnitManager;

// 타일 스폰 완료 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTileSpawnCompleted);

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

	// ================= 도시 스폰/관리 =================
	// 도시 액터 클래스 (블루프린트에서 설정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
	TSubclassOf<ACityActor> CityActorClass;

	// 스폰된 도시 액터들 (육각형 좌표 → 액터)
	UPROPERTY(BlueprintReadOnly, Category = "City")
	TMap<FVector2D, ACityActor*> CityActors;

	// 시작 도시 전부 스폰 (WorldComponent의 StartingCityHexes 기준)
	UFUNCTION(BlueprintCallable, Category = "City")
	void SpawnAllCities();

	// 특정 좌표에 도시 스폰
	UFUNCTION(BlueprintCallable, Category = "City")
	ACityActor* SpawnCityActorAtHex(FVector2D Hex);

	// 특정 좌표의 도시 액터 가져오기
	UFUNCTION(BlueprintCallable, Category = "City")
	ACityActor* GetCityActorAtHex(FVector2D Hex) const;

	// 모든 도시 액터 제거
	UFUNCTION(BlueprintCallable, Category = "City")
	void ClearAllCities();

	// 도시를 플레이어들에게 배정
	UFUNCTION(BlueprintCallable, Category = "City")
	void AssignCitiesToPlayers();

protected:
	// 도시 배정 후 카메라를 플레이어 0의 도시로 이동
	void MoveCameraToPlayerCity();

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

	// 시설 변경 델리게이트 콜백 (자원 메시 제어용)
	UFUNCTION()
	void OnFacilityChanged(FVector2D TileCoordinate);
};

