// Fill out your copyright notice in the Description page of Project Settings.

#include "MouseUI.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "../../World/WorldStruct.h"
#include "UObject/EnumProperty.h"

UMouseUI::UMouseUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UMouseUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 모든 보더를 초기에는 숨김으로 설정
	if (TileInfoBrd)
	{
		TileInfoBrd->SetVisibility(ESlateVisibility::Hidden);
	}

	if (TechInfoBrd)
	{
		TechInfoBrd->SetVisibility(ESlateVisibility::Hidden);
	}

	if (BuildingInfoBrd)
	{
		BuildingInfoBrd->SetVisibility(ESlateVisibility::Hidden);
	}

	if (UnitInfoBrd)
	{
		UnitInfoBrd->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMouseUI::ShowTileInfo(UWorldTile* Tile)
{
	if (!Tile || !TileInfoBrd)
	{
		return;
	}
	
	// 다른 보더들은 숨김
	if (TechInfoBrd)
	{
		TechInfoBrd->SetVisibility(ESlateVisibility::Hidden);
	}
	if (BuildingInfoBrd)
	{
		BuildingInfoBrd->SetVisibility(ESlateVisibility::Hidden);
	}
	if (UnitInfoBrd)
	{
		UnitInfoBrd->SetVisibility(ESlateVisibility::Hidden);
	}
	
	// 타일 정보 보더 표시
	TileInfoBrd->SetVisibility(ESlateVisibility::Visible);
	
	// 타일 정보 텍스트 업데이트
	if (TileInfoTxt)
	{
		FString TileInfoString = GenerateTileInfoString(Tile);
		TileInfoTxt->SetText(FText::FromString(TileInfoString));
	}
}

void UMouseUI::HideTileInfo()
{
	if (TileInfoBrd)
	{
		TileInfoBrd->SetVisibility(ESlateVisibility::Hidden);
	}
}

FString UMouseUI::GenerateTileInfoString(UWorldTile* Tile)
{
	if (!Tile)
	{
		return FString(TEXT("No Tile Data"));
	}

	FString InfoString;

	// 그리드 위치
	FVector2D GridPos = Tile->GetGridPosition();
	InfoString += FString::Printf(TEXT("Position: (%d, %d)\n"), (int32)GridPos.X, (int32)GridPos.Y);

	// 지형 타입
	ETerrainType TerrainType = Tile->GetTerrainType();
	FString TerrainString = UEnum::GetValueAsString(TerrainType);
	FString TerrainName = TerrainString.RightChop(TerrainString.Find(TEXT("::")) + 2);
	InfoString += FString::Printf(TEXT("Terrain: %s\n"), *TerrainName);

	// 땅 타입 (땅인 경우에만)
	if (TerrainType == ETerrainType::Land)
	{
		ELandType LandType = Tile->GetLandType();
		FString LandString = UEnum::GetValueAsString(LandType);
		FString LandName = LandString.RightChop(LandString.Find(TEXT("::")) + 2);
		InfoString += FString::Printf(TEXT("Land: %s\n"), *LandName);

		// 기후 타입
		EClimateType ClimateType = Tile->GetClimateType();
		FString ClimateString = UEnum::GetValueAsString(ClimateType);
		FString ClimateName = ClimateString.RightChop(ClimateString.Find(TEXT("::")) + 2);
		InfoString += FString::Printf(TEXT("Climate: %s\n"), *ClimateName);

		// 숲 여부
		if (Tile->HasForest())
		{
			InfoString += TEXT("Forest: Yes\n");
		}
	}

	// 자원 정보
	EBonusResource BonusResource = Tile->GetBonusResource();
	if (BonusResource != EBonusResource::None)
	{
		FString ResourceString = UEnum::GetValueAsString(BonusResource);
		FString ResourceName = ResourceString.RightChop(ResourceString.Find(TEXT("::")) + 2);
		InfoString += FString::Printf(TEXT("Bonus: %s\n"), *ResourceName);
	}

	EStrategicResource StrategicResource = Tile->GetStrategicResource();
	if (StrategicResource != EStrategicResource::None)
	{
		FString ResourceString = UEnum::GetValueAsString(StrategicResource);
		FString ResourceName = ResourceString.RightChop(ResourceString.Find(TEXT("::")) + 2);
		InfoString += FString::Printf(TEXT("Strategic: %s\n"), *ResourceName);
	}

	ELuxuryResource LuxuryResource = Tile->GetLuxuryResource();
	if (LuxuryResource != ELuxuryResource::None)
	{
		FString ResourceString = UEnum::GetValueAsString(LuxuryResource);
		FString ResourceName = ResourceString.RightChop(ResourceString.Find(TEXT("::")) + 2);
		InfoString += FString::Printf(TEXT("Luxury: %s\n"), *ResourceName);
	}

	// 생산량 정보
	int32 Food = Tile->GetTotalFoodYield();
	int32 Production = Tile->GetTotalProductionYield();
	int32 Gold = Tile->GetTotalGoldYield();
	int32 Science = Tile->GetTotalScienceYield();
	int32 Faith = Tile->GetTotalFaithYield();

	InfoString += FString::Printf(TEXT("\nYields:\n"));
	InfoString += FString::Printf(TEXT("Food: %d\n"), Food);
	InfoString += FString::Printf(TEXT("Production: %d\n"), Production);
	if (Gold > 0) InfoString += FString::Printf(TEXT("Gold: %d\n"), Gold);
	if (Science > 0) InfoString += FString::Printf(TEXT("Science: %d\n"), Science);
	if (Faith > 0) InfoString += FString::Printf(TEXT("Faith: %d\n"), Faith);

	// 소유 정보
	if (Tile->IsOwned())
	{
		InfoString += FString::Printf(TEXT("\nOwned by Player: %d"), Tile->GetOwnerPlayerID());
	}

	return InfoString;
}
