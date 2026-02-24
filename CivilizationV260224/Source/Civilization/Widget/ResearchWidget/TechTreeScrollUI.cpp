// Fill out your copyright notice in the Description page of Project Settings.

#include "TechTreeScrollUI.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "../../SuperGameInstance.h"
#include "../../SuperPlayerState.h"
#include "../../Research/ResearchComponent.h"
#include "../../Research/Research.h"

void UTechTreeScrollUI::NativeConstruct()
{
	Super::NativeConstruct();

	// InfoTechIconImg 초기값: 기술 데이터테이블 NoSelect 아이콘
	if (InfoTechIconImg)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
			{
				if (ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0))
				{
					if (UResearchComponent* ResearchComp = PlayerState->GetResearchComponent())
					{
						FTechData NoSelectData = ResearchComp->GetTechDataFromTable(FName("NoSelect"));
						if (!NoSelectData.TechIcon.IsNull())
						{
							UTexture2D* IconTex = NoSelectData.TechIcon.LoadSynchronous();
							if (IconTex)
							{
								InfoTechIconImg->SetBrushFromTexture(IconTex);
							}
						}
					}
				}
			}
		}
	}

	// Img 초기값 Hidden
	if (PotteryImg)
	{
		PotteryImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (AnimalHusbandryImg)
	{
		AnimalHusbandryImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (MiningImg)
	{
		MiningImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (IrrigationImg)
	{
		IrrigationImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (WritingImg)
	{
		WritingImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (ArcheryImg)
	{
		ArcheryImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (MasonryImg)
	{
		MasonryImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (BronzeWorkingImg)
	{
		BronzeWorkingImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (WheelImg)
	{
		WheelImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (MathematicsImg)
	{
		MathematicsImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (CurrencyImg)
	{
		CurrencyImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (MilitaryTacticsImg)
	{
		MilitaryTacticsImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (ConstructionImg)
	{
		ConstructionImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (IronWorkingImg)
	{
		IronWorkingImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (EngineeringImg)
	{
		EngineeringImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (EducationImg)
	{
		EducationImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (ApprenticeshipImg)
	{
		ApprenticeshipImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (StirrupsImg)
	{
		StirrupsImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (MilitaryEngineeringImg)
	{
		MilitaryEngineeringImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (CastlesImg)
	{
		CastlesImg->SetVisibility(ESlateVisibility::Hidden);
	}
	if (MachineryImg)
	{
		MachineryImg->SetVisibility(ESlateVisibility::Hidden);
	}

	// ExitBtn 클릭 바인딩
	if (ExitBtn)
	{
		ExitBtn->OnClicked.AddDynamic(this, &UTechTreeScrollUI::OnExitBtnClicked);
	}

	// TechCompleteTxt, TechNotCompleteTxt 초기 Hidden
	if (TechCompleteTxt)
	{
		TechCompleteTxt->SetVisibility(ESlateVisibility::Hidden);
	}
	if (TechNotCompleteTxt)
	{
		TechNotCompleteTxt->SetVisibility(ESlateVisibility::Hidden);
	}

	// Btn 호버/언호버 바인딩
	if (PotteryBtn)
	{
		PotteryBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnPotteryBtnHovered);
		PotteryBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnPotteryBtnUnhovered);
	}
	if (AnimalHusbandryBtn)
	{
		AnimalHusbandryBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnAnimalHusbandryBtnHovered);
		AnimalHusbandryBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnAnimalHusbandryBtnUnhovered);
	}
	if (MiningBtn)
	{
		MiningBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnMiningBtnHovered);
		MiningBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnMiningBtnUnhovered);
	}
	if (IrrigationBtn)
	{
		IrrigationBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnIrrigationBtnHovered);
		IrrigationBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnIrrigationBtnUnhovered);
	}
	if (WritingBtn)
	{
		WritingBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnWritingBtnHovered);
		WritingBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnWritingBtnUnhovered);
	}
	if (ArcheryBtn)
	{
		ArcheryBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnArcheryBtnHovered);
		ArcheryBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnArcheryBtnUnhovered);
	}
	if (MasonryBtn)
	{
		MasonryBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnMasonryBtnHovered);
		MasonryBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnMasonryBtnUnhovered);
	}
	if (BronzeWorkingBtn)
	{
		BronzeWorkingBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnBronzeWorkingBtnHovered);
		BronzeWorkingBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnBronzeWorkingBtnUnhovered);
	}
	if (WheelBtn)
	{
		WheelBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnWheelBtnHovered);
		WheelBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnWheelBtnUnhovered);
	}
	if (MathematicsBtn)
	{
		MathematicsBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnMathematicsBtnHovered);
		MathematicsBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnMathematicsBtnUnhovered);
	}
	if (CurrencyBtn)
	{
		CurrencyBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnCurrencyBtnHovered);
		CurrencyBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnCurrencyBtnUnhovered);
	}
	if (MilitaryTacticsBtn)
	{
		MilitaryTacticsBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnMilitaryTacticsBtnHovered);
		MilitaryTacticsBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnMilitaryTacticsBtnUnhovered);
	}
	if (ConstructionBtn)
	{
		ConstructionBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnConstructionBtnHovered);
		ConstructionBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnConstructionBtnUnhovered);
	}
	if (IronWorkingBtn)
	{
		IronWorkingBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnIronWorkingBtnHovered);
		IronWorkingBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnIronWorkingBtnUnhovered);
	}
	if (EngineeringBtn)
	{
		EngineeringBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnEngineeringBtnHovered);
		EngineeringBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnEngineeringBtnUnhovered);
	}
	if (EducationBtn)
	{
		EducationBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnEducationBtnHovered);
		EducationBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnEducationBtnUnhovered);
	}
	if (ApprenticeshipBtn)
	{
		ApprenticeshipBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnApprenticeshipBtnHovered);
		ApprenticeshipBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnApprenticeshipBtnUnhovered);
	}
	if (StirrupsBtn)
	{
		StirrupsBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnStirrupsBtnHovered);
		StirrupsBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnStirrupsBtnUnhovered);
	}
	if (MilitaryEngineeringBtn)
	{
		MilitaryEngineeringBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnMilitaryEngineeringBtnHovered);
		MilitaryEngineeringBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnMilitaryEngineeringBtnUnhovered);
	}
	if (CastlesBtn)
	{
		CastlesBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnCastlesBtnHovered);
		CastlesBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnCastlesBtnUnhovered);
	}
	if (MachineryBtn)
	{
		MachineryBtn->OnHovered.AddDynamic(this, &UTechTreeScrollUI::OnMachineryBtnHovered);
		MachineryBtn->OnUnhovered.AddDynamic(this, &UTechTreeScrollUI::OnMachineryBtnUnhovered);
	}

	UpdateTechCompleteIndicators();
}

void UTechTreeScrollUI::UpdateTechCompleteIndicators()
{
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

	ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0);
	if (!PlayerState)
	{
		return;
	}

	UResearchComponent* ResearchComp = PlayerState->GetResearchComponent();
	if (!ResearchComp)
	{
		return;
	}

	if (PotteryImg)
	{
		PotteryImg->SetVisibility(ResearchComp->IsTechResearched(FName("Pottery")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (AnimalHusbandryImg)
	{
		AnimalHusbandryImg->SetVisibility(ResearchComp->IsTechResearched(FName("AnimalHusbandry")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (MiningImg)
	{
		MiningImg->SetVisibility(ResearchComp->IsTechResearched(FName("Mining")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (IrrigationImg)
	{
		IrrigationImg->SetVisibility(ResearchComp->IsTechResearched(FName("Irrigation")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (WritingImg)
	{
		WritingImg->SetVisibility(ResearchComp->IsTechResearched(FName("Writing")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (ArcheryImg)
	{
		ArcheryImg->SetVisibility(ResearchComp->IsTechResearched(FName("Archery")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (MasonryImg)
	{
		MasonryImg->SetVisibility(ResearchComp->IsTechResearched(FName("Masonry")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (BronzeWorkingImg)
	{
		BronzeWorkingImg->SetVisibility(ResearchComp->IsTechResearched(FName("BronzeWorking")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (WheelImg)
	{
		WheelImg->SetVisibility(ResearchComp->IsTechResearched(FName("Wheel")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (MathematicsImg)
	{
		MathematicsImg->SetVisibility(ResearchComp->IsTechResearched(FName("Mathematics")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (CurrencyImg)
	{
		CurrencyImg->SetVisibility(ResearchComp->IsTechResearched(FName("Currency")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (MilitaryTacticsImg)
	{
		MilitaryTacticsImg->SetVisibility(ResearchComp->IsTechResearched(FName("MilitaryTactics")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (ConstructionImg)
	{
		ConstructionImg->SetVisibility(ResearchComp->IsTechResearched(FName("Construction")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (IronWorkingImg)
	{
		IronWorkingImg->SetVisibility(ResearchComp->IsTechResearched(FName("IronWorking")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (EngineeringImg)
	{
		EngineeringImg->SetVisibility(ResearchComp->IsTechResearched(FName("Engineering")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (EducationImg)
	{
		EducationImg->SetVisibility(ResearchComp->IsTechResearched(FName("Education")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (ApprenticeshipImg)
	{
		ApprenticeshipImg->SetVisibility(ResearchComp->IsTechResearched(FName("Apprenticeship")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (StirrupsImg)
	{
		StirrupsImg->SetVisibility(ResearchComp->IsTechResearched(FName("Stirrups")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (MilitaryEngineeringImg)
	{
		MilitaryEngineeringImg->SetVisibility(ResearchComp->IsTechResearched(FName("MilitaryEngineering")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (CastlesImg)
	{
		CastlesImg->SetVisibility(ResearchComp->IsTechResearched(FName("Castles")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (MachineryImg)
	{
		MachineryImg->SetVisibility(ResearchComp->IsTechResearched(FName("Machinery")) ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UTechTreeScrollUI::SetInfoFromTech(FName TechRowName)
{
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

	ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0);
	if (!PlayerState)
	{
		return;
	}

	UResearchComponent* ResearchComp = PlayerState->GetResearchComponent();
	if (!ResearchComp)
	{
		return;
	}

	FTechData TechData = ResearchComp->GetTechDataFromTable(TechRowName);
	if (TechData.ScienceCost == 0 && TechData.TechName.IsEmpty())
	{
		return;
	}

	// InfoTechIconImg
	if (InfoTechIconImg)
	{
		if (!TechData.TechIcon.IsNull())
		{
			UTexture2D* IconTex = TechData.TechIcon.LoadSynchronous();
			if (IconTex)
			{
				InfoTechIconImg->SetBrushFromTexture(IconTex);
			}
			else
			{
				InfoTechIconImg->SetBrushFromTexture(nullptr);
			}
		}
		else
		{
			InfoTechIconImg->SetBrushFromTexture(nullptr);
		}
	}

	// InfoTechNameTxt
	if (InfoTechNameTxt)
	{
		InfoTechNameTxt->SetText(FText::FromString(TechData.TechName));
	}

	// TechInfoTxt (Unlocked Facility: X 형식)
	if (TechInfoTxt)
	{
		TechInfoTxt->SetText(FText::FromString(BuildUnlockInfoString(TechData)));
	}

	// TechDescriptionTxt
	if (TechDescriptionTxt)
	{
		TechDescriptionTxt->SetText(FText::FromString(TechData.TechDescription));
	}

	// TechCompleteTxt / TechNotCompleteTxt
	bool bResearched = ResearchComp->IsTechResearched(TechRowName);
	if (TechCompleteTxt)
	{
		TechCompleteTxt->SetVisibility(bResearched ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (TechNotCompleteTxt)
	{
		TechNotCompleteTxt->SetVisibility(bResearched ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
	}
}

void UTechTreeScrollUI::ClearInfoPanel()
{
	// InfoTechIconImg를 NoSelect 아이콘으로 복원
	if (InfoTechIconImg)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
			{
				if (ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0))
				{
					if (UResearchComponent* ResearchComp = PlayerState->GetResearchComponent())
					{
						FTechData NoSelectData = ResearchComp->GetTechDataFromTable(FName("NoSelect"));
						if (!NoSelectData.TechIcon.IsNull())
						{
							UTexture2D* IconTex = NoSelectData.TechIcon.LoadSynchronous();
							if (IconTex)
							{
								InfoTechIconImg->SetBrushFromTexture(IconTex);
							}
						}
					}
				}
			}
		}
	}
	if (InfoTechNameTxt)
	{
		InfoTechNameTxt->SetText(FText::GetEmpty());
	}
	if (TechInfoTxt)
	{
		TechInfoTxt->SetText(FText::GetEmpty());
	}
	if (TechDescriptionTxt)
	{
		TechDescriptionTxt->SetText(FText::GetEmpty());
	}
	if (TechCompleteTxt)
	{
		TechCompleteTxt->SetVisibility(ESlateVisibility::Hidden);
	}
	if (TechNotCompleteTxt)
	{
		TechNotCompleteTxt->SetVisibility(ESlateVisibility::Hidden);
	}
}

FString UTechTreeScrollUI::BuildUnlockInfoString(const FTechData& TechData) const
{
	TArray<FString> Lines;

	for (const FName& RowName : TechData.UnlockFacilities)
	{
		Lines.Add(FString::Printf(TEXT("Unlocked Facility: %s"), *RowName.ToString()));
	}
	for (const FName& RowName : TechData.UnlockBuildings)
	{
		Lines.Add(FString::Printf(TEXT("Unlocked Building: %s"), *RowName.ToString()));
	}
	for (const FName& RowName : TechData.UnlockUnits)
	{
		Lines.Add(FString::Printf(TEXT("Unlocked Unit: %s"), *RowName.ToString()));
	}

	return FString::Join(Lines, TEXT("\n"));
}

void UTechTreeScrollUI::OnPotteryBtnHovered()
{
	SetInfoFromTech(FName("Pottery"));
}

void UTechTreeScrollUI::OnPotteryBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnAnimalHusbandryBtnHovered()
{
	SetInfoFromTech(FName("AnimalHusbandry"));
}

void UTechTreeScrollUI::OnAnimalHusbandryBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnMiningBtnHovered()
{
	SetInfoFromTech(FName("Mining"));
}

void UTechTreeScrollUI::OnMiningBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnIrrigationBtnHovered()
{
	SetInfoFromTech(FName("Irrigation"));
}

void UTechTreeScrollUI::OnIrrigationBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnWritingBtnHovered()
{
	SetInfoFromTech(FName("Writing"));
}

void UTechTreeScrollUI::OnWritingBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnArcheryBtnHovered()
{
	SetInfoFromTech(FName("Archery"));
}

void UTechTreeScrollUI::OnArcheryBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnMasonryBtnHovered()
{
	SetInfoFromTech(FName("Masonry"));
}

void UTechTreeScrollUI::OnMasonryBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnBronzeWorkingBtnHovered()
{
	SetInfoFromTech(FName("BronzeWorking"));
}

void UTechTreeScrollUI::OnBronzeWorkingBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnWheelBtnHovered()
{
	SetInfoFromTech(FName("Wheel"));
}

void UTechTreeScrollUI::OnWheelBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnMathematicsBtnHovered()
{
	SetInfoFromTech(FName("Mathematics"));
}

void UTechTreeScrollUI::OnMathematicsBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnCurrencyBtnHovered()
{
	SetInfoFromTech(FName("Currency"));
}

void UTechTreeScrollUI::OnCurrencyBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnMilitaryTacticsBtnHovered()
{
	SetInfoFromTech(FName("MilitaryTactics"));
}

void UTechTreeScrollUI::OnMilitaryTacticsBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnConstructionBtnHovered()
{
	SetInfoFromTech(FName("Construction"));
}

void UTechTreeScrollUI::OnConstructionBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnIronWorkingBtnHovered()
{
	SetInfoFromTech(FName("IronWorking"));
}

void UTechTreeScrollUI::OnIronWorkingBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnEngineeringBtnHovered()
{
	SetInfoFromTech(FName("Engineering"));
}

void UTechTreeScrollUI::OnEngineeringBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnEducationBtnHovered()
{
	SetInfoFromTech(FName("Education"));
}

void UTechTreeScrollUI::OnEducationBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnApprenticeshipBtnHovered()
{
	SetInfoFromTech(FName("Apprenticeship"));
}

void UTechTreeScrollUI::OnApprenticeshipBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnStirrupsBtnHovered()
{
	SetInfoFromTech(FName("Stirrups"));
}

void UTechTreeScrollUI::OnStirrupsBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnMilitaryEngineeringBtnHovered()
{
	SetInfoFromTech(FName("MilitaryEngineering"));
}

void UTechTreeScrollUI::OnMilitaryEngineeringBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnCastlesBtnHovered()
{
	SetInfoFromTech(FName("Castles"));
}

void UTechTreeScrollUI::OnCastlesBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnMachineryBtnHovered()
{
	SetInfoFromTech(FName("Machinery"));
}

void UTechTreeScrollUI::OnMachineryBtnUnhovered()
{
	ClearInfoPanel();
}

void UTechTreeScrollUI::OnExitBtnClicked()
{
	OnExitClicked.Broadcast();
}
