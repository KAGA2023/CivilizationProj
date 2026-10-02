#include "LobbyGameState.h"
#include "SuperGameInstance.h"
#include "SuperGameController.h"
#include "Multiplayer/MultiplayerSessionSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"

ALobbyGameState::ALobbyGameState()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	NetUpdateFrequency = 10.0f;

	WorldConfig.PlayerCount = 2;
	WorldConfig.OceanPercentage = 0.4f;
}

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyGameState, WorldConfig);
	DOREPLIFETIME(ALobbyGameState, HostCountryName);
	DOREPLIFETIME(ALobbyGameState, ClientCountryName);
}

void ALobbyGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

	if (HasAuthority() && PlayerArray.Num() < 2)
	{
		SetClientCountry(TEXT("NoSelect"));
	}
}

void ALobbyGameState::SetWorldConfig(const FWorldConfig& NewWorldConfig)
{
	if (!HasAuthority())
	{
		return;
	}

	WorldConfig = NewWorldConfig;
	WorldConfig.PlayerCount = 2;
	OnLobbyStateChanged.Broadcast();
	MulticastApplyWorldConfig(WorldConfig);
}

void ALobbyGameState::SetHostCountry(FName CountryRowName)
{
	if (!HasAuthority() || CountryRowName.IsNone())
	{
		return;
	}

	HostCountryName = CountryRowName.ToString();
	OnLobbyStateChanged.Broadcast();
	MulticastApplyCountries(HostCountryName, ClientCountryName);
}

void ALobbyGameState::SetClientCountry(FName CountryRowName)
{
	if (!HasAuthority() || CountryRowName.IsNone())
	{
		return;
	}

	ClientCountryName = CountryRowName.ToString();
	OnLobbyStateChanged.Broadcast();
	MulticastApplyCountries(HostCountryName, ClientCountryName);
}

void ALobbyGameState::OnRep_LobbyState()
{
	OnLobbyStateChanged.Broadcast();
}

void ALobbyGameState::MulticastApplyWorldConfig_Implementation(FWorldConfig InWorldConfig)
{
	WorldConfig = InWorldConfig;
	OnLobbyStateChanged.Broadcast();
}

void ALobbyGameState::MulticastApplyCountries_Implementation(const FString& InHostCountry, const FString& InClientCountry)
{
	HostCountryName = InHostCountry;
	ClientCountryName = InClientCountry;
	OnLobbyStateChanged.Broadcast();
}

void ALobbyGameState::CommitLobbyToGameInstances()
{
	if (!HasAuthority() || !GetWorld())
	{
		return;
	}

	ApplyLobbyToLocalGameInstance();

	PendingCommitAcks = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ASuperGameController* GameController = Cast<ASuperGameController>(It->Get());
		if (!GameController || GameController->IsLocalController())
		{
			continue;
		}

		++PendingCommitAcks;
		GameController->ClientCommitLobbyToGameInstance();
	}

	if (PendingCommitAcks == 0)
	{
		TravelToLoadingIfReady();
	}
}

void ALobbyGameState::NotifyClientCommitReady()
{
	if (!HasAuthority())
	{
		return;
	}

	--PendingCommitAcks;
	TravelToLoadingIfReady();
}

void ALobbyGameState::TravelToLoadingIfReady()
{
	if (!HasAuthority() || PendingCommitAcks > 0)
	{
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UMultiplayerSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>())
		{
			SessionSubsystem->TravelToLoadingAsHost();
		}
	}
}

void ALobbyGameState::ApplyLobbyToLocalGameInstance()
{
	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	GameInstance->SetWorldConfig(WorldConfig);
	GameInstance->SetCountryNames({ FName(*HostCountryName), FName(*ClientCountryName) });
	GameInstance->SetTargetLevel(TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/Civilization/Maps/InGame.InGame"))));
}
