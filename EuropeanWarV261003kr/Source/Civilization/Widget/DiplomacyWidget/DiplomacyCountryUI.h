// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DiplomacyCountryUI.generated.h"

// 외교 관계 박스에 넣는 국기 표시용 위젯입니다.
UCLASS()
class CIVILIZATION_API UDiplomacyCountryUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// 대상 국가 국기 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg = nullptr;
};
