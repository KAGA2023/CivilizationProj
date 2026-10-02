#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "World/WorldStruct.h"
#include "LobbyGameState.generated.h"

// 로비 월드 설정·국가 선택이 바뀔 때 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyStateChanged);

UCLASS()
class CIVILIZATION_API ALobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ALobbyGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 참가자가 나가면 참가자 국가를 NoSelect로 되돌립니다.
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	// ========== 로비 이벤트 ==========

	// 로비 설정이 복제·변경되면 브로드캐스트합니다.
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnLobbyStateChanged OnLobbyStateChanged;

	// ========== 로비 조회 ==========

	// 로비에서 합의된 월드 설정을 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	const FWorldConfig& GetWorldConfig() const { return WorldConfig; }

	// 호스트(슬롯 0)가 고른 국가 RowName을 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	FName GetHostCountryName() const { return FName(*HostCountryName); }

	// 참가자(슬롯 1)가 고른 국가 RowName을 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	FName GetClientCountryName() const { return FName(*ClientCountryName); }

	// ========== 로비 설정 ==========

	// 월드 설정을 저장하고 참가자에게 복제합니다. 플레이어 수는 2로 고정합니다.
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetWorldConfig(const FWorldConfig& NewWorldConfig);

	// 호스트 국가를 설정하고 참가자에게 복제합니다.
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetHostCountry(FName CountryRowName);

	// 참가자 국가를 설정하고 참가자에게 복제합니다.
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetClientCountry(FName CountryRowName);

	// 로비 설정을 GI에 반영한 뒤 로딩 맵으로 이동합니다.
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void CommitLobbyToGameInstances();

	// 이 기기의 GI에 월드 설정·국가·타겟 레벨을 넣습니다.
	void ApplyLobbyToLocalGameInstance();

	// 참가자가 커밋을 마쳤다고 알립니다. 모두 끝나면 로딩으로 이동합니다.
	void NotifyClientCommitReady();

protected:
	// ========== 복제 상태 ==========

	// 로비에서 합의된 월드 설정. 복제됩니다.
	UPROPERTY(ReplicatedUsing = OnRep_LobbyState)
	FWorldConfig WorldConfig;

	// 호스트(슬롯 0) 국가 RowName. 복제됩니다.
	UPROPERTY(ReplicatedUsing = OnRep_LobbyState)
	FString HostCountryName = TEXT("NoSelect");

	// 참가자(슬롯 1) 국가 RowName. 복제됩니다.
	UPROPERTY(ReplicatedUsing = OnRep_LobbyState)
	FString ClientCountryName = TEXT("NoSelect");

	// 복제된 로비 상태가 바뀌면 UI에 알립니다.
	UFUNCTION()
	void OnRep_LobbyState();

	// 참가자에게 월드 설정을 바로 적용합니다.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastApplyWorldConfig(FWorldConfig InWorldConfig);

	// 참가자에게 호스트/참가자 국가를 바로 적용합니다.
	UFUNCTION(NetMulticast, Reliable)
	void MulticastApplyCountries(const FString& InHostCountry, const FString& InClientCountry);

	// ========== 커밋 / 트래블 ==========

	// 아직 커밋 응답을 안 한 참가자 수
	int32 PendingCommitAcks = 0;

	// 모든 참가자가 커밋하면 호스트가 로딩 맵으로 트래블합니다.
	void TravelToLoadingIfReady();
};
