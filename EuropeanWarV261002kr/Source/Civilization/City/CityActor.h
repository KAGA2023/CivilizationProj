// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystemComponent.h"
#include "../World/WorldStruct.h"
#include "CityActor.generated.h"

class UWidgetComponent;
class USmallCityUI;

UCLASS()
class CIVILIZATION_API ACityActor : public AActor
{
	GENERATED_BODY()

public:
	ACityActor();

protected:
	virtual void BeginPlay() override;

public:
	// ========== 컴포넌트 ==========

	// 루트 씬 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City")
	USceneComponent* RootSceneComponent;

	// 도시 외형 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City")
	UStaticMeshComponent* CityMesh;

	// ========== 선택 ==========

	// 지금 선택된 도시인지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
	bool bIsSelected;

	// ========== 초기화 / 외형 ==========

	// 도시 데이터로 메시와 UI를 맞춥니다.
	UFUNCTION(BlueprintCallable, Category = "City")
	void InitializeCity(const FCityData& InCityData);

	// 레벨·선택 상태를 외형에 반영합니다.
	UFUNCTION(BlueprintCallable, Category = "City")
	void UpdateVisual();

	// 선택 여부를 바꾸고 외형을 갱신합니다.
	UFUNCTION(BlueprintCallable, Category = "City")
	void SetSelected(bool bSelected);

	// 체력 비율에 따라 스모그를 켭니다. 75%/50%/25% 이하면 1/2/3개를 켭니다.
	UFUNCTION(BlueprintCallable, Category = "City")
	void UpdateCitySmogVisibility(int32 CurrentHP, int32 MaxHP);

	// 머리 위 SmallCityUI의 국가 이름·이미지·체력바를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category = "City")
	void UpdateSmallCityUI(const FString& CountryName, class UTexture2D* CountryTexture, int32 CurrentHP, int32 MaxHP);

	// 머리 위 SmallCityUI 위젯을 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "City")
	USmallCityUI* GetSmallCityUI() const;

private:
	// ========== UI / 이펙트 ==========

	// 도시 머리 위 UI. 유닛 SmallUnitUI와 같은 방식입니다.
	UPROPERTY(VisibleAnywhere, Category = "City UI")
	UWidgetComponent* SmallCityWidgetComponent = nullptr;

	// 스모그 파티클 1. 위치 (-100,0,0)
	UPROPERTY(VisibleAnywhere, Category = "City Smog")
	UParticleSystemComponent* SmogParticle1 = nullptr;

	// 스모그 파티클 2. 위치 (0,-100,0)
	UPROPERTY(VisibleAnywhere, Category = "City Smog")
	UParticleSystemComponent* SmogParticle2 = nullptr;

	// 스모그 파티클 3. 위치 (0,100,0)
	UPROPERTY(VisibleAnywhere, Category = "City Smog")
	UParticleSystemComponent* SmogParticle3 = nullptr;
};
