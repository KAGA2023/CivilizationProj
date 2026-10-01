#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "World/WorldStruct.h"
#include "LobbyGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyStateChanged);

UCLASS()
class CIVILIZATION_API ALobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ALobbyGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnLobbyStateChanged OnLobbyStateChanged;

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	const FWorldConfig& GetWorldConfig() const { return WorldConfig; }

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	FName GetHostCountryName() const { return FName(*HostCountryName); }

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	FName GetClientCountryName() const { return FName(*ClientCountryName); }

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetWorldConfig(const FWorldConfig& NewWorldConfig);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetHostCountry(FName CountryRowName);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetClientCountry(FName CountryRowName);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void CommitLobbyToGameInstances();

	void ApplyLobbyToLocalGameInstance();
	void NotifyClientCommitReady();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_LobbyState)
	FWorldConfig WorldConfig;

	UPROPERTY(ReplicatedUsing = OnRep_LobbyState)
	FString HostCountryName = TEXT("NoSelect");

	UPROPERTY(ReplicatedUsing = OnRep_LobbyState)
	FString ClientCountryName = TEXT("NoSelect");

	UFUNCTION()
	void OnRep_LobbyState();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastApplyWorldConfig(FWorldConfig InWorldConfig);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastApplyCountries(const FString& InHostCountry, const FString& InClientCountry);

	int32 PendingCommitAcks = 0;
	void TravelToLoadingIfReady();
};
