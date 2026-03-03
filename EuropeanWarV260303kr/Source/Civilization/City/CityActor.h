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
	// 루트 씬 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City")
	USceneComponent* RootSceneComponent;

	// 도시 메시 컴포넌트(도시 외형)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City")
	UStaticMeshComponent* CityMesh;

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

	// 도시 체력 비율에 따른 스모그 파티클 visible 갱신 (75% / 50% / 25% 이하 시 1/2/3개 표시, 초과 시 invisible)
	UFUNCTION(BlueprintCallable, Category = "City")
	void UpdateCitySmogVisibility(int32 CurrentHP, int32 MaxHP);

	// 머리 위 SmallCityUI 갱신 (국가 이름/이미지, 체력바) — UpdateCitySmogVisibility 호출처에서 함께 호출
	UFUNCTION(BlueprintCallable, Category = "City")
	void UpdateSmallCityUI(const FString& CountryName, class UTexture2D* CountryTexture, int32 CurrentHP, int32 MaxHP);

	// SmallCityUI 위젯 접근
	UFUNCTION(BlueprintCallable, Category = "City")
	USmallCityUI* GetSmallCityUI() const;

private:
	// 도시 머리 위 UI 위젯 컴포넌트 (유닛 SmallUnitUI와 동일 방식)
	UPROPERTY(VisibleAnywhere, Category = "City UI")
	UWidgetComponent* SmallCityWidgetComponent = nullptr;

	// 스모그 파티클 3개: (-100,0,0), (0,-100,0), (0,100,0) — 타일 약탈과 동일 위치
	UPROPERTY(VisibleAnywhere, Category = "City Smog")
	UParticleSystemComponent* SmogParticle1 = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "City Smog")
	UParticleSystemComponent* SmogParticle2 = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "City Smog")
	UParticleSystemComponent* SmogParticle3 = nullptr;
};


