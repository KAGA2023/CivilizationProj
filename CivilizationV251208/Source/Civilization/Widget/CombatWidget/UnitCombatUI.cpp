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
		int32 AttackerAttackStrength = AttackerStatusComp->GetAttackStrength();
		PlayerUnitAtkTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), AttackerAttackStrength)));
	}

	if (EnemyUnitAtkTxt)
	{
		int32 DefenderDefenseStrength = DefenderStatusComp->GetDefenseStrength();
		EnemyUnitAtkTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), DefenderDefenseStrength)));
	}

	// HP 텍스트와 바는 데미지 계산 후에 업데이트됨 (아래에서 처리)

	// 타일 효과 텍스트 설정
	if (PlayerUnitTileEffectTxt && AttackerCombatComp)
	{
		int32 AttackerRange = AttackerStatusComp->GetRange();
		FText TerrainBonusText = AttackerCombatComp->GetTerrainBonusText(AttackerHex, DefenderHex, AttackerRange);
		PlayerUnitTileEffectTxt->SetText(TerrainBonusText);
	}

	if (EnemyUnitTileEffectTxt && DefenderCombatComp)
	{
		int32 DefenderRange = DefenderStatusComp->GetRange();
		FText TerrainBonusText = DefenderCombatComp->GetTerrainBonusText(DefenderHex, AttackerHex, DefenderRange);
		EnemyUnitTileEffectTxt->SetText(TerrainBonusText);
	}

	// 데미지 텍스트 설정
	if (PlayerUnitDmgTxt && AttackerCombatComp)
	{
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
		
		int32 FinalDamage = BaseDamage + CombatBonus;
		PlayerUnitDmgTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), FinalDamage)));
	}

	if (EnemyUnitDmgTxt && DefenderCombatComp)
	{
		// 반격 데미지 계산 (거리 1이고 층수 차이 <= 1일 때만)
		int32 HexDistance = 0;
		if (UWorld* World = GetWorld())
		{
			if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
			{
				if (UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent())
				{
					HexDistance = WorldComponent->GetHexDistance(AttackerHex, DefenderHex);
				}
			}
		}
		
		int32 PredictedCounterDamage = 0;
		if (HexDistance == 1)
		{
			// 층수 차이 확인
			int32 AttackerFloor = AttackerCombatComp->GetFloorLevelAtHex(AttackerHex);
			int32 DefenderFloor = DefenderCombatComp->GetFloorLevelAtHex(DefenderHex);
			int32 FloorDifference = FMath::Abs(AttackerFloor - DefenderFloor);
			
			// 층수 차이가 1 이하일 때만 반격
			if (FloorDifference <= 1)
			{
				int32 DefenderDefenseStrength = DefenderStatusComp->GetDefenseStrength();
				int32 DefenderCurrentHealth = DefenderStatusComp->GetCurrentHealth();
				int32 DefenderMaxHealth = DefenderStatusComp->GetMaxHealth();
				int32 BaseCounterDamage = DefenderCombatComp->CalculateCounterDamage(DefenderDefenseStrength, DefenderCurrentHealth, DefenderMaxHealth);
				
				// 지형 보너스 적용 (ExecuteCombat과 동일한 로직)
				int32 DefenderRange = DefenderStatusComp->GetRange();
				int32 CombatBonus = 0;
				if (DefenderRange <= 1)
				{
					// 근접 유닛: 지형 보너스 적용
					CombatBonus = DefenderCombatComp->CalculateCombatBonus(DefenderHex, AttackerHex);
				}
				// 원거리 유닛은 공격력 보너스 없음
				
				PredictedCounterDamage = BaseCounterDamage + CombatBonus;
			}
		}
		
		EnemyUnitDmgTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), PredictedCounterDamage)));
	}

	// 전투 예측: 상대 유닛을 죽일 수 있으면서 공격자가 죽지 않아야 함
	bool bCanKillDefender = false;
	bool bAttackerSurvives = false;
	int32 FinalDamage = 0;
	int32 PredictedCounterDamage = 0;
	
	if (AttackerCombatComp && AttackerStatusComp && DefenderStatusComp && DefenderCombatComp)
	{
		// 예상 공격 데미지 계산
		int32 AttackerAttackStrength = AttackerStatusComp->GetAttackStrength();
		int32 AttackerCurrentHealth = AttackerStatusComp->GetCurrentHealth();
		int32 AttackerMaxHealth = AttackerStatusComp->GetMaxHealth();
		int32 BaseDamage = AttackerCombatComp->CalculateAttackDamage(AttackerAttackStrength, AttackerCurrentHealth, AttackerMaxHealth);
		
		// 지형 보너스 적용
		int32 AttackerRange = AttackerStatusComp->GetRange();
		int32 CombatBonus = 0;
		if (AttackerRange <= 1)
		{
			CombatBonus = AttackerCombatComp->CalculateCombatBonus(AttackerHex, DefenderHex);
		}
		
		FinalDamage = BaseDamage + CombatBonus;
		
		// 방어자의 현재 체력 및 최대 체력
		int32 DefenderCurrentHealth = DefenderStatusComp->GetCurrentHealth();
		int32 DefenderMaxHealth = DefenderStatusComp->GetMaxHealth();
		
		// 방어자를 죽일 수 있는지 확인
		if (DefenderCurrentHealth - FinalDamage <= 0)
		{
			bCanKillDefender = true;
		}
		
		// 반격 데미지 계산 (거리 1이고 층수 차이 <= 1일 때만)
		int32 HexDistance = 0;
		if (UWorld* World = GetWorld())
		{
			if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
			{
				if (UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent())
				{
					HexDistance = WorldComponent->GetHexDistance(AttackerHex, DefenderHex);
				}
			}
		}
		
		if (HexDistance == 1)
		{
			// 층수 차이 확인
			int32 AttackerFloor = AttackerCombatComp->GetFloorLevelAtHex(AttackerHex);
			int32 DefenderFloor = DefenderCombatComp->GetFloorLevelAtHex(DefenderHex);
			int32 FloorDifference = FMath::Abs(AttackerFloor - DefenderFloor);
			
			// 층수 차이가 1 이하일 때만 반격
			if (FloorDifference <= 1)
			{
				int32 DefenderDefenseStrength = DefenderStatusComp->GetDefenseStrength();
				int32 DefenderCurrentHealthForCounter = DefenderStatusComp->GetCurrentHealth();
				int32 BaseCounterDamage = DefenderCombatComp->CalculateCounterDamage(DefenderDefenseStrength, DefenderCurrentHealthForCounter, DefenderMaxHealth);
				
				// 지형 보너스 적용
				int32 DefenderRange = DefenderStatusComp->GetRange();
				int32 CounterCombatBonus = 0;
				if (DefenderRange <= 1)
				{
					CounterCombatBonus = DefenderCombatComp->CalculateCombatBonus(DefenderHex, AttackerHex);
				}
				
				PredictedCounterDamage = BaseCounterDamage + CounterCombatBonus;
			}
		}
		
		// 공격자가 죽지 않는지 확인
		if (AttackerCurrentHealth - PredictedCounterDamage > 0)
		{
			bAttackerSurvives = true;
		}

		// HP 텍스트 설정 (데미지 계산 후)
		// PlayerUnitHpTxt: 현재 체력 -> 데미지 받고 난 체력
		if (PlayerUnitHpTxt)
		{
			int32 AttackerHealthAfterDamage = FMath::Max(0, AttackerCurrentHealth - PredictedCounterDamage);
			// RichText 형식으로 빨간색 표시
			FString HpText = FString::Printf(TEXT("%d -> %d"), AttackerCurrentHealth, AttackerHealthAfterDamage);
			PlayerUnitHpTxt->SetText(FText::FromString(HpText));
		}

		// EnemyUnitHpTxt: 현재 체력 -> 데미지 받고 난 체력
		if (EnemyUnitHpTxt)
		{
			int32 DefenderHealthAfterDamage = FMath::Max(0, DefenderCurrentHealth - FinalDamage);
			// RichText 형식으로 빨간색 표시
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
	}
	else
	{
		// 데미지 계산이 불가능한 경우 기본값 표시
		if (PlayerUnitHpTxt)
		{
			int32 AttackerCurrentHealth = AttackerStatusComp->GetCurrentHealth();
			int32 AttackerMaxHealth = AttackerStatusComp->GetMaxHealth();
			PlayerUnitHpTxt->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), AttackerCurrentHealth, AttackerMaxHealth)));
		}

		if (EnemyUnitHpTxt)
		{
			int32 DefenderCurrentHealth = DefenderStatusComp->GetCurrentHealth();
			int32 DefenderMaxHealthLocal = DefenderStatusComp->GetMaxHealth();
			EnemyUnitHpTxt->SetText(FText::FromString(FString::Printf(TEXT("%d/%d"), DefenderCurrentHealth, DefenderMaxHealthLocal)));
		}

		if (PlayerUnitHpBar)
		{
			int32 AttackerCurrentHealth = AttackerStatusComp->GetCurrentHealth();
			int32 AttackerMaxHealth = AttackerStatusComp->GetMaxHealth();
			float HpRatio = (AttackerMaxHealth > 0) ? static_cast<float>(AttackerCurrentHealth) / static_cast<float>(AttackerMaxHealth) : 0.0f;
			HpRatio = FMath::Clamp(HpRatio, 0.0f, 1.0f);
			PlayerUnitHpBar->SetPercent(HpRatio);
		}

		if (PlayerUnitHpMinusBar)
		{
			int32 AttackerCurrentHealth = AttackerStatusComp->GetCurrentHealth();
			int32 AttackerMaxHealth = AttackerStatusComp->GetMaxHealth();
			float CurrentHpRatio = (AttackerMaxHealth > 0) ? static_cast<float>(AttackerCurrentHealth) / static_cast<float>(AttackerMaxHealth) : 0.0f;
			CurrentHpRatio = FMath::Clamp(CurrentHpRatio, 0.0f, 1.0f);
			PlayerUnitHpMinusBar->SetPercent(CurrentHpRatio);
		}

		if (EnemyUnitHpBar)
		{
			int32 DefenderCurrentHealth = DefenderStatusComp->GetCurrentHealth();
			int32 DefenderMaxHealthLocal = DefenderStatusComp->GetMaxHealth();
			float HpRatio = (DefenderMaxHealthLocal > 0) ? static_cast<float>(DefenderCurrentHealth) / static_cast<float>(DefenderMaxHealthLocal) : 0.0f;
			HpRatio = FMath::Clamp(HpRatio, 0.0f, 1.0f);
			EnemyUnitHpBar->SetPercent(HpRatio);
		}

		if (EnemyUnitHpMinusBar)
		{
			int32 DefenderCurrentHealth = DefenderStatusComp->GetCurrentHealth();
			int32 DefenderMaxHealthLocal = DefenderStatusComp->GetMaxHealth();
			float CurrentHpRatio = (DefenderMaxHealthLocal > 0) ? static_cast<float>(DefenderCurrentHealth) / static_cast<float>(DefenderMaxHealthLocal) : 0.0f;
			CurrentHpRatio = FMath::Clamp(CurrentHpRatio, 0.0f, 1.0f);
			EnemyUnitHpMinusBar->SetPercent(CurrentHpRatio);
		}
	}

	// 결과 이미지 및 Versus 텍스트 설정
	// 방어자를 죽일 수 있으면서 공격자가 죽지 않아야 승리
	if (bCanKillDefender && bAttackerSurvives)
	{
		// 승리 가능: VersusTxt를 '승리'로 설정
		if (VersusTxt)
		{
			VersusTxt->SetText(FText::FromString(TEXT("승리")));
		}
		
		// WinImg만 visible, 나머지는 hidden
		if (WinImg)
		{
			WinImg->SetVisibility(ESlateVisibility::Visible);
		}
		if (DrawImg)
		{
			DrawImg->SetVisibility(ESlateVisibility::Hidden);
		}
		if (LoseImg)
		{
			LoseImg->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	// 방어자와 공격자가 모두 죽으면 전멸
	else if (bCanKillDefender && !bAttackerSurvives)
	{
		// 전멸: VersusTxt를 '전멸'로 설정
		if (VersusTxt)
		{
			VersusTxt->SetText(FText::FromString(TEXT("전멸")));
		}
		
		// DrawImg만 visible, 나머지는 hidden
		if (WinImg)
		{
			WinImg->SetVisibility(ESlateVisibility::Hidden);
		}
		if (DrawImg)
		{
			DrawImg->SetVisibility(ESlateVisibility::Visible);
		}
		if (LoseImg)
		{
			LoseImg->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	// 방어자를 죽일 수 없고, 공격자가 죽는다면 패배
	else if (!bCanKillDefender && !bAttackerSurvives)
	{
		// 패배: VersusTxt를 '패배'로 설정
		if (VersusTxt)
		{
			VersusTxt->SetText(FText::FromString(TEXT("패배")));
		}
		
		// LoseImg만 visible, 나머지는 hidden
		if (WinImg)
		{
			WinImg->SetVisibility(ESlateVisibility::Hidden);
		}
		if (DrawImg)
		{
			DrawImg->SetVisibility(ESlateVisibility::Hidden);
		}
		if (LoseImg)
		{
			LoseImg->SetVisibility(ESlateVisibility::Visible);
		}
	}
	// 방어자와 공격자가 모두 생존하는 경우
	else if (!bCanKillDefender && bAttackerSurvives)
	{
		// 데미지 차이 계산
		int32 DamageDifference = FinalDamage - PredictedCounterDamage;
		
		if (DamageDifference >= 10)
		{
			// 유리: VersusTxt를 '유리'로 설정
			if (VersusTxt)
			{
				VersusTxt->SetText(FText::FromString(TEXT("유리")));
			}
			
			// WinImg만 visible, 나머지는 hidden
			if (WinImg)
			{
				WinImg->SetVisibility(ESlateVisibility::Visible);
			}
			if (DrawImg)
			{
				DrawImg->SetVisibility(ESlateVisibility::Hidden);
			}
			if (LoseImg)
			{
				LoseImg->SetVisibility(ESlateVisibility::Hidden);
			}
		}
		else if (DamageDifference <= -10)
		{
			// 불리: VersusTxt를 '불리'로 설정
			if (VersusTxt)
			{
				VersusTxt->SetText(FText::FromString(TEXT("불리")));
			}
			
			// LoseImg만 visible, 나머지는 hidden
			if (WinImg)
			{
				WinImg->SetVisibility(ESlateVisibility::Hidden);
			}
			if (DrawImg)
			{
				DrawImg->SetVisibility(ESlateVisibility::Hidden);
			}
			if (LoseImg)
			{
				LoseImg->SetVisibility(ESlateVisibility::Visible);
			}
		}
		else
		{
			// 대등: VersusTxt를 '대등'으로 설정
			if (VersusTxt)
			{
				VersusTxt->SetText(FText::FromString(TEXT("대등")));
			}
			
			// DrawImg만 visible, 나머지는 hidden
			if (WinImg)
			{
				WinImg->SetVisibility(ESlateVisibility::Hidden);
			}
			if (DrawImg)
			{
				DrawImg->SetVisibility(ESlateVisibility::Visible);
			}
			if (LoseImg)
			{
				LoseImg->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}
	else
	{
		// 그 외의 경우: 모든 결과 이미지 숨김
		if (WinImg)
		{
			WinImg->SetVisibility(ESlateVisibility::Hidden);
		}
		if (DrawImg)
		{
			DrawImg->SetVisibility(ESlateVisibility::Hidden);
		}
		if (LoseImg)
		{
			LoseImg->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
