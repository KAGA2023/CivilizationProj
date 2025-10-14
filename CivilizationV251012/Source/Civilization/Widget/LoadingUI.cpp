// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadingUI.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Kismet/GameplayStatics.h"
#include "../SuperGameInstance.h"
#include "../WorldComponent.h"
#include "../WorldSpawner.h"
#include "EngineUtils.h"

void ULoadingUI::OnLevelLoaded()
{
	// 레벨 로딩 완료 (70%)
	CurrentLoadingStage = ELoadingStage::TileSpawning;
	targetPercent = 0.7f;
	
	// 3단계: 타일 스폰 시작 (70% → 90%)
	SpawnTilesAsync();
}

void ULoadingUI::OnWorldGenerated(bool bSuccess)
{
	// 월드 생성 완료 (50%)
	CurrentLoadingStage = ELoadingStage::LevelLoading;
	targetPercent = 0.5f;
	
	// 월드 생성 실패 시 처리
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("[LoadingUI] World Generation Failed!"));
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

void ULoadingUI::OnTileSpawnProgress(int32 CurrentCount, int32 TotalCount)
{
	// 타일 스폰 진행률 계산 (70% → 90%)
	float SpawnProgress = (float)CurrentCount / (float)TotalCount;
	targetPercent = 0.7f + (SpawnProgress * 0.2f); // 70% + (진행률 * 20%)
}

void ULoadingUI::OnTileSpawnCompleted()
{
	// 타일 스폰 완료 (90%)
	CurrentLoadingStage = ELoadingStage::Finalizing;
	targetPercent = 0.9f;
	
	UE_LOG(LogTemp, Log, TEXT("[LoadingUI] Tile spawning completed!"));
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
	// 레벨 로딩 시작
	FString LevelName = GameInst->GetTargetLevelName().ToString();
	FLoadPackageAsyncDelegate LoadedDelegate;
	LoadedDelegate.BindUFunction(this, FName("OnLevelLoaded"));
	LoadPackageAsync(LevelName, LoadedDelegate);
	
	// 레벨 로딩 중 진행률 (50% → 70%)
	targetPercent = 0.7f;
}

void ULoadingUI::SpawnTilesAsync()
{
	// 로딩된 레벨에서 WorldSpawner 찾기
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AWorldSpawner> It(World); It; ++It)
		{
			WorldSpawner = *It;
			break;
		}
	}
	
	if (!WorldSpawner)
	{
		UE_LOG(LogTemp, Error, TEXT("[LoadingUI] WorldSpawner not found in level!"));
		// 스폰 없이 완료 처리
		OnTileSpawnCompleted();
		return;
	}
	
	// WorldComponent 전달
	if (WorldComponent)
	{
		WorldSpawner->SetWorldComponent(WorldComponent);
	}
	
	// 진행률 이벤트 바인딩
	WorldSpawner->OnTileSpawnProgress.AddDynamic(this, &ULoadingUI::OnTileSpawnProgress);
	WorldSpawner->OnTileSpawnCompleted.AddDynamic(this, &ULoadingUI::OnTileSpawnCompleted);
	
	// 타일 스폰 시작
	WorldSpawner->SpawnAllTiles();
	
	UE_LOG(LogTemp, Log, TEXT("[LoadingUI] Tile spawning started..."));
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
	
	// 1단계: 월드 생성 시작 (0% → 70%)
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
			ProgressSpeed = 0.05f; // 월드 생성 (느림)
			break;
		case ELoadingStage::LevelLoading:
			ProgressSpeed = 0.15f; // 레벨 로딩 (빠름)
			break;
		case ELoadingStage::TileSpawning:
			ProgressSpeed = 0.1f;  // 타일 스폰 (보통)
			break;
		case ELoadingStage::Finalizing:
			ProgressSpeed = 0.2f;  // 최종 완료 (빠름)
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
	
	// 완료 시 게임 시작
	if (FMath::IsNearlyEqual(curPercent, 1.f, 0.01f))
	{
		// 최종 완료 처리
		CurrentLoadingStage = ELoadingStage::Finalizing;
		targetPercent = 1.0f;
		
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
		switch (CurrentLoadingStage)
		{
		case ELoadingStage::WorldGeneration:
			StageText = TEXT("Generating World");
			break;
		case ELoadingStage::LevelLoading:
			StageText = TEXT("Loading Level");
			break;
		case ELoadingStage::TileSpawning:
			StageText = TEXT("Spawning Tiles");
			break;
		case ELoadingStage::Finalizing:
			StageText = TEXT("Finalizing");
			break;
		}
		
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

