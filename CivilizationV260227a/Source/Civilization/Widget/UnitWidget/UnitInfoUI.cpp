// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitInfoUI.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "../../Unit/UnitCharacterBase.h"
#include "../../Status/UnitStatusComponent.h"
#include "../../Combat/UnitCombatComponent.h"
#include "../../SuperGameInstance.h"
#include "../../Unit/UnitManager.h"

void UUnitInfoUI::SetupForUnit(AUnitCharacterBase* Unit)
{
	CachedUnit = Unit;

	if (!Unit)
	{
		return;
	}

	UUnitStatusComponent* StatusComp = Unit->GetUnitStatusComponent();
	if (!StatusComp)
	{
		return;
	}

	// HpBar
	if (HpBar)
	{
		int32 CurrentHealth = StatusComp->GetCurrentHealth();
		int32 MaxHealth = StatusComp->GetMaxHealth();
		float HpPercent = (MaxHealth > 0) ? static_cast<float>(CurrentHealth) / static_cast<float>(MaxHealth) : 0.0f;
		HpPercent = FMath::Clamp(HpPercent, 0.0f, 1.0f);
		HpBar->SetPercent(HpPercent);
	}

	// UnitNameTxt
	if (UnitNameTxt)
	{
		UnitNameTxt->SetText(StatusComp->GetBaseStat().UnitName);
	}

	// UnitAttackStrengthTxt
	if (UnitAttackStrengthTxt)
	{
		UnitAttackStrengthTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), StatusComp->GetAttackStrength())));
	}

	// UnitDefenceStrengthTxt
	if (UnitDefenceStrengthTxt)
	{
		UnitDefenceStrengthTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), StatusComp->GetDefenseStrength())));
	}

	// UnitHealthTxt (현재/최대)
	if (UnitHealthTxt)
	{
		int32 CurrentHealth = StatusComp->GetCurrentHealth();
		int32 MaxHealth = StatusComp->GetMaxHealth();
		UnitHealthTxt->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentHealth, MaxHealth)));
	}

	// UnitRangeTxt (원거리 유닛은 현재 지형 보너스 반영: 평지 그대로, 언덕 +1, 산 +2)
	if (UnitRangeTxt && GetWorld())
	{
		int32 BaseRange = StatusComp->GetRange();
		FString RangeStr;
		if (BaseRange > 1)
		{
			// 원거리 유닛: 현재 서 있는 타일 기준 사거리 보너스
			int32 RangeBonus = 0;
			if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance()))
			{
				if (UUnitManager* UnitManager = SuperGameInst->GetUnitManager())
				{
					if (UUnitCombatComponent* CombatComp = Unit->GetUnitCombatComponent())
					{
						FVector2D MyHex = UnitManager->GetHexPositionForUnit(Unit);
						RangeBonus = CombatComp->CalculateRangeBonus(MyHex);
					}
				}
			}
			if (RangeBonus > 0)
			{
				RangeStr = FString::Printf(TEXT("%d+%d"), BaseRange, RangeBonus);
			}
			else
			{
				RangeStr = FString::Printf(TEXT("%d"), BaseRange);
			}
		}
		else
		{
			RangeStr = FString::Printf(TEXT("%d"), BaseRange);
		}
		UnitRangeTxt->SetText(FText::FromString(RangeStr));
	}

	// UnitMovementPointTxt (현재 턴 남은 이동력)
	if (UnitMovementPointTxt && GetWorld())
	{
		int32 RemainingMovement = 0;
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance()))
		{
			if (UUnitManager* UnitManager = SuperGameInst->GetUnitManager())
			{
				RemainingMovement = UnitManager->GetUnitRemainingMovement(Unit);
			}
		}
		UnitMovementPointTxt->SetText(FText::FromString(FString::Printf(TEXT("%d"), RemainingMovement)));
	}
}

void UUnitInfoUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (DeathBtn)
	{
		DeathBtn->OnClicked.AddDynamic(this, &UUnitInfoUI::OnDeathBtnClicked);
	}
}

void UUnitInfoUI::OnDeathBtnClicked()
{
	AUnitCharacterBase* Unit = CachedUnit.Get();
	if (!Unit || !GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	UUnitManager* UnitManager = SuperGameInst->GetUnitManager();
	if (!UnitManager)
	{
		return;
	}

	FVector2D HexPos = UnitManager->GetHexPositionForUnit(Unit);
	UnitManager->DestroyUnit(Unit, HexPos);
	CachedUnit.Reset();
}
