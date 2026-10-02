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

	// 기술 슬롯을 클릭했을 때 브로드캐스트합니다.
	UPROPERTY(BlueprintAssignable, Category = "Tech Slot")
	FOnTechSlotClicked OnTechSlotClicked;

	// 이 슬롯이 가리키는 기술 RowName
	UPROPERTY(BlueprintReadWrite, Category = "Tech Slot")
	FName TechRowName = NAME_None;

	// ========== 표시 설정 ==========

	// 기술 아이콘 이미지를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Tech Slot")
	void SetTechImage(UTexture2D* Texture);

	// 기술 이름 텍스트를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Tech Slot")
	void SetTechText(const FString& Text);

	// 연구 완료까지 남은 턴 수 텍스트를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Tech Slot")
	void SetTurnText(int32 Turns);

protected:
	virtual void NativeConstruct() override;

	// 연구 시작 버튼을 누르면 클릭 델리게이트를 방송합니다.
	UFUNCTION()
	void OnStartDevelopingBtnClicked();

	// ========== BindWidget ==========

	// 기술 아이콘 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* TechImg = nullptr;

	// 기술 이름 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TechTxt = nullptr;

	// 연구 완료까지 남은 턴 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TurnTxt = nullptr;

	// 이 기술 연구를 시작하는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* StartDevelopingBtn = nullptr;
};
