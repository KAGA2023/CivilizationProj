# 전투 시각화 구현 진행 상황 정리

## ✅ 완료된 작업 (1-3단계)

### 1단계: UnitAIController에 전투 기능 추가

**수정된 파일:**
- `Civilization/AICon/UnitAIController.h`
- `Civilization/AICon/UnitAIController.cpp`

**추가된 내용:**
1. 전투 상태 관리 변수
   - `CombatAttacker`: 공격자 참조
   - `CombatDefender`: 방어자 참조
   - `CurrentCombatResult`: 전투 결과 저장
   - `bIsInCombat`: 전투 중 여부
   - `AttackerOriginalHexPosition`: 공격자 원래 위치 (복귀용)

2. Blackboard 키 추가
   - `KeyIsInCombat`: 전투 중 상태
   - `KeyCombatComplete`: 전투 완료 상태

3. 전투 관련 함수
   - `StartCombatVisualization()`: 전투 시각화 시작
   - `CompleteCombatVisualization()`: 전투 시각화 완료 처리
   - `IsInCombat()`: 전투 중 여부 확인
   - `GetCombatAttacker()`, `GetCombatDefender()`, `GetCurrentCombatResult()`: 전투 데이터 접근
   - `GetAttackerOriginalHexPosition()`: 공격자 원래 위치 접근

**주의사항:**
- `UnitAIController.h`에 `#include "../Combat/UnitCombatStruct.h"` 추가됨 (FCombatResult 사용을 위해)

### 2단계: UnitManager 수정

**수정된 파일:**
- `Civilization/Unit/UnitManager.h`
- `Civilization/Unit/UnitManager.cpp`

**수정된 내용:**
1. `ExecuteCombatBetweenSelectedUnits()` 수정
   - 전투 계산은 즉시 실행 (기존 로직 유지)
   - 결과를 AIController에 전달하여 시각화 시작
   - 전투 시작 시점에 `OnCombatExecuted.Broadcast()` 호출 (UI 닫기용)

2. `OnCombatVisualizationComplete()` 추가
   - 전투 시각화 완료 후 호출되는 콜백 함수
   - 유닛 제거 처리 (사망한 유닛)
   - 파라미터: 공격자, 방어자, 전투 결과, Hex 좌표들

**주의사항:**
- `UnitManager.h`에 `struct FCombatResult;` 전방 선언 추가됨

### 3단계: 비헤이비어트리 테스크 생성

**생성된 테스크 파일들:**

1. ✅ `BTTask_CompleteCombatVisualization.h` / `.cpp`
   - 역할: 전투 시각화 완료 처리
   - AIController의 `CompleteCombatVisualization()` 호출
   - 즉시 Succeeded 반환

2. ✅ `BTTask_CheckCounterAttack.h` / `.cpp`
   - 역할: 반격 가능 여부 확인
   - `GetCurrentCombatResult()`에서 `DefenderDamageDealt > 0` 확인
   - 공격자/방어자 생존 여부 확인
   - Blackboard에 `CanCounterAttack` (Bool) 키로 결과 저장
   - 반격 가능: Succeeded, 불가능: Failed 반환

3. ✅ `BTTask_PlayCombatAnimation.h` / `.cpp`
   - 역할: 전투 애니메이션 재생
   - 공격자/방어자 구분하여 적절한 Montage 재생
   - `FUnitData`에서 `AttackMontage` 또는 `HitMontage` 가져오기
   - `TickTask`로 애니메이션 완료 대기
   - Montage가 None이면 Failed 반환 (애니메이션 없으면 스킵 불가)

4. ✅ `BTTask_MoveAttackerToDefenderTile.h` / `.cpp`
   - 역할: 공격자가 방어자 타일로 이동
   - 방어자 타일의 공격자 배치 위치 계산 (방어자로부터 50.0f 떨어진 위치)
   - 한 층 위로 이동 시 점프 처리 (평지→언덕, 언덕→산)
   - 한 층 아래로 이동 시 자연 낙하 처리
   - 시작 타일 저장하여 점프 조건 정확히 체크

5. ✅ `BTTask_RotateDefenderToFaceAttacker.h` / `.cpp`
   - 역할: 방어자가 공격자를 향해 회전
   - 공격자 위치 계산하여 방어자가 바라보도록 회전

**구현 세부사항:**
- `BTTask_PlayCombatAnimation`: `UnitCharacterBase::GetUnitData()`를 통해 `FUnitData` 접근
- `BTTask_MoveAttackerToDefenderTile`: 헥스 좌표를 정수로 반올림하여 `GetTileAtHex()` 정확히 동작하도록 수정

### 추가 수정사항

**UnitCharacterBase:**
- `UnitDataRowName` 멤버 변수 추가 (RowName 저장용)
- `GetUnitData()` 함수 추가 (C++ 전용, FUnitData 접근)
- `InitializeUnit()`에서 `UnitDataRowName` 저장

**FUnitData (UnitDataStruct.h):**
- 애니메이션 Montage 변수 추가:
  - `AttackMontage`: 공격 몽타주
  - `DeathMontage`: 죽음 몽타주
  - `SelectMontage`: 선택 몽타주
  - `HitMontage`: 피격 몽타주

**MainHUD.cpp:**
- 전투 시작 시점에 UI가 닫히도록 `OnCombatExecuted` 델리게이트 활용 (이미 구현됨)

**UnitCharacterBase:**
- 유닛끼리 충돌하지 않도록 설정
  - CapsuleComponent: `SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore)`
  - SkeletalMeshComponent: `SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore)`

---

## 📋 앞으로 해야 할 작업

### 4단계: 비헤이비어트리 구성 (블루프린트 작업)

**1. Blackboard (BB_Unit) 설정**
   - 경로: `/Game/Civilization/AI/BB_Unit`
   - 추가할 키들:
     - `IsInCombat` (Bool) - 전투 중 여부
     - `CombatComplete` (Bool) - 전투 완료 여부
     - `CanCounterAttack` (Bool) - 반격 가능 여부 (BTTask_CheckCounterAttack에서 설정)

**2. BehaviorTree (BT_Unit) 구성**
   - 경로: `/Game/Civilization/AI/BT_Unit`
   - 구성 예시:
     ```
     Selector (Root - 전투 시퀀스)
     │
     ├─ Decorator: Blackboard (조건: IsInCombat == true)
     │  └─ Sequence (전투 시각화)
     │     │
     │     ├─ Sequence (공격 단계)
     │     │  ├─ BTTask_MoveAttackerToDefenderTile
     │     │  │  └─ (공격자가 방어자 타일로 이동)
     │     │  │
     │     │  ├─ BTTask_RotateDefenderToFaceAttacker
     │     │  │  └─ (방어자가 공격자를 향해 회전)
     │     │  │
     │     │  └─ BTTask_PlayCombatAnimation
     │     │     └─ (공격자: AttackMontage, 방어자: HitMontage 재생)
     │     │
     │     ├─ BTTask_CheckCounterAttack
     │     │  └─ (반격 가능 여부 확인 및 Blackboard에 CanCounterAttack 설정)
     │     │
     │     ├─ Decorator: Blackboard (조건: CanCounterAttack == true)
     │     │  └─ Sequence (반격 단계 - 조건부)
     │     │     └─ BTTask_PlayCombatAnimation
     │     │        └─ (방어자: AttackMontage, 공격자: HitMontage 재생)
     │     │
     │     └─ BTTask_CompleteCombatVisualization
     │        └─ (전투 시각화 완료 처리)
     │
     └─ (기존 이동/일반 행동 로직)
     ```

**구성 순서:**
1. Root Selector 생성
2. Decorator: Blackboard 추가 (IsInCombat == true)
3. Sequence (전투 시각화) 추가
4. Sequence (공격 단계) 추가
   - BTTask_MoveAttackerToDefenderTile
   - BTTask_RotateDefenderToFaceAttacker
   - BTTask_PlayCombatAnimation
5. BTTask_CheckCounterAttack 추가
6. Decorator: Blackboard 추가 (CanCounterAttack == true)
7. Sequence (반격 단계) 추가
   - BTTask_PlayCombatAnimation
8. BTTask_CompleteCombatVisualization 추가

### 5단계: 애니메이션 준비 (블루프린트 작업)

**블루프린트 작업:**
1. 유닛 애니메이션 블루프린트에 전투 애니메이션 추가
   - 공격 애니메이션 (AttackMontage)
   - 피격 애니메이션 (HitMontage)
   - 반격 애니메이션 (AttackMontage 재사용 또는 별도)
   - 사망 애니메이션 (DeathMontage) (선택)

2. 데이터 테이블 (DT_UnitData) 설정
   - 각 유닛 Row에 Montage 할당:
     - AttackMontage: 공격 애니메이션
     - HitMontage: 피격 애니메이션
     - DeathMontage: 사망 애니메이션 (선택)
     - SelectMontage: 선택 애니메이션 (선택)

3. 애니메이션 노티파이 (선택사항)
   - 데미지 적용 타이밍 마커
   - 이펙트 재생 타이밍 마커

### 6단계: 테스트 및 디버깅

1. 단계별 테스트
   - 공격 애니메이션만 테스트
   - 반격 시퀀스 테스트
   - 전체 전투 시퀀스 테스트

2. 예외 처리 확인
   - 유닛 사망 시 즉시 사망 애니메이션
   - 반격 불가 시 반격 단계 스킵
   - AIController가 없는 경우 폴백 처리
   - Montage가 None인 경우 처리 (현재는 Failed 반환)

---

## 🔑 핵심 참고사항

### 현재 구조
- **전투 계산**: `UnitCombatComponent::ExecuteCombat()`에서 즉시 실행 (데미지 적용 포함)
- **전투 시각화**: `AUnitAIController::StartCombatVisualization()`에서 비동기로 시작
- **전투 완료**: `AUnitAIController::CompleteCombatVisualization()` → `UnitManager::OnCombatVisualizationComplete()`

### 데이터 흐름
1. `UnitManager::ExecuteCombatBetweenSelectedUnits()`
   - 전투 계산 실행
   - `AIController->StartCombatVisualization()` 호출
   - `OnCombatExecuted.Broadcast()` 호출 (UI 닫기)

2. `AUnitAIController::StartCombatVisualization()`
   - 전투 데이터 저장
   - Blackboard 업데이트 (`IsInCombat = true`)
   - 비헤이비어트리 실행 (전투 시퀀스)

3. 비헤이비어트리 테스크들
   - 공격자 이동 → 방어자 회전 → 공격 애니메이션 → 반격 체크 → 반격 애니메이션 (조건부) → 완료

4. `AUnitAIController::CompleteCombatVisualization()`
   - `UnitManager::OnCombatVisualizationComplete()` 호출
   - 전투 데이터 초기화

5. `UnitManager::OnCombatVisualizationComplete()`
   - 유닛 제거 (사망한 유닛)
   - 후처리

### 중요 파일 위치
- **AIController**: `Civilization/AICon/UnitAIController.h`, `.cpp`
- **UnitManager**: `Civilization/Unit/UnitManager.h`, `.cpp`
- **전투 컴포넌트**: `Civilization/Combat/UnitCombatComponent.h`, `.cpp`
- **전투 구조체**: `Civilization/Combat/UnitCombatStruct.h`
- **MainHUD**: `Civilization/Widget/MainHUD.h`, `.cpp`
- **Task들**: `Civilization/AICon/Task/` 폴더
- **UnitCharacterBase**: `Civilization/Unit/UnitCharacterBase.h`, `.cpp`
- **FUnitData**: `Civilization/Status/UnitDataStruct.h`

### 다음 작업 시작 시 확인사항
1. Blackboard에 `IsInCombat`, `CombatComplete`, `CanCounterAttack` 키가 추가되어 있는지 확인
2. BehaviorTree 구조 확인
3. 유닛 애니메이션 블루프린트에 전투 애니메이션이 있는지 확인
4. 데이터 테이블에 Montage가 할당되어 있는지 확인

---

## 💡 구현 팁

### 비헤이비어트리 테스크 작성 시
- `GetAIOwner()`로 AIController 참조 획득
- `Cast<AUnitAIController>(GetAIOwner())`로 타입 변환
- AIController의 함수들을 통해 전투 데이터 접근
- `FinishExecute(true)` 또는 `FinishExecute(false)`로 완료 신호 전달
- `FinishLatentTask()`로 비동기 작업 완료 신호 전달

### 애니메이션 처리 시
- 유닛의 SkeletalMeshComponent에서 AnimInstance 가져오기
- Montage 재생 또는 AnimInstance의 변수 설정
- 애니메이션 완료는 `Montage_IsPlaying()` 또는 노티파이로 감지
- `TickTask`를 사용하여 애니메이션 완료 대기

### 반격 처리 시
- `FCombatResult::DefenderDamageDealt > 0`으로 반격 가능 여부 확인
- 반격이 없으면 해당 시퀀스 스킵
- Decorator나 조건 체크 테스크 활용

### 이동 처리 시
- 헥스 좌표를 정수로 반올림하여 `GetTileAtHex()` 정확히 동작
- 시작 타일 저장하여 점프 조건 정확히 체크
- 한 층 위/아래 이동 시 점프/낙하 처리

---

이 정리를 내일 다른 컴퓨터의 Cursor AI에 제공하면 작업을 이어갈 수 있습니다!

