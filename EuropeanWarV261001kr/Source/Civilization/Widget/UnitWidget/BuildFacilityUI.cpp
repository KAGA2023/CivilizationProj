#include "BuildFacilityUI.h"
#include "../../SuperGameInstance.h"
#include "../../SuperPlayerState.h"
#include "../../Facility/FacilityManager.h"
#include "../../Unit/UnitManager.h"
#include "../../World/WorldComponent.h"
#include "../../World/WorldStruct.h"

void UBuildFacilityUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (BuildFarmBtn)
	{
		BuildFarmBtn->OnClicked.AddDynamic(this, &UBuildFacilityUI::OnClickedBuildFarm);
	}
	if (BuildPastureBtn)
	{
		BuildPastureBtn->OnClicked.AddDynamic(this, &UBuildFacilityUI::OnClickedBuildPasture);
	}
	if (BuildMineBtn)
	{
		BuildMineBtn->OnClicked.AddDynamic(this, &UBuildFacilityUI::OnClickedBuildMine);
	}
	if (DestroyFacilityBtn)
	{
		DestroyFacilityBtn->OnClicked.AddDynamic(this, &UBuildFacilityUI::OnClickedDestroyFacility);
	}
	if (BuildPlantationBtn)
	{
		BuildPlantationBtn->OnClicked.AddDynamic(this, &UBuildFacilityUI::OnClickedBuildPlantation);
	}
	if (BuildLumberMillBtn)
	{
		BuildLumberMillBtn->OnClicked.AddDynamic(this, &UBuildFacilityUI::OnClickedBuildLumberMill);
	}
	if (BuildMarketBtn)
	{
		BuildMarketBtn->OnClicked.AddDynamic(this, &UBuildFacilityUI::OnClickedBuildMarket);
	}
	if (BuildSchoolBtn)
	{
		BuildSchoolBtn->OnClicked.AddDynamic(this, &UBuildFacilityUI::OnClickedBuildSchool);
	}
	if (BuildVillageBtn)
	{
		BuildVillageBtn->OnClicked.AddDynamic(this, &UBuildFacilityUI::OnClickedBuildVillage);
	}
	if (RepairFacilityBtn)
	{
		RepairFacilityBtn->OnClicked.AddDynamic(this, &UBuildFacilityUI::OnClickedRepairFacility);
	}
}

void UBuildFacilityUI::SetupForTile(UWorldTile* InTile)
{
	CachedTile = InTile;
	CachedHex = InTile ? InTile->GetGridPosition() : FVector2D::ZeroVector;

	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			CachedPlayerState = SuperGameInstance->GetPlayerState(0);
			CachedFacilityManager = SuperGameInstance->GetFacilityManager();
			CachedWorldComponent = SuperGameInstance->GetGeneratedWorldComponent();
			CachedUnitManager = SuperGameInstance->GetUnitManager();

			// FacilityManager의 OnFacilityChanged 델리게이트 구독
			if (CachedFacilityManager)
			{
				// 중복 바인딩 방지: 기존 바인딩 제거 후 다시 바인딩
				CachedFacilityManager->OnFacilityChanged.RemoveDynamic(this, &UBuildFacilityUI::OnFacilityChanged);
				CachedFacilityManager->OnFacilityChanged.AddDynamic(this, &UBuildFacilityUI::OnFacilityChanged);
			}
		}
	}

	// 버튼 활성/비활성 갱신
	if (CachedPlayerState)
	{
		const TArray<FName> AvailableFacilities = CachedPlayerState->GetAvailableFacilities();
		UpdateButtonStates(AvailableFacilities);
	}
	else
	{
		// 플레이어 스테이트가 없으면 전부 비활성화
		UpdateButtonStates(TArray<FName>());
	}
}

void UBuildFacilityUI::OnClickedBuildFarm()
{
	BuildFacilityByRowName(TEXT("Farm"));
}

void UBuildFacilityUI::OnClickedBuildPasture()
{
	BuildFacilityByRowName(TEXT("Pasture"));
}

void UBuildFacilityUI::OnClickedBuildMine()
{
	BuildFacilityByRowName(TEXT("Mine"));
}

void UBuildFacilityUI::OnClickedBuildPlantation()
{
	BuildFacilityByRowName(TEXT("Plantation"));
}

void UBuildFacilityUI::OnClickedBuildMarket()
{
	BuildFacilityByRowName(TEXT("Market"));
}

void UBuildFacilityUI::OnClickedBuildSchool()
{
	BuildFacilityByRowName(TEXT("School"));
}

void UBuildFacilityUI::OnClickedBuildVillage()
{
	BuildFacilityByRowName(TEXT("Village"));
}

void UBuildFacilityUI::OnClickedBuildLumberMill()
{
	BuildFacilityByRowName(TEXT("LumberMill"));
}

void UBuildFacilityUI::OnClickedRepairFacility()
{
	if (!CachedUnitManager)
	{
		return;
	}
	CachedUnitManager->ClearAllTileBrightness();
	CachedUnitManager->ClearSelectedUnit();
	CachedUnitManager->RequestBuilderRepairFacility(CachedHex);
	// 실제 수리는 몽타주 종료 후 실행되며, OnFacilityChanged에서 버튼 상태 갱신
}

void UBuildFacilityUI::OnClickedDestroyFacility()
{
	if (!CachedUnitManager || !CachedTile)
	{
		return;
	}
	CachedUnitManager->ClearAllTileBrightness();
	CachedUnitManager->ClearSelectedUnit();
	const FVector2D Hex = CachedTile->GetGridPosition();
	CachedUnitManager->RequestBuilderDestroyFacility(Hex);
	// 실제 제거는 몽타주 종료 후 실행되며, OnFacilityChanged에서 버튼 상태 갱신
}

void UBuildFacilityUI::BuildFacilityByRowName(const FName& RowName)
{
	if (!CachedPlayerState || !CachedTile || RowName.IsNone())
	{
		return;
	}
	if (CachedUnitManager)
	{
		CachedUnitManager->ClearAllTileBrightness();
		CachedUnitManager->ClearSelectedUnit();
	}
	const FVector2D Hex = CachedTile->GetGridPosition();
	CachedPlayerState->BuildFacility(RowName, Hex);
}

void UBuildFacilityUI::UpdateButtonStates(const TArray<FName>& AvailableFacilities)
{
	// 컨텍스트가 없으면 모든 버튼 비활성화
	if (!CachedTile || !CachedFacilityManager || !CachedPlayerState)
	{
		if (BuildFarmBtn)        BuildFarmBtn->SetIsEnabled(false);
		if (BuildPastureBtn)     BuildPastureBtn->SetIsEnabled(false);
		if (BuildMineBtn)        BuildMineBtn->SetIsEnabled(false);
		if (BuildPlantationBtn)  BuildPlantationBtn->SetIsEnabled(false);
		if (BuildLumberMillBtn)  BuildLumberMillBtn->SetIsEnabled(false);
		if (BuildMarketBtn)      BuildMarketBtn->SetIsEnabled(false);
		if (BuildSchoolBtn)      BuildSchoolBtn->SetIsEnabled(false);
		if (BuildVillageBtn)     BuildVillageBtn->SetIsEnabled(false);
		if (DestroyFacilityBtn)  DestroyFacilityBtn->SetIsEnabled(false);
		if (RepairFacilityBtn)   RepairFacilityBtn->SetIsEnabled(false);
		return;
	}

	// 각 시설 버튼에 대해 조건을 그대로 if 문으로 작성
	if (BuildFarmBtn)
	{
		bool bEnable = false;
		if (AvailableFacilities.Contains(TEXT("Farm")))
		{
			bEnable = CachedFacilityManager->CanBuildFacilityOnTile(TEXT("Farm"), CachedTile);
		}
		BuildFarmBtn->SetIsEnabled(bEnable);
	}

	if (BuildPastureBtn)
	{
		bool bEnable = false;
		if (AvailableFacilities.Contains(TEXT("Pasture")))
		{
			bEnable = CachedFacilityManager->CanBuildFacilityOnTile(TEXT("Pasture"), CachedTile);
		}
		BuildPastureBtn->SetIsEnabled(bEnable);
	}

	if (BuildMineBtn)
	{
		bool bEnable = false;
		if (AvailableFacilities.Contains(TEXT("Mine")))
		{
			bEnable = CachedFacilityManager->CanBuildFacilityOnTile(TEXT("Mine"), CachedTile);
		}
		BuildMineBtn->SetIsEnabled(bEnable);
	}

	if (BuildPlantationBtn)
	{
		bool bEnable = false;
		if (AvailableFacilities.Contains(TEXT("Plantation")))
		{
			bEnable = CachedFacilityManager->CanBuildFacilityOnTile(TEXT("Plantation"), CachedTile);
		}
		BuildPlantationBtn->SetIsEnabled(bEnable);
	}

	if (BuildLumberMillBtn)
	{
		bool bEnable = false;
		if (AvailableFacilities.Contains(TEXT("LumberMill")))
		{
			bEnable = CachedFacilityManager->CanBuildFacilityOnTile(TEXT("LumberMill"), CachedTile);
		}
		BuildLumberMillBtn->SetIsEnabled(bEnable);
	}

	if (BuildMarketBtn)
	{
		bool bEnable = false;
		if (AvailableFacilities.Contains(TEXT("Market")))
		{
			bEnable = CachedFacilityManager->CanBuildFacilityOnTile(TEXT("Market"), CachedTile);
		}
		BuildMarketBtn->SetIsEnabled(bEnable);
	}

	if (BuildSchoolBtn)
	{
		bool bEnable = false;
		if (AvailableFacilities.Contains(TEXT("School")))
		{
			bEnable = CachedFacilityManager->CanBuildFacilityOnTile(TEXT("School"), CachedTile);
		}
		BuildSchoolBtn->SetIsEnabled(bEnable);
	}

	if (BuildVillageBtn)
	{
		bool bEnable = false;
		if (AvailableFacilities.Contains(TEXT("Village")))
		{
			bEnable = CachedFacilityManager->CanBuildFacilityOnTile(TEXT("Village"), CachedTile);
		}
		BuildVillageBtn->SetIsEnabled(bEnable);
	}

	// 파괴 버튼은 해당 타일에 시설이 있을 때만 활성화
	if (DestroyFacilityBtn)
	{
		bool bEnableDestroy = false;

		if (CachedFacilityManager && CachedWorldComponent && CachedTile)
		{
			const FVector2D Hex = CachedTile->GetGridPosition();
			bEnableDestroy = CachedFacilityManager->HasFacilityAtTile(Hex);
		}

		DestroyFacilityBtn->SetIsEnabled(bEnableDestroy);
	}

	// 수리 버튼: 자신 소유 + 시설 있음 + 약탈 상태일 때만 활성화 (플레이어 0 = 로컬 플레이어)
	if (RepairFacilityBtn)
	{
		const bool bCanRepair = CachedFacilityManager && CachedWorldComponent
			&& CachedFacilityManager->CanRepairFacilityAtTile(CachedHex, 0, CachedWorldComponent);
		RepairFacilityBtn->SetIsEnabled(bCanRepair);
	}
}

void UBuildFacilityUI::OnFacilityChanged(FVector2D TileCoordinate)
{
	// 현재 UI가 표시 중인 타일과 일치하는 경우에만 버튼 상태 업데이트
	if (CachedTile && CachedTile->GetGridPosition() == TileCoordinate)
	{
		if (CachedPlayerState)
		{
			const TArray<FName> AvailableFacilities = CachedPlayerState->GetAvailableFacilities();
			UpdateButtonStates(AvailableFacilities);
		}
		else
		{
			UpdateButtonStates(TArray<FName>());
		}
	}
}


