// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldStruct.h"
#include "WorldTileActor.generated.h"

/**
 * 월드 타일을 나타내는 3D 액터
 * UWorldTile 데이터를 기반으로 비주얼을 표시하고 클릭 이벤트를 처리합니다.
 */
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

	// 타일 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	UStaticMeshComponent* TileMesh;

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

protected:
	// 선택 상태
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	bool bIsSelected;

	// 머티리얼 인스턴스 (다이나믹)
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

	// 선택 시 하이라이트 색상
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	FLinearColor HighlightColor;

public:
	// 기본 머티리얼 (public으로 변경)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UMaterialInterface* BaseMaterial;

	// 지형별 스태틱 메시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UStaticMesh* OceanMesh;

	// 기후대별 스태틱 메시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UStaticMesh* TemperateMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UStaticMesh* DesertMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UStaticMesh* TundraMesh;
};

