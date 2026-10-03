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
	// ========== 로딩 콜백 / 단계 ==========

	// 타겟 레벨 패키지 비동기 로드가 끝나면 진행률 목표를 70%로 올립니다.
	UFUNCTION()
	void OnLevelLoaded();
	
	// 월드 생성이 끝나면 GI에 컴포넌트를 넘기고 레벨 로딩을 시작합니다.
	UFUNCTION()
	void OnWorldGenerated(bool bSuccess);
	
	// 세이브 또는 월드 설정으로 월드를 비동기 생성합니다.
	void GenerateWorldAsync();
	
	// GI 타겟 레벨 패키지를 비동기로 불러옵니다.
	void LoadLevelAsync();
	
	// Loading 점 깜빡임 텍스트를 갱신합니다.
	void UpdateLoadingText(float DeltaTime);
	// 네 모서리 보더를 시계 방향으로 빛나게 합니다.
	void UpdateBorderAnimation(float DeltaTime);
	// 지금 멀티 세션(방 만들기/참가) 중인지 반환합니다.
	bool IsMultiplayerSession() const;
	// 이 기기가 멀티 호스트인지 반환합니다.
	bool IsMultiplayerHost() const;
	// 호스트만 생성된 월드를 보낸 뒤 인게임으로 ServerTravel합니다.
	void TravelToInGameIfHost();
	
	// 현재 진행률과 목표 진행률
	float curPercent{}, targetPercent{};
	// Loading 점 깜빡이는 애니메이션 타이머
	float DotTimer{};
	// 보더 회전하며 빛나는 애니메이션 타이머
	float BorderTimer{};
	
	// 레벨 전환을 한 번만 시작하기 위한 플래그
	bool bLevelTransitionStarted = false;
	
	// 로딩에서 만드는 월드 타일 데이터
	UPROPERTY()
	class UWorldComponent* WorldComponent = nullptr;
	
	// 월드 생성(0~50%)과 레벨 로딩(50~70%)을 나눕니다.
	enum class ELoadingStage
	{
		WorldGeneration,
		LevelLoading
	};
	// 지금 진행 중인 로딩 단계
	ELoadingStage CurrentLoadingStage = ELoadingStage::WorldGeneration;

public:
	// ========== 로드 모드 ==========

	// 메인메뉴에서 고른 슬롯으로 로드 모드에 들어갑니다.
	UFUNCTION(BlueprintCallable, Category = "Load Mode")
	void SetLoadMode(int32 SlotIndex);

protected:
	// 세이브에서 월드를 복원하는 로드 모드인지 여부
	UPROPERTY(BlueprintReadWrite, Category = "Load Mode")
	bool bIsLoadMode = false;

	// 로드할 슬롯 인덱스. 0이면 미설정입니다.
	UPROPERTY(BlueprintReadWrite, Category = "Load Mode")
	int32 LoadSlotIndex = 0;

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

	// 진행률을 초기화하고 월드 생성을 시작합니다. 참가자는 대기로 들어갑니다.
	virtual void NativeConstruct() override;
	// 진행률·텍스트·보더를 갱신하고 70%에서 레벨을 엽니다.
	virtual void NativeTick(const FGeometry& MyGeometry, float DeltaTime) override;
};
