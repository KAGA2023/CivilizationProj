# 문명6 모작 포트폴리오 영상 스크립트 (3~5분)

- **총 목표 길이**: 5분 이내
- **구간당**: 약 25~35초
- **톤**: 차분한 개발자, 프로그래밍 용어 사용 (배열, 참조, Stencil, 델리게이트, TMap 등)
- **화면**: 게임 플레이 위주, 필요 시 코드/다이어그램 짧게

---

## 0. 인트로 몽타주 (0:00 ~ 0:20)

**화면**: 게임 주요 장면 빠른 컷 (전투, 도시 UI, 월드 맵, 유닛 이동 하이라이트, AI 턴, 세이브/로드 등)

**나레이션 (한두 문장)**  
> C++과 언리얼 엔진으로 구현한 문명6 스타일 턴제 전략 게임 포트폴리오입니다. 하이라이트부터 세이브·로드까지, 구현한 시스템 순서로 간단히 정리해 보겠습니다.

---

## 1. 하이라이트 (0:20 ~ 0:50) — 약 30초

**화면**: 유닛 선택 → 이동 가능 타일 밝게/불가 타일 어둡게, 공격 가능 적 빨간 외곽선. (게임 화면 위주)

**스크립트**  
- 유닛을 선택하면, 1턴 안에 갈 수 있는 타일을 BFS로 계산해서 `TArray<FVector2D>`에 넣습니다.  
- 이 타일들은 **Custom Depth Stencil**으로 구분합니다. 이동 가능 타일은 Stencil 1로 밝게, 이동 불가 타일은 Stencil 2로 어둡게 세팅하고, 포스트 프로세스 머티리얼에서 Stencil 값에 따라 밝기를 나눕니다.  
- 공격 가능한 적 유닛은 같은 방식으로 Stencil 4를 줘서 빨간 외곽선으로 표시합니다.  
- `UnitManager`의 `ShowMovementRangeWithBrightness`, `ClearAllTileBrightness`에서 타일 액터에 `SetTileBrightness(StencilValue)`를 호출하고, 적용한 타일은 `BrightnessAffectedTiles` 배열에 넣어서 나중에 한 번에 초기화합니다.

**참고 코드**: `UnitManager.cpp` ShowMovementRangeWithBrightness, ClearAllTileBrightness / `WorldTileActor` SetTileBrightness, ResetBrightness

---

## 2. 월드 생성 (0:50 ~ 1:20) — 약 30초

**화면**: 로딩 후 맵이 채워지는 장면, 또는 판게아 형태 지형이 보이는 인게임 맵.

**스크립트**  
- 월드는 **런타임**에 생성됩니다. `WorldComponent`가 `FWorldConfig`를 받아서, 반지름 기준으로 육각형 좌표를 돌며 `TMap<FVector2D, UWorldTile*> HexTiles`에 타일 데이터를 채웁니다.  
- 지형은 `GenerateTerrain()` → `GeneratePangaeaTerrain()`으로 판게아 형태를 만들고, 그 다음 `GenerateLandTypes`, `GenerateClimateZones`, `GenerateResources` 등으로 땅 타입·기후·자원을 채웁니다.  
- 데이터가 준비되면 `WorldSpawner`가 `SpawnAllTiles()`로 각 Hex 좌표마다 `AWorldTileActor`를 스폰하고, `TMap<FVector2D, AWorldTileActor*> TileActors`에 넣어서 데이터와 액터를 1:1로 매핑합니다.

**참고 코드**: `WorldComponent` GenerateWorld, InitializeHexTiles, GeneratePangaeaTerrain / `WorldSpawner` SpawnAllTiles

---

## 3. 월드 관리 (1:20 ~ 1:45) — 약 25초

**화면**: 타일 기반 맵 줌아웃, 타일 클릭/호버 시 정보 나오는 장면.

**스크립트**  
- 타일 데이터는 `UWorldTile`로, 좌표는 `FVector2D` axial/offset 육각형입니다. `WorldComponent`가 `GetTileAtHex`, `GetHexNeighbors`, `GetHexDistance`, `HexToWorld`, `WorldToHex` 같은 유틸을 제공하고, 이동 비용·전투 보너스는 타일 데이터에서 계산해 캐시합니다.  
- 타일 소유권은 `SuperPlayerState`의 `TArray<FVector2D> OwnedTileCoordinates`에 넣고, 국경선은 `BorderManager`가 이 좌표들을 참조해서 그립니다.  
- 호버 시 어떤 타일인지는 `WorldTileActor`에서 레이/클릭으로 Hex를 구한 뒤, `WorldComponent->GetTileAtHex(Hex)`로 데이터를 읽어 UI에 넘깁니다.

**참고 코드**: `WorldComponent` HexTiles, GetTileAtHex, GetHexNeighbors, HexToWorld / `SuperPlayerState` OwnedTileCoordinates

---

## 4. 플레이어·도시·연구·자료 관리 (1:45 ~ 2:25) — 약 40초

**화면**: 상단 자원 UI, 도시 패널(생산 목록·진행도), 연구 창. (게임 화면 위주)

**스크립트**  
- 플레이어 수가 정해지면 그 수만큼 `ASuperPlayerState`를 만들어 `GameInstance`의 `TArray<ASuperPlayerState*> PlayerStates`에 넣습니다.  
- 각 `SuperPlayerState`는 자원을 `int32` 멤버로, 소유 타일은 `TArray<FVector2D> OwnedTileCoordinates`, 소유 유닛은 `TArray<AUnitCharacterBase*> OwnedUnits`로 들고 있고, 도시·연구는 `UCityComponent`, `UResearchComponent` 참조를 하나씩 가집니다.  
- 도시 생산·연구 진행도가 바뀌면 `CityComponent`·`ResearchComponent`에서 델리게이트를 브로드캐스트하고, UI 위젯은 이 델리게이트에 바인딩해서 숫자와 진행 바를 갱신합니다.  
- 건설 가능 건물·유닛·기술 목록은 데이터 테이블과 선행 조건을 비교해서 배열로 계산하고, UI는 그 배열을 참조해 버튼 목록을 만듭니다.

**참고 코드**: `SuperGameInstance` PlayerStates / `SuperPlayerState` 자원·CityComponent·ResearchComponent·OwnedTileCoordinates·OwnedUnits / `CityComponent`·`ResearchComponent` 델리게이트

---

## 5. 유닛 관리 (2:25 ~ 3:00) — 약 35초

**화면**: 유닛 선택 → 이동 범위 하이라이트 → 타일 클릭해 이동, 전투 유닛 선택 후 적 공격.

**스크립트**  
- 유닛 위치는 `UnitManager`의 `TMap<FVector2D, AUnitCharacterBase*> HexToUnitMap`으로 관리하고, 이동 시에는 기존 Hex에서 제거한 뒤 목표 Hex에 넣습니다.  
- 경로 탐색은 **A***입니다. `FAStarNode`에 Hex 좌표, G·H·F 비용, Parent를 두고, `FindPath(StartHex, EndHex, MoverPlayerIndex)`에서 열린/닫힌 집합으로 최단 경로를 구합니다. 이동 비용은 타일·시설·층수(고지대)를 반영하고, 국경은 `CanPlayerEnterTile`로 체크해서 적국 땅은 전쟁/동맹일 때만 통과합니다.  
- 이동 가능 범위는 BFS로 최대 이동력 안의 타일을 `TArray<FVector2D>`로 구한 뒤, 아까 말한 Stencil 밝기로 표시합니다.  
- 전투는 `HandleCombatSelection`으로 공격자·방어자 타일을 두 번 클릭해 고른 뒤 `ExecuteCombatBetweenSelectedUnits()`로 실행하고, `UnitVisualizationComponent`의 전투 스테이트머신으로 공격·반격·사망 연출을 순서대로 재생한 뒤 `OnCombatVisualizationComplete`로 로직 쪽에 알립니다.

**참고 코드**: `UnitManager` HexToUnitMap, FindPath, FAStarNode, CalculateMovementRange, ShowMovementRangeWithBrightness, HandleCombatSelection, ExecuteCombatBetweenSelectedUnits / `UnitVisualizationComponent` ECombatVisualizationState, StartCombatVisualization

---

## 6. 시설 관리 (3:00 ~ 3:25) — 약 25초

**화면**: 건설자로 타일 선택 → 시설 건설, 시설이 올라간 타일들.

**스크립트**  
- 시설 데이터는 `FacilityManager`의 `TMap<FVector2D, FFacilityData> BuiltFacilities`에 타일 좌표별로 들어가고, 월드에 보이는 액터는 `TMap<FVector2D, AFacilityActor*> BuiltFacilityActors`로 같은 좌표에 매핑합니다.  
- 건설 가능 여부는 `CanBuildFacilityOnTile(FacilityRowName, Tile)`에서 타일 지형·자원 조건과 플레이어의 `AvailableFacilities` 배열을 보고 판단합니다.  
- 건설자는 `UnitManager`의 `RequestBuilderBuildFacility(Hex, FacilityRowName)`으로 요청하면, 전투 몽타주 한 번 재생 후 `FacilityManager->BuildFacility`를 호출하고, 건설자 유닛은 제거됩니다. 약탈·수리도 같은 매니저의 `SetFacilityPillaged`, `RepairFacility`로 처리합니다.

**참고 코드**: `FacilityManager` BuiltFacilities, BuiltFacilityActors, BuildFacility, CanBuildFacilityOnTile / `UnitManager` RequestBuilderBuildFacility

---

## 7. AI 턴 (3:25 ~ 4:00) — 약 35초

**화면**: AI 턴 진행 시 유닛 이동·전투·도시 생산 등이 자동으로 돌아가는 장면.

**스크립트**  
- AI는 `AIPlayerManager`의 `TMap<int32, FAIPlayerStruct> AIPlayers`로 플레이어 인덱스별로 관리되고, 턴이 오면 `StartAITurn(PlayerIndex)` → `UpdateStateMachine(PlayerIndex)`를 반복 호출합니다.  
- `FAIPlayerStruct` 안에 `EAITurnState CurrentState`가 있고, 외교 → 연구 → 도시 생산 → 타일 구매 → 시설 → 건설자 이동 → 건설자 시설 건설 → 전투 유닛 이동 → 전투 유닛 전투 순으로 상태가 바뀌다가 `TurnComplete`가 되면 다음 플레이어로 넘깁니다.  
- 유닛 이동·전투처럼 시간이 걸리는 작업은 비동기로 돌리고, `OnUnitMovementFinished`, `OnCombatActionFinished` 콜백이 오면 `PendingPostAsyncState`로 다음 상태로 넘어가거나 턴을 끝냅니다. 전투 연출은 플레이어와 동일하게 `UnitVisualizationComponent` 스테이트머신으로 시각화합니다.

**참고 코드**: `AIPlayerManager` StartAITurn, UpdateStateMachine, ProcessXXXState, OnUnitMovementFinished, OnCombatActionFinished / `AIPlayerStruct` EAITurnState, CurrentState, PendingPostAsyncState

---

## 8. 외교 관리 (4:00 ~ 4:25) — 약 25초

**화면**: 외교 UI에서 전쟁/평화/동맹 상태 보기 또는 선포하는 장면.

**스크립트**  
- `DiplomacyManager`는 `TMap<FDiplomacyPairKey, FDiplomacyPairState> PairStates`로 플레이어 쌍별 전쟁/평화/동맹 상태를 들고 있고, `GetStatus(PlayerA, PlayerB)`로 조회합니다.  
- 전쟁 선포·평화·동맹은 `DeclareWar`, `MakePeace`, `MakeAlliance`로 상태를 바꾸고, `OnDiplomacyStatusChanged` 델리게이트를 브로드캐스트해서 UI와 국경 진입 조건을 갱신합니다.  
- 외교 액션(제안·요청)은 `IssueAction`, `ResolveAction`으로 대기 목록에 넣었다가 수락/거절 시 처리하고, 호감도는 `Attitudes` 맵으로 From→To 점수를 관리합니다.

**참고 코드**: `DiplomacyManager` PairStates, GetStatus, DeclareWar, MakePeace, MakeAlliance, IssueAction, ResolveAction

---

## 9. 세이브·로드 (4:25 ~ 5:00) — 약 35초

**화면**: 일시정지 메뉴에서 세이브 슬롯 선택 후 저장, 메인 메뉴에서 로드 후 인게임 복원되는 장면.

**스크립트**  
- `SaveLoadManager`가 `CollectGameStateForSave(OutSaveData)`로 현재 상태를 한 번에 수집합니다. 플레이어는 `CollectPlayerData`, 도시는 `CollectCityData`, 유닛은 `CollectUnitData`, 월드 타일·시설은 `CollectWorldData`, 외교는 `CollectDiplomacyData`로 각각 `FGameSaveData` 안의 배열·TMap에 채웁니다.  
- 저장은 `SaveGameToSlot(SlotIndex, SaveGameName)`에서 `USuperSaveGame`에 `FGameSaveData`를 넣고 언리얼 `SaveGameToSlot` API로 슬롯에 씁니다.  
- 로드는 `LoadGameFromSlot(SlotIndex)`로 슬롯에서 읽은 뒤, 레벨 이동이 필요하면 `GameInstance`에 `PendingLoadData`와 `PendingLoadSlotIndex`를 넣고, 인게임 레벨이 뜬 다음 `RestoreGameStateFromSave(SaveData)`를 호출합니다. 월드는 `WorldComponent->GenerateWorldFromSaveData`로 복원하고, 플레이어·도시·유닛·시설·외교를 순서대로 `RestoreXXX` 함수들로 복원합니다.

**참고 코드**: `SaveLoadManager` CollectGameStateForSave, SaveGameToSlot, LoadGameFromSlot, RestoreGameStateFromSave / `SaveLoadStruct` FGameSaveData, FPlayerSaveData, FWorldSaveData 등 / `WorldComponent` GenerateWorldFromSaveData

---

## 체크리스트 (촬영 전)

- [ ] 각 구간당 25~35초 맞추기 (총 5분 이내)
- [ ] 화면은 게임 플레이 위주, 코드는 필요할 때만 짧게
- [ ] 개발자 톤 유지 (배열, TMap, 참조, Stencil, 델리게이트, A*, 스테이트 등)
- [ ] 인트로 몽타주 20초로 시선 끌기
