// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldStruct.h"
#include "WorldTileActor.generated.h"

UCLASS()
class CIVILIZATION_API AWorldTileActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AWorldTileActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// 루트 씬 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	USceneComponent* RootSceneComponent;

	// 타일 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	UStaticMeshComponent* TileMesh;

	// 숲 메시 컴포넌트 (타일 위에 표시)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	UStaticMeshComponent* ForestMesh;

	// 자원 메시 컴포넌트 (타일 위에 표시)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	UStaticMeshComponent* ResourceMesh;

	// 참조하는 타일 데이터
	UPROPERTY(BlueprintReadOnly, Category = "Tile")
	UWorldTile* TileData;

	// 타일 데이터 설정
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void SetTileData(UWorldTile* NewTileData);

	// 타일 외형 업데이트 (건물 지을 때 등)
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void UpdateVisual();

	// 클릭 이벤트
	UFUNCTION()
	void OnTileClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	// 타일 선택/해제
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void SetSelected(bool bSelected);

	// 자원 메시 가져오기 (데이터테이블에서)
	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetStrategicResourceMesh(EStrategicResource Resource) const;

	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetBonusResourceMesh(EBonusResource Resource) const;

	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetLuxuryResourceMesh(ELuxuryResource Resource) const;

	// 기후/숲 메시 가져오기 (데이터테이블에서)
	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetClimateTileMesh(EClimateType Climate) const;

	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetClimateForestMesh(EClimateType Climate) const;

protected:
	// 선택 상태
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	bool bIsSelected;

public:
	// 바다 메시 (데이터테이블이 없는 경우를 위한 기본값)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UStaticMesh* OceanMesh;
};

