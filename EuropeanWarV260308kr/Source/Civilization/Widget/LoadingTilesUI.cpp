// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadingTilesUI.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "../World/WorldSpawner.h"
#include "EngineUtils.h"
#include "../SaveLoad/SaveLoadManager.h"
#include "../SuperGameInstance.h"

void ULoadingTilesUI::OnTileSpawnCompleted()
{
	bSpawnCompleted = true;
	
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			if (SuperGameInst->bIsLoadingFromMainMenu && WorldSpawner)
			{
				// WorldSpawner에서 저장된 데이터로 게임 상태 복원 (매니저들, 플레이어 스테이트, 시설 등)
				WorldSpawner->RestoreGameStateFromSave(SuperGameInst->PendingLoadData);
				
				// 로드 모드 플래그 해제
				SuperGameInst->bIsLoadingFromMainMenu = false;
				bDataRestored = true;
			}
		}
	}
}

void ULoadingTilesUI::StartTileSpawning()
{
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
		// 한 틱 늦게 등장하는 경우를 위해 재시도 (BeginPlay 순서 이슈 방지)
		if (StartTileSpawningRetryCount < MaxStartTileSpawningRetries)
		{
			StartTileSpawningRetryCount++;
			TicksBeforeStartTileSpawning = 1; // 다음 틱에 다시 시도
			return;
		}
		OnTileSpawnCompleted();
		return;
	}
	WorldSpawner->OnTileSpawnCompleted.AddDynamic(this, &ULoadingTilesUI::OnTileSpawnCompleted);
	WorldSpawner->SpawnAllTiles();
	WorldSpawner->SpawnAllCities();
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
	bDataRestored = false;
	TicksBeforeStartTileSpawning = 2;   // 2틱 지연으로 BeginPlay 선행 보장
	StartTileSpawningRetryCount = 0;
}

void ULoadingTilesUI::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);
	
	// 2틱 지연 후 StartTileSpawning 실행 (GameMode/WorldSpawner BeginPlay 선행 보장)
	if (TicksBeforeStartTileSpawning > 0)
	{
		TicksBeforeStartTileSpawning--;
		if (TicksBeforeStartTileSpawning > 0)
		{
			return; // 아직 대기 중이면 진행률/전환 로직 스킵 (선택적)
		}
		StartTileSpawning();
	}
	
	UWorld* World = GetWorld();
	USuperGameInstance* GameInst = World ? Cast<USuperGameInstance>(World->GetGameInstance()) : nullptr;
	
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
	
	if (CurrentLoadingStage == ELoadingStage::TileSpawning && 
	    bSpawnCompleted && 
	    (!GameInst || !GameInst->bIsLoadingFromMainMenu || bDataRestored) &&
	    curPercent >= 0.9f)
	{
		CurrentLoadingStage = ELoadingStage::Finalizing;
		targetPercent = 1.0f;
	}
	
	if (curPercent >= 1.0f)
	{
		// 로딩 화면 종료 직전 알림 → 블루프린트에서 이 시점에 MainHUD 생성 시 복원값으로 한 번만 세팅됨
		if (World)
		{
			if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
			{
				SuperGameInst->NotifyLoadingScreenFinished();
			}
		}
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


