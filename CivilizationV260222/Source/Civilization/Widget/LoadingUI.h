// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingUI.generated.h"

class UWorldComponent;
class AWorldSpawner;

UCLASS()
class CIVILIZATION_API ULoadingUI : public UUserWidget
{
	GENERATED_BODY()

private:
	UFUNCTION()
	void OnLevelLoaded(); // 레벨 로딩 완료 시 호출되는 콜백 함수
	
	UFUNCTION()
	void OnWorldGenerated(bool bSuccess); // 월드 생성 완료 시 호출되는 콜백 함수
	
	void GenerateWorldAsync(); // 비동기 월드 생성 시작
	
	void LoadLevelAsync(); // 비동기 레벨 로딩 시작
	
	void UpdateLoadingText(float DeltaTime); // 로딩 텍스트 업데이트
	void UpdateBorderAnimation(float DeltaTime); // 보더 애니메이션 업데이트
	
	float curPercent{}, targetPercent{}; // 현재 진행률과 목표 진행률
	float DotTimer{}; // Loading 점 깜빡이는 애니메이션 타이머
	float BorderTimer{}; // 보더 회전하며 빛나는 애니메이션 타이머
	
	bool bLevelTransitionStarted = false; // 레벨 전환 시작 플래그 (무한 루프 방지)
	
	// 월드 생성용 컴포넌트
	UPROPERTY()
	class UWorldComponent* WorldComponent = nullptr;
	
	// 로딩 단계 관리
	enum class ELoadingStage
	{
		WorldGeneration, // 월드 생성 중 (0% → 50%)
		LevelLoading     // 레벨 로딩 중 (50% → 70%)
	};
	ELoadingStage CurrentLoadingStage = ELoadingStage::WorldGeneration;

public:
	// 로드 모드 진입 함수 (Blueprint 호출용)
	UFUNCTION(BlueprintCallable, Category = "Load Mode")
	void SetLoadMode(int32 SlotIndex);

protected:
	// 로드 모드 관련
	UPROPERTY(BlueprintReadWrite, Category = "Load Mode")
	bool bIsLoadMode = false; // 로드 모드인지 여부

	UPROPERTY(BlueprintReadWrite, Category = "Load Mode")
	int32 LoadSlotIndex = 0; // 로드할 슬롯 인덱스

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
