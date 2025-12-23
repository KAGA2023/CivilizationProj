// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FacilityStruct.h"
#include "FacilityActor.generated.h"

UCLASS()
class CIVILIZATION_API AFacilityActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AFacilityActor();

protected:
	virtual void BeginPlay() override;

public:	
	// 루트 씬 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Facility")
	USceneComponent* RootSceneComponent;

	// 시설 메시 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Facility")
	UStaticMeshComponent* FacilityMesh;

	// 약탈 여부
	UPROPERTY(BlueprintReadOnly, Category = "Facility")
	bool bIsPillaged = false;

	// 시설 메시 설정 (FacilityManager에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Facility")
	void SetFacilityMesh(UStaticMesh* InFacilityMesh, UStaticMesh* InPillagedMesh = nullptr);

	// 약탈 상태 설정
	UFUNCTION(BlueprintCallable, Category = "Facility")
	void SetPillaged(bool bInIsPillaged);

	// 시설 외형 업데이트
	UFUNCTION(BlueprintCallable, Category = "Facility")
	void UpdateVisual();

	// 약탈 상태 복구
	UFUNCTION(BlueprintCallable, Category = "Facility")
	void RepairFacility();

protected:
	// 시설 메시 (정상 상태)
	UPROPERTY()
	UStaticMesh* FacilityMeshAsset = nullptr;

	// 시설 메시 (약탈 상태)
	UPROPERTY()
	UStaticMesh* PillagedMeshAsset = nullptr;
};

