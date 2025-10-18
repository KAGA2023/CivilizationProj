// Fill out your copyright notice in the Description page of Project Settings.


#include "SuperGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "WorldComponent.h"

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
