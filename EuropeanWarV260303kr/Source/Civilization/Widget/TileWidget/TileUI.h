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

	// 타일 가격 업데이트
	UFUNCTION(BlueprintCallable, Category = "Tile UI")
	void UpdateTileCost(int32 Cost);

	// 호버 상태 설정
	UFUNCTION(BlueprintCallable, Category = "Tile UI")
	void SetHovered(bool bHovered);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* PurchaseTileImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* SelectPurchaseTileImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TileCostTxt = nullptr;
};

