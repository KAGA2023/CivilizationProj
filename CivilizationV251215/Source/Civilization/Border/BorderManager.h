// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../World/WorldStruct.h"
#include "BorderManager.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UWorldComponent;
class AActor;

UCLASS()
class CIVILIZATION_API UBorderManager : public UObject
{
	GENERATED_BODY()

public:
	UBorderManager();

	// 초기화 (WorldComponent 참조 및 부모 액터 설정)
	UFUNCTION(BlueprintCallable, Category = "Border")
	void Initialize(UWorldComponent* InWorldComponent, AActor* InParentActor);

	// 플레이어의 국경선 업데이트 (소유 타일 좌표 배열 전달)
	UFUNCTION(BlueprintCallable, Category = "Border")
	void UpdatePlayerBorder(int32 PlayerIndex, const TArray<FVector2D>& OwnedTileCoordinates);

	// 모든 플레이어의 국경선 업데이트
	UFUNCTION(BlueprintCallable, Category = "Border")
	void UpdateAllBorders();

	// 특정 플레이어의 국경선 제거
	UFUNCTION(BlueprintCallable, Category = "Border")
	void RemovePlayerBorder(int32 PlayerIndex);

	// 모든 국경선 제거
	UFUNCTION(BlueprintCallable, Category = "Border")
	void ClearAllBorders();

	// 플레이어 색상 설정
	UFUNCTION(BlueprintCallable, Category = "Border")
	void SetPlayerColor(int32 PlayerIndex, FLinearColor Color);

	// 플레이어 색상 가져오기
	UFUNCTION(BlueprintCallable, Category = "Border")
	FLinearColor GetPlayerColor(int32 PlayerIndex) const;

protected:
	// WorldComponent 참조
	UPROPERTY()
	TObjectPtr<UWorldComponent> WorldComponent;

	// 부모 액터 (ProceduralMeshComponent 부착용)
	UPROPERTY()
	TObjectPtr<AActor> ParentActor;

	// 플레이어별 ProceduralMeshComponent
	UPROPERTY()
	TMap<int32, TObjectPtr<UProceduralMeshComponent>> PlayerBorderMeshes;

	// 플레이어별 MaterialInstanceDynamic (색상 설정용)
	UPROPERTY()
	TMap<int32, TObjectPtr<UMaterialInstanceDynamic>> PlayerMaterials;

	// 플레이어별 색상
	UPROPERTY()
	TMap<int32, FLinearColor> PlayerColors;

	// 기본 머티리얼 (플레이어별 머티리얼이 없을 때 사용)
	UPROPERTY(EditAnywhere, Category = "Border")
	TObjectPtr<UMaterialInterface> DefaultBorderMaterial;

	// 국경선 두께 (월드 단위)
	UPROPERTY(EditAnywhere, Category = "Border")
	float BorderThickness = 10.0f;

	// 국경선 높이 오프셋 (타일 위에 표시)
	UPROPERTY(EditAnywhere, Category = "Border")
	float BorderHeightOffset = 10.0f;

	// Emissive 색상 강도 (라이팅 영향 최소화를 위해 높게 설정)
	UPROPERTY(EditAnywhere, Category = "Border")
	float EmissiveIntensity = 20.0f;

private:
	// 엣지가 외곽 엣지인지 확인 (인접 타일이 같은 플레이어 소유인지 체크)
	bool IsOuterEdge(const FVector2D& TileCoord, int32 EdgeIndex, const TArray<FVector2D>& OwnedTileCoordinates);

	// 헥스 좌표의 특정 엣지의 인접 타일 좌표 가져오기
	FVector2D GetNeighborHexAtEdge(const FVector2D& HexCoord, int32 EdgeIndex);

	// 헥스 좌표를 월드 위치로 변환
	FVector HexToWorldPosition(const FVector2D& HexCoord, float HeightOffset = 0.0f);

	// Pointy top 헥스 타일의 6개 꼭짓점 계산
	TArray<FVector> GetHexVertices(const FVector2D& HexCoord, float HeightOffset);

	// ProceduralMeshComponent 생성 또는 가져오기
	UProceduralMeshComponent* GetOrCreateBorderMesh(int32 PlayerIndex);

	// 메시 생성 (플레이어별 국경선 데이터로부터)
	void GenerateBorderMesh(int32 PlayerIndex, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, const TArray<FVector>& Normals, const TArray<FVector2D>& UVs, const TArray<FLinearColor>& Colors);
};


