// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DiplomacyCountryUI.generated.h"

/**
 * 국기 표시용 위젯
 */
UCLASS()
class CIVILIZATION_API UDiplomacyCountryUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg = nullptr;
};
