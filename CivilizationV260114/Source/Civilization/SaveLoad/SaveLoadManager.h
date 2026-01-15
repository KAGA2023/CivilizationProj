// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SaveLoadStruct.h"
#include "SaveLoadManager.generated.h"

class USuperGameInstance;
class ASuperPlayerState;
class UCityComponent;
class AUnitCharacterBase;
class UWorldComponent;
class UFacilityManager;
class UDiplomacyManager;
class UTurnComponent;
class ASuperGameModeBase;

// 최대 세이브 슬롯 수
constexpr int32 MAX_SAVE_SLOTS = 5;
constexpr int32 MIN_SAVE_SLOT = 1;
constexpr int32 MAX_SAVE_SLOT = 5;

UCLASS()
class CIVILIZATION_API USaveLoadManager : public UObject
{
    GENERATED_BODY()

public:
    USaveLoadManager();

    // ========== 초기화 ==========
    UFUNCTION(BlueprintCallable, Category = "Save Load Manager")
    void SetGameInstance(USuperGameInstance* InGameInstance);

    // ========== 세이브 함수 ==========
    // 현재 게임 상태를 수집하여 세이브 데이터 생성
    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool CollectGameStateForSave(FGameSaveData& OutSaveData);

    // 특정 슬롯에 게임 저장 (1~5)
    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool SaveGameToSlot(int32 SlotIndex, const FString& SaveGameName);

    // ========== 로드 함수 ==========
    // 특정 슬롯에서 게임 로드
    UFUNCTION(BlueprintCallable, Category = "Load System")
    bool LoadGameFromSlot(int32 SlotIndex);

    // 세이브 데이터를 게임 상태로 복원
    UFUNCTION(BlueprintCallable, Category = "Load System")
    bool RestoreGameStateFromSave(const FGameSaveData& SaveData);

    // ========== 세이브 슬롯 관리 ==========
    // 슬롯에 세이브 파일이 있는지 확인
    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool DoesSaveGameExist(int32 SlotIndex);

    // 슬롯의 세이브 파일 정보 가져오기
    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool GetSaveGameInfo(int32 SlotIndex, FString& OutSaveGameName, FDateTime& OutSaveDateTime);

    // 슬롯의 세이브 파일 삭제
    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool DeleteSaveGame(int32 SlotIndex);

    // 모든 세이브 슬롯 정보 가져오기 (1~5)
    UFUNCTION(BlueprintCallable, Category = "Save System")
    TArray<FSaveSlotInfo> GetAllSaveSlotInfos();

    // ========== 데이터 수집 헬퍼 함수들 (내부용) ==========
    // 플레이어 데이터 수집
    void CollectPlayerData(ASuperPlayerState* PlayerState, FPlayerSaveData& OutPlayerData);

    // 도시 데이터 수집
    void CollectCityData(UCityComponent* CityComponent, FCitySaveData& OutCityData);

    // 유닛 데이터 수집
    void CollectUnitData(AUnitCharacterBase* Unit, FUnitSaveData& OutUnitData);

    // 월드 타일 데이터 수집
    void CollectWorldData(UWorldComponent* WorldComponent, UFacilityManager* FacilityManager, TMap<FVector2D, FWorldSaveData>& OutWorldDataMap);

    // 외교 데이터 수집
    void CollectDiplomacyData(UDiplomacyManager* DiplomacyManager, TMap<FDiplomacyPairKey, FDiplomacyPairState>& OutDiplomacyStateMap, TArray<FDiplomacyAction>& OutDiplomacyActionHistory, TMap<FAttitudeKey, int32>& OutAttitudes, int32& OutNextActionId);

    // ========== 데이터 복원 헬퍼 함수들 (내부용) ==========
    // 플레이어 데이터 복원
    void RestorePlayerData(const FPlayerSaveData& PlayerData, ASuperPlayerState* PlayerState);

    // 도시 데이터 복원
    void RestoreCityData(const FCitySaveData& CityData, ASuperPlayerState* PlayerState);

    // 유닛 데이터 복원
    void RestoreUnitData(const FUnitSaveData& UnitData, int32 PlayerIndex);

    // 월드 타일 데이터 복원
    void RestoreWorldData(const TMap<FVector2D, FWorldSaveData>& WorldDataMap, UWorldComponent* WorldComponent, UFacilityManager* FacilityManager);

    // 외교 데이터 복원
    void RestoreDiplomacyData(const TMap<FDiplomacyPairKey, FDiplomacyPairState>& DiplomacyStateMap, const TArray<FDiplomacyAction>& DiplomacyActionHistory, const TMap<FAttitudeKey, int32>& Attitudes, int32 NextActionId, UDiplomacyManager* DiplomacyManager);

private:
    // GameInstance 참조
    UPROPERTY()
    USuperGameInstance* GameInstance = nullptr;

    // 슬롯 이름 가져오기
    FString GetSaveSlotName(int32 SlotIndex) const;

    // 슬롯 인덱스 유효성 검사
    bool IsValidSlotIndex(int32 SlotIndex) const;
};

