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

// 세이브 슬롯 범위
constexpr int32 MIN_SAVE_SLOT = 1;
constexpr int32 MAX_SAVE_SLOT = 5;

UCLASS()
class CIVILIZATION_API USaveLoadManager : public UObject
{
    GENERATED_BODY()

public:
    USaveLoadManager();

    // ========== 초기화 ==========

    // GI를 연결합니다. 세이브/로드가 매니저를 찾을 때 씁니다.
    UFUNCTION(BlueprintCallable, Category = "Save Load Manager")
    void SetGameInstance(USuperGameInstance* InGameInstance);

    // ========== 세이브 ==========

    // 지금 게임 상태를 모아 세이브 데이터를 만듭니다.
    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool CollectGameStateForSave(FGameSaveData& OutSaveData);

    // 슬롯 1~5에 게임을 저장합니다.
    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool SaveGameToSlot(int32 SlotIndex, const FString& SaveGameName);

    // ========== 로드 ==========

    // 해당 슬롯에서 게임을 읽습니다.
    UFUNCTION(BlueprintCallable, Category = "Load System")
    bool LoadGameFromSlot(int32 SlotIndex);

    // 세이브 데이터를 인게임 상태로 되돌립니다.
    UFUNCTION(BlueprintCallable, Category = "Load System")
    bool RestoreGameStateFromSave(const FGameSaveData& SaveData);

    // ========== 슬롯 관리 ==========

    // 해당 슬롯에 세이브 파일이 있는지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool DoesSaveGameExist(int32 SlotIndex);

    // 슬롯의 세이브 이름과 저장 시각을 가져옵니다.
    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool GetSaveGameInfo(int32 SlotIndex, FString& OutSaveGameName, FDateTime& OutSaveDateTime);

    // 해당 슬롯의 세이브 파일을 지웁니다.
    UFUNCTION(BlueprintCallable, Category = "Save System")
    bool DeleteSaveGame(int32 SlotIndex);

    // 슬롯 1~5의 존재 여부를 모두 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Save System")
    TArray<FSaveSlotInfo> GetAllSaveSlotInfos();

    // ========== 수집 헬퍼 ==========

    // PlayerState에서 자원·도시·유닛을 모읍니다.
    void CollectPlayerData(ASuperPlayerState* PlayerState, FPlayerSaveData& OutPlayerData);

    // 도시 체력·건물·생산 진행도를 모읍니다.
    void CollectCityData(UCityComponent* CityComponent, FCitySaveData& OutCityData);

    // 유닛 위치와 현재 스탯을 모읍니다.
    void CollectUnitData(AUnitCharacterBase* Unit, FUnitSaveData& OutUnitData);

    // 타일 지형·자원·시설을 모읍니다.
    void CollectWorldData(UWorldComponent* WorldComponent, UFacilityManager* FacilityManager, TMap<FVector2D, FWorldSaveData>& OutWorldDataMap);

    // 전쟁·평화·호감도를 모읍니다.
    void CollectDiplomacyData(UDiplomacyManager* DiplomacyManager, TMap<FDiplomacyPairKey, FDiplomacyPairState>& OutDiplomacyStateMap, TArray<FDiplomacyAction>& OutDiplomacyActionHistory, TMap<FAttitudeKey, int32>& OutAttitudes, int32& OutNextActionId);

    // ========== 복원 헬퍼 ==========

    // 자원·소유 타일·연구를 PlayerState에 되돌립니다.
    void RestorePlayerData(const FPlayerSaveData& PlayerData, ASuperPlayerState* PlayerState);

    // 도시 체력·건물·생산을 되돌립니다.
    void RestoreCityData(const FCitySaveData& CityData, ASuperPlayerState* PlayerState);

    // 유닛을 다시 스폰하고 스탯을 맞춥니다.
    void RestoreUnitData(const FUnitSaveData& UnitData, int32 PlayerIndex);

    // 타일 지형·자원·시설을 되돌립니다.
    void RestoreWorldData(const TMap<FVector2D, FWorldSaveData>& WorldDataMap, UWorldComponent* WorldComponent, UFacilityManager* FacilityManager);

    // 외교 상태와 호감도를 되돌립니다.
    void RestoreDiplomacyData(const TMap<FDiplomacyPairKey, FDiplomacyPairState>& DiplomacyStateMap, const TArray<FDiplomacyAction>& DiplomacyActionHistory, const TMap<FAttitudeKey, int32>& Attitudes, int32 NextActionId, UDiplomacyManager* DiplomacyManager);

private:
    // ========== 내부 ==========

    // 세이브/로드가 매니저를 찾을 때 쓰는 GI
    UPROPERTY()
    USuperGameInstance* GameInstance = nullptr;

    // 슬롯 번호를 파일 이름으로 바꿉니다.
    FString GetSaveSlotName(int32 SlotIndex) const;

    // 슬롯이 1~5인지 검사합니다.
    bool IsValidSlotIndex(int32 SlotIndex) const;
};
