// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadingTilesUI.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "../WorldSpawner.h"
#include "../WorldComponent.h"  //시작 유닛 소환
#include "../Unit/UnitCharacterBase.h"  //시작 유닛 소환
#include "../SuperGameInstance.h"  //시작 유닛 소환
#include "../Unit/UnitManager.h"  //시작 유닛 소환
#include "EngineUtils.h"

void ULoadingTilesUI::OnTileSpawnCompleted()
{
	// 스폰 완료 플래그 설정 (점프 없이 계속 증가)
	bSpawnCompleted = true;
	
	// 월드 중심에 유닛 소환  //시작 유닛 소환
	SpawnStartingUnit();
}

void ULoadingTilesUI::SpawnStartingUnit()  //시작 유닛 소환
{
	// SuperGameInstance를 통해 UnitManager에 접근
	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!GameInstance)
	{
		return;
	}
	
	UUnitManager* UnitManager = GameInstance->GetUnitManager();
	if (!UnitManager)
	{
		return;
	}
	
	// 월드 중심(Q=0, R=0)에 Warrior1 유닛 소환
	UnitManager->SpawnUnitAtHex(FVector2D(0, 0), FName("Warrior1"));
}

void ULoadingTilesUI::StartTileSpawning()
{
	// 현재 레벨에서 WorldSpawner 찾기
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
		// 스폰 없이 완료 처리
		OnTileSpawnCompleted();
		return;
	}
	
	// 스폰 완료 이벤트 바인딩
	WorldSpawner->OnTileSpawnCompleted.AddDynamic(this, &ULoadingTilesUI::OnTileSpawnCompleted);
	
	// 타일 스폰 시작 (WorldComponent는 WorldSpawner가 GameInstance에서 가져옴)
	WorldSpawner->SpawnAllTiles();

	// 도시 스폰 시작 (WorldComponent의 StartingCityHexes 기준)
	WorldSpawner->SpawnAllCities();

	// 도시를 플레이어들에게 배정
	WorldSpawner->AssignCitiesToPlayers();
}

void ULoadingTilesUI::NativeConstruct()
{
	Super::NativeConstruct();
	Bar->SetPercent(0.7f);
	PercentText->SetText(FText::FromString(TEXT("(70%)")));
	LoadingText->SetText(FText::FromString(TEXT("Loading . . .")));
	DotTimer = 0.f;
	BorderTimer = 0.f;
	
	// 보더 초기 opacity 설정
	if (Border1) Border1->SetRenderOpacity(0.2f);
	if (Border2) Border2->SetRenderOpacity(0.2f);
	if (Border3) Border3->SetRenderOpacity(0.2f);
	if (Border4) Border4->SetRenderOpacity(0.2f);
	
	// 로딩 단계 초기화
	CurrentLoadingStage = ELoadingStage::TileSpawning;
	curPercent = 0.7f;
	targetPercent = 0.9f; // 자동으로 90%까지 증가
	bSpawnCompleted = false;
	
	// 타일 스폰 시작 (백그라운드에서 실행)
	StartTileSpawning();
}

void ULoadingTilesUI::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);
	
	// 실제 진행률에 따라 업데이트
	if (curPercent < targetPercent)
	{
		// 단계별 진행률 속도 조정
		float ProgressSpeed = 0.0f;
		switch (CurrentLoadingStage)
		{
		case ELoadingStage::TileSpawning:
			ProgressSpeed = 0.5f;  // 타일 스폰
			break;
		case ELoadingStage::Finalizing:
			ProgressSpeed = 0.5f;  // 최종 완료
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
	
	// 90% 도달 시 Finalizing으로 전환 (스폰 완료 시에만)
	if (CurrentLoadingStage == ELoadingStage::TileSpawning && 
	    bSpawnCompleted && 
	    curPercent >= 0.9f)
	{
		CurrentLoadingStage = ELoadingStage::Finalizing;
		targetPercent = 1.0f;
	}
	
	// 완료 시 위젯 닫기 (100% 도달)
	if (curPercent >= 1.0f)
	{
		// 위젯 닫기
		RemoveFromParent();
	}
}

void ULoadingTilesUI::UpdateLoadingText(float DeltaTime)
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

void ULoadingTilesUI::UpdateBorderAnimation(float DeltaTime)
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


