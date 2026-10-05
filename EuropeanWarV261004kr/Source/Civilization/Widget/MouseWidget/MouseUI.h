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
	// ========== BindWidget ==========

	// 타일 정보를 감싸는 보더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* TileInfoBrd = nullptr;

	// 타일 지형·자원·시설 정보를 보여주는 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TileInfoTxt = nullptr;

	// 기술 정보를 감싸는 보더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* TechInfoBrd = nullptr;

	// 건물 정보를 감싸는 보더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* BuildingInfoBrd = nullptr;

	// 유닛 정보를 감싸는 보더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* UnitInfoBrd = nullptr;

	// ========== 타일 정보 ==========

	// 호버한 타일 정보를 문자열로 채워 표시합니다.
	UFUNCTION(BlueprintCallable, Category = "Mouse UI")
	void ShowTileInfo(class UWorldTile* Tile);

	// 타일 정보 보더를 숨깁니다.
	UFUNCTION(BlueprintCallable, Category = "Mouse UI")
	void HideTileInfo();

private:
	// 타일 지형·자원·시설을 한 줄 문자열로 만듭니다.
	FString GenerateTileInfoString(class UWorldTile* Tile);
};
