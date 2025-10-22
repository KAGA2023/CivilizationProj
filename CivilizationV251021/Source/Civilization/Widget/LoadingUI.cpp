// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadingUI.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Kismet/GameplayStatics.h"
#include "../SuperGameInstance.h"
#include "../WorldComponent.h"

void ULoadingUI::OnLevelLoaded()
{
	// 레벨 로딩 완료 (70%)
	targetPercent = 0.7f;
}

void ULoadingUI::OnWorldGenerated(bool bSuccess)
{
	// 월드 생성 완료 (50%)
	CurrentLoadingStage = ELoadingStage::LevelLoading;
	targetPercent = 0.5f;
	
	// 월드 생성 실패 시 처리
	if (!bSuccess)
	{
		return;
	}
	
	// GameInstance에 월드 컴포넌트 저장
	if (WorldComponent && GameInst)
	{
		GameInst->SetGeneratedWorldComponent(WorldComponent);
	}
	
	// 2단계: 레벨 로딩 시작 (50% → 70%)
	LoadLevelAsync();
}


void ULoadingUI::GenerateWorldAsync()
{
	// 안전장치: 기존 월드 컴포넌트가 있으면 정리
	if (GameInst)
	{
		GameInst->ClearGeneratedWorldComponent();
	}
	
	// 새 월드 컴포넌트 생성
	WorldComponent = NewObject<UWorldComponent>(this);
	
	// GameInstance에서 설정 가져오기
	FWorldConfig Settings = GameInst->GetWorldConfig();
	WorldComponent->SetWorldConfig(Settings);
	
	// 월드 생성 완료 이벤트 바인딩
	WorldComponent->OnWorldGenerated.AddDynamic(this, &ULoadingUI::OnWorldGenerated);
	
	// 월드 생성 시작
	WorldComponent->GenerateWorld();
	
	// 월드 생성 중 진행률 (0% → 50%)
	targetPercent = 0.5f;
}

void ULoadingUI::LoadLevelAsync()
{
	// SuperGameInstance에서 타겟 레벨 패키지 경로 가져오기
	if (!GameInst)
	{
		return;
	}
	
	// 전체 패키지 경로 가져오기 (예: "/Game/Maps/InGame")
	FString LevelPackageName = GameInst->GetTargetLevelPackageName();
	
	if (LevelPackageName.IsEmpty())
	{
		return;
	}
	
	// 비동기 로딩 델리게이트 설정 (UFUNCTION 방식)
	FLoadPackageAsyncDelegate LoadedDelegate;
	LoadedDelegate.BindUFunction(this, FName("OnLevelLoaded"));
	
	// 비동기 패키지 로딩 시작
	LoadPackageAsync(LevelPackageName, LoadedDelegate);
	
	// 레벨 로딩 중 진행률 (50% → 70%)
	targetPercent = 0.7f;
}


void ULoadingUI::NativeConstruct()
{
	Super::NativeConstruct();
	Bar->SetPercent(0.f);
	PercentText->SetText(FText::FromString(TEXT("(0%)")));
	LoadingText->SetText(FText::FromString(TEXT("Loading")));
	DotTimer = 0.f;
	BorderTimer = 0.f;
	
	// 보더 초기 opacity 설정
	if (Border1) Border1->SetRenderOpacity(0.2f);
	if (Border2) Border2->SetRenderOpacity(0.2f);
	if (Border3) Border3->SetRenderOpacity(0.2f);
	if (Border4) Border4->SetRenderOpacity(0.2f);
	
	// 로딩 단계 초기화
	CurrentLoadingStage = ELoadingStage::WorldGeneration;
	curPercent = 0.0f;
	targetPercent = 0.0f;
	
	// 1단계: 월드 생성 시작 (0% → 50%)
	GenerateWorldAsync();
}

void ULoadingUI::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);
	
	// 실제 진행률에 따라 업데이트
	if (curPercent < targetPercent)
	{
		// 단계별 진행률 속도 조정
		float ProgressSpeed = 0.0f;
		switch (CurrentLoadingStage)
		{
		case ELoadingStage::WorldGeneration:
			ProgressSpeed = 0.5f; // 월드 생성
			break;
		case ELoadingStage::LevelLoading:
			ProgressSpeed = 0.5f; // 레벨 로딩
			break;
		}
		
		curPercent = FMath::Clamp(curPercent + DeltaTime * ProgressSpeed, 0.f, targetPercent);
		Bar->SetPercent(curPercent);
		
		// PercentText 업데이트
		int32 PercentValue = FMath::RoundToInt(curPercent * 100);
		FString PercentString = FString::Printf(TEXT("(%d%%)"), PercentValue);
		PercentText->SetText(FText::FromString(PercentString));
		
		// 로딩 텍스트 업데이트
		UpdateLoadingText(DeltaTime);
		
		// 보더 애니메이션 업데이트
		UpdateBorderAnimation(DeltaTime);
	}
	
	// 완료 시 게임 시작 (70% 도달)
	if (curPercent >= 0.7f)
	{
		// 인게임 레벨로 전환
		UGameplayStatics::OpenLevel(this, GameInst->GetTargetLevelName());
	}
}

void ULoadingUI::UpdateLoadingText(float DeltaTime)
{
	// 0.3초마다 Loading . . . 점 깜빡
	DotTimer += DeltaTime;
	if (DotTimer >= 0.3f)
	{
		DotTimer = 0.f;
		
		static int32 DotCount = 0;
		DotCount = (DotCount + 1) % 4;  // 0, 1, 2, 3 순환
		
		FString Dots = TEXT("");
		for (int32 i = 0; i < DotCount; i++)
		{
			Dots += TEXT(". ");
		}
		
		// 단계별 로딩 텍스트
		FString StageText = TEXT("Loading");
		FString LoadingString = FString::Printf(TEXT("%s %s"), *StageText, *Dots);
		LoadingText->SetText(FText::FromString(LoadingString));
	}
}

void ULoadingUI::UpdateBorderAnimation(float DeltaTime)
{
	// 0.3초마다 보더 회전
	BorderTimer += DeltaTime;
	if (BorderTimer >= 0.3f)
	{
		BorderTimer = 0.f;
		
		static int32 BrightBorderIndex = 0;
		int32 PreviousBorderIndex = (BrightBorderIndex - 1 + 4) % 4;  // 이전 보더 인덱스
		BrightBorderIndex = (BrightBorderIndex + 1) % 4;
		
		// 모든 보더 초기화
		if (Border1) Border1->SetRenderOpacity(0.2f);
		if (Border2) Border2->SetRenderOpacity(0.2f);
		if (Border3) Border3->SetRenderOpacity(0.2f);
		if (Border4) Border4->SetRenderOpacity(0.2f);

		// 이전 보더를 중간 밝기로 설정
		switch (PreviousBorderIndex)
		{
		case 0:  // 좌상
			if (Border1) Border2->SetRenderOpacity(0.6f);
			break;
		case 1:  // 우상
			if (Border2) Border3->SetRenderOpacity(0.6f);
			break;
		case 2:  // 우하
			if (Border3) Border4->SetRenderOpacity(0.6f);
			break;
		case 3:  // 좌하
			if (Border4) Border1->SetRenderOpacity(0.6f);
			break;
		}
		
		// 현재 밝은 보더만 밝게 설정 (시계방향)
		switch (BrightBorderIndex)
		{
		case 0:  // 좌상
			if (Border1) Border1->SetRenderOpacity(1.0f);
			break;
		case 1:  // 우상
			if (Border2) Border2->SetRenderOpacity(1.0f);
			break;
		case 2:  // 우하
			if (Border3) Border3->SetRenderOpacity(1.0f);
			break;
		case 3:  // 좌하
			if (Border4) Border4->SetRenderOpacity(1.0f);
			break;
		}
	}
}

