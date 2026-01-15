// Fill out your copyright notice in the Description page of Project Settings.


#include "SuperGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"
#include "UObject/SoftObjectPath.h"
#include "World/WorldComponent.h"
#include "Unit/UnitManager.h"
#include "Facility/FacilityManager.h"
#include "Border/BorderManager.h"
#include "Diplomacy/DiplomacyManager.h"
#include "AIPlayer/AIPlayerManager.h"
#include "SuperPlayerState.h"
#include "SaveLoad/SaveLoadManager.h"

USuperGameInstance::USuperGameInstance()
{
	// 기본 월드 설정으로 초기화
	CurrentWorldConfig = FWorldConfig();
}

void USuperGameInstance::Init()
{
	Super::Init();

	// 국가 데이터 테이블 로드
	if (!CountryDataTable)
	{
		FSoftObjectPath CountryDataTablePath(TEXT("/Game/Civilization/Data/DT_CountryData.DT_CountryData"));
		CountryDataTable = Cast<UDataTable>(CountryDataTablePath.TryLoad());
	}

	// SaveLoadManager 생성 및 초기화 (게임 인스턴스 레벨에서 관리)
	if (!SaveLoadManager)
	{
		SaveLoadManager = NewObject<USaveLoadManager>(this);
		if (SaveLoadManager)
		{
			SaveLoadManager->SetGameInstance(this);
		}
	}
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

void USuperGameInstance::SetFacilityManager(UFacilityManager* InFacilityManager)
{
	FacilityManager = InFacilityManager;
}

void USuperGameInstance::ClearFacilityManager()
{
	if (FacilityManager)
	{
		// 가비지 컬렉션 대상으로 표시
		FacilityManager->MarkAsGarbage();
		FacilityManager = nullptr;
	}
}

void USuperGameInstance::SetBorderManager(UBorderManager* InBorderManager)
{
	BorderManager = InBorderManager;
}

void USuperGameInstance::ClearBorderManager()
{
	if (BorderManager)
	{
		// 모든 국경선 제거
		BorderManager->ClearAllBorders();
		
		// 가비지 컬렉션 대상으로 표시
		BorderManager->MarkAsGarbage();
		BorderManager = nullptr;
	}
}

void USuperGameInstance::SetDiplomacyManager(UDiplomacyManager* InDiplomacyManager)
{
	DiplomacyManager = InDiplomacyManager;
}

void USuperGameInstance::ClearDiplomacyManager()
{
	if (DiplomacyManager)
	{
		DiplomacyManager->MarkAsGarbage();
		DiplomacyManager = nullptr;
	}
}

void USuperGameInstance::SetAIPlayerManager(UAIPlayerManager* InAIPlayerManager)
{
	AIPlayerManager = InAIPlayerManager;
}

void USuperGameInstance::ClearAIPlayerManager()
{
	if (AIPlayerManager)
	{
		AIPlayerManager->MarkAsGarbage();
		AIPlayerManager = nullptr;
	}
}

void USuperGameInstance::AddPlayerState(ASuperPlayerState* PlayerState)
{
	if (!PlayerState)
	{
		return;
	}
	
	// 이미 같은 PlayerIndex가 있으면 교체
	int32 ExistingIndex = INDEX_NONE;
	for (int32 i = 0; i < PlayerStates.Num(); i++)
	{
		if (PlayerStates[i] && PlayerStates[i]->PlayerIndex == PlayerState->PlayerIndex)
		{
			ExistingIndex = i;
			break;
		}
	}
	
	if (ExistingIndex != INDEX_NONE)
	{
		// 기존 PlayerState 교체
		PlayerStates[ExistingIndex] = PlayerState;
	}
	else if (!PlayerStates.Contains(PlayerState))
	{
		// 새로 추가
		PlayerStates.Add(PlayerState);
	}
}

ASuperPlayerState* USuperGameInstance::GetPlayerState(int32 PlayerIndex) const
{
	// PlayerIndex로 찾기
	for (ASuperPlayerState* PlayerState : PlayerStates)
	{
		if (PlayerState && PlayerState->PlayerIndex == PlayerIndex)
		{
			return PlayerState;
		}
	}
	return nullptr;
}

void USuperGameInstance::ClearAllPlayerStates()
{
	PlayerStates.Empty();
}

void USuperGameInstance::SetSaveLoadManager(USaveLoadManager* InSaveLoadManager)
{
	SaveLoadManager = InSaveLoadManager;
	if (SaveLoadManager)
	{
		SaveLoadManager->SetGameInstance(this);
	}
}

void USuperGameInstance::ClearSaveLoadManager()
{
	if (SaveLoadManager)
	{
		SaveLoadManager->MarkAsGarbage();
		SaveLoadManager = nullptr;
	}
}
