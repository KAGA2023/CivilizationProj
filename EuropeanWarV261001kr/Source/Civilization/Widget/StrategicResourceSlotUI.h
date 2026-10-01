// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "StrategicResourceSlotUI.generated.h"

UCLASS()
class CIVILIZATION_API UStrategicResourceSlotUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// 아이콘 이미지 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Strategic Resource Slot")
	void SetResourceIcon(UTexture2D* Texture);

	// 수량 텍스트 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Strategic Resource Slot")
	void SetResourceStockText(int32 Stock);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* ResourceIconImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ResourceStockTxt = nullptr;
};

