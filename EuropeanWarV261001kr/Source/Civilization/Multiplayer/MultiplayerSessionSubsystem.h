// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "FindSessionsCallbackProxy.h"
#include "MultiplayerSessionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMultiplayerCreateSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMultiplayerFindSessionsComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMultiplayerJoinSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMultiplayerDestroySessionComplete, bool, bWasSuccessful);

/**
 * Create / Find / Join 세션을 담당하는 GameInstance 서브시스템.
 * 성공 시 InGame이 아니라 로비 맵으로 들어가고, 월드 생성은 로비에서 호스트가 시작을 누를 때 합니다.
 * 나중에 Steam으로 바꿀 때는 bUseLAN만 내리면 됩니다.
 */
UCLASS()
class CIVILIZATION_API UMultiplayerSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static constexpr int32 DefaultMaxPlayers = 2;

	UPROPERTY(BlueprintReadWrite, Category = "Session")
	bool bUseLAN = true;

	// 호스트가 listen으로 여는 로비 맵 이름 (/Game/Civilization/Maps/Lobby)
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FName LobbyLevelName = TEXT("/Game/Civilization/Maps/Lobby");

	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FName LoadingLevelName = TEXT("/Game/Civilization/Maps/Loading");

	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateSession(int32 MaxPlayers = 2);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindSessions();

	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinSession(APlayerController* PlayerController, const FBlueprintSessionResult& SearchResult);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void DestroySession();

	// 호스트만 호출. 로비 맵을 listen으로 열어 참가자를 기다립니다.
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool TravelToLobbyAsHost();

	// 호스트만 호출. 세션을 유지한 채 호스트와 참가자를 Loading으로 옮깁니다.
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool TravelToLoadingAsHost();

	// 호스트만 호출. 세션을 유지한 채 호스트와 참가자를 InGame으로 옮깁니다.
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool TravelToInGameAsHost();

	void SendGeneratedWorldThenTravel();
	void NotifyNetworkWorldClientReady();

	const TArray<FBlueprintSessionResult>& GetLastSessionResults() const { return LastSessionResults; }

	UFUNCTION(BlueprintCallable, Category = "Session")
	bool IsHostingSession() const { return bIsHostingSession; }

	UFUNCTION(BlueprintCallable, Category = "Session")
	bool IsMultiplayerSession() const { return bIsMultiplayerSession; }

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FOnMultiplayerCreateSessionComplete OnCreateSessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FOnMultiplayerFindSessionsComplete OnFindSessionsComplete;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FOnMultiplayerJoinSessionComplete OnJoinSessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FOnMultiplayerDestroySessionComplete OnDestroySessionComplete;

protected:
	IOnlineSessionPtr GetSessionInterface() const;

	void StartCreateSession();
	void StartDestroySession();
	void StartJoinSession();
	void ClearSessionDelegates();
	bool TravelAsHost(const FString& LevelPackageName);

	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	TArray<FBlueprintSessionResult> LastSessionResults;

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;

	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	TWeakObjectPtr<APlayerController> JoiningPlayerController;
	FBlueprintSessionResult PendingJoinResult;

	int32 DesiredMaxPlayers = DefaultMaxPlayers;
	bool bCreateAfterDestroy = false;
	bool bJoinAfterDestroy = false;
	bool bIsHostingSession = false;
	bool bIsMultiplayerSession = false;
	int32 PendingWorldSyncAcks = 0;
};
