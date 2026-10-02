// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldComponent.h"
#include "WorldTileActor.h"
#include "../City/CityActor.h"
#include "WorldSpawner.generated.h"

class UUnitManager;

// 모든 타일 액터 스폰이 끝나면 한 번 브로드캐스트합니다.
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
	// ========== 월드 / 타일 ==========

	// 타일 데이터를 들고 있는 월드 컴포넌트
	UPROPERTY(BlueprintReadWrite, Category = "World")
	UWorldComponent* WorldComponent;

	// hex 좌표 → 스폰된 타일 액터
	UPROPERTY(BlueprintReadOnly, Category = "World")
	TMap<FVector2D, AWorldTileActor*> TileActors;

	// 스폰할 타일 액터 클래스. 블루프린트에서 지정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
	TSubclassOf<AWorldTileActor> TileActorClass;

	// 타일 스폰이 끝나면 브로드캐스트합니다.
	UPROPERTY(BlueprintAssignable, Category = "World Events")
	FOnTileSpawnCompleted OnTileSpawnCompleted;

	// WorldComponent의 모든 타일을 액터로 스폰합니다.
	UFUNCTION(BlueprintCallable, Category = "World")
	void SpawnAllTiles();

	// 타일 데이터 하나로 액터를 스폰합니다.
	UFUNCTION(BlueprintCallable, Category = "World")
	AWorldTileActor* SpawnTileActor(UWorldTile* TileData);

	// 해당 hex 타일 외형을 다시 맞춥니다.
	UFUNCTION(BlueprintCallable, Category = "World")
	void UpdateTileVisual(FVector2D HexPosition);

	// 해당 hex의 타일 액터를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "World")
	AWorldTileActor* GetTileActorAtHex(FVector2D HexPosition) const;

	// 모든 타일 액터를 제거합니다.
	UFUNCTION(BlueprintCallable, Category = "World")
	void ClearAllTiles();

	// ========== 도시 ==========

	// 스폰할 도시 액터 클래스. 블루프린트에서 지정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
	TSubclassOf<ACityActor> CityActorClass;

	// hex 좌표 → 스폰된 도시 액터
	UPROPERTY(BlueprintReadOnly, Category = "City")
	TMap<FVector2D, ACityActor*> CityActors;

	// StartingCityHexes 기준으로 시작 도시를 모두 스폰합니다.
	UFUNCTION(BlueprintCallable, Category = "City")
	void SpawnAllCities();

	// 해당 hex에 도시 액터를 스폰합니다.
	UFUNCTION(BlueprintCallable, Category = "City")
	ACityActor* SpawnCityActorAtHex(FVector2D Hex);

	// 해당 hex의 도시 액터를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "City")
	ACityActor* GetCityActorAtHex(FVector2D Hex) const;

	// 해당 hex의 도시 액터만 파괴합니다. 패배한 슬롯 도시 제거에 씁니다.
	UFUNCTION(BlueprintCallable, Category = "City")
	void DestroyCityActorAtHex(FVector2D Hex);

	// 모든 도시 액터를 제거합니다.
	UFUNCTION(BlueprintCallable, Category = "City")
	void ClearAllCities();

	// 시작 도시를 문명 슬롯에 나눠 줍니다.
	UFUNCTION(BlueprintCallable, Category = "City")
	void AssignCitiesToPlayers();

	// ========== 세이브 / 로드 ==========

	// 세이브 데이터로 매니저·플레이어·시설을 복원합니다.
	UFUNCTION(BlueprintCallable, Category = "Save Load")
	void RestoreGameStateFromSave(const struct FGameSaveData& SaveData);

protected:
	// 도시 배정 후 카메라를 내 도시로 옮깁니다.
	void MoveCameraToPlayerCity();

	// ========== 비동기 스폰 ==========

	// 지금 타일을 스폰 중인지 여부
	UPROPERTY(BlueprintReadOnly, Category = "World")
	bool bIsSpawning;

	// 비동기 스폰 타이머
	FTimerHandle SpawnTimerHandle;

	// 아직 스폰하지 않은 타일
	TArray<UWorldTile*> TilesToSpawn;

	// 다음에 스폰할 타일 인덱스
	int32 CurrentSpawnIndex;

	// 한 프레임에 타일을 나눠 스폰합니다.
	void ProcessAsyncSpawn();

	// 시설이 바뀌면 자원 메시 표시를 맞춥니다.
	UFUNCTION()
	void OnFacilityChanged(FVector2D TileCoordinate);
};
