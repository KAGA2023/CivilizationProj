// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "CountryStruct.generated.h"

USTRUCT(BlueprintType)
struct CIVILIZATION_API FCountryData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// ========== 국가 정보 ==========

	// 국가 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Country Info")
	FString CountryName;

	// 외교 UI 등에 쓰는 국가 큰 이미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Country Info")
	TSoftObjectPtr<UTexture2D> CountryLargeImg;

	// 국가 왕 이미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Country Info")
	TSoftObjectPtr<UTexture2D> CountryKingImg;

	// 국경선 색상
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Country Info")
	FLinearColor BorderColor = FLinearColor::White;

	FCountryData()
	{
		CountryName = TEXT("New Country");
		CountryLargeImg = nullptr;
		CountryKingImg = nullptr;
		BorderColor = FLinearColor::White;
	}
};
