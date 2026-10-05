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

	// 목표를 향해 직선 이동하며, 도착하면 Destroy합니다.
	virtual void Tick(float DeltaTime) override;

	// 시작·목표 위치와 속도로 발사체를 초기화합니다. 목표에 닿으면 Destroy합니다.
	UFUNCTION(BlueprintCallable, Category = "Ranged Projectile")
	void InitProjectile(FVector StartLocation, FVector TargetLocation, float SpeedUnitsPerSecond);

protected:
	virtual void BeginPlay() override;

	// ========== 컴포넌트 ==========

	// 루트 씬 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	USceneComponent* RootSceneComponent;

	// 화살 파티클 이펙트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	UNiagaraComponent* ProjectileEffect;

	// ========== 이동 ==========

	// 이 거리 안으로 들어오면 도착으로 보고 Destroy합니다.
	UPROPERTY(EditDefaultsOnly, Category = "Projectile", meta = (ClampMin = "1.0"))
	float ArrivalDistance = 30.0f;

	// 발사 시작 위치
	FVector StartLoc;

	// 목표 위치
	FVector TargetLoc;

	// 정규화된 이동 방향
	FVector Direction;

	// 초당 이동 속도
	float Speed = 0.0f;

	// InitProjectile을 호출했는지 여부
	bool bInitialized = false;
};
