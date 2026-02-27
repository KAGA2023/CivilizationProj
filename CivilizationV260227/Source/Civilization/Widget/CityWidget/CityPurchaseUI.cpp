// Fill out your copyright notice in the Description page of Project Settings.

#include "CityPurchaseUI.h"
#include "Components/VerticalBox.h"
#include "../../SuperPlayerState.h"
#include "../../City/CityComponent.h"
#include "../../City/CityStruct.h"
#include "../../Status/UnitStatusStruct.h"
#include "../../World/WorldComponent.h"
#include "../../World/WorldStruct.h"
#include "../../SuperGameInstance.h"
#include "CityPurchaseSlotUI.h"
#include "Engine/DataTable.h"

UCityPurchaseUI::UCityPurchaseUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CachedPlayerState = nullptr;
	CachedCityComponent = nullptr;
}

void UCityPurchaseUI::NativeConstruct()
{
	Super::NativeConstruct();
}

void UCityPurchaseUI::SetupPurchaseUI(ASuperPlayerState* PlayerState)
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

	// 초기 슬롯 생성
	TArray<FName> AvailableBuildings = CachedCityComponent->GetAvailableBuildings();
	TArray<FName> AvailableUnits = CachedCityComponent->GetAvailableUnits();

	CreateBuildingSlots(AvailableBuildings);
	CreateUnitSlots(AvailableUnits);
}

void UCityPurchaseUI::BindToAvailableProductionsUpdated()
{
	if (!CachedCityComponent)
	{
		return;
	}

	// 기존 바인딩 해제
	CachedCityComponent->OnAvailableProductionsUpdated.RemoveDynamic(this, &UCityPurchaseUI::OnAvailableProductionsUpdated);

	// 새로운 바인딩
	CachedCityComponent->OnAvailableProductionsUpdated.AddDynamic(this, &UCityPurchaseUI::OnAvailableProductionsUpdated);
}

void UCityPurchaseUI::OnAvailableProductionsUpdated(TArray<FName> AvailableBuildings, TArray<FName> AvailableUnits)
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

void UCityPurchaseUI::CreateBuildingSlots(const TArray<FName>& BuildingNames)
{
	if (!BuildingVB || !CachedCityComponent)
	{
		return;
	}

	// CityPurchaseSlotUI 클래스 로드
	UClass* SlotClass = LoadClass<UCityPurchaseSlotUI>(nullptr, TEXT("/Game/Civilization/Widget/CityWidget/W_CityPurchaseSlot.W_CityPurchaseSlot_C"));
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
		UCityPurchaseSlotUI* PurchaseSlot = CreateWidget<UCityPurchaseSlotUI>(this, SlotClass);
		if (PurchaseSlot)
		{
			// PurchaseID 설정
			PurchaseSlot->PurchaseID = BuildingName;

			// 이미지 및 텍스트 설정
			UTexture2D* BuildingIcon = nullptr;
			if (!BuildingData.BuildingIcon.IsNull())
			{
				BuildingIcon = BuildingData.BuildingIcon.LoadSynchronous();
			}
			PurchaseSlot->SetPurchaseImage(BuildingIcon);
			PurchaseSlot->SetPurchaseItemText(BuildingData.BuildingName);

			// 골드 비용 설정
			PurchaseSlot->SetGoldCostText(BuildingData.GoldCost);

			// VerticalBox에 추가
			BuildingVB->AddChild(PurchaseSlot);

			// 슬롯 클릭 델리게이트 바인딩
			PurchaseSlot->OnPurchaseSlotClicked.AddDynamic(this, &UCityPurchaseUI::OnPurchaseSlotClicked);

			// 위젯 가시성 설정
			PurchaseSlot->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UCityPurchaseUI::CreateUnitSlots(const TArray<FName>& UnitNames)
{
	if (!UnitVB || !CachedCityComponent)
	{
		return;
	}

	// CityPurchaseSlotUI 클래스 로드
	UClass* SlotClass = LoadClass<UCityPurchaseSlotUI>(nullptr, TEXT("/Game/Civilization/Widget/CityWidget/W_CityPurchaseSlot.W_CityPurchaseSlot_C"));
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
		UCityPurchaseSlotUI* PurchaseSlot = CreateWidget<UCityPurchaseSlotUI>(this, SlotClass);
		if (PurchaseSlot)
		{
			// PurchaseID 설정
			PurchaseSlot->PurchaseID = UnitName;

			// 이미지 및 텍스트 설정
			UTexture2D* UnitIcon = nullptr;
			if (!UnitData.UnitIcon.IsNull())
			{
				UnitIcon = UnitData.UnitIcon.LoadSynchronous();
			}
			PurchaseSlot->SetPurchaseImage(UnitIcon);
			PurchaseSlot->SetPurchaseItemText(UnitData.UnitName.ToString());

			// 전략자원 필요 시 해당 UI 표시, 아니면 골드 비용만 표시
			if (UnitData.RequiredResources.Num() > 0 && UnitData.RequiredResourceAmounts.Num() > 0)
			{
				EStrategicResource Resource = UnitData.RequiredResources[0];
				int32 RequiredAmount = UnitData.RequiredResourceAmounts[0];
				UTexture2D* ResourceIcon = nullptr;
				if (UWorld* World = GetWorld())
				{
					if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
					{
						if (UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent())
						{
							UDataTable* ResourceDT = WorldComponent->GetStrategicResourceDataTable();
							if (ResourceDT)
							{
								FString EnumStr = UEnum::GetValueAsString(Resource);
								FName RowName = FName(*EnumStr.RightChop(EnumStr.Find(TEXT("::")) + 2));
								if (FStrategicResourceData* Row = ResourceDT->FindRow<FStrategicResourceData>(RowName, TEXT("StrategicResourceIcon")))
								{
									if (!Row->ResourceIcon.IsNull())
									{
										ResourceIcon = Row->ResourceIcon.LoadSynchronous();
									}
								}
							}
						}
					}
				}
				PurchaseSlot->SetStrategicResourceDisplay(UnitData.GoldCost, RequiredAmount, ResourceIcon);
			}
			else
			{
				PurchaseSlot->SetGoldCostText(UnitData.GoldCost);
			}

			// VerticalBox에 추가
			UnitVB->AddChild(PurchaseSlot);

			// 슬롯 클릭 델리게이트 바인딩
			PurchaseSlot->OnPurchaseSlotClicked.AddDynamic(this, &UCityPurchaseUI::OnPurchaseSlotClicked);

			// 위젯 가시성 설정
			PurchaseSlot->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UCityPurchaseUI::ClearAllSlots()
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

void UCityPurchaseUI::OnPurchaseSlotClicked(FName PurchaseID)
{
	if (!CachedPlayerState || !CachedCityComponent || PurchaseID == NAME_None)
	{
		return;
	}

	// BuildingVB에 있는지 확인 (건물인지 유닛인지 판단)
	bool bIsBuilding = false;
	if (BuildingVB)
	{
		for (int32 i = 0; i < BuildingVB->GetChildrenCount(); ++i)
		{
			UCityPurchaseSlotUI* PurchaseSlot = Cast<UCityPurchaseSlotUI>(BuildingVB->GetChildAt(i));
			if (PurchaseSlot && PurchaseSlot->PurchaseID == PurchaseID)
			{
				bIsBuilding = true;
				break;
			}
		}
	}

	// 건물이면 건물 구매, 유닛이면 유닛 구매
	if (bIsBuilding)
	{
		CachedPlayerState->PurchaseBuildingWithGold(PurchaseID);
	}
	else
	{
		CachedPlayerState->PurchaseUnitWithGold(PurchaseID);
	}
}

