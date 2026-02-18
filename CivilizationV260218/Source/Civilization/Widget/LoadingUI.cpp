// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadingUI.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Kismet/GameplayStatics.h"
#include "../SuperGameInstance.h"
#include "../World/WorldComponent.h"
#include "../SaveLoad/SaveLoadManager.h"
#include "../SaveLoad/SaveLoadStruct.h"

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

void ULoadingUI::SetLoadMode(int32 SlotIndex)
{
	bIsLoadMode = true;
	LoadSlotIndex = SlotIndex;
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
	
	// WorldConfig 설정
	FWorldConfig Settings;
	if (bIsLoadMode && GameInst && GameInst->GetSaveLoadManager())
	{
		// 로드 모드: 세이브 파일에서 WorldConfig 복원
		FString SlotName = FString::Printf(TEXT("SaveSlot%d"), LoadSlotIndex);
		USuperSaveGame* SaveGameObject = Cast<USuperSaveGame>(
			UGameplayStatics::LoadGameFromSlot(SlotName, 0));
		if (SaveGameObject)
		{
			Settings = SaveGameObject->SaveData.WorldConfig;
			// SaveData를 GameInstance에 임시 저장
			GameInst->PendingLoadData = SaveGameObject->SaveData;
			GameInst->bIsLoadingFromMainMenu = true;
		}
		else
		{
			// 세이브 파일 로드 실패 시 로드 모드 취소하고 일반 모드로 전환
			bIsLoadMode = false;
			Settings = GameInst->GetWorldConfig();
		}
	}
	else
	{
		// 일반 모드: GameInstance에서 설정 가져오기
		Settings = GameInst->GetWorldConfig();
	}
	
	WorldComponent->SetWorldConfig(Settings);
	
	// 월드 생성 완료 이벤트 바인딩
	WorldComponent->OnWorldGenerated.AddDynamic(this, &ULoadingUI::OnWorldGenerated);
	
	// 월드 생성 시작
	if (bIsLoadMode && GameInst && GameInst->GetSaveLoadManager())
	{
		// 로드 모드: 저장된 데이터로부터 월드 생성
		if (GameInst->PendingLoadData.WorldDataMap.Num() > 0)
		{
			WorldComponent->GenerateWorldFromSaveData(
				GameInst->PendingLoadData.WorldDataMap, 
				Settings, 
				GameInst->PendingLoadData.PlayerDataArray
			);
		}
		else
		{
			// PendingLoadData가 없으면 일반 생성으로 폴백
			WorldComponent->GenerateWorld();
		}
	}
	else
	{
		// 일반 모드: 랜덤 월드 생성
		WorldComponent->GenerateWorld();
	}
	
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
	bLevelTransitionStarted = false; // 레벨 전환 플래그 초기화
	
	// GameInstance에서 로드 모드 정보 확인 (메인메뉴에서 로드하는 경우)
	if (GameInst && GameInst->PendingLoadSlotIndex > 0)
	{
		SetLoadMode(GameInst->PendingLoadSlotIndex);
		GameInst->PendingLoadSlotIndex = 0; // 사용 후 초기화
	}
	
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
	
	// 완료 시 게임 시작 (70% 도달, 한 번만 실행)
	if (curPercent >= 0.7f && !bLevelTransitionStarted)
	{
		bLevelTransitionStarted = true; // 레벨 전환 시작 플래그 설정
		// 인게임 레벨로 전환
		if (GameInst)
		{
			FName TargetLevelName = GameInst->GetTargetLevelName();
			// TargetLevelName이 유효한지 확인 (NAME_None이 아닌지)
			if (TargetLevelName != NAME_None && !TargetLevelName.IsNone())
			{
				UGameplayStatics::OpenLevel(this, TargetLevelName);
			}
			else
			{
				// TargetLevel이 설정되지 않은 경우 기본값으로 InGame 사용
				UGameplayStatics::OpenLevel(this, TEXT("InGame"));
			}
		}
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

