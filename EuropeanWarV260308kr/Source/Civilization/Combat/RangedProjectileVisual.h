// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RangedProjectileVisual.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;

/**
 * 원거리 전투용 발사체 시각 액터.
 * 시작 위치에서 목표 위치로 일정 속도로 직선 이동하고, 방어자 메시에 닿으면 Destroy.
 */
UCLASS()
class CIVILIZATION_API ARangedProjectileVisual : public AActor
{
	GENERATED_BODY()

public:
	ARangedProjectileVisual();

	virtual void Tick(float DeltaTime) override;

	/** 발사: 시작 위치, 목표 위치, 이동 속도(유닛/초). 직선으로 일정 속도 이동, 목표 도달 시 Destroy. */
	UFUNCTION(BlueprintCallable, Category = "Ranged Projectile")
	void InitProjectile(FVector StartLocation, FVector TargetLocation, float SpeedUnitsPerSecond);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	UNiagaraComponent* ProjectileEffect;

	/** 도착 판정 거리(이 거리 이내로 들어오면 Destroy) */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile", meta = (ClampMin = "1.0"))
	float ArrivalDistance = 30.0f;

	FVector StartLoc;
	FVector TargetLoc;
	FVector Direction;
	float Speed = 0.0f;
	bool bInitialized = false;
};
