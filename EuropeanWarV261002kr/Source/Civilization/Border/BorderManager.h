// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../World/WorldStruct.h"
#include "BorderManager.generated.h"

class UProceduralMeshComponent;
class UStaticMeshComponent;
class UStaticMesh;
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

	// ========== 초기화 / 갱신 ==========

	// WorldComponent와 메시를 붙일 부모 액터를 연결합니다.
	UFUNCTION(BlueprintCallable, Category = "Border")
	void Initialize(UWorldComponent* InWorldComponent, AActor* InParentActor);

	// 해당 슬롯의 소유 타일 국경선을 다시 그립니다.
	UFUNCTION(BlueprintCallable, Category = "Border")
	void UpdatePlayerBorder(int32 PlayerIndex, const TArray<FVector2D>& OwnedTileCoordinates);

	// 모든 슬롯의 국경선을 다시 그립니다.
	UFUNCTION(BlueprintCallable, Category = "Border")
	void UpdateAllBorders();

	// 해당 슬롯의 국경선을 지웁니다.
	UFUNCTION(BlueprintCallable, Category = "Border")
	void RemovePlayerBorder(int32 PlayerIndex);

	// 모든 국경선을 지웁니다.
	UFUNCTION(BlueprintCallable, Category = "Border")
	void ClearAllBorders();

	// ========== 색상 ==========

	// 해당 슬롯 국경선 색을 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Border")
	void SetPlayerColor(int32 PlayerIndex, FLinearColor Color);

	// 해당 슬롯 국경선 색을 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Border")
	FLinearColor GetPlayerColor(int32 PlayerIndex) const;

protected:
	// ========== 참조 ==========

	// 타일 좌표를 월드 위치로 바꿀 때 쓰는 월드 컴포넌트
	UPROPERTY()
	TObjectPtr<UWorldComponent> WorldComponent;

	// ProceduralMeshComponent를 붙일 부모 액터
	UPROPERTY()
	TObjectPtr<AActor> ParentActor;

	// ========== 슬롯별 메시 ==========

	// 슬롯별 국경선 프로시저럴 메시
	UPROPERTY()
	TMap<int32, TObjectPtr<UProceduralMeshComponent>> PlayerBorderMeshes;

	// 슬롯별 국경 꼭짓점 구. TMap 값 제한으로 UPROPERTY를 쓰지 않고 ParentActor 자식으로 둡니다.
	TMap<int32, TArray<UStaticMeshComponent*>> PlayerBorderSphereComponents;

	// 슬롯별 색상용 머티리얼 인스턴스
	UPROPERTY()
	TMap<int32, TObjectPtr<UMaterialInstanceDynamic>> PlayerMaterials;

	// 슬롯별 국경선 색
	UPROPERTY()
	TMap<int32, FLinearColor> PlayerColors;

	// ========== 기본 에셋 / 수치 ==========

	// 슬롯 머티리얼이 없을 때 쓰는 기본 머티리얼
	UPROPERTY(EditAnywhere, Category = "Border")
	TObjectPtr<UMaterialInterface> DefaultBorderMaterial;

	// 꼭짓점에 찍는 기본 구 메시
	UPROPERTY()
	TObjectPtr<UStaticMesh> DefaultSphereMesh;

	// 꼭짓점 구 크기. 국경선 두께에 비례합니다.
	UPROPERTY(EditAnywhere, Category = "Border")
	float SphereScale = 0.5f;

	// 국경선 두께 (월드 단위)
	UPROPERTY(EditAnywhere, Category = "Border")
	float BorderThickness = 10.0f;

	// 타일 위로 띄우는 높이
	UPROPERTY(EditAnywhere, Category = "Border")
	float BorderHeightOffset = 10.0f;

	// 발광 강도. 라이팅 영향을 줄이려고 높게 둡니다.
	UPROPERTY(EditAnywhere, Category = "Border")
	float EmissiveIntensity = 50.0f;

private:
	// ========== 메시 생성 ==========

	// 인접 타일이 같은 슬롯 소유가 아니면 외곽 엣지로 봅니다.
	bool IsOuterEdge(const FVector2D& TileCoord, int32 EdgeIndex, const TArray<FVector2D>& OwnedTileCoordinates);

	// 해당 엣지 너머 이웃 hex를 반환합니다.
	FVector2D GetNeighborHexAtEdge(const FVector2D& HexCoord, int32 EdgeIndex);

	// hex를 월드 위치로 바꿉니다.
	FVector HexToWorldPosition(const FVector2D& HexCoord, float HeightOffset = 0.0f);

	// pointy-top 헥스의 꼭짓점 6개를 계산합니다.
	TArray<FVector> GetHexVertices(const FVector2D& HexCoord, float HeightOffset);

	// 해당 슬롯의 국경 메시를 찾거나 만듭니다.
	UProceduralMeshComponent* GetOrCreateBorderMesh(int32 PlayerIndex);

	// 정점·삼각형으로 국경 메시를 채웁니다. 색은 MaterialInstanceDynamic이 담당합니다.
	void GenerateBorderMesh(int32 PlayerIndex, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, const TArray<FVector>& Normals, const TArray<FVector2D>& UVs);

	// 꼭짓점마다 국경선과 같은 머티리얼의 구를 만듭니다.
	void CreateSpheresAtVertices(int32 PlayerIndex, const TArray<FVector>& CornerPositions);

	// 비슷한 위치가 없으면 꼭짓점 목록에 넣습니다.
	void AddBorderCornerIfNew(TArray<FVector>& InOutCornerList, const FVector& NewPosition);
};
