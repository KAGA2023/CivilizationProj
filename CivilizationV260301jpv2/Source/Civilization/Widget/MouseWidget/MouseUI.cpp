// Fill out your copyright notice in the Description page of Project Settings.

#include "MouseUI.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "../../World/WorldStruct.h"
#include "UObject/EnumProperty.h"
#include "../../SuperGameInstance.h"
#include "../../Facility/FacilityManager.h"
#include "../../SuperPlayerState.h"

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
	// 1행: WorldStruct의 GetFullTileName과 동일 기준 (기후/지형/숲 or 바다)
	if (Tile->GetTerrainType() == ETerrainType::Land)
	{
		InfoString += Tile->GetClimateTypeName();   // 초원 / 사막 / 설원
		InfoString += TEXT(" / ");
		InfoString += Tile->GetLandTypeName();     // 평지 / 언덕 / 산
		if (Tile->HasForest())
		{
			InfoString += TEXT(" / 森");
		}
	}
	else if (Tile->GetTerrainType() == ETerrainType::Ocean)
	{
		InfoString += TEXT("海洋");
	}
	else
	{
		InfoString += TEXT("Unknown");
	}
	InfoString += TEXT("\n");

	// 소유자: 국명 or 없음
	InfoString += TEXT("所有者: ");
	if (Tile->IsOwned())
	{
		if (UWorld* World = GetWorld())
		{
			if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
			{
				if (ASuperPlayerState* OwnerState = SuperGameInst->GetPlayerState(Tile->GetOwnerPlayerID()))
				{
					FString CountryName = OwnerState->GetCountryName();
					InfoString += CountryName.IsEmpty() ? FString::Printf(TEXT("プレイヤー%d"), Tile->GetOwnerPlayerID()) : CountryName;
				}
				else
				{
					InfoString += FString::Printf(TEXT("プレイヤー%d"), Tile->GetOwnerPlayerID());
				}
			}
			else
			{
				InfoString += FString::Printf(TEXT("プレイヤー%d"), Tile->GetOwnerPlayerID());
			}
		}
		else
		{
			InfoString += FString::Printf(TEXT("プレイヤー%d"), Tile->GetOwnerPlayerID());
		}
	}
	else
	{
		InfoString += TEXT("なし");
	}
	InfoString += TEXT("\n");

	// 자원 카테고리: <사치자원> / <보너스자원> / <전략자원> / <자원없음>
	EResourceCategory ResCat = Tile->GetResourceCategory();
	switch (ResCat)
	{
	case EResourceCategory::Luxury:    InfoString += TEXT("<奢侈資源>\n"); break;
	case EResourceCategory::Bonus:     InfoString += TEXT("<ボーナス資源>\n"); break;
	case EResourceCategory::Strategic: InfoString += TEXT("<戦略資源>\n"); break;
	default:                           InfoString += TEXT("<資源なし>\n"); break;
	}

	// 자원: WorldStruct의 GetResourceName() 기준 (표시명 or 없음)
	InfoString += TEXT("資源: ");
	FString ResourceNameStr = Tile->GetResourceName();
	if (ResourceNameStr.IsEmpty() || ResourceNameStr == TEXT("None"))
	{
		InfoString += TEXT("なし");
	}
	else
	{
		InfoString += ResourceNameStr;
	}
	InfoString += TEXT("\n");

	InfoString += TEXT("--------------------\n");

	// 시설: 이름 or 없음
	InfoString += TEXT("施設: ");
	FString FacilityNameStr;
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			if (UFacilityManager* FacilityManager = SuperGameInst->GetFacilityManager())
			{
				if (FacilityManager->HasFacilityAtTile(Tile->GetGridPosition()))
				{
					FFacilityData FacilityData = FacilityManager->GetFacilityAtTile(Tile->GetGridPosition());
					FacilityNameStr = FacilityData.FacilityName;
				}
			}
		}
	}
	if (FacilityNameStr.IsEmpty())
	{
		InfoString += TEXT("なし");
	}
	else
	{
		InfoString += FacilityNameStr;
	}
	InfoString += TEXT("\n");

	// 식량·생산력·골드·과학력
	int32 Food = Tile->GetTotalFoodYield();
	int32 Production = Tile->GetTotalProductionYield();
	int32 Gold = Tile->GetTotalGoldYield();
	int32 Science = Tile->GetTotalScienceYield();

	InfoString += FString::Printf(TEXT("食料: %d\n"), Food);
	InfoString += FString::Printf(TEXT("生産力: %d\n"), Production);
	InfoString += FString::Printf(TEXT("ゴールド: %d\n"), Gold);
	InfoString += FString::Printf(TEXT("科学力: %d\n"), Science);

	return InfoString;
}
