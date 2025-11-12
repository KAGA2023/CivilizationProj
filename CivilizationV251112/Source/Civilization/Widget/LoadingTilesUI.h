// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingTilesUI.generated.h"

class UWorldComponent;
class AWorldSpawner;
class AUnitCharacterBase;  //시작 유닛 소환
class USuperGameInstance;  //시작 유닛 소환
class UUnitManager;  //시작 유닛 소환

UCLASS()
class CIVILIZATION_API ULoadingTilesUI : public UUserWidget
{
	GENERATED_BODY()

private:
	UFUNCTION()
	void OnTileSpawnCompleted(); // 타일 스폰 완료 콜백
	
	void StartTileSpawning(); // 타일 스폰 시작
	void SpawnStartingUnit(); // 시작 유닛 소환
	
	void UpdateLoadingText(float DeltaTime); // 로딩 텍스트 업데이트
	void UpdateBorderAnimation(float DeltaTime); // 보더 애니메이션 업데이트
	
	float curPercent{}, targetPercent{}; // 현재 진행률과 목표 진행률
	float DotTimer{}; // Loading 점 깜빡이는 애니메이션 타이머
	float BorderTimer{}; // 보더 회전하며 빛나는 애니메이션 타이머
	
	// 타일 스폰용 WorldSpawner
	UPROPERTY()
	class AWorldSpawner* WorldSpawner = nullptr;
	
	// 스폰 완료 플래그
	bool bSpawnCompleted = false;
	
	// 로딩 단계 관리
	enum class ELoadingStage
	{
		TileSpawning,    // 타일 스폰 중 (70% → 90%)
		Finalizing       // 최종 완료 (90% → 100%)
	};
	ELoadingStage CurrentLoadingStage = ELoadingStage::TileSpawning;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* Bar; // 로딩 진행률을 표시하는 프로그레스 바
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PercentText; // 퍼센트 텍스트를 표시하는 텍스트 블록
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* LoadingText; // "Loading..." 텍스트를 표시하는 텍스트 블록
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* Border1; // 좌상단 보더 (회전 애니메이션용)
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* Border2; // 우상단 보더 (회전 애니메이션용)
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* Border3; // 우하단 보더 (회전 애니메이션용)
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* Border4; // 좌하단 보더 (회전 애니메이션용)
	
	virtual void NativeConstruct() override; // 위젯 생성 시 호출되는 함수
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override; // 매 프레임 호출되는 업데이트 함수
};

