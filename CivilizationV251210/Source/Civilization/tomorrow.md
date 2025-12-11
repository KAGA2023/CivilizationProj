# 전투 시각화 구현 진행 상황 정리

## ✅ 완료된 작업 (1-2단계)

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

2. Blackboard 키 추가
   - `KeyIsInCombat`: 전투 중 상태
   - `KeyCombatComplete`: 전투 완료 상태

3. 전투 관련 함수
   - `StartCombatVisualization()`: 전투 시각화 시작
   - `CompleteCombatVisualization()`: 전투 시각화 완료 처리
   - `IsInCombat()`: 전투 중 여부 확인
   - `GetCombatAttacker()`, `GetCombatDefender()`, `GetCurrentCombatResult()`: 전투 데이터 접근

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

### 추가 수정사항

**MainHUD.cpp:**
- 전투 시작 시점에 UI가 닫히도록 `OnCombatExecuted` 델리게이트 활용 (이미 구현됨)

---

## 📋 앞으로 해야 할 작업

### 3단계: 비헤이비어트리 테스크 생성

**생성할 테스크 파일들:**
1. `BTTask_CompleteCombatVisualization.h` / `.cpp`
   - 역할: 전투 시각화 완료 처리
   - AIController의 `CompleteCombatVisualization()` 호출

2. `BTTask_CheckCounterAttack.h` / `.cpp`
   - 역할: 반격 가능 여부 확인
   - `GetCurrentCombatResult()`에서 `DefenderDamageDealt > 0` 확인

3. `BTTask_WaitForCombatAnimation.h` / `.cpp` (선택사항)
   - 역할: 애니메이션 완료 대기
   - 또는 기존 `BTTask_Wait` 활용 가능

4. `BTTask_PlayCombatAnimation.h` / `.cpp`
   - 역할: 전투 애니메이션 재생
   - 공격자/방어자 애니메이션 재생 및 완료 대기

5. `BTTask_ApplyCombatDamage.h` / `.cpp` (선택사항)
   - 역할: 데미지 적용 타이밍 제어
   - 데미지 숫자 UI 표시 등

**구현 순서 권장:**
1. BTTask_CompleteCombatVisualization (가장 간단)
2. BTTask_CheckCounterAttack
3. BTTask_WaitForCombatAnimation (또는 기존 Wait 사용)
4. BTTask_PlayCombatAnimation
5. BTTask_ApplyCombatDamage (선택사항)

### 4단계: 비헤이비어트리 구성

**블루프린트 작업:**
1. Blackboard (BB_Unit) 설정
   - `IsInCombat` (Bool) 키 추가
   - `CombatComplete` (Bool) 키 추가
   - 경로: `/Game/Civilization/AI/BB_Unit`

2. BehaviorTree (BT_Unit) 구성
   - 전투 시퀀스 노드 추가
   - 경로: `/Game/Civilization/AI/BT_Unit`
   - 구성 예시:
     ```
     Selector (전투 시퀀스)
     ├─ Sequence (공격 단계)
     │  ├─ BTTask_PlayCombatAnimation
     │  ├─ BTTask_ApplyCombatDamage (선택)
     │  └─ BTTask_WaitForCombatAnimation
     │
     ├─ Decorator (반격 체크)
     │  └─ BTTask_CheckCounterAttack
     │     └─ Sequence (반격 단계 - 조건부)
     │        ├─ BTTask_PlayCombatAnimation
     │        ├─ BTTask_ApplyCombatDamage (선택)
     │        └─ BTTask_WaitForCombatAnimation
     │
     └─ BTTask_CompleteCombatVisualization
     ```

### 5단계: 애니메이션 준비

**블루프린트 작업:**
1. 유닛 애니메이션 블루프린트에 전투 애니메이션 추가
   - 공격 애니메이션
   - 피격 애니메이션
   - 반격 애니메이션
   - 사망 애니메이션 (선택)

2. 애니메이션 노티파이 (선택사항)
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
   - 공격 애니메이션 → 반격 체크 → 반격 애니메이션 (조건부) → 완료

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

### 다음 작업 시작 시 확인사항
1. Blackboard에 `IsInCombat`, `CombatComplete` 키가 추가되어 있는지 확인
2. BehaviorTree 구조 확인
3. 유닛 애니메이션 블루프린트에 전투 애니메이션이 있는지 확인

---

## 💡 구현 팁

### 비헤이비어트리 테스크 작성 시
- `GetAIOwner()`로 AIController 참조 획득
- `Cast<AUnitAIController>(GetAIOwner())`로 타입 변환
- AIController의 함수들을 통해 전투 데이터 접근
- `FinishExecute(true)` 또는 `FinishExecute(false)`로 완료 신호 전달

### 애니메이션 처리 시
- 유닛의 SkeletalMeshComponent에서 AnimInstance 가져오기
- Montage 재생 또는 AnimInstance의 변수 설정
- 애니메이션 완료는 노티파이 또는 타이머로 감지

### 반격 처리 시
- `FCombatResult::DefenderDamageDealt > 0`으로 반격 가능 여부 확인
- 반격이 없으면 해당 시퀀스 스킵
- Decorator나 조건 체크 테스크 활용

---

이 정리를 내일 다른 컴퓨터의 Cursor AI에 제공하면 작업을 이어갈 수 있습니다!
