// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingTilesUI.generated.h"

class UWorldComponent;
class AWorldSpawner;

UCLASS()
class CIVILIZATION_API ULoadingTilesUI : public UUserWidget
{
	GENERATED_BODY()

private:
	// ========== 타일 스폰 ==========

	// 타일 스폰이 끝나면 로드 모드면 세이브 상태를 복원합니다.
	UFUNCTION()
	void OnTileSpawnCompleted();
	
	// WorldSpawner를 찾아 타일·도시를 스폰하고 플레이어에 도시를 배정합니다.
	void StartTileSpawning();
	
	// Loading 점 깜빡임 텍스트를 갱신합니다.
	void UpdateLoadingText(float DeltaTime);
	// 네 모서리 보더를 시계 방향으로 빛나게 합니다.
	void UpdateBorderAnimation(float DeltaTime);
	
	// 현재 진행률과 목표 진행률
	float curPercent{}, targetPercent{};
	// Loading 점 깜빡이는 애니메이션 타이머
	float DotTimer{};
	// 보더 회전하며 빛나는 애니메이션 타이머
	float BorderTimer{};
	
	// 타일을 실제로 스폰하는 WorldSpawner
	UPROPERTY()
	class AWorldSpawner* WorldSpawner = nullptr;
	
	// 타일 스폰이 끝났는지 여부
	bool bSpawnCompleted = false;
	
	// 세이브 데이터 복원이 끝났는지 여부
	bool bDataRestored = false;
	
	// GameMode / WorldSpawner BeginPlay 이후에 StartTileSpawning을 돌리기 위한 틱 지연
	int32 TicksBeforeStartTileSpawning = 2;
	// WorldSpawner를 못 찾았을 때 재시도한 횟수
	int32 StartTileSpawningRetryCount = 0;
	// WorldSpawner 재시도 최대 횟수
	static constexpr int32 MaxStartTileSpawningRetries = 15;
	
	// 타일 스폰(70~90%)과 마무리(90~100%)를 나눕니다.
	enum class ELoadingStage
	{
		TileSpawning,
		Finalizing
	};
	// 지금 진행 중인 로딩 단계
	ELoadingStage CurrentLoadingStage = ELoadingStage::TileSpawning;

protected:
	// ========== BindWidget ==========

	// 로딩 진행률을 표시하는 프로그레스 바
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* Bar;
	
	// 퍼센트 숫자를 표시하는 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PercentText;
	
	// "Loading..." 점 깜빡임을 표시하는 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* LoadingText;
	
	// 좌상단 보더 (회전 애니메이션용)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* Border1;
	
	// 우상단 보더 (회전 애니메이션용)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* Border2;
	
	// 우하단 보더 (회전 애니메이션용)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* Border3;
	
	// 좌하단 보더 (회전 애니메이션용)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* Border4;
	
	// ========== 수명 주기 ==========

	// 진행률을 70%부터 시작하고 보더를 초기화합니다.
	virtual void NativeConstruct() override;
	// 지연 후 스폰을 시작하고 100%가 되면 MainHUD 생성 타이밍을 알립니다.
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;
};
