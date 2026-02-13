// Fill out your copyright notice in the Description page of Project Settings.

#include "CityProductionUI.h"
#include "Components/VerticalBox.h"
#include "../../SuperPlayerState.h"
#include "../../City/CityComponent.h"
#include "../../City/CityStruct.h"
#include "../../Status/UnitStatusStruct.h"
#include "../../World/WorldComponent.h"
#include "../../SuperGameInstance.h"
#include "../../Facility/FacilityManager.h"
#include "CityProductionSlotUI.h"
#include "Engine/DataTable.h"

UCityProductionUI::UCityProductionUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CachedPlayerState = nullptr;
	CachedCityComponent = nullptr;
}

void UCityProductionUI::NativeConstruct()
{
	Super::NativeConstruct();
}

void UCityProductionUI::SetupProductionUI(ASuperPlayerState* PlayerState)
{
	if (!PlayerState || !BuildingVB || !UnitVB)
	{
		return;
	}

	CachedPlayerState = PlayerState;

	// CityComponent 가져오기
	CachedCityComponent = PlayerState->GetCityComponent();
	if (!CachedCityComponent)
	{
		return;
	}

	// 건설 가능 목록 강제 갱신 (데이터 테이블 로드 및 목록 업데이트)
	CachedCityComponent->UpdateAvailableProductions();

	// 델리게이트 바인딩
	BindToAvailableProductionsUpdated();
	BindToFacilityDelegates();

	// 초기 슬롯 생성
	TArray<FName> AvailableBuildings = CachedCityComponent->GetAvailableBuildings();
	TArray<FName> AvailableUnits = CachedCityComponent->GetAvailableUnits();
	CreateBuildingSlots(AvailableBuildings);
	CreateUnitSlots(AvailableUnits);
}

void UCityProductionUI::BindToAvailableProductionsUpdated()
{
	if (!CachedCityComponent)
	{
		return;
	}

	// 기존 바인딩 해제
	CachedCityComponent->OnAvailableProductionsUpdated.RemoveDynamic(this, &UCityProductionUI::OnAvailableProductionsUpdated);

	// 새로운 바인딩
	CachedCityComponent->OnAvailableProductionsUpdated.AddDynamic(this, &UCityProductionUI::OnAvailableProductionsUpdated);
}

void UCityProductionUI::OnAvailableProductionsUpdated(TArray<FName> AvailableBuildings, TArray<FName> AvailableUnits)
{
	if (!BuildingVB || !UnitVB)
	{
		return;
	}

	// 기존 슬롯 제거
	ClearAllSlots();

	// 새로운 슬롯 생성
	CreateBuildingSlots(AvailableBuildings);
	CreateUnitSlots(AvailableUnits);
}

void UCityProductionUI::CreateBuildingSlots(const TArray<FName>& BuildingNames)
{
	if (!BuildingVB || !CachedCityComponent)
	{
		return;
	}

	// CityProductionSlotUI 클래스 로드
	UClass* SlotClass = LoadClass<UCityProductionSlotUI>(nullptr, TEXT("/Game/Civilization/Widget/CityWidget/W_CityProductionSlot.W_CityProductionSlot_C"));
	if (!SlotClass)
	{
		return;
	}

	// 각 건물에 대해 슬롯 생성
	for (const FName& BuildingName : BuildingNames)
	{
		if (BuildingName == NAME_None)
		{
			continue;
		}

		// 건물 데이터 가져오기
		FBuildingData BuildingData = CachedCityComponent->GetBuildingDataFromTable(BuildingName);
		if (BuildingData.BuildingType == EBuildingType::None)
		{
			continue;
		}

		// 위젯 생성
		UCityProductionSlotUI* ProductionSlot = CreateWidget<UCityProductionSlotUI>(this, SlotClass);
		if (ProductionSlot)
		{
			// ProductionID 설정
			ProductionSlot->ProductionID = BuildingName;

			// 이미지 및 텍스트 설정
			UTexture2D* BuildingIcon = nullptr;
			if (!BuildingData.BuildingIcon.IsNull())
			{
				BuildingIcon = BuildingData.BuildingIcon.LoadSynchronous();
			}
			ProductionSlot->SetProductionImage(BuildingIcon);
			ProductionSlot->SetProductionItemText(BuildingData.BuildingName);

			// 턴 수 계산 및 설정 (건물은 생산력 사용)
			int32 TurnProduction = 0;
			if (CachedPlayerState)
			{
				if (UWorld* World = GetWorld())
				{
					if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
					{
						if (UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent())
						{
							TurnProduction = CachedPlayerState->CalculateTotalProductionYield(WorldComponent);
						}
					}
				}
			}
			
			int32 Turns = 0;
			if (TurnProduction > 0 && BuildingData.ProductionCost > 0)
			{
				Turns = FMath::CeilToInt((float)BuildingData.ProductionCost / (float)TurnProduction);
			}
			else if (TurnProduction <= 0)
			{
				Turns = 99;
			}
			ProductionSlot->SetTurnText(Turns);

			// VerticalBox에 추가
			BuildingVB->AddChild(ProductionSlot);

			// 슬롯 클릭 델리게이트 바인딩
			ProductionSlot->OnProductionSlotClicked.AddDynamic(this, &UCityProductionUI::OnProductionSlotClicked);

			// 위젯 가시성 설정
			ProductionSlot->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UCityProductionUI::CreateUnitSlots(const TArray<FName>& UnitNames)
{
	if (!UnitVB || !CachedCityComponent)
	{
		return;
	}

	// CityProductionSlotUI 클래스 로드
	UClass* SlotClass = LoadClass<UCityProductionSlotUI>(nullptr, TEXT("/Game/Civilization/Widget/CityWidget/W_CityProductionSlot.W_CityProductionSlot_C"));
	if (!SlotClass)
	{
		return;
	}

	// 각 유닛에 대해 슬롯 생성
	for (const FName& UnitName : UnitNames)
	{
		if (UnitName == NAME_None)
		{
			continue;
		}

		// 유닛 데이터 가져오기
		FUnitBaseStat UnitData = CachedCityComponent->GetUnitDataFromTable(UnitName);
		if (UnitData.UnitClass == EUnitClass::None)
		{
			continue;
		}

		// 위젯 생성
		UCityProductionSlotUI* ProductionSlot = CreateWidget<UCityProductionSlotUI>(this, SlotClass);
		if (ProductionSlot)
		{
			// ProductionID 설정
			ProductionSlot->ProductionID = UnitName;

			// 이미지 및 텍스트 설정
			UTexture2D* UnitIcon = nullptr;
			if (!UnitData.UnitIcon.IsNull())
			{
				UnitIcon = UnitData.UnitIcon.LoadSynchronous();
			}
			ProductionSlot->SetProductionImage(UnitIcon);
			ProductionSlot->SetProductionItemText(UnitData.UnitName.ToString());

			// 턴 수 계산 및 설정 (유닛은 식량 사용)
			int32 TurnFood = 0;
			if (CachedPlayerState)
			{
				if (UWorld* World = GetWorld())
				{
					if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
					{
						if (UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent())
						{
							TurnFood = CachedPlayerState->CalculateTotalFoodYield(WorldComponent);
						}
					}
				}
			}
			
			int32 Turns = 0;
			if (TurnFood > 0 && UnitData.FoodCost > 0)
			{
				Turns = FMath::CeilToInt((float)UnitData.FoodCost / (float)TurnFood);
			}
			else if (TurnFood <= 0)
			{
				Turns = 99;
			}
			ProductionSlot->SetTurnText(Turns);

			// VerticalBox에 추가
			UnitVB->AddChild(ProductionSlot);

			// 슬롯 클릭 델리게이트 바인딩
			ProductionSlot->OnProductionSlotClicked.AddDynamic(this, &UCityProductionUI::OnProductionSlotClicked);

			// 위젯 가시성 설정
			ProductionSlot->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UCityProductionUI::ClearAllSlots()
{
	if (BuildingVB)
	{
		BuildingVB->ClearChildren();
	}

	if (UnitVB)
	{
		UnitVB->ClearChildren();
	}
}

void UCityProductionUI::OnProductionSlotClicked(FName ProductionID)
{
	if (!CachedPlayerState || !CachedCityComponent || ProductionID == NAME_None)
	{
		return;
	}

	// BuildingVB에 있는지 확인 (건물인지 유닛인지 판단)
	bool bIsBuilding = false;
	if (BuildingVB)
	{
		for (int32 i = 0; i < BuildingVB->GetChildrenCount(); ++i)
		{
			UCityProductionSlotUI* ProductionSlot = Cast<UCityProductionSlotUI>(BuildingVB->GetChildAt(i));
			if (ProductionSlot && ProductionSlot->ProductionID == ProductionID)
			{
				bIsBuilding = true;
				break;
			}
		}
	}

	// 건물이면 건물 생산 시작, 유닛이면 유닛 생산 시작
	if (bIsBuilding)
	{
		CachedPlayerState->StartBuildingProduction(ProductionID);
	}
	else
	{
		CachedPlayerState->StartUnitProduction(ProductionID);
	}
}

void UCityProductionUI::OnFacilityChanged(FVector2D TileCoordinate)
{
	// 시설 변경 시 모든 슬롯의 TurnTxt 갱신 (시설로 인한 타일 생산량 변화 반영)
	UpdateAllSlotsTurnText();
}

void UCityProductionUI::BindToFacilityDelegates()
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

	// 기존 바인딩 해제
	UnbindFromFacilityDelegates();

	// 시설 변경 델리게이트 바인딩
	FacilityManager->OnFacilityChanged.AddDynamic(this, &UCityProductionUI::OnFacilityChanged);
}

void UCityProductionUI::UnbindFromFacilityDelegates()
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
		FacilityManager->OnFacilityChanged.RemoveDynamic(this, &UCityProductionUI::OnFacilityChanged);
	}
}

void UCityProductionUI::UpdateAllSlotsTurnText()
{
	if (!CachedPlayerState || !CachedCityComponent)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent();
	if (!WorldComponent)
	{
		return;
	}

	// 건물 슬롯의 TurnTxt 갱신
	if (BuildingVB)
	{
		for (int32 i = 0; i < BuildingVB->GetChildrenCount(); ++i)
		{
			UCityProductionSlotUI* ProductionSlot = Cast<UCityProductionSlotUI>(BuildingVB->GetChildAt(i));
			if (ProductionSlot && ProductionSlot->ProductionID != NAME_None)
			{
				// 건물 데이터 가져오기
				FBuildingData BuildingData = CachedCityComponent->GetBuildingDataFromTable(ProductionSlot->ProductionID);
				if (BuildingData.BuildingType != EBuildingType::None)
				{
					// 턴 수 계산 및 설정 (건물은 생산력 사용)
					int32 TurnProduction = CachedPlayerState->CalculateTotalProductionYield(WorldComponent);
					
					int32 Turns = 0;
					if (TurnProduction > 0 && BuildingData.ProductionCost > 0)
					{
						Turns = FMath::CeilToInt((float)BuildingData.ProductionCost / (float)TurnProduction);
					}
					else if (TurnProduction <= 0)
					{
						Turns = 99;
					}
					ProductionSlot->SetTurnText(Turns);
				}
			}
		}
	}

	// 유닛 슬롯의 TurnTxt 갱신
	if (UnitVB)
	{
		for (int32 i = 0; i < UnitVB->GetChildrenCount(); ++i)
		{
			UCityProductionSlotUI* ProductionSlot = Cast<UCityProductionSlotUI>(UnitVB->GetChildAt(i));
			if (ProductionSlot && ProductionSlot->ProductionID != NAME_None)
			{
				// 유닛 데이터 가져오기
				FUnitBaseStat UnitData = CachedCityComponent->GetUnitDataFromTable(ProductionSlot->ProductionID);
				if (UnitData.UnitClass != EUnitClass::None)
				{
					// 턴 수 계산 및 설정 (유닛은 식량 사용)
					int32 TurnFood = CachedPlayerState->CalculateTotalFoodYield(WorldComponent);
					
					int32 Turns = 0;
					if (TurnFood > 0 && UnitData.FoodCost > 0)
					{
						Turns = FMath::CeilToInt((float)UnitData.FoodCost / (float)TurnFood);
					}
					else if (TurnFood <= 0)
					{
						Turns = 99;
					}
					ProductionSlot->SetTurnText(Turns);
				}
			}
		}
	}
}

