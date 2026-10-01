// Fill out your copyright notice in the Description page of Project Settings.

#include "CityUI.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "../../SuperGameInstance.h"
#include "../../SuperPlayerState.h"
#include "../../World/WorldComponent.h"
#include "../../World/WorldTileActor.h"
#include "../../City/CityComponent.h"
#include "../../City/CityStruct.h"
#include "../../Status/UnitStatusStruct.h"
#include "../../Facility/FacilityManager.h"
#include "Kismet/GameplayStatics.h"

void UCityUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기 텍스트 설정
	if (ScienceTxt)
	{
		ScienceTxt->SetText(FText::FromString(TEXT("+0")));
	}

	if (GoldTxt)
	{
		GoldTxt->SetText(FText::FromString(TEXT("+0")));
	}

	if (ProductionTxt)
	{
		ProductionTxt->SetText(FText::FromString(TEXT("+0")));
	}

	if (FoodTxt)
	{
		FoodTxt->SetText(FText::FromString(TEXT("+0")));
	}

	// ProductionWid는 기본적으로 Visible, PurchaseWid는 Hidden
	if (ProductionWid)
	{
		ProductionWid->SetVisibility(ESlateVisibility::Visible);
	}

	if (PurchaseWid)
	{
		PurchaseWid->SetVisibility(ESlateVisibility::Hidden);
	}

	// 버튼 클릭 이벤트 바인딩
	if (ProductionBtn)
	{
		ProductionBtn->OnClicked.AddDynamic(this, &UCityUI::OnProductionBtnClicked);
	}

	if (PurchaseBtn)
	{
		PurchaseBtn->OnClicked.AddDynamic(this, &UCityUI::OnPurchaseBtnClicked);
	}

	if (PurchaseTileBtn)
	{
		PurchaseTileBtn->OnClicked.AddDynamic(this, &UCityUI::OnPurchaseTileBtnClicked);
	}

	if (CloseBtn)
	{
		CloseBtn->OnClicked.AddDynamic(this, &UCityUI::OnCloseBtnClicked);
	}

	// 도시 컴포넌트 참조 가져오기 및 델리게이트 바인딩
	BindToProductionDelegates();
	BindToFacilityDelegates();
	BindToPurchaseDelegates();
}

void UCityUI::UpdateCityData()
{
	// GetGameInstance -> CastToSuperGameInstance -> GetPlayerState(0)
	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0);
	if (!PlayerState)
	{
		return;
	}

	UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent();
	if (!WorldComponent)
	{
		return;
	}

	// CalculateTotalScienceYield를 +x 형식으로 ScienceTxt에 표시
	if (ScienceTxt)
	{
		int32 ScienceYield = PlayerState->CalculateTotalScienceYield(WorldComponent);
		FString ScienceString = FString::Printf(TEXT("+%d"), ScienceYield);
		ScienceTxt->SetText(FText::FromString(ScienceString));
	}

	// CalculateTotalGoldYield를 +x 형식으로 GoldTxt에 표시
	if (GoldTxt)
	{
		int32 GoldYield = PlayerState->CalculateTotalGoldYield(WorldComponent);
		FString GoldString = FString::Printf(TEXT("+%d"), GoldYield);
		GoldTxt->SetText(FText::FromString(GoldString));
	}

	// CalculateTotalProductionYield를 +x 형식으로 ProductionTxt에 표시
	if (ProductionTxt)
	{
		int32 ProductionYield = PlayerState->CalculateTotalProductionYield(WorldComponent);
		FString ProductionString = FString::Printf(TEXT("+%d"), ProductionYield);
		ProductionTxt->SetText(FText::FromString(ProductionString));
	}

	// CalculateTotalFoodYield를 +x 형식으로 FoodTxt에 표시
	if (FoodTxt)
	{
		int32 FoodYield = PlayerState->CalculateTotalFoodYield(WorldComponent);
		FString FoodString = FString::Printf(TEXT("+%d"), FoodYield);
		FoodTxt->SetText(FText::FromString(FoodString));
	}

	// 생산 정보 업데이트
	UpdateProductionInfo();
}

void UCityUI::OnProductionBtnClicked()
{
	// ProductionBtn 클릭 시 ProductionWid를 Visible, PurchaseWid를 Hidden으로 설정
	if (ProductionWid)
	{
		ProductionWid->SetVisibility(ESlateVisibility::Visible);
	}

	if (PurchaseWid)
	{
		PurchaseWid->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UCityUI::OnPurchaseBtnClicked()
{
	// PurchaseBtn 클릭 시 PurchaseWid를 Visible, ProductionWid를 Hidden으로 설정
	if (PurchaseWid)
	{
		PurchaseWid->SetVisibility(ESlateVisibility::Visible);
	}

	if (ProductionWid)
	{
		ProductionWid->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UCityUI::NativeDestruct()
{
	// 델리게이트 바인딩 해제
	UnbindFromProductionDelegates();
	UnbindFromFacilityDelegates();
	UnbindFromPurchaseDelegates();

	// 구매 모드 해제
	if (bIsTilePurchaseMode)
	{
		ExitPurchaseMode();
	}

	Super::NativeDestruct();
}

void UCityUI::BindToProductionDelegates()
{
	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0);
	if (!PlayerState)
	{
		return;
	}

	// CityComponent 가져오기
	CachedCityComponent = PlayerState->GetCityComponent();
	if (!CachedCityComponent)
	{
		return;
	}

	// 기존 바인딩 해제 후 새로 바인딩
	UnbindFromProductionDelegates();

	// 생산 시작 델리게이트 바인딩
	CachedCityComponent->OnProductionStarted.AddDynamic(this, &UCityUI::OnProductionStarted);

	// 생산 완료 델리게이트 바인딩
	CachedCityComponent->OnProductionCompleted.AddDynamic(this, &UCityUI::OnProductionCompleted);

	// 생산 진행도 변경 델리게이트 바인딩
	CachedCityComponent->OnProductionProgressChanged.AddDynamic(this, &UCityUI::OnProductionProgressChanged);

	// 초기 생산 정보 업데이트
	UpdateProductionInfo();
}

void UCityUI::UnbindFromProductionDelegates()
{
	if (CachedCityComponent)
	{
		// 델리게이트 바인딩 해제
		CachedCityComponent->OnProductionStarted.RemoveDynamic(this, &UCityUI::OnProductionStarted);
		CachedCityComponent->OnProductionCompleted.RemoveDynamic(this, &UCityUI::OnProductionCompleted);
		CachedCityComponent->OnProductionProgressChanged.RemoveDynamic(this, &UCityUI::OnProductionProgressChanged);
	}
}

void UCityUI::OnProductionStarted(FName ProductionID)
{
	// 생산 시작 시 생산 정보 업데이트
	UpdateProductionInfo();
}

void UCityUI::OnProductionCompleted(FName ProductionID)
{
	// 도시 데이터 업데이트 (건물 추가로 인한 생산량 변화 반영)
	UpdateCityData();
}

void UCityUI::OnProductionProgressChanged()
{
	// 생산 진행도 변경 시 생산 정보 업데이트 (진행도 바 포함)
	UpdateProductionInfo();
}

void UCityUI::UpdateProductionInfo()
{
	if (!CachedCityComponent)
	{
		// CityComponent가 없으면 초기화 시도
		BindToProductionDelegates();
		if (!CachedCityComponent)
		{
			// 여전히 없으면 생산 정보 초기화
			if (ProducingTxt)
			{
				ProducingTxt->SetText(FText::GetEmpty());
			}
			if (ProducingImg)
			{
				ProducingImg->SetBrushFromTexture(nullptr);
			}
			if (ProducingBar)
			{
				ProducingBar->SetPercent(0.0f);
			}
			return;
		}
	}

	// 현재 생산 상태 가져오기
	FCityCurrentStat CurrentStat = CachedCityComponent->GetCurrentStat();

	// 생산 중이 아니면 NoSelect 데이터 표시
	if (CurrentStat.ProductionType == EProductionType::None || CurrentStat.ProductionName == NAME_None)
	{
		// NoSelect 건물 데이터 가져오기
		FBuildingData NoSelectData = CachedCityComponent->GetBuildingDataFromTable(FName("NoSelect"));
		
		if (NoSelectData.BuildingType == EBuildingType::None && NoSelectData.BuildingName.IsEmpty())
		{
			// NoSelect 데이터가 없으면 빈 값으로 설정
			if (ProducingTxt)
			{
				ProducingTxt->SetText(FText::GetEmpty());
			}
			if (ProducingImg)
			{
				ProducingImg->SetBrushFromTexture(nullptr);
			}
			if (ProducingBar)
			{
				ProducingBar->SetPercent(0.0f);
			}
			return;
		}

		// NoSelect 건물 이름 표시
		if (ProducingTxt)
		{
			ProducingTxt->SetText(FText::FromString(NoSelectData.BuildingName));
		}

		// NoSelect 건물 아이콘 표시
		if (ProducingImg)
		{
			if (!NoSelectData.BuildingIcon.IsNull())
			{
				UTexture2D* NoSelectIcon = NoSelectData.BuildingIcon.LoadSynchronous();
				if (NoSelectIcon)
				{
					ProducingImg->SetBrushFromTexture(NoSelectIcon);
				}
				else
				{
					ProducingImg->SetBrushFromTexture(nullptr);
				}
			}
			else
			{
				ProducingImg->SetBrushFromTexture(nullptr);
			}
		}

		// 진행도 바는 0%
		if (ProducingBar)
		{
			ProducingBar->SetPercent(0.0f);
		}

		return;
	}

	// 건물 생산 중인 경우
	if (CurrentStat.ProductionType == EProductionType::Building)
	{
		// 건물 데이터 가져오기
		FBuildingData BuildingData = CachedCityComponent->GetBuildingDataFromTable(CurrentStat.ProductionName);
		
		if (BuildingData.BuildingType != EBuildingType::None)
		{
			// 건물 이름 표시
			if (ProducingTxt)
			{
				ProducingTxt->SetText(FText::FromString(BuildingData.BuildingName));
			}

			// 건물 아이콘 표시
			if (ProducingImg)
			{
				if (!BuildingData.BuildingIcon.IsNull())
				{
					UTexture2D* BuildingIcon = BuildingData.BuildingIcon.LoadSynchronous();
					if (BuildingIcon)
					{
						ProducingImg->SetBrushFromTexture(BuildingIcon);
					}
					else
					{
						ProducingImg->SetBrushFromTexture(nullptr);
					}
				}
				else
				{
					ProducingImg->SetBrushFromTexture(nullptr);
				}
			}

			// 건물 진행도 바 업데이트 (ProductionProgress / ProductionCost)
			if (ProducingBar)
			{
				float ProgressPercent = 0.0f;
				if (CurrentStat.ProductionCost > 0)
				{
					ProgressPercent = FMath::Clamp((float)CurrentStat.ProductionProgress / (float)CurrentStat.ProductionCost, 0.0f, 1.0f);
				}
				ProducingBar->SetPercent(ProgressPercent);
			}
		}
	}
	// 유닛 생산 중인 경우
	else if (CurrentStat.ProductionType == EProductionType::Unit)
	{
		// 유닛 데이터 가져오기
		FUnitBaseStat UnitData = CachedCityComponent->GetUnitDataFromTable(CurrentStat.ProductionName);
		
		if (UnitData.UnitClass != EUnitClass::None)
		{
			// 유닛 이름 표시
			if (ProducingTxt)
			{
				ProducingTxt->SetText(UnitData.UnitName);
			}

			// 유닛 아이콘 표시
			if (ProducingImg)
			{
				if (!UnitData.UnitIcon.IsNull())
				{
					UTexture2D* UnitIcon = UnitData.UnitIcon.LoadSynchronous();
					if (UnitIcon)
					{
						ProducingImg->SetBrushFromTexture(UnitIcon);
					}
					else
					{
						ProducingImg->SetBrushFromTexture(nullptr);
					}
				}
				else
				{
					ProducingImg->SetBrushFromTexture(nullptr);
				}
			}

			// 유닛 진행도 바 업데이트 (FoodProgress / FoodCost)
			if (ProducingBar)
			{
				float ProgressPercent = 0.0f;
				if (CurrentStat.FoodCost > 0)
				{
					ProgressPercent = FMath::Clamp((float)CurrentStat.FoodProgress / (float)CurrentStat.FoodCost, 0.0f, 1.0f);
				}
				ProducingBar->SetPercent(ProgressPercent);
			}
		}
	}
}

void UCityUI::BindToFacilityDelegates()
{
	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	UFacilityManager* FacilityManager = SuperGameInst->GetFacilityManager();
	if (!FacilityManager)
	{
		return;
	}

	// 기존 바인딩 해제 후 새로 바인딩
	UnbindFromFacilityDelegates();

	// 시설 변경 델리게이트 바인딩
	FacilityManager->OnFacilityChanged.AddDynamic(this, &UCityUI::OnFacilityChanged);
}

void UCityUI::UnbindFromFacilityDelegates()
{
	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	UFacilityManager* FacilityManager = SuperGameInst->GetFacilityManager();
	if (FacilityManager)
	{
		// 델리게이트 바인딩 해제
		FacilityManager->OnFacilityChanged.RemoveDynamic(this, &UCityUI::OnFacilityChanged);
	}
}

// ========== 타일 구매 시스템 ==========
void UCityUI::OnCloseBtnClicked()
{
	// 타일 구매 모드 중이면 해제 (CloseBtn으로도 구매 모드 끄기)
	if (bIsTilePurchaseMode)
	{
		ExitPurchaseMode();
	}
}

void UCityUI::OnPurchaseTileBtnClicked()
{
	// 구매 모드 토글
	bIsTilePurchaseMode = !bIsTilePurchaseMode;

	if (bIsTilePurchaseMode)
	{
		// 구매 모드 활성화: 델리게이트 다시 바인딩 (타일 액터가 나중에 스폰될 수 있으므로)
		BindToPurchaseDelegates();
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance()))
		{
			SuperGameInst->SetTilePurchaseMode(true);
		}
		// 구매 가능한 타일 찾기 및 하이라이트
		FindPurchaseableTiles();
		HighlightPurchaseableTiles();
	}
	else
	{
		// 구매 모드 비활성화: 하이라이트 제거
		ExitPurchaseMode();
	}
}

void UCityUI::FindPurchaseableTiles()
{
	// PurchaseableTileCoordinates 초기화
	PurchaseableTileCoordinates.Empty();

	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0);
	UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent();

	if (!PlayerState || !WorldComponent)
	{
		return;
	}

	// 소유한 타일 좌표들 가져오기
	TArray<FVector2D> OwnedTiles = PlayerState->GetOwnedTileCoordinates();

	// 중복 방지를 위한 Set 사용
	TSet<FVector2D> PurchaseableTileSet;

	// 각 소유 타일의 인접 타일 확인
	for (const FVector2D& OwnedTileCoord : OwnedTiles)
	{
		// 인접 타일 좌표들 가져오기
		TArray<FVector2D> NeighborCoords = WorldComponent->GetHexNeighbors(OwnedTileCoord);

		// 각 인접 타일이 구매 가능한지 확인
		for (const FVector2D& NeighborCoord : NeighborCoords)
		{
			// 이미 확인한 타일은 건너뛰기
			if (PurchaseableTileSet.Contains(NeighborCoord))
			{
				continue;
			}

			// 구매 가능한지 확인
			if (PlayerState->CanPurchaseTile(NeighborCoord, WorldComponent))
			{
				PurchaseableTileSet.Add(NeighborCoord);
			}
		}
	}

	// Set을 Array로 변환
	PurchaseableTileCoordinates = PurchaseableTileSet.Array();
}

void UCityUI::HighlightPurchaseableTiles()
{
	if (!GetWorld())
	{
		return;
	}

	// 먼저 모든 하이라이트 제거
	ClearPurchaseableTileHighlights();

	// 모든 WorldTileActor 찾기
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWorldTileActor::StaticClass(), FoundActors);

	// 각 타일 액터 확인
	for (AActor* Actor : FoundActors)
	{
		if (AWorldTileActor* TileActor = Cast<AWorldTileActor>(Actor))
		{
			if (UWorldTile* Tile = TileActor->TileData)
			{
				FVector2D TileCoord = Tile->GetGridPosition();

				// 구매 가능한 타일이면 하이라이트 활성화
				if (PurchaseableTileCoordinates.Contains(TileCoord))
				{
					TileActor->SetPurchaseableHighlight(true);
				}
			}
		}
	}
}

void UCityUI::ClearPurchaseableTileHighlights()
{
	if (!GetWorld())
	{
		return;
	}

	// 모든 WorldTileActor 찾기
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWorldTileActor::StaticClass(), FoundActors);

	// 모든 타일 액터의 하이라이트 제거
	for (AActor* Actor : FoundActors)
	{
		if (AWorldTileActor* TileActor = Cast<AWorldTileActor>(Actor))
		{
			TileActor->SetPurchaseableHighlight(false);
		}
	}
}

void UCityUI::OnPurchaseTileClickedHandler(FVector2D TileCoordinate)
{
	// 구매 모드가 아니면 무시
	if (!bIsTilePurchaseMode)
	{
		return;
	}

	// 클릭한 타일이 구매 가능한지 확인
	if (!PurchaseableTileCoordinates.Contains(TileCoordinate))
	{
		return;
	}

	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0);
	UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent();

	if (!PlayerState || !WorldComponent)
	{
		return;
	}

	// 타일 구매 실행
	if (PlayerState->PurchaseTile(TileCoordinate, WorldComponent))
	{
		// 구매 성공
		// 구매 가능한 타일 목록 재계산
		FindPurchaseableTiles();
		
		// 하이라이트 업데이트
		HighlightPurchaseableTiles();
		
		// 도시 데이터 UI 업데이트 (골드 등)
		UpdateCityData();
	}
	else
	{
		// 구매 실패 (골드 부족 등)
	}
}

void UCityUI::OnGoldChanged(int32 NewGold)
{
	// 구매 모드일 때만 재계산
	if (bIsTilePurchaseMode)
	{
		// 구매 가능한 타일 목록은 동일하지만, 골드 부족으로 인해 일부 타일이 구매 불가능할 수 있음
		// 간단하게 하이라이트만 업데이트 (실제 구매 가능 여부는 클릭 시 재확인)
		HighlightPurchaseableTiles();
		
		// 도시 데이터 업데이트 (골드 표시)
		UpdateCityData();
	}
}

void UCityUI::ExitPurchaseMode()
{
	bIsTilePurchaseMode = false;
	if (GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance()))
		{
			SuperGameInst->SetTilePurchaseMode(false);
		}
	}
	ClearPurchaseableTileHighlights();
	PurchaseableTileCoordinates.Empty();
	
	// 구매 관련 델리게이트 언바인드
	UnbindFromPurchaseDelegates();
}

void UCityUI::BindToPurchaseDelegates()
{
	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0);
	if (!PlayerState)
	{
		return;
	}

	// 기존 바인딩 해제 후 새로 바인딩
	UnbindFromPurchaseDelegates();

	// 골드 변경 델리게이트 바인딩
	PlayerState->OnGoldChanged.AddDynamic(this, &UCityUI::OnGoldChanged);

	// 모든 WorldTileActor의 OnGeneralTileClicked 델리게이트 바인딩
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWorldTileActor::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (AWorldTileActor* TileActor = Cast<AWorldTileActor>(Actor))
		{
			TileActor->OnGeneralTileClicked.AddDynamic(this, &UCityUI::OnPurchaseTileClickedHandler);
		}
	}
}

void UCityUI::UnbindFromPurchaseDelegates()
{
	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0);
	if (PlayerState)
	{
		// 골드 변경 델리게이트 바인딩 해제
		PlayerState->OnGoldChanged.RemoveDynamic(this, &UCityUI::OnGoldChanged);
	}

	// 모든 WorldTileActor의 OnGeneralTileClicked 델리게이트 바인딩 해제
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWorldTileActor::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (AWorldTileActor* TileActor = Cast<AWorldTileActor>(Actor))
		{
			TileActor->OnGeneralTileClicked.RemoveDynamic(this, &UCityUI::OnPurchaseTileClickedHandler);
		}
	}
}

void UCityUI::OnFacilityChanged(FVector2D TileCoordinate)
{
	// 시설 변경 시 도시 데이터 업데이트 (시설로 인한 타일 생산량 변화 반영)
	UpdateCityData();
}

