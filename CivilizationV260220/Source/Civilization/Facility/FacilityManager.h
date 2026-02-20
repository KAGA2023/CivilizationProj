// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "FacilityStruct.h"
#include "FacilityManager.generated.h"

class UWorldTile;
class UWorldComponent;

// 시설 변경 델리게이트 (해당 타일의 시설이 생성/파괴/변경되었음을 알림)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFacilityChanged, FVector2D, TileCoordinate);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CIVILIZATION_API UFacilityManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UFacilityManager();

protected:
    virtual void BeginPlay() override;

public:
    // ========== 시설 데이터 관리 ==========
    // 시설 데이터 테이블 로드
    UFUNCTION(BlueprintCallable, Category = "Facility Data")
    void LoadFacilityDataTable();
    // 시설 데이터 테이블
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility Data")
    UDataTable* FacilityDataTable = nullptr;

    // 건설된 시설들 (타일 좌표 -> 시설 데이터)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Facilities")
    TMap<FVector2D, FFacilityData> BuiltFacilities;

    // 스폰된 시설 액터들 (타일 좌표 -> 시설 액터)
    UPROPERTY()
    TMap<FVector2D, class AFacilityActor*> BuiltFacilityActors;

    // ========== 시설 존재 여부 확인 ==========
    // 특정 타일에 시설이 있는지 확인
    UFUNCTION(BlueprintCallable, Category = "Facility Management")
    bool HasFacilityAtTile(FVector2D TileCoordinate) const;

    // 특정 타일의 시설 데이터 가져오기
    UFUNCTION(BlueprintCallable, Category = "Facility Management")
    FFacilityData GetFacilityAtTile(FVector2D TileCoordinate) const;

    // ========== 건설 가능 여부 확인 (타일 조건만) ==========
    // 특정 시설을 특정 타일에 건설할 수 있는지 확인 (타일 조건만, 기술 조건 제외)
    UFUNCTION(BlueprintCallable, Category = "Facility Management")
    bool CanBuildFacilityOnTile(FName FacilityRowName, UWorldTile* Tile) const;

    // ========== 시설 건설 실행 ==========
    // 월드/유닛 상태를 포함한 실제 건설 수행 (건설자 제거 포함)
    UFUNCTION(BlueprintCallable, Category = "Facility Management")
    bool BuildFacility(FName FacilityRowName, FVector2D TileCoordinate, UWorldComponent* WorldComponent);

    // ========== 시설 제거 ==========
	// 해당 타일의 시설 제거 (타일 모디파이어 제거 포함)
	UFUNCTION(BlueprintCallable, Category = "Facility Management")
	bool DestroyFacility(FVector2D TileCoordinate, UWorldComponent* WorldComponent);

    // ========== 시설 약탈 관리 ==========
	// 해당 타일의 시설 약탈 상태 설정
	UFUNCTION(BlueprintCallable, Category = "Facility Management")
	bool SetFacilityPillaged(FVector2D TileCoordinate, bool bIsPillaged, UWorldComponent* WorldComponent);

	// 해당 타일의 시설을 수리할 수 있는지 (자신 소유 + 시설 있음 + 약탈 상태)
	UFUNCTION(BlueprintCallable, Category = "Facility Management")
	bool CanRepairFacilityAtTile(FVector2D TileCoordinate, int32 PlayerIndex, UWorldComponent* WorldComponent) const;

	// 해당 타일의 시설 수리 (약탈 해제)
	UFUNCTION(BlueprintCallable, Category = "Facility Management")
	bool RepairFacility(FVector2D TileCoordinate, UWorldComponent* WorldComponent);

    // ========== 시설 액터 관리 ==========
    // 특정 타일에 시설 액터 스폰
    UFUNCTION(BlueprintCallable, Category = "Facility Actor Management")
    class AFacilityActor* SpawnFacilityAtHex(FVector2D TileCoordinate, FName FacilityRowName, UWorldComponent* WorldComponent);

    // 특정 타일의 시설 액터 가져오기
    UFUNCTION(BlueprintCallable, Category = "Facility Actor Management")
    class AFacilityActor* GetFacilityActorAtHex(FVector2D TileCoordinate) const;

    // 특정 타일의 시설 액터 제거
    UFUNCTION(BlueprintCallable, Category = "Facility Actor Management")
    void RemoveFacilityActorAtHex(FVector2D TileCoordinate);

    // 모든 시설 액터 제거
    UFUNCTION(BlueprintCallable, Category = "Facility Actor Management")
    void ClearAllFacilityActors();

    // ========== 델리게이트 ==========
	// 시설 변경 델리게이트 (생성/파괴 공통)
    UPROPERTY(BlueprintAssignable, Category = "Facility Events")
	FOnFacilityChanged OnFacilityChanged;
};
