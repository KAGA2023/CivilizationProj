// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldStruct.h"
#include "WorldTileActor.generated.h"

// 전방 선언
class AUnitCharacterBase;

// 플레이어 0의 도시 타일 클릭 이벤트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerCityTileClicked);

// 건설자 클릭 이벤트 (이 타일에서 플레이어의 건설자 감지 시 브로드캐스트)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuilderTileClicked, UWorldTile*, Tile, FVector2D, TileCoordinate);

// 일반 타일 클릭 이벤트 (건설자/도시 타일이 아닌 경우)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGeneralTileClicked, FVector2D, TileCoordinate);

// 전투 타일 호버 시작 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatTileHoverBegin, UWorldTile*, Tile);

// 전투 타일 호버 종료 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatTileHoverEnd, UWorldTile*, Tile);

// 타일 호버 시작 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTileHoverBegin, UWorldTile*, Tile);

// 타일 호버 종료 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTileHoverEnd, UWorldTile*, Tile);

UCLASS()
class CIVILIZATION_API AWorldTileActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AWorldTileActor();

protected:
	virtual void BeginPlay() override;

public:
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

	// 타일 위젯 컴포넌트 (구매 가능 타일 하이라이트용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	class UWidgetComponent* TileWidget;

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

	// 호버 이벤트
	UFUNCTION()
	void OnBeginCursorOver(UPrimitiveComponent* TouchedComponent);

	UFUNCTION()
	void OnEndCursorOver(UPrimitiveComponent* TouchedComponent);

	// WorldComponent 접근을 위한 헬퍼 함수
	UFUNCTION(BlueprintCallable, Category = "Tile")
	class UWorldComponent* GetWorldComponent() const;

	// 타일 선택/해제
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void SetSelected(bool bSelected);

	// 구매 가능 타일 하이라이트 설정
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void SetPurchaseableHighlight(bool bHighlight);

	// 자원 메시 가져오기 (데이터테이블에서)
	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetStrategicResourceMesh(EStrategicResource Resource) const;

	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetBonusResourceMesh(EBonusResource Resource) const;

	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetLuxuryResourceMesh(ELuxuryResource Resource) const;

	// 숲+자원 합본 메시 가져오기 (데이터테이블에서)
	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetBonusForestResourceMesh(EBonusResource Resource) const;

	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetStrategicForestResourceMesh(EStrategicResource Resource) const;

	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetLuxuryForestResourceMesh(ELuxuryResource Resource) const;

	// 기후/숲 메시 가져오기 (데이터테이블에서)
	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetClimateTileMesh(EClimateType Climate) const;

	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetClimateForestMesh(EClimateType Climate) const;

	// 자원 메시 표시/숨김 제어
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void SetResourceVisibility(bool bVisible);

	// 숲 메시 표시/숨김 제어 (시설 건설 시 자원 메시와 동일하게 처리)
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void SetForestVisibility(bool bVisible);

	// Custom Depth Stencil 제어 함수들 (타일 밝기 조절)
	UFUNCTION(BlueprintCallable, Category = "Tile Brightness")
	void SetTileBrightness(int32 StencilValue); // 0: 보통, 1: 밝게, 2: 어둡게

	UFUNCTION(BlueprintCallable, Category = "Tile Brightness")
	void EnableCustomDepth(bool bEnable); // Custom Depth 활성화/비활성화

	UFUNCTION(BlueprintCallable, Category = "Tile Brightness")
	void ResetBrightness(); // 밝기 초기화 (Stencil 0)

protected:
	// 선택 상태
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	bool bIsSelected;

	// 구매 가능 타일 하이라이트 상태
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	bool bIsPurchaseableHighlighted = false;

public:
	// 바다 메시 (데이터테이블이 없는 경우를 위한 기본값)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UStaticMesh* OceanMesh;

	// 플레이어 0의 도시 타일 클릭 이벤트
	UPROPERTY(BlueprintAssignable, Category = "City Events")
	FOnPlayerCityTileClicked OnPlayerCityTileClicked;

	// 건설자 클릭 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Builder Events")
	FOnBuilderTileClicked OnBuilderTileClicked;

	// 일반 타일 클릭 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Tile Events")
	FOnGeneralTileClicked OnGeneralTileClicked;

	// 전투 타일 호버 시작 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Combat Events")
	FOnCombatTileHoverBegin OnCombatTileHoverBegin;

	// 전투 타일 호버 종료 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Combat Events")
	FOnCombatTileHoverEnd OnCombatTileHoverEnd;

	// 타일 호버 시작 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Tile Events")
	FOnTileHoverBegin OnTileHoverBegin;

	// 타일 호버 종료 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Tile Events")
	FOnTileHoverEnd OnTileHoverEnd;
};

