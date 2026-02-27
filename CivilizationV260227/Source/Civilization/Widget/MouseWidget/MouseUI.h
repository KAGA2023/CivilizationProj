// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MouseUI.generated.h"

UCLASS()
class CIVILIZATION_API UMouseUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UMouseUI(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

public:
	// 타일 정보 보더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* TileInfoBrd = nullptr;

	// 타일 정보 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TileInfoTxt = nullptr;

	// 기술 정보 보더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* TechInfoBrd = nullptr;

	// 건물 정보 보더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* BuildingInfoBrd = nullptr;

	// 유닛 정보 보더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* UnitInfoBrd = nullptr;

	// 타일 정보 표시
	UFUNCTION(BlueprintCallable, Category = "Mouse UI")
	void ShowTileInfo(class UWorldTile* Tile);

	// 타일 정보 숨김
	UFUNCTION(BlueprintCallable, Category = "Mouse UI")
	void HideTileInfo();

private:
	// 타일 정보를 문자열로 변환
	FString GenerateTileInfoString(class UWorldTile* Tile);
};
