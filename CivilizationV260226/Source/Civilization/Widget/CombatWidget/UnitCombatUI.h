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
	// 결과 이미지들
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* WinImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* DrawImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* LoseImg = nullptr;

	// 플레이어 유닛 정보
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PlayerUnitNameTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* PlayerUnitHpBar = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* PlayerUnitHpMinusBar = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PlayerUnitHpTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PlayerUnitAtkTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PlayerUnitTileEffectTxt = nullptr;

	// VS 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* VersusTxt = nullptr;

	// 적 유닛 정보
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* EnemyUnitNameTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* EnemyUnitHpBar = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* EnemyUnitHpMinusBar = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* EnemyUnitHpTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* EnemyUnitAtkTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* EnemyUnitTileEffectTxt = nullptr;

	// 전투 예측 정보 설정
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetupForCombat(class AUnitCharacterBase* Attacker, class AUnitCharacterBase* Defender, FVector2D AttackerHex, FVector2D DefenderHex);

	// 도시 공격 전투 예측 정보 설정
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetupForCombatAgainstCity(class AUnitCharacterBase* Attacker, class UCityComponent* CityComponent, FVector2D AttackerHex, FVector2D CityHex);

private:
	// 공격 데미지 계산 (기본 데미지 + 지형 보너스)
	int32 CalculateAttackDamageWithBonus(class UUnitStatusComponent* AttackerStatusComp, class UUnitCombatComponent* AttackerCombatComp, FVector2D AttackerHex, FVector2D DefenderHex) const;
	
	// 반격 데미지 계산 (기본 반격 데미지 + 지형 보너스)
	int32 CalculateCounterDamageWithBonus(class UUnitStatusComponent* DefenderStatusComp, class UUnitCombatComponent* DefenderCombatComp, FVector2D DefenderHex, FVector2D AttackerHex, int32 HexDistance, int32 FloorDifference) const;
	
	// 결과 이미지 표시 헬퍼
	void SetCombatResultImage(const FString& ResultText, bool bShowWin, bool bShowDraw, bool bShowLose) const;

	// 도시 공격 데미지 계산 (기본 데미지 + 지형 보너스)
	int32 CalculateAttackDamageAgainstCityWithBonus(class UUnitStatusComponent* AttackerStatusComp, class UUnitCombatComponent* AttackerCombatComp, FVector2D AttackerHex, FVector2D CityHex) const;
};
