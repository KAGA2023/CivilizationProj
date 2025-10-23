// Fill out your copyright notice in the Description page of Project Settings.


#include "SuperGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "WorldComponent.h"
#include "Unit/UnitManager.h"

USuperGameInstance::USuperGameInstance()
{
	// 기본 월드 설정으로 초기화
	CurrentWorldConfig = FWorldConfig();
}

void USuperGameInstance::Init()
{
	Super::Init();
}

void USuperGameInstance::OpenLevel(TSoftObjectPtr<UWorld> newLevel)
{
	TargetLevel = newLevel;
	if (OldLevel.IsValid())
	{
		UGameplayStatics::UnloadStreamLevel(this, *OldLevel.GetAssetName(), FLatentActionInfo(), false);
	}
	UGameplayStatics::OpenLevel(this, TEXT("Loading"));
	OldLevel = TargetLevel;
}

void USuperGameInstance::SetWorldConfig(const FWorldConfig& NewSettings)
{
	CurrentWorldConfig = NewSettings;
}

void USuperGameInstance::ResetWorldConfigToDefault()
{
	CurrentWorldConfig = FWorldConfig();
}

void USuperGameInstance::SetGeneratedWorldComponent(UWorldComponent* WorldComponent)
{
	GeneratedWorldComponent = WorldComponent;
}

void USuperGameInstance::ClearGeneratedWorldComponent()
{
	if (GeneratedWorldComponent)
	{
		// 월드 컴포넌트의 월드 데이터 정리
		GeneratedWorldComponent->ClearWorld();
		
		// 가비지 컬렉션 대상으로 표시
		GeneratedWorldComponent->MarkAsGarbage();
		GeneratedWorldComponent = nullptr;
	}
}

void USuperGameInstance::SetUnitManager(UUnitManager* InUnitManager)
{
	UnitManager = InUnitManager;
	
	// UnitManager에 WorldComponent 설정
	if (UnitManager && GeneratedWorldComponent)
	{
		UnitManager->SetWorldComponent(GeneratedWorldComponent);
	}
}

void USuperGameInstance::ClearUnitManager()
{
	if (UnitManager)
	{
		// 모든 유닛 정리
		UnitManager->ClearAllUnits();
		
		// 가비지 컬렉션 대상으로 표시
		UnitManager->MarkAsGarbage();
		UnitManager = nullptr;
	}
}
