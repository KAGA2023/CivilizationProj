// Fill out your copyright notice in the Description page of Project Settings.

#include "OpenResearchUI.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "../../SuperGameInstance.h"
#include "../../SuperPlayerState.h"
#include "../../Research/ResearchComponent.h"
#include "../../Research/Research.h"

void UOpenResearchUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (OpenResearchBtn)
	{
		OpenResearchBtn->OnClicked.AddDynamic(this, &UOpenResearchUI::OnOpenResearchBtnClicked);
	}

	// 초기 표시: ResearchComponent는 MainHUD에서 타이머로 나중에 호출될 때 준비됨
	UpdateResearchInfo();
}

void UOpenResearchUI::NativeDestruct()
{
	UnbindFromResearchDelegates();
	Super::NativeDestruct();
}

void UOpenResearchUI::OnOpenResearchBtnClicked()
{
	OnOpenResearchButtonClicked.Broadcast();
}

void UOpenResearchUI::UpdateResearchInfo()
{
	// ResearchUI와 동일: 호출 시마다 ResearchComponent 조회 (OpenResearchUI는 HUD와 함께 일찍 생성되므로 나중에 호출될 때 준비됨)
	UResearchComponent* Component = nullptr;
	if (GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance()))
		{
			if (ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0))
			{
				Component = PlayerState->GetResearchComponent();
			}
		}
	}

	if (!Component)
	{
		if (DevelopingTxt)
		{
			DevelopingTxt->SetText(FText::GetEmpty());
		}
		if (DevelopingImg)
		{
			DevelopingImg->SetBrushFromTexture(nullptr);
		}
		if (DevelopingBar)
		{
			DevelopingBar->SetPercent(0.0f);
		}
		if (CachedTechComponent)
		{
			UnbindFromResearchDelegates();
			CachedTechComponent = nullptr;
		}
		return;
	}

	// 컴포넌트를 얻었고 아직 델리게이트를 바인딩하지 않았으면 바인딩 (ResearchUI와 동일하게 연구 시작/완료/진행도 시 갱신)
	if (Component != CachedTechComponent)
	{
		UnbindFromResearchDelegates();
		CachedTechComponent = Component;
		CachedTechComponent->OnTechResearchStarted.AddDynamic(this, &UOpenResearchUI::OnTechResearchStarted);
		CachedTechComponent->OnTechResearchCompleted.AddDynamic(this, &UOpenResearchUI::OnTechResearchCompleted);
		CachedTechComponent->OnTechResearchProgressChanged.AddDynamic(this, &UOpenResearchUI::OnTechResearchProgressChanged);
	}

	FResearchCurrentStat CurrentStat = CachedTechComponent->GetCurrentStat();

	FName TechNameToDisplay = CurrentStat.DevelopingName;
	if (TechNameToDisplay == NAME_None)
	{
		TechNameToDisplay = FName("NoSelect");
	}

	FTechData TechData = CachedTechComponent->GetTechDataFromTable(TechNameToDisplay);

	if (TechData.ScienceCost == 0 && TechData.TechName.IsEmpty())
	{
		if (DevelopingTxt)
		{
			DevelopingTxt->SetText(FText::GetEmpty());
		}
		if (DevelopingImg)
		{
			DevelopingImg->SetBrushFromTexture(nullptr);
		}
		if (DevelopingBar)
		{
			DevelopingBar->SetPercent(0.0f);
		}
		return;
	}

	if (DevelopingTxt)
	{
		DevelopingTxt->SetText(FText::FromString(TechData.TechName));
	}

	if (DevelopingImg)
	{
		if (!TechData.TechIcon.IsNull())
		{
			UTexture2D* TechIcon = TechData.TechIcon.LoadSynchronous();
			if (TechIcon)
			{
				DevelopingImg->SetBrushFromTexture(TechIcon);
			}
			else
			{
				DevelopingImg->SetBrushFromTexture(nullptr);
			}
		}
		else
		{
			DevelopingImg->SetBrushFromTexture(nullptr);
		}
	}

	if (DevelopingBar)
	{
		float ProgressPercent = 0.0f;
		if (CurrentStat.DevelopingName != NAME_None && CurrentStat.DevelopingCost > 0)
		{
			ProgressPercent = FMath::Clamp((float)CurrentStat.DevelopingProgress / (float)CurrentStat.DevelopingCost, 0.0f, 1.0f);
		}
		DevelopingBar->SetPercent(ProgressPercent);
	}
}


void UOpenResearchUI::UnbindFromResearchDelegates()
{
	if (CachedTechComponent)
	{
		CachedTechComponent->OnTechResearchStarted.RemoveDynamic(this, &UOpenResearchUI::OnTechResearchStarted);
		CachedTechComponent->OnTechResearchCompleted.RemoveDynamic(this, &UOpenResearchUI::OnTechResearchCompleted);
		CachedTechComponent->OnTechResearchProgressChanged.RemoveDynamic(this, &UOpenResearchUI::OnTechResearchProgressChanged);
	}
}

void UOpenResearchUI::OnTechResearchStarted(FName TechID)
{
	UpdateResearchInfo();
}

void UOpenResearchUI::OnTechResearchCompleted(FName TechID)
{
	UpdateResearchInfo();
}

void UOpenResearchUI::OnTechResearchProgressChanged()
{
	UpdateResearchInfo();
}
