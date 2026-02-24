// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitCombatUI.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "../../Unit/UnitCharacterBase.h"
#include "../../Status/UnitStatusComponent.h"
#include "../../Combat/UnitCombatComponent.h"
#include "../../World/WorldComponent.h"
#include "../../SuperGameInstance.h"
#include "../../City/CityComponent.h"

void UUnitCombatUI::SetupForCombat(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender, FVector2D AttackerHex, FVector2D DefenderHex)
{
	if (!Attacker || !Defender)
	{
		return;
	}

	// 공격자/방어자 정보 가져오기
	UUnitStatusComponent* AttackerStatusComp = Attacker->GetUnitStatusComponent();
	UUnitStatusComponent* DefenderStatusComp = Defender->GetUnitStatusComponent();
	UUnitCombatComponent* AttackerCombatComp = Attacker->GetUnitCombatComponent();
	UUnitCombatComponent* DefenderCombatComp = Defender->GetUnitCombatComponent();
	
	if (!AttackerStatusComp || !DefenderStatusComp || !AttackerCombatComp || !DefenderCombatComp)
	{
		return;
	}

	// ========== 1단계: 한 번만 가져올 데이터들을 함수 시작 부분에서 미리 계산 ==========
	
	// WorldComponent 가져오기 (한 번만)
	UWorldComponent* WorldComponent = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			WorldComponent = SuperGameInst->GetGeneratedWorldComponent();
		}
	}

	// 체력 정보 한 번만 가져오기
	int32 AttackerCurrentHealth = AttackerStatusComp->GetCurrentHealth();
	int32 AttackerMaxHealth = AttackerStatusComp->GetMaxHealth();
	int32 DefenderCurrentHealth = DefenderStatusComp->GetCurrentHealth();
	int32 DefenderMaxHealth = DefenderStatusComp->GetMaxHealth();

	// 공격력/방어력 한 번만 가져오기
	int32 AttackerAttackStrength = AttackerStatusComp->GetAttackStrength();
	int32 DefenderDefenseStrength = DefenderStatusComp->GetDefenseStrength();

	// HexDistance 한 번만 계산
	int32 HexDistance = WorldComponent ? WorldComponent->GetHexDistance(AttackerHex, DefenderHex) : 0;

	// 층수 정보 한 번만 계산
	int32 AttackerFloor = AttackerCombatComp->GetFloorLevelAtHex(AttackerHex);
	int32 DefenderFloor = DefenderCombatComp->GetFloorLevelAtHex(DefenderHex);
	int32 FloorDifference = FMath::Abs(AttackerFloor - DefenderFloor);

	// ========== 2단계: 데미지 계산 (한 번만 수행) ==========
	
	int32 FinalDamage = CalculateAttackDamageWithBonus(AttackerStatusComp, AttackerCombatComp, AttackerHex, DefenderHex);
	
	// 반격 조건 확인 (ExecuteCombat과 동일한 로직)
	// 1) HexDistance == 1 (인접)
	// 2) 공격자와 방어자 모두 근거리 유닛 (Range == 1)
	// 3) 층수 차이 1 이하
	int32 AttackerRange = AttackerStatusComp->GetRange();
	int32 DefenderRange = DefenderStatusComp->GetRange();
	bool bCanCounter = (HexDistance == 1 && AttackerRange == 1 && DefenderRange == 1 && FloorDifference <= 1);
	
	// 반격 데미지 계산 (반격 조건이 만족될 때만)
	int32 CalculatedCounterDamage = 0;
	if (bCanCounter)
	{
		CalculatedCounterDamage = CalculateCounterDamageWithBonus(DefenderStatusComp, DefenderCombatComp, DefenderHex, AttackerHex, HexDistance, FloorDifference);
	}
	
	// 반격 데미지로 공격자를 죽일 수 없도록 제한 (최소 체력 1 보장)
	int32 PredictedCounterDamage = 0;
	if (bCanCounter && CalculatedCounterDamage > 0)
	{
		// 방어자가 공격 데미지를 받은 후에도 살아있을 것으로 가정 (반격 가능)
		// 공격자의 현재 체력 확인
		int32 AttackerCurrentHealthAfterAttack = AttackerCurrentHealth; // 공격자는 아직 데미지를 받지 않음
		
		// 반격 데미지로 공격자를 죽일 수 없도록 제한 (최소 체력 1 보장)
		int32 MaxAllowedCounterDamage = FMath::Max(0, AttackerCurrentHealthAfterAttack - 1);
		PredictedCounterDamage = FMath::Min(CalculatedCounterDamage, MaxAllowedCounterDamage);
	}

	// ========== 3단계: 기본 UI 정보 설정 ==========
	
	// 유닛 이름 설정
	if (PlayerUnitNameTxt)
	{
		FUnitBaseStat AttackerBaseStat = AttackerStatusComp->GetBaseStat();
		PlayerUnitNameTxt->SetText(AttackerBaseStat.UnitName);
	}

	if (EnemyUnitNameTxt)
	{
		FUnitBaseStat DefenderBaseStat = DefenderStatusComp->GetBaseStat();
		EnemyUnitNameTxt->SetText(DefenderBaseStat.UnitName);
	}

	// 공격력 설정
	if (PlayerUnitAtkTxt)
	{
		PlayerUnitAtkTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), AttackerAttackStrength)));
	}

	if (EnemyUnitAtkTxt)
	{
		EnemyUnitAtkTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), DefenderDefenseStrength)));
	}

	// 타일 효과 텍스트 설정
	if (PlayerUnitTileEffectTxt && AttackerCombatComp)
	{
		// AttackerRange는 이미 위에서 선언됨
		FText TerrainBonusText = AttackerCombatComp->GetTerrainBonusText(AttackerHex, DefenderHex, AttackerRange);
		PlayerUnitTileEffectTxt->SetText(TerrainBonusText);
	}

	if (EnemyUnitTileEffectTxt && DefenderCombatComp)
	{
		// DefenderRange는 이미 위에서 선언됨
		FText TerrainBonusText = DefenderCombatComp->GetTerrainBonusText(DefenderHex, AttackerHex, DefenderRange);
		EnemyUnitTileEffectTxt->SetText(TerrainBonusText);
	}

	// ========== 4단계: 전투 예측 및 HP UI 업데이트 ==========
	
	bool bCanKillDefender = (DefenderCurrentHealth - FinalDamage <= 0);
	bool bAttackerSurvives = (AttackerCurrentHealth - PredictedCounterDamage > 0);

	// HP 텍스트 설정 (데미지 계산 후)
	// PlayerUnitHpTxt: 현재 체력 -> 데미지 받고 난 체력
	if (PlayerUnitHpTxt)
	{
		int32 AttackerHealthAfterDamage = FMath::Max(0, AttackerCurrentHealth - PredictedCounterDamage);
		FString HpText = FString::Printf(TEXT("%d -> %d"), AttackerCurrentHealth, AttackerHealthAfterDamage);
		PlayerUnitHpTxt->SetText(FText::FromString(HpText));
	}

	// EnemyUnitHpTxt: 현재 체력 -> 데미지 받고 난 체력
	if (EnemyUnitHpTxt)
	{
		int32 DefenderHealthAfterDamage = FMath::Max(0, DefenderCurrentHealth - FinalDamage);
		FString HpText = FString::Printf(TEXT("%d -> %d"), DefenderCurrentHealth, DefenderHealthAfterDamage);
		EnemyUnitHpTxt->SetText(FText::FromString(HpText));
	}

	// HP 바 설정 (데미지 받고 난 체력 기준)
	// PlayerUnitHpBar: GetCurrentHealth() -> GetCurrentHealth() - PredictedCounterDamage
	if (PlayerUnitHpBar)
	{
		int32 AttackerHealthAfterDamage = FMath::Max(0, AttackerCurrentHealth - PredictedCounterDamage);
		float HpRatio = (AttackerMaxHealth > 0) ? static_cast<float>(AttackerHealthAfterDamage) / static_cast<float>(AttackerMaxHealth) : 0.0f;
		HpRatio = FMath::Clamp(HpRatio, 0.0f, 1.0f);
		PlayerUnitHpBar->SetPercent(HpRatio);
	}

	// PlayerUnitHpMinusBar: 현재 체력만큼 설정
	if (PlayerUnitHpMinusBar)
	{
		float CurrentHpRatio = (AttackerMaxHealth > 0) ? static_cast<float>(AttackerCurrentHealth) / static_cast<float>(AttackerMaxHealth) : 0.0f;
		CurrentHpRatio = FMath::Clamp(CurrentHpRatio, 0.0f, 1.0f);
		PlayerUnitHpMinusBar->SetPercent(CurrentHpRatio);
	}

	// EnemyUnitHpBar: GetCurrentHealth() -> GetCurrentHealth() - FinalDamage
	if (EnemyUnitHpBar)
	{
		int32 DefenderHealthAfterDamage = FMath::Max(0, DefenderCurrentHealth - FinalDamage);
		float HpRatio = (DefenderMaxHealth > 0) ? static_cast<float>(DefenderHealthAfterDamage) / static_cast<float>(DefenderMaxHealth) : 0.0f;
		HpRatio = FMath::Clamp(HpRatio, 0.0f, 1.0f);
		EnemyUnitHpBar->SetPercent(HpRatio);
	}

	// EnemyUnitHpMinusBar: 현재 체력만큼 설정
	if (EnemyUnitHpMinusBar)
	{
		float CurrentHpRatio = (DefenderMaxHealth > 0) ? static_cast<float>(DefenderCurrentHealth) / static_cast<float>(DefenderMaxHealth) : 0.0f;
		CurrentHpRatio = FMath::Clamp(CurrentHpRatio, 0.0f, 1.0f);
		EnemyUnitHpMinusBar->SetPercent(CurrentHpRatio);
	}

	// ========== 5단계: 결과 이미지 및 Versus 텍스트 설정 ==========
	
	// 방어자를 죽일 수 있으면서 공격자가 죽지 않아야 승리
	if (bCanKillDefender && bAttackerSurvives)
	{
		SetCombatResultImage(TEXT("승리"), true, false, false);
	}
	// 방어자와 공격자가 모두 죽으면 전멸
	else if (bCanKillDefender && !bAttackerSurvives)
	{
		SetCombatResultImage(TEXT("전멸"), false, true, false);
	}
	// 방어자를 죽일 수 없고, 공격자가 죽는다면 패배
	else if (!bCanKillDefender && !bAttackerSurvives)
	{
		SetCombatResultImage(TEXT("패배"), false, false, true);
	}
	// 방어자와 공격자가 모두 생존하는 경우
	else if (!bCanKillDefender && bAttackerSurvives)
	{
		// 데미지 차이 계산
		int32 DamageDifference = FinalDamage - PredictedCounterDamage;
		
		if (DamageDifference >= 10)
		{
			SetCombatResultImage(TEXT("유리"), true, false, false);
		}
		else if (DamageDifference <= -10)
		{
			SetCombatResultImage(TEXT("불리"), false, false, true);
		}
		else
		{
			SetCombatResultImage(TEXT("대등"), false, true, false);
		}
	}
	else
	{
		// 그 외의 경우: 모든 결과 이미지 숨김
		SetCombatResultImage(TEXT(""), false, false, false);
	}
}

int32 UUnitCombatUI::CalculateAttackDamageWithBonus(UUnitStatusComponent* AttackerStatusComp, UUnitCombatComponent* AttackerCombatComp, FVector2D AttackerHex, FVector2D DefenderHex) const
{
	if (!AttackerStatusComp || !AttackerCombatComp)
	{
		return 0;
	}

	int32 AttackerAttackStrength = AttackerStatusComp->GetAttackStrength();
	int32 AttackerCurrentHealth = AttackerStatusComp->GetCurrentHealth();
	int32 AttackerMaxHealth = AttackerStatusComp->GetMaxHealth();
	int32 BaseDamage = AttackerCombatComp->CalculateAttackDamage(AttackerAttackStrength, AttackerCurrentHealth, AttackerMaxHealth);
	
	// 지형 보너스 적용 (ExecuteCombat과 동일한 로직)
	int32 AttackerRange = AttackerStatusComp->GetRange();
	int32 CombatBonus = 0;
	if (AttackerRange <= 1)
	{
		// 근접 유닛: 지형 보너스 적용
		CombatBonus = AttackerCombatComp->CalculateCombatBonus(AttackerHex, DefenderHex);
	}
	// 원거리 유닛은 공격력 보너스 없음
	
	return BaseDamage + CombatBonus;
}

int32 UUnitCombatUI::CalculateCounterDamageWithBonus(UUnitStatusComponent* DefenderStatusComp, UUnitCombatComponent* DefenderCombatComp, FVector2D DefenderHex, FVector2D AttackerHex, int32 HexDistance, int32 FloorDifference) const
{
	if (!DefenderStatusComp || !DefenderCombatComp)
	{
		return 0;
	}

	// 반격 조건 확인 (ExecuteCombat과 동일한 로직)
	// 1) HexDistance == 1 (인접)
	// 2) 공격자와 방어자 모두 근거리 유닛 (Range == 1)
	// 3) 층수 차이 1 이하
	int32 AttackerRange = 0; // 공격자 Range는 파라미터로 받아야 함
	int32 DefenderRange = DefenderStatusComp->GetRange();
	
	// 반격 조건 체크 (거리 1이고, 양쪽 모두 근거리 유닛이고, 층수 차이 <= 1일 때만)
	// 주의: AttackerRange는 이 함수에서 알 수 없으므로, SetupForCombat에서 체크 후 호출해야 함
	if (HexDistance != 1 || FloorDifference > 1)
	{
		return 0;
	}

	int32 DefenderDefenseStrength = DefenderStatusComp->GetDefenseStrength();
	int32 DefenderCurrentHealth = DefenderStatusComp->GetCurrentHealth();
	int32 DefenderMaxHealth = DefenderStatusComp->GetMaxHealth();
	int32 BaseCounterDamage = DefenderCombatComp->CalculateCounterDamage(DefenderDefenseStrength, DefenderCurrentHealth, DefenderMaxHealth);
	
	// 지형 보너스 적용 (ExecuteCombat과 동일한 로직)
	int32 CombatBonus = 0;
	if (DefenderRange <= 1)
	{
		// 근접 유닛: 지형 보너스 적용
		CombatBonus = DefenderCombatComp->CalculateCombatBonus(DefenderHex, AttackerHex);
	}
	// 원거리 유닛은 공격력 보너스 없음
	
	int32 FinalCounterDamage = (BaseCounterDamage > 0) ? BaseCounterDamage + CombatBonus : 0;
	
	// 반격 데미지로 공격자를 죽일 수 없도록 제한 (최소 체력 1 보장)
	// 주의: 공격자 체력은 이 함수에서 알 수 없으므로, SetupForCombat에서 제한 적용
	// 여기서는 계산된 반격 데미지만 반환
	
	return FinalCounterDamage;
}

void UUnitCombatUI::SetCombatResultImage(const FString& ResultText, bool bShowWin, bool bShowDraw, bool bShowLose) const
{
	if (VersusTxt)
	{
		VersusTxt->SetText(FText::FromString(ResultText));
	}
	
	if (WinImg)
	{
		WinImg->SetVisibility(bShowWin ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (DrawImg)
	{
		DrawImg->SetVisibility(bShowDraw ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (LoseImg)
	{
		LoseImg->SetVisibility(bShowLose ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UUnitCombatUI::SetupForCombatAgainstCity(AUnitCharacterBase* Attacker, UCityComponent* CityComponent, FVector2D AttackerHex, FVector2D CityHex)
{
	if (!Attacker || !CityComponent)
	{
		return;
	}

	// 공격자 정보 가져오기
	UUnitStatusComponent* AttackerStatusComp = Attacker->GetUnitStatusComponent();
	UUnitCombatComponent* AttackerCombatComp = Attacker->GetUnitCombatComponent();
	
	if (!AttackerStatusComp || !AttackerCombatComp)
	{
		return;
	}

	// ========== 1단계: 한 번만 가져올 데이터들을 함수 시작 부분에서 미리 계산 ==========
	
	// WorldComponent 가져오기 (한 번만)
	UWorldComponent* WorldComponent = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			WorldComponent = SuperGameInst->GetGeneratedWorldComponent();
		}
	}

	// 체력 정보 한 번만 가져오기
	int32 AttackerCurrentHealth = AttackerStatusComp->GetCurrentHealth();
	int32 AttackerMaxHealth = AttackerStatusComp->GetMaxHealth();
	int32 CityCurrentHealth = CityComponent->GetCurrentHealth();
	int32 CityMaxHealth = CityComponent->GetMaxHealth();

	// 공격력 한 번만 가져오기
	int32 AttackerAttackStrength = AttackerStatusComp->GetAttackStrength();

	// ========== 2단계: 데미지 계산 (한 번만 수행) ==========
	
	int32 FinalDamage = CalculateAttackDamageAgainstCityWithBonus(AttackerStatusComp, AttackerCombatComp, AttackerHex, CityHex);
	
	// 도시는 반격 없음
	int32 PredictedCounterDamage = 0;

	// ========== 3단계: 기본 UI 정보 설정 ==========
	
	// 유닛 이름 설정 (공격자)
	if (PlayerUnitNameTxt)
	{
		FUnitBaseStat AttackerBaseStat = AttackerStatusComp->GetBaseStat();
		PlayerUnitNameTxt->SetText(AttackerBaseStat.UnitName);
	}

	// 도시 이름 설정 (방어자) - "도시"로 표시
	if (EnemyUnitNameTxt)
	{
		EnemyUnitNameTxt->SetText(FText::FromString(TEXT("도시")));
	}

	// 공격력 설정
	if (PlayerUnitAtkTxt)
	{
		PlayerUnitAtkTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), AttackerAttackStrength)));
	}

	// 도시는 방어력 없음 - "0" 표시
	if (EnemyUnitAtkTxt)
	{
		EnemyUnitAtkTxt->SetText(FText::FromString(TEXT("0")));
	}

	// 타일 효과 텍스트 설정 (공격자)
	if (PlayerUnitTileEffectTxt && AttackerCombatComp)
	{
		int32 AttackerRange = AttackerStatusComp->GetRange();
		FText TerrainBonusText = AttackerCombatComp->GetTerrainBonusText(AttackerHex, CityHex, AttackerRange);
		PlayerUnitTileEffectTxt->SetText(TerrainBonusText);
	}

	// 도시는 지형 보너스 없음 - 빈 문자열
	if (EnemyUnitTileEffectTxt)
	{
		EnemyUnitTileEffectTxt->SetText(FText::GetEmpty());
	}

	// ========== 4단계: 전투 예측 및 HP UI 업데이트 ==========
	
	bool bCanDestroyCity = (CityCurrentHealth - FinalDamage <= 0);
	bool bAttackerSurvives = true; // 도시는 반격 없으므로 항상 생존

	// HP 텍스트 설정 (데미지 계산 후)
	// PlayerUnitHpTxt: 현재 체력 -> 현재 체력 (반격 없음)
	if (PlayerUnitHpTxt)
	{
		FString HpText = FString::Printf(TEXT("%d -> %d"), AttackerCurrentHealth, AttackerCurrentHealth);
		PlayerUnitHpTxt->SetText(FText::FromString(HpText));
	}

	// EnemyUnitHpTxt: 현재 체력 -> 데미지 받고 난 체력
	if (EnemyUnitHpTxt)
	{
		int32 CityHealthAfterDamage = FMath::Max(0, CityCurrentHealth - FinalDamage);
		FString HpText = FString::Printf(TEXT("%d -> %d"), CityCurrentHealth, CityHealthAfterDamage);
		EnemyUnitHpTxt->SetText(FText::FromString(HpText));
	}

	// HP 바 설정 (데미지 받고 난 체력 기준)
	// PlayerUnitHpBar: 현재 체력 (변화 없음)
	if (PlayerUnitHpBar)
	{
		float HpRatio = (AttackerMaxHealth > 0) ? static_cast<float>(AttackerCurrentHealth) / static_cast<float>(AttackerMaxHealth) : 0.0f;
		HpRatio = FMath::Clamp(HpRatio, 0.0f, 1.0f);
		PlayerUnitHpBar->SetPercent(HpRatio);
	}

	// PlayerUnitHpMinusBar: 현재 체력만큼 설정
	if (PlayerUnitHpMinusBar)
	{
		float CurrentHpRatio = (AttackerMaxHealth > 0) ? static_cast<float>(AttackerCurrentHealth) / static_cast<float>(AttackerMaxHealth) : 0.0f;
		CurrentHpRatio = FMath::Clamp(CurrentHpRatio, 0.0f, 1.0f);
		PlayerUnitHpMinusBar->SetPercent(CurrentHpRatio);
	}

	// EnemyUnitHpBar: 도시 체력 -> 데미지 받고 난 체력
	if (EnemyUnitHpBar)
	{
		int32 CityHealthAfterDamage = FMath::Max(0, CityCurrentHealth - FinalDamage);
		float HpRatio = (CityMaxHealth > 0) ? static_cast<float>(CityHealthAfterDamage) / static_cast<float>(CityMaxHealth) : 0.0f;
		HpRatio = FMath::Clamp(HpRatio, 0.0f, 1.0f);
		EnemyUnitHpBar->SetPercent(HpRatio);
	}

	// EnemyUnitHpMinusBar: 현재 체력만큼 설정
	if (EnemyUnitHpMinusBar)
	{
		float CurrentHpRatio = (CityMaxHealth > 0) ? static_cast<float>(CityCurrentHealth) / static_cast<float>(CityMaxHealth) : 0.0f;
		CurrentHpRatio = FMath::Clamp(CurrentHpRatio, 0.0f, 1.0f);
		EnemyUnitHpMinusBar->SetPercent(CurrentHpRatio);
	}

	// ========== 5단계: 결과 이미지 및 Versus 텍스트 설정 ==========
	
	// 도시를 파괴할 수 있으면 승리
	if (bCanDestroyCity)
	{
		SetCombatResultImage(TEXT("승리"), true, false, false);
	}
	// 도시를 파괴할 수 없으면 데미지 비율에 따라 판정
	else
	{
		// 데미지가 도시 체력의 50% 이상이면 "유리", 그 외는 "대등"
		float DamageRatio = (CityMaxHealth > 0) ? 
			static_cast<float>(FinalDamage) / static_cast<float>(CityMaxHealth) : 0.0f;
		
		if (DamageRatio >= 0.5f)
		{
			SetCombatResultImage(TEXT("유리"), true, false, false);
		}
		else
		{
			SetCombatResultImage(TEXT("대등"), false, true, false);
		}
	}
}

int32 UUnitCombatUI::CalculateAttackDamageAgainstCityWithBonus(UUnitStatusComponent* AttackerStatusComp, UUnitCombatComponent* AttackerCombatComp, FVector2D AttackerHex, FVector2D CityHex) const
{
	if (!AttackerStatusComp || !AttackerCombatComp)
	{
		return 0;
	}

	int32 AttackerAttackStrength = AttackerStatusComp->GetAttackStrength();
	int32 AttackerCurrentHealth = AttackerStatusComp->GetCurrentHealth();
	int32 AttackerMaxHealth = AttackerStatusComp->GetMaxHealth();
	int32 BaseDamage = AttackerCombatComp->CalculateAttackDamage(AttackerAttackStrength, AttackerCurrentHealth, AttackerMaxHealth);
	
	// 지형 보너스 적용 (ExecuteCombatAgainstCity와 동일한 로직)
	int32 AttackerRange = AttackerStatusComp->GetRange();
	int32 CombatBonus = 0;
	if (AttackerRange == 1)
	{
		// 근접 유닛: 지형 보너스 적용
		CombatBonus = AttackerCombatComp->CalculateCombatBonus(AttackerHex, CityHex);
	}
	// 원거리 유닛은 공격력 보너스 없음
	
	return BaseDamage + CombatBonus;
}
