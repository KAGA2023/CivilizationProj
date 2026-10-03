// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldStruct.h"
#include "WorldTileActor.generated.h"

// 전방 선언
class AUnitCharacterBase;

// 호스트/싱글(슬롯 0) 도시 타일을 클릭하면 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerCityTileClicked);

// 이 타일에서 내 건설자를 클릭하면 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuilderTileClicked, UWorldTile*, Tile, FVector2D, TileCoordinate);

// 건설자·도시가 아닌 일반 타일 클릭을 알립니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGeneralTileClicked, FVector2D, TileCoordinate);

// 전투 타일 호버를 시작합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatTileHoverBegin, UWorldTile*, Tile);

// 전투 타일 호버를 끝냅니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatTileHoverEnd, UWorldTile*, Tile);

// 타일 호버를 시작합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTileHoverBegin, UWorldTile*, Tile);

// 타일 호버를 끝냅니다.
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
	// ========== 컴포넌트 ==========

	// 루트 씬 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	USceneComponent* RootSceneComponent;

	// 타일 바닥 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	UStaticMeshComponent* TileMesh;

	// 타일 위 숲 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	UStaticMeshComponent* ForestMesh;

	// 타일 위 자원 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	UStaticMeshComponent* ResourceMesh;

	// 구매 가능 타일 하이라이트용 위젯
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tile")
	class UWidgetComponent* TileWidget;

	// ========== 데이터 ==========

	// 이 액터가 보여주는 타일 데이터
	UPROPERTY(BlueprintReadOnly, Category = "Tile")
	UWorldTile* TileData;

	// 타일 데이터를 연결하고 외형을 맞춥니다.
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void SetTileData(UWorldTile* NewTileData);

	// 건물·자원 변경을 외형에 반영합니다.
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void UpdateVisual();

	// ========== 입력 ==========

	// 타일 클릭을 받아 도시·건설자·일반 클릭으로 나눕니다.
	UFUNCTION()
	void OnTileClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

	// 커서가 타일 위에 올라오면 호버를 시작합니다.
	UFUNCTION()
	void OnBeginCursorOver(UPrimitiveComponent* TouchedComponent);

	// 커서가 타일에서 벗어나면 호버를 끝냅니다.
	UFUNCTION()
	void OnEndCursorOver(UPrimitiveComponent* TouchedComponent);

	// 이 타일이 속한 WorldComponent를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Tile")
	class UWorldComponent* GetWorldComponent() const;

	// ========== 선택 / 하이라이트 ==========

	// 선택 여부를 바꾸고 외형을 맞춥니다.
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void SetSelected(bool bSelected);

	// 구매 가능 하이라이트를 켜거나 끕니다.
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void SetPurchaseableHighlight(bool bHighlight);

	// ========== 메시 조회 ==========

	// 데이터 테이블에서 전략 자원 메시를 가져옵니다.
	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetStrategicResourceMesh(EStrategicResource Resource) const;

	// 데이터 테이블에서 보너스 자원 메시를 가져옵니다.
	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetBonusResourceMesh(EBonusResource Resource) const;

	// 데이터 테이블에서 사치 자원 메시를 가져옵니다.
	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetLuxuryResourceMesh(ELuxuryResource Resource) const;

	// 숲+보너스 자원 합본 메시를 가져옵니다.
	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetBonusForestResourceMesh(EBonusResource Resource) const;

	// 숲+전략 자원 합본 메시를 가져옵니다.
	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetStrategicForestResourceMesh(EStrategicResource Resource) const;

	// 숲+사치 자원 합본 메시를 가져옵니다.
	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetLuxuryForestResourceMesh(ELuxuryResource Resource) const;

	// 기후 타일 바닥 메시를 가져옵니다.
	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetClimateTileMesh(EClimateType Climate) const;

	// 기후 숲 메시를 가져옵니다.
	UFUNCTION(BlueprintCallable, Category = "Tile")
	UStaticMesh* GetClimateForestMesh(EClimateType Climate) const;

	// ========== 표시 / 밝기 ==========

	// 자원 메시를 보이거나 숨깁니다.
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void SetResourceVisibility(bool bVisible);

	// 숲 메시를 보이거나 숨깁니다. 시설 건설 시 자원과 같이 처리합니다.
	UFUNCTION(BlueprintCallable, Category = "Tile")
	void SetForestVisibility(bool bVisible);

	// Custom Depth Stencil로 밝기를 바꿉니다. 0=보통, 1=밝게, 2=어둡게.
	UFUNCTION(BlueprintCallable, Category = "Tile Brightness")
	void SetTileBrightness(int32 StencilValue);

	// Custom Depth를 켜거나 끕니다.
	UFUNCTION(BlueprintCallable, Category = "Tile Brightness")
	void EnableCustomDepth(bool bEnable);

	// 밝기를 Stencil 0으로 되돌립니다.
	UFUNCTION(BlueprintCallable, Category = "Tile Brightness")
	void ResetBrightness();

protected:
	// ========== 내부 상태 ==========

	// 지금 선택된 타일인지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	bool bIsSelected;

	// 구매 가능 하이라이트가 켜져 있는지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	bool bIsPurchaseableHighlighted = false;

public:
	// ========== 기본 메시 / 이벤트 ==========

	// 데이터 테이블이 없을 때 쓰는 바다 메시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile")
	UStaticMesh* OceanMesh;

	// 호스트/싱글(슬롯 0) 도시 타일 클릭
	UPROPERTY(BlueprintAssignable, Category = "City Events")
	FOnPlayerCityTileClicked OnPlayerCityTileClicked;

	// 내 건설자가 있는 타일 클릭
	UPROPERTY(BlueprintAssignable, Category = "Builder Events")
	FOnBuilderTileClicked OnBuilderTileClicked;

	// 일반 타일 클릭
	UPROPERTY(BlueprintAssignable, Category = "Tile Events")
	FOnGeneralTileClicked OnGeneralTileClicked;

	// 전투 타일 호버 시작
	UPROPERTY(BlueprintAssignable, Category = "Combat Events")
	FOnCombatTileHoverBegin OnCombatTileHoverBegin;

	// 전투 타일 호버 종료
	UPROPERTY(BlueprintAssignable, Category = "Combat Events")
	FOnCombatTileHoverEnd OnCombatTileHoverEnd;

	// 타일 호버 시작
	UPROPERTY(BlueprintAssignable, Category = "Tile Events")
	FOnTileHoverBegin OnTileHoverBegin;

	// 타일 호버 종료
	UPROPERTY(BlueprintAssignable, Category = "Tile Events")
	FOnTileHoverEnd OnTileHoverEnd;
};
