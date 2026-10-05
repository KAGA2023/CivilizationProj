// Fill out your copyright notice in the Description page of Project Settings.


#include "SuperGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "UObject/SoftObjectPath.h"
#include "World/WorldComponent.h"
#include "Unit/UnitManager.h"
#include "Facility/FacilityManager.h"
#include "Border/BorderManager.h"
#include "Diplomacy/DiplomacyManager.h"
#include "AIPlayer/AIPlayerManager.h"
#include "SuperPlayerState.h"
#include "SaveLoad/SaveLoadManager.h"
#include "Multiplayer/MultiplayerSessionSubsystem.h"

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
	ResetLocalPlayerToSinglePlayer();
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

void USuperGameInstance::SetCountryNames(const TArray<FName>& InCountryNames)
{
	CountryNames = InCountryNames;
	if (CountryNames.Num() > 8)
	{
		CountryNames.SetNum(8);
	}
}

void USuperGameInstance::SetGeneratedWorldComponent(UWorldComponent* WorldComponent)
{
	GeneratedWorldComponent = WorldComponent;
	if (GeneratedWorldComponent && GeneratedWorldComponent->GetOuter() != this)
	{
		GeneratedWorldComponent->Rename(nullptr, this, REN_DoNotDirty | REN_DontCreateRedirectors);
	}
}

void USuperGameInstance::BeginIncomingNetworkWorld(int32 TotalTiles, const TArray<FVector2D>& CityHexes)
{
	IncomingNetworkWorldTiles.Reset();
	IncomingNetworkWorldTiles.Reserve(FMath::Max(TotalTiles, 0));
	IncomingNetworkCityHexes = CityHexes;
}

void USuperGameInstance::AppendIncomingNetworkWorldTiles(const TArray<FWorldTileNetData>& Tiles)
{
	IncomingNetworkWorldTiles.Append(Tiles);
}

bool USuperGameInstance::BuildNetworkWorldPayload(TArray<FWorldTileNetData>& OutTiles, TArray<FVector2D>& OutCityHexes) const
{
	OutTiles.Reset();
	OutCityHexes.Reset();

	if (!GeneratedWorldComponent || !SaveLoadManager)
	{
		return false;
	}

	TMap<FVector2D, FWorldSaveData> WorldDataMap;
	SaveLoadManager->CollectWorldData(GeneratedWorldComponent, nullptr, WorldDataMap);
	if (WorldDataMap.Num() == 0)
	{
		return false;
	}

	OutTiles.Reserve(WorldDataMap.Num());
	for (const TPair<FVector2D, FWorldSaveData>& Pair : WorldDataMap)
	{
		FWorldTileNetData TileData;
		TileData.Hex = Pair.Key;
		TileData.Tile = Pair.Value;
		OutTiles.Add(TileData);
	}

	OutCityHexes = GeneratedWorldComponent->GetStartingCityHexes();
	return true;
}

void USuperGameInstance::ApplyNetworkWorldPayload()
{
	TMap<FVector2D, FWorldSaveData> WorldDataMap;
	WorldDataMap.Reserve(IncomingNetworkWorldTiles.Num());
	for (const FWorldTileNetData& TileData : IncomingNetworkWorldTiles)
	{
		WorldDataMap.Add(TileData.Hex, TileData.Tile);
	}

	TArray<FPlayerSaveData> PlayerDataArray;
	for (int32 Index = 0; Index < IncomingNetworkCityHexes.Num(); ++Index)
	{
		FPlayerSaveData PlayerData;
		PlayerData.PlayerIndex = Index;
		PlayerData.CountryRowName = CountryNames.IsValidIndex(Index) ? CountryNames[Index] : NAME_None;
		PlayerData.bHasCity = true;
		PlayerData.CityData.CityCoordinate = IncomingNetworkCityHexes[Index];
		PlayerDataArray.Add(PlayerData);
	}

	IncomingNetworkWorldTiles.Empty();
	IncomingNetworkCityHexes.Empty();

	ClearGeneratedWorldComponent();
	UWorldComponent* NetworkWorld = NewObject<UWorldComponent>(this);
	NetworkWorld->SetWorldConfig(CurrentWorldConfig);
	NetworkWorld->GenerateWorldFromSaveData(WorldDataMap, CurrentWorldConfig, PlayerDataArray);
	SetGeneratedWorldComponent(NetworkWorld);
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

void USuperGameInstance::SetLocalPlayerIndex(int32 NewIndex)
{
	LocalPlayerIndex = FMath::Max(NewIndex, 0);
}

void USuperGameInstance::ResetLocalPlayerToSinglePlayer()
{
	LocalPlayerIndex = 0;
}

ASuperPlayerState* USuperGameInstance::GetLocalPlayerState() const
{
	return GetPlayerState(LocalPlayerIndex);
}

bool USuperGameInstance::IsInMultiplayerSession() const
{
	if (const UMultiplayerSessionSubsystem* SessionSubsystem = GetSubsystem<UMultiplayerSessionSubsystem>())
	{
		return SessionSubsystem->IsMultiplayerSession();
	}
	return false;
}

bool USuperGameInstance::IsHumanPlayerIndex(int32 PlayerIndex) const
{
	if (PlayerIndex < 0)
	{
		return false;
	}
	if (!IsInMultiplayerSession())
	{
		return PlayerIndex == 0;
	}
	return PlayerIndex == 0 || PlayerIndex == 1;
}

void USuperGameInstance::CreateAllPlayerStates()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (GetPlayerStateCount() > 0)
	{
		ClearAllPlayerStates();
	}

	int32 TotalPlayerCount = CurrentWorldConfig.PlayerCount;
	if (TotalPlayerCount < 2)
	{
		TotalPlayerCount = 2;
	}
	else if (TotalPlayerCount > 8)
	{
		TotalPlayerCount = 8;
	}

	for (int32 i = 0; i < TotalPlayerCount; i++)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ASuperPlayerState* PlayerState = World->SpawnActor<ASuperPlayerState>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (PlayerState)
		{
			PlayerState->PlayerIndex = i;
			AddPlayerState(PlayerState);
			PlayerState->InitializePlayer();
			if (CountryNames.IsValidIndex(i) && CountryNames[i] != NAME_None)
			{
				PlayerState->CountryRowName = CountryNames[i];
				PlayerState->LoadCountryDataFromTable();
			}
		}
	}
}

void USuperGameInstance::EnsureAllPlayerStates()
{
	if (GetPlayerStateCount() > 0)
	{
		return;
	}
	CreateAllPlayerStates();
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

void USuperGameInstance::NotifyLoadingScreenFinished()
{
	OnLoadingScreenFinished.Broadcast();
}
