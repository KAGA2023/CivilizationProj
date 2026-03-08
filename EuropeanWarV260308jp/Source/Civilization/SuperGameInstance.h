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
	TSoftObjectPtr<UWorld> TargetLevel{}; // 이동할 타겟 레벨
	TSoftObjectPtr<UWorld> OldLevel{}; // 이전 레벨 (언로드용)
public:
	USuperGameInstance(); // 생성자
	virtual void Init() override; // 게임 인스턴스 초기화

	UFUNCTION(BlueprintCallable)
	void OpenLevel(TSoftObjectPtr<UWorld> newLevel);  // 다른 레벨로 이동합니다

	FName GetTargetLevelName() // 현재 타겟 레벨의 이름을 반환합니다
	{
		if (TargetLevel.IsNull())
		{
			return NAME_None;
		}
		// GetAssetName()으로 에셋 이름 가져오기
		FString AssetName = TargetLevel.GetAssetName();
		if (!AssetName.IsEmpty())
		{
			return FName(*AssetName);
		}
		// GetAssetName()이 실패한 경우 패키지 경로에서 추출
		FString PackageName = GetTargetLevelPackageName();
		if (!PackageName.IsEmpty())
		{
			// 경로의 마지막 부분이 레벨 이름 (예: /Game/Civilization/Maps/InGame -> InGame)
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
	
	FString GetTargetLevelPackageName() const // 현재 타겟 레벨의 패키지 경로를 반환합니다
	{
		if (TargetLevel.IsNull())
		{
			return FString();
		}
		return TargetLevel.ToSoftObjectPath().GetLongPackageName();
	}

	// 월드 설정 관리
	UPROPERTY(BlueprintReadWrite, Category = "World Settings") // 현재 월드 설정
	FWorldConfig CurrentWorldConfig;

	UFUNCTION(BlueprintCallable, Category = "World Settings") // 월드 설정 설정
	void SetWorldConfig(const FWorldConfig& NewSettings);

	UFUNCTION(BlueprintCallable, Category = "World Settings") // 월드 설정 가져오기
	FWorldConfig GetWorldConfig() const { return CurrentWorldConfig; }

	// 국가 이름 배열 관리
	UPROPERTY(BlueprintReadWrite, Category = "Country Settings") // 현재 국가 이름 배열
	TArray<FName> CountryNames;

	UFUNCTION(BlueprintCallable, Category = "Country Settings") // 국가 이름 배열 설정
	void SetCountryNames(const TArray<FName>& InCountryNames);

	UFUNCTION(BlueprintCallable, Category = "Country Settings") // 국가 이름 배열 가져오기
	TArray<FName> GetCountryNames() const { return CountryNames; }

	// 월드 컴포넌트 관리 (로딩에서 생성된 월드 컴포넌트 저장)
	UPROPERTY(BlueprintReadWrite, Category = "World Management") // 생성된 월드 컴포넌트
	class UWorldComponent* GeneratedWorldComponent = nullptr;

	UFUNCTION(BlueprintCallable, Category = "World Management") // 월드 컴포넌트 설정
	void SetGeneratedWorldComponent(class UWorldComponent* WorldComponent);

	UFUNCTION(BlueprintCallable, Category = "World Management") // 월드 컴포넌트 가져오기
	class UWorldComponent* GetGeneratedWorldComponent() const { return GeneratedWorldComponent; }

	UFUNCTION(BlueprintCallable, Category = "World Management") // 월드 컴포넌트 정리 (메모리 해제)
	void ClearGeneratedWorldComponent();

	// 유닛 매니저 관리
	UPROPERTY(BlueprintReadWrite, Category = "Unit Management") // 유닛 매니저
	class UUnitManager* UnitManager = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Unit Management") // 유닛 매니저 설정
	void SetUnitManager(class UUnitManager* InUnitManager);

	UFUNCTION(BlueprintCallable, Category = "Unit Management") // 유닛 매니저 가져오기
	class UUnitManager* GetUnitManager() const { return UnitManager; }

	UFUNCTION(BlueprintCallable, Category = "Unit Management") // 유닛 매니저 정리 (메모리 해제)
	void ClearUnitManager();

	// 시설 매니저 관리
	UPROPERTY(BlueprintReadWrite, Category = "Facility Management") // 시설 매니저
	class UFacilityManager* FacilityManager = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Facility Management") // 시설 매니저 설정
	void SetFacilityManager(class UFacilityManager* InFacilityManager);

	UFUNCTION(BlueprintCallable, Category = "Facility Management") // 시설 매니저 가져오기
	class UFacilityManager* GetFacilityManager() const { return FacilityManager; }

	UFUNCTION(BlueprintCallable, Category = "Facility Management") // 시설 매니저 정리 (메모리 해제)
	void ClearFacilityManager();

	// 국경선 매니저 관리
	UPROPERTY(BlueprintReadWrite, Category = "Border Management") // 국경선 매니저
	class UBorderManager* BorderManager = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Border Management") // 국경선 매니저 설정
	void SetBorderManager(class UBorderManager* InBorderManager);

	UFUNCTION(BlueprintCallable, Category = "Border Management") // 국경선 매니저 가져오기
	class UBorderManager* GetBorderManager() const { return BorderManager; }

	UFUNCTION(BlueprintCallable, Category = "Border Management") // 국경선 매니저 정리 (메모리 해제)
	void ClearBorderManager();

	// 외교 매니저 관리
	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy Management") // 외교 매니저
	class UDiplomacyManager* DiplomacyManager = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Diplomacy Management") // 외교 매니저 설정
	void SetDiplomacyManager(class UDiplomacyManager* InDiplomacyManager);

	UFUNCTION(BlueprintCallable, Category = "Diplomacy Management") // 외교 매니저 가져오기
	class UDiplomacyManager* GetDiplomacyManager() const { return DiplomacyManager; }

	UFUNCTION(BlueprintCallable, Category = "Diplomacy Management") // 외교 매니저 정리 (메모리 해제)
	void ClearDiplomacyManager();

	// AI 플레이어 매니저 관리
	UPROPERTY(BlueprintReadWrite, Category = "AI Player Management") // AI 플레이어 매니저
	class UAIPlayerManager* AIPlayerManager = nullptr;

	UFUNCTION(BlueprintCallable, Category = "AI Player Management") // AI 플레이어 매니저 설정
	void SetAIPlayerManager(class UAIPlayerManager* InAIPlayerManager);

	UFUNCTION(BlueprintCallable, Category = "AI Player Management") // AI 플레이어 매니저 가져오기
	class UAIPlayerManager* GetAIPlayerManager() const { return AIPlayerManager; }

	UFUNCTION(BlueprintCallable, Category = "AI Player Management") // AI 플레이어 매니저 정리 (메모리 해제)
	void ClearAIPlayerManager();

	// 플레이어 스테이트 관리
	UPROPERTY(BlueprintReadWrite, Category = "Player Management") // 모든 플레이어 스테이트
	TArray<class ASuperPlayerState*> PlayerStates;

	UFUNCTION(BlueprintCallable, Category = "Player Management") // 플레이어 스테이트 추가
	void AddPlayerState(class ASuperPlayerState* PlayerState);

	UFUNCTION(BlueprintCallable, Category = "Player Management") // 플레이어 스테이트 가져오기
	class ASuperPlayerState* GetPlayerState(int32 PlayerIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Player Management") // 총 플레이어 수
	int32 GetPlayerStateCount() const { return PlayerStates.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Player Management") // 모든 플레이어 스테이트 정리
	void ClearAllPlayerStates();

	// 국가 데이터 테이블 관리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Country Data") // 국가 데이터 테이블
	class UDataTable* CountryDataTable = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Country Data") // 국가 데이터 테이블 가져오기
	class UDataTable* GetCountryDataTable() const { return CountryDataTable; }

	// 세이브/로드 매니저 관리
	UPROPERTY(BlueprintReadWrite, Category = "Save Load Management") // 세이브/로드 매니저
	class USaveLoadManager* SaveLoadManager = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Save Load Management") // 세이브/로드 매니저 설정
	void SetSaveLoadManager(class USaveLoadManager* InSaveLoadManager);

	UFUNCTION(BlueprintCallable, Category = "Save Load Management") // 세이브/로드 매니저 가져오기
	class USaveLoadManager* GetSaveLoadManager() const { return SaveLoadManager; }

	UFUNCTION(BlueprintCallable, Category = "Save Load Management") // 세이브/로드 매니저 정리 (메모리 해제)
	void ClearSaveLoadManager();

	// 메인메뉴에서 로드 시 사용할 임시 저장 데이터
	UPROPERTY(BlueprintReadWrite, Category = "Save Load Management")
	FGameSaveData PendingLoadData; // 레벨 이동 후 복원할 데이터

	UPROPERTY(BlueprintReadWrite, Category = "Save Load Management")
	bool bIsLoadingFromMainMenu = false; // 메인메뉴에서 로드 중인지 여부

	// 메인메뉴에서 로드 시 사용할 슬롯 인덱스
	UPROPERTY(BlueprintReadWrite, Category = "Save Load Management")
	int32 PendingLoadSlotIndex = 0; // 로드할 슬롯 인덱스 (0은 미설정)

	// 타일 구매 모드 (플레이어0이 PurchaseTileBtn으로 구매 가능 타일 클릭 중일 때 true, 유닛 선택 비활성화용)
	UPROPERTY(BlueprintReadWrite, Category = "Tile Purchase")
	bool bIsTilePurchaseMode = false;

	UFUNCTION(BlueprintCallable, Category = "Tile Purchase")
	void SetTilePurchaseMode(bool bEnabled) { bIsTilePurchaseMode = bEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Tile Purchase")
	bool IsTilePurchaseMode() const { return bIsTilePurchaseMode; }

	/** 로딩 화면이 끝날 때 브로드캐스트 (블루프린트에서 MainHUD를 이 시점에 생성하려면 구독). */
	UPROPERTY(BlueprintAssignable, Category = "Save Load Management")
	FOnLoadingScreenFinished OnLoadingScreenFinished;

	UFUNCTION(BlueprintCallable, Category = "Save Load Management")
	void NotifyLoadingScreenFinished();
};
