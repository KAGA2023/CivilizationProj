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
 * Create / Find / Join 세션과 호스트 레벨 이동을 담당합니다.
 * 방 성공 후 로비로 들어가고, 월드는 로비에서 Start를 누를 때 만듭니다.
 * Steam으로 바꿀 때는 bUseLAN만 내리면 됩니다.
 */
UCLASS()
class CIVILIZATION_API UMultiplayerSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ========== 세션 설정 ==========

	// 1v1 고정 인원
	static constexpr int32 DefaultMaxPlayers = 2;

	// true면 LAN(OnlineSubsystemNull). Steam으로 바꿀 때 false.
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	bool bUseLAN = true;

	// 호스트가 listen으로 여는 로비 맵
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FName LobbyLevelName = TEXT("/Game/Civilization/Maps/Lobby");

	// 호스트가 ServerTravel로 여는 로딩 맵
	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FName LoadingLevelName = TEXT("/Game/Civilization/Maps/Loading");

	// ========== 방 만들기 / 찾기 / 나가기 ==========

	// 방을 만듭니다. 이미 세션이 있으면 지운 뒤 다시 만듭니다.
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateSession(int32 MaxPlayers = 2);

	// LAN/Steam에서 방을 검색합니다.
	UFUNCTION(BlueprintCallable, Category = "Session")
	void FindSessions();

	// 검색 결과에 참가합니다. 이미 세션이 있으면 지운 뒤 참가합니다.
	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinSession(APlayerController* PlayerController, const FBlueprintSessionResult& SearchResult);

	// 현재 세션을 나갑니다. GI LocalPlayerIndex를 0으로 되돌립니다.
	UFUNCTION(BlueprintCallable, Category = "Session")
	void DestroySession();

	// ========== 호스트 이동 ==========

	// 로비 맵을 listen으로 엽니다.
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool TravelToLobbyAsHost();

	// 세션을 유지한 채 Loading으로 ServerTravel합니다.
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool TravelToLoadingAsHost();

	// 세션을 유지한 채 InGame으로 ServerTravel합니다.
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool TravelToInGameAsHost();

	// 호스트 월드를 청크로 보낸 뒤, 참가자 ack가 오면 InGame으로 이동합니다.
	void SendGeneratedWorldThenTravel();

	// 참가자가 월드 수신을 끝냈을 때 호스트가 호출합니다.
	void NotifyNetworkWorldClientReady();

	// ========== 상태 ==========

	// 마지막 Find 결과를 반환합니다.
	const TArray<FBlueprintSessionResult>& GetLastSessionResults() const { return LastSessionResults; }

	// 이 기기가 방을 연 호스트인지
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool IsHostingSession() const { return bIsHostingSession; }

	// 지금 멀티 세션 중인지. 싱글 OpenLevel과 구분합니다.
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool IsMultiplayerSession() const { return bIsMultiplayerSession; }

	// ========== 완료 이벤트 ==========

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FOnMultiplayerCreateSessionComplete OnCreateSessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FOnMultiplayerFindSessionsComplete OnFindSessionsComplete;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FOnMultiplayerJoinSessionComplete OnJoinSessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Session")
	FOnMultiplayerDestroySessionComplete OnDestroySessionComplete;

protected:
	// ========== OSS 콜백 ==========

	IOnlineSessionPtr GetSessionInterface() const;

	void StartCreateSession();
	void StartDestroySession();
	void StartJoinSession();
	void ClearSessionDelegates();

	// 호스트만. URL?listen으로 ServerTravel합니다.
	bool TravelAsHost(const FString& LevelPackageName);

	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	// GI LocalPlayerIndex를 맞춥니다. Create=0, Join=1.
	void ApplyLocalPlayerIndex(int32 Index);

	// 방을 완전히 나갈 때만 슬롯을 0으로 되돌립니다. Create/Join 직전 Destroy는 건너뜁니다.
	void ResetLocalPlayerIfLeavingSession();

	// ========== 내부 상태 ==========

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

	// Destroy가 끝난 뒤 Create를 이어서 할지
	bool bCreateAfterDestroy = false;

	// Destroy가 끝난 뒤 Join을 이어서 할지
	bool bJoinAfterDestroy = false;

	bool bIsHostingSession = false;
	bool bIsMultiplayerSession = false;

	// 월드 청크를 받는 참가자 수. 0이 되면 InGame으로 이동합니다.
	int32 PendingWorldSyncAcks = 0;
};
