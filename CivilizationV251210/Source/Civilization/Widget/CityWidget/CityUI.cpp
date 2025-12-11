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
#include "../../City/CityComponent.h"
#include "../../City/CityStruct.h"
#include "../../Status/UnitStatusStruct.h"
#include "../../Facility/FacilityManager.h"

void UCityUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기 텍스트 설정
	if (ScienceTxt)
	{
		ScienceTxt->SetText(FText::FromString(TEXT("+0")));
	}

	if (FaithTxt)
	{
		FaithTxt->SetText(FText::FromString(TEXT("+0")));
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

	// 도시 컴포넌트 참조 가져오기 및 델리게이트 바인딩
	BindToProductionDelegates();
	BindToFacilityDelegates();
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

	// CalculateTotalFaithYield를 +x 형식으로 FaithTxt에 표시
	if (FaithTxt)
	{
		int32 FaithYield = PlayerState->CalculateTotalFaithYield(WorldComponent);
		FString FaithString = FString::Printf(TEXT("+%d"), FaithYield);
		FaithTxt->SetText(FText::FromString(FaithString));
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

	// 생산 중이 아니면 빈 값으로 설정
	if (CurrentStat.ProductionType == EProductionType::None || CurrentStat.ProductionName == NAME_None)
	{
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

void UCityUI::OnFacilityChanged(FVector2D TileCoordinate)
{
	// 시설 변경 시 도시 데이터 업데이트 (시설로 인한 타일 생산량 변화 반영)
	UpdateCityData();
}

