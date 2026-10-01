// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "LuxuryResourceSlotUI.generated.h"

UCLASS()
class CIVILIZATION_API ULuxuryResourceSlotUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// 아이콘 이미지 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Luxury Resource Slot")
	void SetResourceIcon(UTexture2D* Texture);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* ResourceIconImg = nullptr;
};
