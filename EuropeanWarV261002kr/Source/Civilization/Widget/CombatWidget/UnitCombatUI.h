// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnitCombatUI.generated.h"

UCLASS()
class CIVILIZATION_API UUnitCombatUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========== 전투 결과 이미지 ==========

	// 승리 예측일 때 켜는 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* WinImg = nullptr;

	// 무승부 예측일 때 켜는 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* DrawImg = nullptr;

	// 패배 예측일 때 켜는 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* LoseImg = nullptr;

	// ========== 아군 유닛 ==========

	// 아군 유닛 이름 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PlayerUnitNameTxt = nullptr;

	// 아군 유닛 현재 체력 바
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* PlayerUnitHpBar = nullptr;

	// 아군 유닛이 받을 피해를 보여주는 체력 바
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* PlayerUnitHpMinusBar = nullptr;

	// 아군 유닛 체력 숫자 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PlayerUnitHpTxt = nullptr;

	// 아군 유닛 공격력 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PlayerUnitAtkTxt = nullptr;

	// 아군 유닛 지형 보너스 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PlayerUnitTileEffectTxt = nullptr;

	// 대결을 표시하는 VS 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* VersusTxt = nullptr;

	// ========== 적 유닛 ==========

	// 적 유닛 이름 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* EnemyUnitNameTxt = nullptr;

	// 적 유닛 현재 체력 바
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* EnemyUnitHpBar = nullptr;

	// 적 유닛이 받을 피해를 보여주는 체력 바
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* EnemyUnitHpMinusBar = nullptr;

	// 적 유닛 체력 숫자 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* EnemyUnitHpTxt = nullptr;

	// 적 유닛 공격력 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* EnemyUnitAtkTxt = nullptr;

	// 적 유닛 지형 보너스 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* EnemyUnitTileEffectTxt = nullptr;

	// ========== 전투 예측 ==========

	// 유닛 대 유닛 전투 예측을 채웁니다.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetupForCombat(class AUnitCharacterBase* Attacker, class AUnitCharacterBase* Defender, FVector2D AttackerHex, FVector2D DefenderHex);

	// 유닛 대 도시 전투 예측을 채웁니다.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetupForCombatAgainstCity(class AUnitCharacterBase* Attacker, class UCityComponent* CityComponent, FVector2D AttackerHex, FVector2D CityHex);

private:
	// 기본 데미지에 지형 보너스를 더해 공격 피해를 계산합니다.
	int32 CalculateAttackDamageWithBonus(class UUnitStatusComponent* AttackerStatusComp, class UUnitCombatComponent* AttackerCombatComp, FVector2D AttackerHex, FVector2D DefenderHex) const;
	
	// 기본 반격 데미지에 지형 보너스를 더해 반격 피해를 계산합니다.
	int32 CalculateCounterDamageWithBonus(class UUnitStatusComponent* DefenderStatusComp, class UUnitCombatComponent* DefenderCombatComp, FVector2D DefenderHex, FVector2D AttackerHex, int32 HexDistance, int32 FloorDifference) const;
	
	// 승/무/패 결과 이미지 중 해당하는 것만 켭니다.
	void SetCombatResultImage(const FString& ResultText, bool bShowWin, bool bShowDraw, bool bShowLose) const;

	// 기본 데미지에 지형 보너스를 더해 도시 공격 피해를 계산합니다.
	int32 CalculateAttackDamageAgainstCityWithBonus(class UUnitStatusComponent* AttackerStatusComp, class UUnitCombatComponent* AttackerCombatComp, FVector2D AttackerHex, FVector2D CityHex) const;
};
