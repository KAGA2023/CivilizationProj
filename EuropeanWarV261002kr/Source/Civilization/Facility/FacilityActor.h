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
	// ========== 컴포넌트 ==========

	// 루트 씬 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Facility")
	USceneComponent* RootSceneComponent;

	// 시설 외형 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Facility")
	UStaticMeshComponent* FacilityMesh;

	// ========== 상태 ==========

	// 지금 약탈된 상태인지 여부
	UPROPERTY(BlueprintReadOnly, Category = "Facility")
	bool bIsPillaged = false;

	// ========== 외형 ==========

	// 정상·약탈 메시를 저장합니다. FacilityManager가 호출합니다.
	UFUNCTION(BlueprintCallable, Category = "Facility")
	void SetFacilityMesh(UStaticMesh* InFacilityMesh, UStaticMesh* InPillagedMesh = nullptr);

	// 약탈 여부를 바꾸고 외형을 갱신합니다.
	UFUNCTION(BlueprintCallable, Category = "Facility")
	void SetPillaged(bool bInIsPillaged);

	// 약탈 여부에 맞는 메시로 외형을 바꿉니다.
	UFUNCTION(BlueprintCallable, Category = "Facility")
	void UpdateVisual();

	// 약탈을 해제하고 정상 메시로 되돌립니다.
	UFUNCTION(BlueprintCallable, Category = "Facility")
	void RepairFacility();

protected:
	// ========== 메시 에셋 ==========

	// 정상 상태 시설 메시
	UPROPERTY()
	UStaticMesh* FacilityMeshAsset = nullptr;

	// 약탈 상태 시설 메시
	UPROPERTY()
	UStaticMesh* PillagedMeshAsset = nullptr;
};
