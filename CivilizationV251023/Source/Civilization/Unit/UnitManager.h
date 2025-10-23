// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnitManager.generated.h"

class UWorldComponent;
class AUnitCharacterBase;
class USuperGameInstance;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CIVILIZATION_API UUnitManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UUnitManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 유닛 소환 관련 함수들
    UFUNCTION(BlueprintCallable, Category = "Unit Spawning")
    class AUnitCharacterBase* SpawnUnitAtHex(FVector2D HexPosition, const FName& RowName); // 지정된 육각형 좌표에 유닛 소환

    // 유닛 관리 함수들
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    TArray<class AUnitCharacterBase*> GetAllUnits() const; // 모든 유닛 가져오기

    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void RemoveUnit(class AUnitCharacterBase* Unit); // 유닛 제거

    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void ClearAllUnits(); // 모든 유닛 제거

    // 월드 컴포넌트 설정
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void SetWorldComponent(class UWorldComponent* WorldComponent); // 월드 컴포넌트 설정

private:
    // 월드 컴포넌트 참조
    UPROPERTY()
    class UWorldComponent* WorldComponent = nullptr;

    // 소환된 유닛들 관리
    UPROPERTY()
    TArray<class AUnitCharacterBase*> SpawnedUnits;

    // 내부 헬퍼 함수들
    UWorld* GetValidWorld() const; // 유효한 World 가져오기
    USuperGameInstance* GetSuperGameInstance() const; // SuperGameInstance 가져오기
};
