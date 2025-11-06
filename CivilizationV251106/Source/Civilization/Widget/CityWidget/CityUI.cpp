// Fill out your copyright notice in the Description page of Project Settings.

#include "CityUI.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "../../SuperGameInstance.h"
#include "../../SuperPlayerState.h"
#include "../../WorldComponent.h"

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

