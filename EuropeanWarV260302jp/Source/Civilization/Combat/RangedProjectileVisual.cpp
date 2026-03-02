// Fill out your copyright notice in the Description page of Project Settings.

#include "RangedProjectileVisual.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/SceneComponent.h"

ARangedProjectileVisual::ARangedProjectileVisual()
{
	PrimaryActorTick.bCanEverTick = true;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootSceneComponent);

	ProjectileEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileEffect"));
	ProjectileEffect->SetupAttachment(RootSceneComponent);

	// 나이아가라 에셋 경로 (에디터에서 덮어쓸 수 있음)
	static const TCHAR* NiagaraPath = TEXT("/Game/Civilization/Particle/NS_HolyArrow.NS_HolyArrow");
	if (UNiagaraSystem* System = LoadObject<UNiagaraSystem>(nullptr, NiagaraPath))
	{
		ProjectileEffect->SetAsset(System);
	}
}

void ARangedProjectileVisual::BeginPlay()
{
	Super::BeginPlay();
}

void ARangedProjectileVisual::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bInitialized || Speed <= 0.0f)
	{
		return;
	}

	FVector Current = GetActorLocation();
	FVector NewPos = Current + Direction * Speed * DeltaTime;
	SetActorLocation(NewPos);

	float DistToTarget = FVector::Dist(NewPos, TargetLoc);
	if (DistToTarget <= ArrivalDistance)
	{
		Destroy();
	}
}

void ARangedProjectileVisual::InitProjectile(FVector StartLocation, FVector TargetLocation, float SpeedUnitsPerSecond)
{
	StartLoc = StartLocation;
	TargetLoc = TargetLocation;
	Speed = FMath::Max(1.0f, SpeedUnitsPerSecond);

	FVector Delta = TargetLoc - StartLoc;
	float Len = Delta.Length();
	if (Len > KINDA_SMALL_NUMBER)
	{
		Direction = Delta / Len;
	}
	else
	{
		Direction = FVector::ForwardVector;
	}

	SetActorLocation(StartLoc);
	SetActorRotation(Direction.Rotation());
	bInitialized = true;

	if (ProjectileEffect)
	{
		ProjectileEffect->Activate(true);
	}
}
