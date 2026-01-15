# 메인메뉴에서 로드 기능 구현 계획

## 📋 개요
메인메뉴에서 세이브 파일을 로드하여 게임을 시작할 수 있도록 구현합니다.
현재 월드 생성 프로세스와 동일한 방식으로 작동하도록 합니다.

## 🔄 현재 월드 생성 프로세스

### 1단계: LoadingUI (메인메뉴 레벨)
- `NativeConstruct()` → `GenerateWorldAsync()` 호출
- `GenerateWorldAsync()`:
  - WorldComponent 생성
  - GameInstance에서 WorldConfig 가져오기
  - WorldComponent에 WorldConfig 설정
  - `GenerateWorld()` 호출 (데이터상으로 월드 생성)
- `OnWorldGenerated()` 콜백:
  - GameInstance에 WorldComponent 저장
  - `LoadLevelAsync()` 호출 (게임 플레이 레벨 비동기 로딩)
- `OnLevelLoaded()` 콜백:
  - 레벨 로딩 완료
- `NativeTick()`:
  - 70% 도달 시 `OpenLevel()`로 게임 플레이 레벨로 이동

### 2단계: LoadingTilesUI (게임 플레이 레벨)
- `NativeConstruct()` → `StartTileSpawning()` 호출
- `StartTileSpawning()`:
  - WorldSpawner 찾기
  - `SpawnAllTiles()` 호출 (타일 액터 비동기 스폰)
  - `SpawnAllCities()` 호출 (도시 액터 스폰)
  - `AssignCitiesToPlayers()` 호출 (도시 배정)
- `OnTileSpawnCompleted()` 콜백:
  - 스폰 완료 플래그 설정
- `NativeTick()`:
  - 100% 도달 시 위젯 닫기

### 3단계: WorldSpawner (게임 플레이 레벨의 Actor)
- `BeginPlay()`:
  - GameInstance에서 WorldComponent 가져오기
  - 매니저들 생성 (UnitManager, FacilityManager, BorderManager, DiplomacyManager, AIPlayerManager)
- `SpawnAllTiles()`: 타일 액터 비동기 스폰
- `SpawnAllCities()`: 도시 액터 스폰
- `AssignCitiesToPlayers()`: 도시를 플레이어들에게 배정

## 🎯 구현 목표
메인메뉴에서 로드 시에도 동일한 프로세스를 따르되, 다음 차이점이 있습니다:
1. **WorldConfig**: 세이브 파일에서 복원
2. **월드 데이터**: 기본 생성 후 저장된 데이터로 덮어쓰기
3. **게임 상태**: 저장된 데이터로 복원

## 📝 구현 순서

### ✅ 1단계: 데이터 구조 수정

#### 1.1 SaveLoadStruct.h
**파일**: `Civilization/SaveLoad/SaveLoadStruct.h`

**변경 사항**:
- `FGameSaveData` 구조체에 `WorldConfig` 필드 추가
  ```cpp
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
  FWorldConfig WorldConfig;
  // 메인메뉴에서 로드 시 월드를 생성하기 위해 WorldConfig 저장 필요
  ```
- `FGameSaveData` 생성자에 `WorldConfig` 초기화 추가

**위치**: `FGameSaveData` 구조체 내부 (약 350번째 줄 근처)

---

#### 1.2 SaveLoadManager.cpp
**파일**: `Civilization/SaveLoad/SaveLoadManager.cpp`

**변경 사항**:
- `CollectGameStateForSave()` 함수에서 `WorldConfig` 수집 추가
  ```cpp
  // WorldConfig 수집 (메인메뉴에서 로드 시 월드 생성에 필요)
  OutSaveData.WorldConfig = GameInstance->GetWorldConfig();
  ```
- 위치: `CollectGameStateForSave()` 함수 내부, `SaveDateTime` 설정 직후

---

### ✅ 2단계: GameInstance에 로드 모드 지원 추가

#### 2.1 SuperGameInstance.h
**파일**: `Civilization/SuperGameInstance.h`

**변경 사항**:
- 로드 모드 플래그 추가
  ```cpp
  // 메인메뉴에서 로드 시 사용할 임시 저장 데이터
  UPROPERTY(BlueprintReadWrite, Category = "Save Load Management")
  FGameSaveData PendingLoadData; // 레벨 이동 후 복원할 데이터

  UPROPERTY(BlueprintReadWrite, Category = "Save Load Management")
  bool bIsLoadingFromMainMenu = false; // 메인메뉴에서 로드 중인지 여부
  ```
- 위치: `ClearSaveLoadManager()` 함수 선언 직후

**포함 파일 추가 필요**:
```cpp
#include "SaveLoad/SaveLoadStruct.h"
```

---

#### 2.2 SuperGameInstance.cpp
**파일**: `Civilization/SuperGameInstance.cpp`

**변경 사항**:
- `PendingLoadData` 초기화 (필요시)
- `bIsLoadingFromMainMenu` 초기화 (필요시)

---

### ✅ 3단계: LoadingUI 수정 (메인메뉴 레벨)

#### 3.1 LoadingUI.h
**파일**: `Civilization/Widget/LoadingUI.h`

**변경 사항**:
- 로드 모드 관련 변수 추가
  ```cpp
  // 로드 모드 관련
  UPROPERTY(BlueprintReadWrite, Category = "Load Mode")
  bool bIsLoadMode = false; // 로드 모드인지 여부

  UPROPERTY(BlueprintReadWrite, Category = "Load Mode")
  int32 LoadSlotIndex = 0; // 로드할 슬롯 인덱스
  ```
- 위치: `ELoadingStage CurrentLoadingStage` 변수 직후

---

#### 3.2 LoadingUI.cpp
**파일**: `Civilization/Widget/LoadingUI.cpp`

**변경 사항**:

1. **포함 파일 추가**:
   ```cpp
   #include "../SaveLoad/SaveLoadManager.h"
   #include "../SaveLoad/SaveLoadStruct.h"
   ```

2. **`GenerateWorldAsync()` 함수 수정**:
   - 로드 모드일 때 세이브 파일에서 `WorldConfig` 복원
   ```cpp
   void ULoadingUI::GenerateWorldAsync()
   {
       // 안전장치: 기존 월드 컴포넌트가 있으면 정리
       if (GameInst)
       {
           GameInst->ClearGeneratedWorldComponent();
       }
       
       // 새 월드 컴포넌트 생성
       WorldComponent = NewObject<UWorldComponent>(this);
       
       // WorldConfig 설정
       FWorldConfig Settings;
       if (bIsLoadMode && GameInst && GameInst->GetSaveLoadManager())
       {
           // 로드 모드: 세이브 파일에서 WorldConfig 복원
           FString SlotName = FString::Printf(TEXT("SaveSlot%d"), LoadSlotIndex);
           USuperSaveGame* SaveGameObject = Cast<USuperSaveGame>(
               UGameplayStatics::LoadGameFromSlot(SlotName, 0));
           if (SaveGameObject)
           {
               Settings = SaveGameObject->SaveData.WorldConfig;
               // SaveData를 GameInstance에 임시 저장
               GameInst->PendingLoadData = SaveGameObject->SaveData;
               GameInst->bIsLoadingFromMainMenu = true;
           }
       }
       else
       {
           // 일반 모드: GameInstance에서 설정 가져오기
           Settings = GameInst->GetWorldConfig();
       }
       
       WorldComponent->SetWorldConfig(Settings);
       
       // 월드 생성 완료 이벤트 바인딩
       WorldComponent->OnWorldGenerated.AddDynamic(this, &ULoadingUI::OnWorldGenerated);
       
       // 월드 생성 시작
       WorldComponent->GenerateWorld();
       
       // 월드 생성 중 진행률 (0% → 50%)
       targetPercent = 0.5f;
   }
   ```

3. **로드 모드 진입 함수 추가** (Blueprint 호출용):
   ```cpp
   void ULoadingUI::SetLoadMode(int32 SlotIndex)
   {
       bIsLoadMode = true;
       LoadSlotIndex = SlotIndex;
   }
   ```
   - 헤더 파일에도 선언 추가 필요

---

### ✅ 4단계: LoadingTilesUI 수정 (게임 플레이 레벨)

#### 4.1 LoadingTilesUI.h
**파일**: `Civilization/Widget/LoadingTilesUI.h`

**변경 사항**:
- 데이터 복원 완료 플래그 추가
  ```cpp
  // 데이터 복원 완료 플래그
  bool bDataRestored = false;
  ```
- 위치: `bool bSpawnCompleted = false;` 직후

---

#### 4.2 LoadingTilesUI.cpp
**파일**: `Civilization/Widget/LoadingTilesUI.cpp`

**변경 사항**:

1. **포함 파일 추가**:
   ```cpp
   #include "../SaveLoad/SaveLoadManager.h"
   #include "../SuperGameInstance.h"
   ```

2. **`OnTileSpawnCompleted()` 함수 수정**:
   ```cpp
   void ULoadingTilesUI::OnTileSpawnCompleted()
   {
       // 스폰 완료 플래그 설정
       bSpawnCompleted = true;
       
       // 메인메뉴에서 로드 중인 경우 데이터 복원
       if (UWorld* World = GetWorld())
       {
           if (USuperGameInstance* GameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
           {
               if (GameInst->bIsLoadingFromMainMenu && GameInst->GetSaveLoadManager())
               {
                   // 저장된 데이터로 게임 상태 복원
                   GameInst->GetSaveLoadManager()->RestoreGameStateFromSave(GameInst->PendingLoadData);
                   
                   // 로드 모드 플래그 해제
                   GameInst->bIsLoadingFromMainMenu = false;
                   
                   bDataRestored = true;
               }
           }
       }
   }
   ```

3. **`NativeTick()` 함수 수정**:
   - 데이터 복원 완료 후에만 Finalizing 단계로 전환
   ```cpp
   // 90% 도달 시 Finalizing으로 전환 (스폰 완료 + 데이터 복원 완료 시에만)
   if (CurrentLoadingStage == ELoadingStage::TileSpawning && 
       bSpawnCompleted && 
       (!GameInst || !GameInst->bIsLoadingFromMainMenu || bDataRestored) &&
       curPercent >= 0.9f)
   {
       CurrentLoadingStage = ELoadingStage::Finalizing;
       targetPercent = 1.0f;
   }
   ```

---

### ✅ 5단계: SaveLoadManager의 RestoreWorldData 함수 개선

#### 5.1 SaveLoadManager.cpp
**파일**: `Civilization/SaveLoad/SaveLoadManager.cpp`

**변경 사항**:
- `RestoreWorldData()` 함수에 시각 업데이트 및 생산량 재계산 추가
- 타일 데이터 변경 후 타일 액터의 시각적 표현도 업데이트
- 생산량 재계산 추가

**수정 내용**:
```cpp
void USaveLoadManager::RestoreWorldData(...)
{
    // WorldSpawner 찾기 (타일 액터 시각 업데이트용)
    AWorldSpawner* WorldSpawner = nullptr;
    // ... WorldSpawner 찾기 로직 ...

    for (const auto& Pair : WorldDataMap)
    {
        // 지형 정보 복원 (데이터 변경)
        Tile->SetTerrainType(WorldData.TerrainType);
        // ... 기타 데이터 변경 ...

        // 타일 생산량 재계산 (데이터 변경 후 필수)
        WorldComponent->RecalculateTileYields(Tile);

        // 타일 액터 시각 업데이트 (이미 스폰된 경우)
        if (WorldSpawner)
        {
            WorldSpawner->UpdateTileVisual(Pair.Key);
        }
    }
}
```

**포함 파일 추가**:
```cpp
#include "EngineUtils.h"
#include "../World/WorldSpawner.h"
```

**중요**: 
- `SetTerrainType()` 등은 `m_TileData`를 직접 변경하므로 **데이터상으로 완전히 변경**됩니다.
- 단순 외관 변경이 아니라 **내부 데이터 구조가 완전히 교체**됩니다.
- 예: 바다(Ocean) → 육지(Land)로 변경 시, 타일의 모든 속성(지형, 기후, 자원 등)이 저장된 데이터로 교체됩니다.

---

### ✅ 6단계: 월드 데이터 복원 로직 확인

#### 6.1 SaveLoadManager.cpp
**파일**: `Civilization/SaveLoad/SaveLoadManager.cpp`

**확인 사항**:
- `RestoreWorldData()` 함수가 올바르게 작동하는지 확인
- 타일 스폰 완료 후 호출되는지 확인
- 기본 생성된 월드 타일을 저장된 데이터로 덮어쓰는지 확인
- 타일 액터의 시각적 표현도 업데이트되는지 확인

**현재 로직**:
```cpp
void USaveLoadManager::RestoreWorldData(...)
{
    // WorldDataMap을 순회하며 각 타일의 속성 복원
    // 1. SetTerrainType, SetClimateType, SetLandType 등 호출 → m_TileData 직접 변경
    // 2. RecalculateTileYields 호출 → 생산량 재계산
    // 3. UpdateTileVisual 호출 → 타일 액터 시각 업데이트
    // 4. 시설 정보 복원
}
```

이 로직은 기본 생성된 월드를 저장된 데이터로 **완전히 덮어쓰며**, 데이터와 시각 모두 업데이트됩니다.

---

## 🔍 실행 흐름 (메인메뉴에서 로드)

1. **메인메뉴에서 로드 버튼 클릭**
   - Blueprint에서 `LoadingUI` 생성
   - `LoadingUI::SetLoadMode(SlotIndex)` 호출
   - `LoadingUI` 표시

2. **LoadingUI 시작**
   - `NativeConstruct()` → `GenerateWorldAsync()` 호출
   - 세이브 파일에서 `WorldConfig` 로드
   - `WorldConfig`로 월드 생성 시작
   - 월드 생성 완료 → 레벨 로딩 시작
   - 레벨 로딩 완료 → 게임 플레이 레벨로 이동

3. **게임 플레이 레벨 진입**
   - `LoadingTilesUI` 표시
   - `WorldSpawner::BeginPlay()`: 매니저 생성
   - `LoadingTilesUI::StartTileSpawning()`: 타일 스폰 시작

4. **타일 스폰 완료**
   - `OnTileSpawnCompleted()` 호출
   - `RestoreGameStateFromSave()` 호출
   - 저장된 데이터로 월드 타일, 플레이어 데이터, 외교 데이터 등 복원

5. **로딩 완료**
   - `LoadingTilesUI` 닫기
   - 게임 시작

---

## ⚠️ 주의사항

1. **타이밍**: 타일 스폰이 완료된 후에만 데이터 복원해야 함
2. **월드 데이터 덮어쓰기**: 기본 생성된 월드를 저장된 데이터로 덮어쓰는 순서 중요
3. **플래그 관리**: `bIsLoadingFromMainMenu` 플래그를 올바르게 설정/해제
4. **에러 처리**: 세이브 파일이 없거나 손상된 경우 처리

---

## 📌 체크리스트

- [ ] 1단계: `SaveLoadStruct.h`에 `WorldConfig` 필드 추가
- [ ] 1단계: `SaveLoadManager.cpp`에서 `WorldConfig` 수집 로직 추가
- [ ] 2단계: `SuperGameInstance.h`에 로드 모드 관련 변수 추가
- [ ] 2단계: `SuperGameInstance.cpp` 초기화 확인
- [ ] 3단계: `LoadingUI.h`에 로드 모드 변수 추가
- [ ] 3단계: `LoadingUI.cpp`에서 `GenerateWorldAsync()` 수정
- [ ] 3단계: `LoadingUI.h`에 `SetLoadMode()` 함수 선언 추가
- [ ] 3단계: `LoadingUI.cpp`에 `SetLoadMode()` 함수 구현 추가
- [ ] 4단계: `LoadingTilesUI.h`에 `bDataRestored` 플래그 추가
- [ ] 4단계: `LoadingTilesUI.cpp`에서 `OnTileSpawnCompleted()` 수정
- [ ] 4단계: `LoadingTilesUI.cpp`에서 `NativeTick()` 수정
- [ ] 5단계: `RestoreWorldData()` 함수 검토 및 테스트
- [ ] 최종 테스트: 메인메뉴에서 로드 → 게임 시작 확인

---

## 🧪 테스트 시나리오

1. **정상 로드 테스트**
   - 메인메뉴에서 세이브 슬롯 선택
   - 로드 버튼 클릭
   - 월드 생성 → 레벨 로딩 → 타일 스폰 → 데이터 복원 확인

2. **데이터 복원 확인**
   - 플레이어 자원, 인구, 타일 소유권 확인
   - 도시 상태, 건물, 생산 진행도 확인
   - 유닛 위치 및 상태 확인
   - 외교 상태, 호감도 확인
   - 월드 타일 지형, 기후, 자원, 시설 확인

3. **에러 처리 테스트**
   - 존재하지 않는 슬롯 로드 시도
   - 손상된 세이브 파일 로드 시도

---

## 📝 참고사항

- 현재 월드 생성 프로세스를 최대한 유지
- 로드 모드와 일반 모드의 차이점을 최소화
- 기존 코드와의 호환성 유지

