// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../WorldStruct.h"
#include "CityComponent.h"
#include "CityActor.generated.h"

UCLASS()
class CIVILIZATION_API ACityActor : public AActor
{
	GENERATED_BODY()

public:
	ACityActor();

protected:
	virtual void BeginPlay() override;

public:
	// 루트 씬 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City")
	USceneComponent* RootSceneComponent;

	// 도시 메시 컴포넌트(도시 외형)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City")
	UStaticMeshComponent* CityMesh;

	// 도시 로직 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City")
	UCityComponent* CityComponent;

	// 선택 상태
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
	bool bIsSelected;

	// 초기화: 도시 데이터 설정
	UFUNCTION(BlueprintCallable, Category = "City")
	void InitializeCity(const FCityData& InCityData);

	// 도시 외형 갱신(레벨, 선택 상태 등 반영)
	UFUNCTION(BlueprintCallable, Category = "City")
	void UpdateVisual();

	// 선택/해제
	UFUNCTION(BlueprintCallable, Category = "City")
	void SetSelected(bool bSelected);

	// 접근자
	UFUNCTION(BlueprintCallable, Category = "City")
	UCityComponent* GetCityComponent() const { return CityComponent; }
};


