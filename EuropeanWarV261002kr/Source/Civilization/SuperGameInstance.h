// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "World/WorldStruct.h"
#include "SaveLoad/SaveLoadStruct.h"
#include "SuperGameInstance.generated.h"

/** 로딩 타일 UI가 끝나고 RemoveFromParent 직전 한 번 브로드캐스트. 블루프린트에서 이 시점에 MainHUD 생성하면 복원/초기화 완료 후 한 번만 세팅됨. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadingScreenFinished);

UCLASS()
class CIVILIZATION_API USuperGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	USuperGameInstance();
	virtual void Init() override;

	// ========== 레벨 이동 ==========

	// 로딩 맵을 거쳐 이동할 타겟 레벨
	TSoftObjectPtr<UWorld> TargetLevel{};

	// 이전에 연 레벨 (언로드용)
	TSoftObjectPtr<UWorld> OldLevel{};

	// 로딩 맵을 거쳐 다른 레벨로 이동합니다. 싱글 전용 경로이며 LocalPlayerIndex를 0으로 되돌립니다.
	UFUNCTION(BlueprintCallable, Category = "Level")
	void OpenLevel(TSoftObjectPtr<UWorld> newLevel);

	// 다음에 열 타겟 레벨을 지정합니다. 멀티 ServerTravel도 이 값을 봅니다.
	UFUNCTION(BlueprintCallable, Category = "Level")
	void SetTargetLevel(TSoftObjectPtr<UWorld> NewLevel) { TargetLevel = NewLevel; }

	// 현재 타겟 레벨의 에셋 이름을 반환합니다.
	FName GetTargetLevelName()
	{
		if (TargetLevel.IsNull())
		{
			return NAME_None;
		}
		FString AssetName = TargetLevel.GetAssetName();
		if (!AssetName.IsEmpty())
		{
			return FName(*AssetName);
		}
		FString PackageName = GetTargetLevelPackageName();
		if (!PackageName.IsEmpty())
		{
			int32 LastSlashIndex = INDEX_NONE;
			if (PackageName.FindLastChar(TEXT('/'), LastSlashIndex))
			{
				FString LevelName = PackageName.RightChop(LastSlashIndex + 1);
				if (!LevelName.IsEmpty())
				{
					return FName(*LevelName);
				}
			}
		}
		return NAME_None;
	}

	// 현재 타겟 레벨의 패키지 경로를 반환합니다. (예: /Game/Civilization/Maps/InGame)
	FString GetTargetLevelPackageName() const
	{
		if (TargetLevel.IsNull())
		{
			return FString();
		}
		return TargetLevel.ToSoftObjectPath().GetLongPackageName();
	}

	// ========== 월드 설정 ==========

	// 현재 월드 설정 (크기, 지형 비율, 플레이어 수 등)
	UPROPERTY(BlueprintReadWrite, Category = "World Settings")
	FWorldConfig CurrentWorldConfig;

	// 월드 설정을 저장합니다. 로비 커밋·메인메뉴 월드 설정에서 호출합니다.
	UFUNCTION(BlueprintCallable, Category = "World Settings")
	void SetWorldConfig(const FWorldConfig& NewSettings);

	// 현재 월드 설정을 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "World Settings")
	FWorldConfig GetWorldConfig() const { return CurrentWorldConfig; }

	// ========== 국가 ==========

	// 슬롯 순서의 국가 RowName 배열 (0=호스트/싱글, 1=참가자 또는 AI)
	UPROPERTY(BlueprintReadWrite, Category = "Country Settings")
	TArray<FName> CountryNames;

	// 국가 RowName 배열을 설정합니다. 최대 8개로 자릅니다.
	UFUNCTION(BlueprintCallable, Category = "Country Settings")
	void SetCountryNames(const TArray<FName>& InCountryNames);

	// 국가 RowName 배열을 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Country Settings")
	TArray<FName> GetCountryNames() const { return CountryNames; }

	// 국가 정보(이름, 이미지, 색)를 읽는 데이터 테이블
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Country Settings")
	class UDataTable* CountryDataTable = nullptr;

	// 국가 데이터 테이블을 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Country Settings")
	class UDataTable* GetCountryDataTable() const { return CountryDataTable; }

	// ========== 월드 컴포넌트 ==========

	// 로딩에서 만든 월드 타일 데이터. 인게임 WorldSpawner가 이걸 스폰합니다.
	UPROPERTY(BlueprintReadWrite, Category = "World Management")
	class UWorldComponent* GeneratedWorldComponent = nullptr;

	// 생성된 월드 컴포넌트를 저장하고, Outer를 GI로 옮겨 레벨 이동 후에도 유지합니다.
	UFUNCTION(BlueprintCallable, Category = "World Management")
	void SetGeneratedWorldComponent(class UWorldComponent* WorldComponent);

	// 생성된 월드 컴포넌트를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "World Management")
	class UWorldComponent* GetGeneratedWorldComponent() const { return GeneratedWorldComponent; }

	// 월드 컴포넌트를 비우고 메모리에서 해제합니다.
	UFUNCTION(BlueprintCallable, Category = "World Management")
	void ClearGeneratedWorldComponent();

	// 호스트가 보낸 타일 수신을 시작합니다. 총 개수와 시작 도시 hex를 받습니다.
	void BeginIncomingNetworkWorld(int32 TotalTiles, const TArray<FVector2D>& CityHexes);

	// 네트워크로 받은 타일 청크를 이어 붙입니다.
	void AppendIncomingNetworkWorldTiles(const TArray<FWorldTileNetData>& Tiles);

	// 호스트 월드를 참가자에게 보낼 타일 배열로 묶습니다.
	bool BuildNetworkWorldPayload(TArray<FWorldTileNetData>& OutTiles, TArray<FVector2D>& OutCityHexes) const;

	// 받아 둔 타일로 참가자 쪽 월드 컴포넌트를 조립합니다.
	void ApplyNetworkWorldPayload();

	// ========== 유닛 ==========

	// 인게임 유닛 스폰·이동·전투를 담당하는 매니저
	UPROPERTY(BlueprintReadWrite, Category = "Unit Management")
	class UUnitManager* UnitManager = nullptr;

	// 유닛 매니저를 설정하고 현재 월드 컴포넌트를 연결합니다.
	UFUNCTION(BlueprintCallable, Category = "Unit Management")
	void SetUnitManager(class UUnitManager* InUnitManager);

	// 유닛 매니저를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Unit Management")
	class UUnitManager* GetUnitManager() const { return UnitManager; }

	// 모든 유닛을 지우고 매니저를 해제합니다.
	UFUNCTION(BlueprintCallable, Category = "Unit Management")
	void ClearUnitManager();

	// ========== 시설 ==========

	// 타일 위 시설 건설·약탈을 담당하는 매니저
	UPROPERTY(BlueprintReadWrite, Category = "Facility Management")
	class UFacilityManager* FacilityManager = nullptr;

	// 시설 매니저를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Facility Management")
	void SetFacilityManager(class UFacilityManager* InFacilityManager);

	// 시설 매니저를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Facility Management")
	class UFacilityManager* GetFacilityManager() const { return FacilityManager; }

	// 시설 매니저를 해제합니다.
	UFUNCTION(BlueprintCallable, Category = "Facility Management")
	void ClearFacilityManager();

	// ========== 국경 ==========

	// 소유 타일 국경선 메시를 담당하는 매니저
	UPROPERTY(BlueprintReadWrite, Category = "Border Management")
	class UBorderManager* BorderManager = nullptr;

	// 국경 매니저를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Border Management")
	void SetBorderManager(class UBorderManager* InBorderManager);

	// 국경 매니저를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Border Management")
	class UBorderManager* GetBorderManager() const { return BorderManager; }

	// 국경선을 지우고 매니저를 해제합니다.
	UFUNCTION(BlueprintCallable, Category = "Border Management")
	void ClearBorderManager();

	// ========== 외교 ==========

	// 전쟁·평화·동맹·호감도를 담당하는 매니저
	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy Management")
	class UDiplomacyManager* DiplomacyManager = nullptr;

	// 외교 매니저를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy Management")
	void SetDiplomacyManager(class UDiplomacyManager* InDiplomacyManager);

	// 외교 매니저를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy Management")
	class UDiplomacyManager* GetDiplomacyManager() const { return DiplomacyManager; }

	// 외교 매니저를 해제합니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy Management")
	void ClearDiplomacyManager();

	// ========== AI ==========

	// AI 슬롯의 턴 상태 머신을 담당하는 매니저
	UPROPERTY(BlueprintReadWrite, Category = "AI Player Management")
	class UAIPlayerManager* AIPlayerManager = nullptr;

	// AI 매니저를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "AI Player Management")
	void SetAIPlayerManager(class UAIPlayerManager* InAIPlayerManager);

	// AI 매니저를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "AI Player Management")
	class UAIPlayerManager* GetAIPlayerManager() const { return AIPlayerManager; }

	// AI 매니저를 해제합니다.
	UFUNCTION(BlueprintCallable, Category = "AI Player Management")
	void ClearAIPlayerManager();

	// ========== 플레이어 ==========

	// GI가 들고 있는 문명 플레이어 스테이트. 엔진 PlayerState와 별개입니다.
	UPROPERTY(BlueprintReadWrite, Category = "Player Management")
	TArray<class ASuperPlayerState*> PlayerStates;

	// 같은 PlayerIndex가 있으면 교체하고, 없으면 추가합니다.
	UFUNCTION(BlueprintCallable, Category = "Player Management")
	void AddPlayerState(class ASuperPlayerState* PlayerState);

	// PlayerIndex로 문명 플레이어 스테이트를 찾습니다.
	UFUNCTION(BlueprintCallable, Category = "Player Management")
	class ASuperPlayerState* GetPlayerState(int32 PlayerIndex) const;

	// 생성된 문명 플레이어 스테이트 수를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Player Management")
	int32 GetPlayerStateCount() const { return PlayerStates.Num(); }

	// 이 기기의 문명 슬롯을 반환합니다. 싱글/호스트=0, 1v1 참가자=1.
	UFUNCTION(BlueprintCallable, Category = "Player Management")
	int32 GetLocalPlayerIndex() const { return LocalPlayerIndex; }

	// 이 기기의 문명 슬롯을 설정합니다. 0 미만은 0으로 맞춥니다.
	UFUNCTION(BlueprintCallable, Category = "Player Management")
	void SetLocalPlayerIndex(int32 NewIndex);

	// 싱글/메뉴로 돌아갈 때 문명 슬롯을 0으로 되돌립니다.
	UFUNCTION(BlueprintCallable, Category = "Player Management")
	void ResetLocalPlayerToSinglePlayer();

	// 이 기기에 해당하는 문명 플레이어 스테이트를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Player Management")
	class ASuperPlayerState* GetLocalPlayerState() const;

	// 지금 멀티 세션(방 만들기/참가) 중인지 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Player Management")
	bool IsInMultiplayerSession() const;

	// 해당 슬롯이 사람인지 반환합니다. 싱글은 0만, 멀티 1v1은 0과 1.
	UFUNCTION(BlueprintCallable, Category = "Player Management")
	bool IsHumanPlayerIndex(int32 PlayerIndex) const;

	// 모든 문명 플레이어 스테이트를 지우고 다시 만듭니다. 호스트 GameMode가 호출합니다.
	void CreateAllPlayerStates();

	// 배열이 비어 있을 때만 생성합니다. 참가자 로딩처럼 GameMode가 없을 때 씁니다.
	void EnsureAllPlayerStates();

	// 문명 플레이어 스테이트 배열만 비웁니다.
	UFUNCTION(BlueprintCallable, Category = "Player Management")
	void ClearAllPlayerStates();

	// ========== 세이브 / 로드 ==========

	// 세이브 파일 읽기/쓰기를 담당하는 매니저. 싱글 전용입니다.
	UPROPERTY(BlueprintReadWrite, Category = "Save Load Management")
	class USaveLoadManager* SaveLoadManager = nullptr;

	// 세이브/로드 매니저를 설정하고 GI를 연결합니다.
	UFUNCTION(BlueprintCallable, Category = "Save Load Management")
	void SetSaveLoadManager(class USaveLoadManager* InSaveLoadManager);

	// 세이브/로드 매니저를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Save Load Management")
	class USaveLoadManager* GetSaveLoadManager() const { return SaveLoadManager; }

	// 세이브/로드 매니저를 해제합니다.
	UFUNCTION(BlueprintCallable, Category = "Save Load Management")
	void ClearSaveLoadManager();

	// 메인메뉴에서 로드한 뒤 인게임에서 복원할 세이브 데이터
	UPROPERTY(BlueprintReadWrite, Category = "Save Load Management")
	FGameSaveData PendingLoadData;

	// 메인메뉴에서 로드 중이면 true. 인게임 타일 스폰 후 복원에 씁니다.
	UPROPERTY(BlueprintReadWrite, Category = "Save Load Management")
	bool bIsLoadingFromMainMenu = false;

	// 로드할 슬롯 인덱스. 0이면 미설정입니다.
	UPROPERTY(BlueprintReadWrite, Category = "Save Load Management")
	int32 PendingLoadSlotIndex = 0;

	// 로딩 화면이 끝날 때 브로드캐스트합니다. 블루프린트에서 이 때 MainHUD를 생성합니다.
	UPROPERTY(BlueprintAssignable, Category = "Save Load Management")
	FOnLoadingScreenFinished OnLoadingScreenFinished;

	// 로딩 화면 종료를 알립니다. LoadingTilesUI가 호출합니다.
	UFUNCTION(BlueprintCallable, Category = "Save Load Management")
	void NotifyLoadingScreenFinished();

	// ========== 타일 구매 ==========

	// 타일 구매 모드 (플레이어0이 PurchaseTileBtn으로 구매 가능 타일 클릭 중일 때 true, 유닛 선택 비활성화용)
	UPROPERTY(BlueprintReadWrite, Category = "Tile Purchase")
	bool bIsTilePurchaseMode = false;

	// 타일 구매 모드를 켜거나 끕니다.
	UFUNCTION(BlueprintCallable, Category = "Tile Purchase")
	void SetTilePurchaseMode(bool bEnabled) { bIsTilePurchaseMode = bEnabled; }

	// 지금 타일 구매 모드인지 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Tile Purchase")
	bool IsTilePurchaseMode() const { return bIsTilePurchaseMode; }

private:
	// ========== 네트워크 월드 수신 / 로컬 슬롯 ==========

	// 참가자가 호스트에게 받는 타일 버퍼
	TArray<FWorldTileNetData> IncomingNetworkWorldTiles;

	// 참가자가 호스트에게 받는 시작 도시 hex
	TArray<FVector2D> IncomingNetworkCityHexes;

	// 이 기기의 문명 슬롯. 싱글/호스트=0, 1v1 참가자=1.
	UPROPERTY()
	int32 LocalPlayerIndex = 0;
};
