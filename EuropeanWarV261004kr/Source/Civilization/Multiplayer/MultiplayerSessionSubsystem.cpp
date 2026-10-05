// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "../SuperGameInstance.h"
#include "../SuperGameController.h"

void UMultiplayerSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionSubsystem::HandleCreateSessionComplete);
	DestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionSubsystem::HandleDestroySessionComplete);
	FindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMultiplayerSessionSubsystem::HandleFindSessionsComplete);
	JoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionSubsystem::HandleJoinSessionComplete);
}

void UMultiplayerSessionSubsystem::Deinitialize()
{
	ClearSessionDelegates();
	Super::Deinitialize();
}

IOnlineSessionPtr UMultiplayerSessionSubsystem::GetSessionInterface() const
{
	return Online::GetSessionInterface(GetWorld());
}

void UMultiplayerSessionSubsystem::ClearSessionDelegates()
{
	const IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return;
	}

	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
}

void UMultiplayerSessionSubsystem::CreateSession(int32 MaxPlayers)
{
	DesiredMaxPlayers = FMath::Clamp(MaxPlayers, 2, 2);
	bCreateAfterDestroy = false;

	const IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		OnCreateSessionComplete.Broadcast(false);
		return;
	}

	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		bCreateAfterDestroy = true;
		StartDestroySession();
		return;
	}

	StartCreateSession();
}

void UMultiplayerSessionSubsystem::DestroySession()
{
	bCreateAfterDestroy = false;
	bJoinAfterDestroy = false;
	StartDestroySession();
}

void UMultiplayerSessionSubsystem::ApplyLocalPlayerIndex(int32 Index)
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		GameInstance->SetLocalPlayerIndex(Index);
	}
}

void UMultiplayerSessionSubsystem::ResetLocalPlayerIfLeavingSession()
{
	if (bCreateAfterDestroy || bJoinAfterDestroy)
	{
		return;
	}
	ApplyLocalPlayerIndex(0);
}

void UMultiplayerSessionSubsystem::StartDestroySession()
{
	const IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		bIsHostingSession = false;
		bIsMultiplayerSession = false;
		ResetLocalPlayerIfLeavingSession();
		if (bCreateAfterDestroy)
		{
			bCreateAfterDestroy = false;
			OnCreateSessionComplete.Broadcast(false);
			return;
		}
		if (bJoinAfterDestroy)
		{
			bJoinAfterDestroy = false;
			OnJoinSessionComplete.Broadcast(false);
			return;
		}
		OnDestroySessionComplete.Broadcast(false);
		return;
	}

	SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);

	if (!SessionInterface->GetNamedSession(NAME_GameSession))
	{
		bIsHostingSession = false;
		bIsMultiplayerSession = false;
		if (bCreateAfterDestroy)
		{
			bCreateAfterDestroy = false;
			StartCreateSession();
			return;
		}
		if (bJoinAfterDestroy)
		{
			bJoinAfterDestroy = false;
			StartJoinSession();
			return;
		}
		ApplyLocalPlayerIndex(0);
		OnDestroySessionComplete.Broadcast(true);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		bIsHostingSession = false;
		bIsMultiplayerSession = false;
		if (bJoinAfterDestroy)
		{
			bJoinAfterDestroy = false;
			OnJoinSessionComplete.Broadcast(false);
			return;
		}
		if (!bCreateAfterDestroy)
		{
			ApplyLocalPlayerIndex(0);
			OnDestroySessionComplete.Broadcast(false);
		}
	}
}

void UMultiplayerSessionSubsystem::StartCreateSession()
{
	const IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		OnCreateSessionComplete.Broadcast(false);
		return;
	}

	FOnlineSessionSettings SessionSettings;
	SessionSettings.NumPublicConnections = DesiredMaxPlayers;
	SessionSettings.bIsLANMatch = bUseLAN;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowJoinViaPresence = false;
	SessionSettings.bUsesPresence = false;
	SessionSettings.bUseLobbiesIfAvailable = false;
	SessionSettings.Set(SETTING_MAPNAME, FString(TEXT("Lobby")), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	bool bStartedCreate = false;
	if (UWorld* World = GetWorld())
	{
		if (const ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController())
		{
			if (const FUniqueNetIdPtr UniqueNetId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId())
			{
				bStartedCreate = SessionInterface->CreateSession(*UniqueNetId, NAME_GameSession, SessionSettings);
			}
		}
	}
	if (!bStartedCreate)
	{
		bStartedCreate = SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
	}

	if (!bStartedCreate)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		OnCreateSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionSubsystem::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	const IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}

	bIsHostingSession = false;
	bIsMultiplayerSession = false;

	if (bCreateAfterDestroy)
	{
		bCreateAfterDestroy = false;
		StartCreateSession();
		return;
	}

	if (bJoinAfterDestroy)
	{
		bJoinAfterDestroy = false;
		StartJoinSession();
		return;
	}

	ApplyLocalPlayerIndex(0);
	OnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	const IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	bIsHostingSession = bWasSuccessful;
	bIsMultiplayerSession = bWasSuccessful;
	ApplyLocalPlayerIndex(0);

	if (bWasSuccessful && SessionInterface.IsValid())
	{
		SessionInterface->StartSession(SessionName);
	}

	OnCreateSessionComplete.Broadcast(bWasSuccessful);
}

bool UMultiplayerSessionSubsystem::TravelToLobbyAsHost()
{
	if (LobbyLevelName.IsNone())
	{
		return false;
	}

	UWorld* World = nullptr;
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		World = GameInstance->GetWorld();
	}
	if (!World)
	{
		return false;
	}

	UGameplayStatics::OpenLevel(World, LobbyLevelName, true, TEXT("listen"));
	return true;
}

bool UMultiplayerSessionSubsystem::TravelToLoadingAsHost()
{
	return TravelAsHost(LoadingLevelName.ToString());
}

bool UMultiplayerSessionSubsystem::TravelToInGameAsHost()
{
	FString LevelPackageName;
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		LevelPackageName = GameInstance->GetTargetLevelPackageName();
	}
	if (LevelPackageName.IsEmpty())
	{
		LevelPackageName = TEXT("/Game/Civilization/Maps/InGame");
	}
	return TravelAsHost(LevelPackageName);
}

bool UMultiplayerSessionSubsystem::TravelAsHost(const FString& LevelPackageName)
{
	if (!bIsHostingSession || LevelPackageName.IsEmpty())
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World || !World->GetAuthGameMode())
	{
		return false;
	}

	World->ServerTravel(LevelPackageName + TEXT("?listen"), true);
	return true;
}

void UMultiplayerSessionSubsystem::SendGeneratedWorldThenTravel()
{
	if (!bIsHostingSession || !GetWorld())
	{
		return;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	TArray<FWorldTileNetData> Tiles;
	TArray<FVector2D> CityHexes;
	if (!GameInstance || !GameInstance->BuildNetworkWorldPayload(Tiles, CityHexes))
	{
		TravelToInGameAsHost();
		return;
	}

	PendingWorldSyncAcks = 0;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASuperGameController* GameController = Cast<ASuperGameController>(It->Get());
		if (!GameController || GameController->IsLocalController())
		{
			continue;
		}

		++PendingWorldSyncAcks;
		GameController->ClientBeginNetworkWorld(Tiles.Num(), CityHexes);

		constexpr int32 ChunkSize = 100;
		for (int32 Index = 0; Index < Tiles.Num(); Index += ChunkSize)
		{
			const int32 Count = FMath::Min(ChunkSize, Tiles.Num() - Index);
			TArray<FWorldTileNetData> Chunk;
			Chunk.Append(Tiles.GetData() + Index, Count);
			GameController->ClientReceiveNetworkWorldTiles(Chunk);
		}

		GameController->ClientFinishNetworkWorld();
	}

	if (PendingWorldSyncAcks == 0)
	{
		TravelToInGameAsHost();
	}
}

void UMultiplayerSessionSubsystem::NotifyNetworkWorldClientReady()
{
	--PendingWorldSyncAcks;
	if (PendingWorldSyncAcks <= 0)
	{
		TravelToInGameAsHost();
	}
}

void UMultiplayerSessionSubsystem::FindSessions()
{
	LastSessionResults.Reset();

	const IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		OnFindSessionsComplete.Broadcast(false);
		return;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = 20;
	SessionSearch->bIsLanQuery = bUseLAN;
	SessionSearch->TimeoutInSeconds = 10.0f;

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	bool bStartedFind = false;
	if (UWorld* World = GetWorld())
	{
		if (const ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController())
		{
			if (const FUniqueNetIdPtr UniqueNetId = LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId())
			{
				bStartedFind = SessionInterface->FindSessions(*UniqueNetId, SessionSearch.ToSharedRef());
			}
		}
	}
	if (!bStartedFind)
	{
		bStartedFind = SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}

	if (!bStartedFind)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		OnFindSessionsComplete.Broadcast(false);
	}
}

void UMultiplayerSessionSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
	const IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	LastSessionResults.Reset();
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
		{
			FBlueprintSessionResult BlueprintResult;
			BlueprintResult.OnlineResult = Result;
			LastSessionResults.Add(BlueprintResult);
		}
	}

	OnFindSessionsComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionSubsystem::JoinSession(APlayerController* PlayerController, const FBlueprintSessionResult& SearchResult)
{
	if (!PlayerController && GetWorld())
	{
		PlayerController = GetWorld()->GetFirstPlayerController();
	}

	JoiningPlayerController = PlayerController;
	PendingJoinResult = SearchResult;
	bJoinAfterDestroy = false;

	const IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid() || !PlayerController)
	{
		OnJoinSessionComplete.Broadcast(false);
		return;
	}

	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		bJoinAfterDestroy = true;
		StartDestroySession();
		return;
	}

	StartJoinSession();
}

void UMultiplayerSessionSubsystem::StartJoinSession()
{
	APlayerController* PlayerController = JoiningPlayerController.Get();
	const IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid() || !PlayerController)
	{
		OnJoinSessionComplete.Broadcast(false);
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
	if (!SessionInterface->JoinSession(0, NAME_GameSession, PendingJoinResult.OnlineResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		OnJoinSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionSubsystem::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	const IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	const bool bWasSuccessful = (Result == EOnJoinSessionCompleteResult::Success);
	bIsHostingSession = false;
	bIsMultiplayerSession = bWasSuccessful;
	ApplyLocalPlayerIndex(bWasSuccessful ? 1 : 0);

	if (bWasSuccessful && SessionInterface.IsValid())
	{
		if (APlayerController* PlayerController = JoiningPlayerController.Get())
		{
			FString ConnectString;
			if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
			{
				PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
			}
		}
	}

	OnJoinSessionComplete.Broadcast(bWasSuccessful);
}
