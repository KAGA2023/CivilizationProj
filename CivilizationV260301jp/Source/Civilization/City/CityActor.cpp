// Fill out your copyright notice in the Description page of Project Settings.

#include "CityActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "UObject/ConstructorHelpers.h"
#include "../Widget/UnitWidget/SmallCityUI.h"

ACityActor::ACityActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;

	// 도시 메시
	CityMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CityMesh"));
	CityMesh->SetupAttachment(RootSceneComponent);
	CityMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CityMesh->SetGenerateOverlapEvents(false);

	// 기본 메시 로드 및 설정
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> CityMeshAsset(TEXT("/Game/Civilization/Tiles/City.City"));
		if (CityMeshAsset.Succeeded())
		{
			CityMesh->SetStaticMesh(CityMeshAsset.Object);
		}
		// Z축(야우) 180도 회전
		//CityMesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	}

	// 도시 머리 위 UI 위젯 (유닛과 동일 위치/회전, DrawSize 250x100)
	SmallCityWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("SmallCityWidgetComponent"));
	SmallCityWidgetComponent->SetupAttachment(RootSceneComponent);
	SmallCityWidgetComponent->SetRelativeLocation(FVector(100.0f, 0.0f, 130.0f));
	SmallCityWidgetComponent->SetUsingAbsoluteRotation(true);
	SmallCityWidgetComponent->SetWorldRotation(FRotator(60.0f, 180.0f, 0.0f));
	SmallCityWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	SmallCityWidgetComponent->SetDrawSize(FVector2D(250.0f, 100.0f));
	SmallCityWidgetComponent->SetCastShadow(false);
	SmallCityWidgetComponent->SetVisibility(true);

	// 스모그 파티클 3개 (타일 약탈과 동일 위치: (-100,0,0), (0,-100,0), (0,100,0))
	SmogParticle1 = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SmogParticle1"));
	SmogParticle1->SetupAttachment(RootSceneComponent);
	SmogParticle1->SetRelativeLocation(FVector(-100.0f, 0.0f, 0.0f));
	SmogParticle1->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
	SmogParticle1->SetVisibility(false);
	SmogParticle1->bAutoActivate = false;

	SmogParticle2 = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SmogParticle2"));
	SmogParticle2->SetupAttachment(RootSceneComponent);
	SmogParticle2->SetRelativeLocation(FVector(0.0f, -100.0f, 0.0f));
	SmogParticle2->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
	SmogParticle2->SetVisibility(false);
	SmogParticle2->bAutoActivate = false;

	SmogParticle3 = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SmogParticle3"));
	SmogParticle3->SetupAttachment(RootSceneComponent);
	SmogParticle3->SetRelativeLocation(FVector(0.0f, 100.0f, 0.0f));
	SmogParticle3->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
	SmogParticle3->SetVisibility(false);
	SmogParticle3->bAutoActivate = false;

	// 기본값
	bIsSelected = false;
}

void ACityActor::BeginPlay()
{
	Super::BeginPlay();

	// SmallCity 위젯 클래스 로드
	if (SmallCityWidgetComponent)
	{
		UClass* WidgetClass = LoadClass<UUserWidget>(nullptr, TEXT("/Game/Civilization/Widget/UnitWidget/W_SmallCity.W_SmallCity_C"));
		if (WidgetClass)
		{
			SmallCityWidgetComponent->SetWidgetClass(WidgetClass);
		}
	}

	// 스모그 파티클 템플릿 로드 (FacilityManager와 동일 에셋)
	if (UParticleSystem* SmokeTemplate = LoadObject<UParticleSystem>(nullptr, TEXT("/Game/Civilization/Particle/P_SmokeCiv.P_SmokeCiv")))
	{
		if (SmogParticle1) { SmogParticle1->SetTemplate(SmokeTemplate); }
		if (SmogParticle2) { SmogParticle2->SetTemplate(SmokeTemplate); }
		if (SmogParticle3) { SmogParticle3->SetTemplate(SmokeTemplate); }
	}
}

void ACityActor::InitializeCity(const FCityData& InCityData)
{
	// CityComponent는 이제 PlayerState에서 관리됨
	UpdateVisual();
}

void ACityActor::UpdateVisual()
{
	// 외형 갱신은 외부 시스템에서 처리. 현재 함수는 자리만 유지.
}

void ACityActor::SetSelected(bool bSelectedIn)
{
	bIsSelected = bSelectedIn;
	UpdateVisual();
}

void ACityActor::UpdateCitySmogVisibility(int32 CurrentHP, int32 MaxHP)
{
	if (MaxHP <= 0)
	{
		return;
	}

	const float Ratio = static_cast<float>(CurrentHP) / static_cast<float>(MaxHP);

	const bool bShow1 = (Ratio <= 0.75f);
	const bool bShow2 = (Ratio <= 0.50f);
	const bool bShow3 = (Ratio <= 0.25f);

	if (SmogParticle1)
	{
		SmogParticle1->SetVisibility(bShow1);
		if (bShow1) { SmogParticle1->Activate(true); }
		else { SmogParticle1->Deactivate(); }
	}
	if (SmogParticle2)
	{
		SmogParticle2->SetVisibility(bShow2);
		if (bShow2) { SmogParticle2->Activate(true); }
		else { SmogParticle2->Deactivate(); }
	}
	if (SmogParticle3)
	{
		SmogParticle3->SetVisibility(bShow3);
		if (bShow3) { SmogParticle3->Activate(true); }
		else { SmogParticle3->Deactivate(); }
	}
}

void ACityActor::UpdateSmallCityUI(const FString& CountryName, UTexture2D* CountryTexture, int32 CurrentHP, int32 MaxHP)
{
	if (USmallCityUI* SmallUI = GetSmallCityUI())
	{
		SmallUI->SetCountryNameTxt(CountryName);
		SmallUI->SetCountryImg(CountryTexture);
		if (MaxHP > 0)
		{
			SmallUI->SetHPBar(static_cast<float>(CurrentHP) / static_cast<float>(MaxHP));
		}
	}
}

USmallCityUI* ACityActor::GetSmallCityUI() const
{
	if (!SmallCityWidgetComponent)
	{
		return nullptr;
	}
	return Cast<USmallCityUI>(SmallCityWidgetComponent->GetUserWidgetObject());
}


