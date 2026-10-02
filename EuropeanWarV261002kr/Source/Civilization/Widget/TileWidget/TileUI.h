// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TileUI.generated.h"

UCLASS()
class CIVILIZATION_API UTileUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 타일 구매 가격 텍스트를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category = "Tile UI")
	void UpdateTileCost(int32 Cost);

	// 호버 강조 이미지를 켜거나 끕니다.
	UFUNCTION(BlueprintCallable, Category = "Tile UI")
	void SetHovered(bool bHovered);

protected:
	// ========== BindWidget ==========

	// 구매 가능 타일을 표시하는 기본 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* PurchaseTileImg = nullptr;

	// 호버 중일 때 켜는 선택 강조 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* SelectPurchaseTileImg = nullptr;

	// 타일 구매 가격 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TileCostTxt = nullptr;
};
