// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "TechSlotUI.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTechSlotClicked, FName, TechRowName);

UCLASS()
class CIVILIZATION_API UTechSlotUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UTechSlotUI(const FObjectInitializer& ObjectInitializer);

	// 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Tech Slot")
	FOnTechSlotClicked OnTechSlotClicked;

	// 기술 RowName
	UPROPERTY(BlueprintReadWrite, Category = "Tech Slot")
	FName TechRowName = NAME_None;

	// 이미지 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Tech Slot")
	void SetTechImage(UTexture2D* Texture);

	// 기술 이름 텍스트 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Tech Slot")
	void SetTechText(const FString& Text);

	// 턴 수 텍스트 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Tech Slot")
	void SetTurnText(int32 Turns);

protected:
	virtual void NativeConstruct() override;

	// 버튼 클릭 핸들러
	UFUNCTION()
	void OnStartDevelopingBtnClicked();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* TechImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TechTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TurnTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* StartDevelopingBtn = nullptr;
};

