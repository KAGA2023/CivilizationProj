#include "BuildFacilityUI.h"
#include "../../SuperGameInstance.h"
#include "../../SuperPlayerState.h"
#include "../../Facility/FacilityManager.h"
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
	if (BuildCampBtn)
	{
		BuildCampBtn->OnClicked.AddDynamic(this, &UBuildFacilityUI::OnClickedBuildCamp);
	}
	if (BuildMineBtn)
	{
		BuildMineBtn->OnClicked.AddDynamic(this, &UBuildFacilityUI::OnClickedBuildMine);
	}
	if (DestroyFacilityBtn)
	{
		DestroyFacilityBtn->OnClicked.AddDynamic(this, &UBuildFacilityUI::OnClickedDestroyFacility);
	}
	if (BuildQuarryBtn)
	{
		BuildQuarryBtn->OnClicked.AddDynamic(this, &UBuildFacilityUI::OnClickedBuildQuarry);
	}
	if (BuildPlantationBtn)
	{
		BuildPlantationBtn->OnClicked.AddDynamic(this, &UBuildFacilityUI::OnClickedBuildPlantation);
	}
	if (BuildLumberMillBtn)
	{
		BuildLumberMillBtn->OnClicked.AddDynamic(this, &UBuildFacilityUI::OnClickedBuildLumberMill);
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

void UBuildFacilityUI::OnClickedBuildCamp()
{
	BuildFacilityByRowName(TEXT("Camp"));
}

void UBuildFacilityUI::OnClickedBuildMine()
{
	BuildFacilityByRowName(TEXT("Mine"));
}

void UBuildFacilityUI::OnClickedBuildQuarry()
{
	BuildFacilityByRowName(TEXT("Quarry"));
}

void UBuildFacilityUI::OnClickedBuildPlantation()
{
	BuildFacilityByRowName(TEXT("Plantation"));
}

void UBuildFacilityUI::OnClickedBuildLumberMill()
{
	BuildFacilityByRowName(TEXT("LumberMill"));
}

void UBuildFacilityUI::OnClickedDestroyFacility()
{
	if (!CachedFacilityManager || !CachedWorldComponent)
	{
		// 컨텍스트가 설정되지 않았으면 아무 것도 하지 않음
		return;
	}

	// 현재 타일의 시설 제거
	if (CachedTile)
	{
		const FVector2D Hex = CachedTile->GetGridPosition();
		CachedFacilityManager->DestroyFacility(Hex, CachedWorldComponent);
	}
}

void UBuildFacilityUI::BuildFacilityByRowName(const FName& RowName)
{
	if (!CachedPlayerState || !CachedTile || RowName.IsNone())
	{
		return;
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
		if (BuildCampBtn)        BuildCampBtn->SetIsEnabled(false);
		if (BuildMineBtn)        BuildMineBtn->SetIsEnabled(false);
		if (BuildQuarryBtn)      BuildQuarryBtn->SetIsEnabled(false);
		if (BuildPlantationBtn)  BuildPlantationBtn->SetIsEnabled(false);
		if (BuildLumberMillBtn)  BuildLumberMillBtn->SetIsEnabled(false);
		if (DestroyFacilityBtn)  DestroyFacilityBtn->SetIsEnabled(false);
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

	if (BuildCampBtn)
	{
		bool bEnable = false;
		if (AvailableFacilities.Contains(TEXT("Camp")))
		{
			bEnable = CachedFacilityManager->CanBuildFacilityOnTile(TEXT("Camp"), CachedTile);
		}
		BuildCampBtn->SetIsEnabled(bEnable);
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

	if (BuildQuarryBtn)
	{
		bool bEnable = false;
		if (AvailableFacilities.Contains(TEXT("Quarry")))
		{
			bEnable = CachedFacilityManager->CanBuildFacilityOnTile(TEXT("Quarry"), CachedTile);
		}
		BuildQuarryBtn->SetIsEnabled(bEnable);
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
}


