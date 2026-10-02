// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "FacilityStruct.h"
#include "FacilityManager.generated.h"

class UWorldTile;
class UWorldComponent;
class UParticleSystem;
class UParticleSystemComponent;
class USoundBase;

// 해당 타일 시설이 생기거나 바뀌거나 없어지면 알립니다.
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
    // ========== 시설 데이터 ==========

    // 시설 데이터 테이블을 로드합니다.
    UFUNCTION(BlueprintCallable, Category = "Facility Data")
    void LoadFacilityDataTable();

    // 시설 데이터 테이블
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility Data")
    UDataTable* FacilityDataTable = nullptr;

    // hex → 건설된 시설 데이터
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Facilities")
    TMap<FVector2D, FFacilityData> BuiltFacilities;

    // hex → 스폰된 시설 액터
    UPROPERTY()
    TMap<FVector2D, class AFacilityActor*> BuiltFacilityActors;

    // ========== 조회 ==========

    // 해당 타일에 시설이 있는지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Facility Management")
    bool HasFacilityAtTile(FVector2D TileCoordinate) const;

    // 해당 타일의 시설 데이터를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Facility Management")
    FFacilityData GetFacilityAtTile(FVector2D TileCoordinate) const;

    // 타일 조건만 보고 건설 가능한지 반환합니다. 기술 조건은 빼줍니다.
    UFUNCTION(BlueprintCallable, Category = "Facility Management")
    bool CanBuildFacilityOnTile(FName FacilityRowName, UWorldTile* Tile) const;

    // ========== 건설 ==========

    // 시설을 짓고 건설자를 제거합니다.
    UFUNCTION(BlueprintCallable, Category = "Facility Management")
    bool BuildFacility(FName FacilityRowName, FVector2D TileCoordinate, UWorldComponent* WorldComponent);

    // 건설 몽타주 시작 때 스팀 이펙트를 띄웁니다.
    UFUNCTION(BlueprintCallable, Category = "Facility Management")
    void SpawnBuildSteamAtTile(FVector2D TileCoordinate, UWorldComponent* WorldComponent);

    // ========== 제거 / 약탈 / 수리 ==========

    // 해당 타일 시설과 타일 모디파이어를 제거합니다.
    UFUNCTION(BlueprintCallable, Category = "Facility Management")
    bool DestroyFacility(FVector2D TileCoordinate, UWorldComponent* WorldComponent);

    // 해당 타일 시설의 약탈 여부를 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "Facility Management")
    bool SetFacilityPillaged(FVector2D TileCoordinate, bool bIsPillaged, UWorldComponent* WorldComponent);

    // 내 소유이고 약탈된 시설이면 수리할 수 있습니다.
    UFUNCTION(BlueprintCallable, Category = "Facility Management")
    bool CanRepairFacilityAtTile(FVector2D TileCoordinate, int32 PlayerIndex, UWorldComponent* WorldComponent) const;

    // 약탈을 해제하고 시설을 수리합니다.
    UFUNCTION(BlueprintCallable, Category = "Facility Management")
    bool RepairFacility(FVector2D TileCoordinate, UWorldComponent* WorldComponent);

    // ========== 시설 액터 ==========

    // 해당 hex에 시설 액터를 스폰합니다.
    UFUNCTION(BlueprintCallable, Category = "Facility Actor Management")
    class AFacilityActor* SpawnFacilityAtHex(FVector2D TileCoordinate, FName FacilityRowName, UWorldComponent* WorldComponent);

    // 해당 hex의 시설 액터를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Facility Actor Management")
    class AFacilityActor* GetFacilityActorAtHex(FVector2D TileCoordinate) const;

    // 해당 hex의 시설 액터를 제거합니다.
    UFUNCTION(BlueprintCallable, Category = "Facility Actor Management")
    void RemoveFacilityActorAtHex(FVector2D TileCoordinate);

    // 모든 시설 액터를 제거합니다.
    UFUNCTION(BlueprintCallable, Category = "Facility Actor Management")
    void ClearAllFacilityActors();

    // ========== 이벤트 ==========

    // 시설이 생기거나 바뀌거나 없어지면 브로드캐스트합니다.
    UPROPERTY(BlueprintAssignable, Category = "Facility Events")
    FOnFacilityChanged OnFacilityChanged;

private:
    // ========== 이펙트 ==========

    // 약탈 타일마다 연기 파티클 3개. TMap 값에 TArray를 써서 UPROPERTY는 뺍니다.
    TMap<FVector2D, TArray<UParticleSystemComponent*>> PillagedSmokeComponents;

    // 약탈 연기 파티클 에셋
    UPROPERTY()
    UParticleSystem* PillagedSmokeTemplate = nullptr;

    // 해당 타일의 약탈 연기를 지웁니다.
    void DestroyPillagedSmokeAtTile(FVector2D TileCoordinate);

    // 해당 위치에 약탈 연기를 스폰하고 맵에 넣습니다.
    void SpawnPillagedSmokeAtLocation(FVector WorldLocation, FVector2D TileCoordinate);

    // 건설 스팀 파티클 에셋
    UPROPERTY()
    UParticleSystem* BuildSteamTemplate = nullptr;

    // 건설 시작 때 타일 중심에 스팀 5개를 띄웁니다. 재생 후 자동으로 사라집니다.
    void SpawnBuildSteamAtLocation(FVector WorldLocation);

    // 약탈 때 재생할 파괴 사운드
    UPROPERTY()
    USoundBase* PillagedDestroySound = nullptr;
};
