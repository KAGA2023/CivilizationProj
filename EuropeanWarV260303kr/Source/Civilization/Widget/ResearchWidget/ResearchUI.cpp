// Fill out your copyright notice in the Description page of Project Settings.

#include "ResearchUI.h"
#include "TechTreeScrollUI.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "TechUI.h"
#include "../../SuperGameInstance.h"
#include "../../SuperPlayerState.h"
#include "../../World/WorldComponent.h"
#include "../../Research/ResearchComponent.h"
#include "../../Research/Research.h"
#include "../../Facility/FacilityManager.h"

void UResearchUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 클릭 이벤트 바인딩
	if (OpenTechTreeBtn)
	{
		OpenTechTreeBtn->OnClicked.AddDynamic(this, &UResearchUI::OnOpenTechTreeBtnClicked);
	}

	// TechTreeScrollUI ExitBtn 델리게이트 바인딩 (CloseTechTreeUI 재생)
	if (TechTreeScrollUIWidget)
	{
		TechTreeScrollUIWidget->OnExitClicked.AddDynamic(this, &UResearchUI::OnTechTreeExitClicked);
	}

	// 연구 컴포넌트 참조 가져오기 및 델리게이트 바인딩
	BindToResearchDelegates();
	BindToFacilityDelegates();
}

void UResearchUI::UpdateResearchData()
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

	// 연구 정보 업데이트
	UpdateResearchInfo();
}

void UResearchUI::OnOpenTechTreeBtnClicked()
{
	if (OpenTechTreeUI)
	{
		PlayAnimation(OpenTechTreeUI, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
	}
	if (TechTreeScrollUIWidget)
	{
		TechTreeScrollUIWidget->UpdateTechCompleteIndicators();
	}
}

void UResearchUI::OnTechTreeExitClicked()
{
	if (CloseTechTreeUI)
	{
		PlayAnimation(CloseTechTreeUI, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f);
	}
}

void UResearchUI::UpdateResearchInfo()
{
	if (!CachedTechComponent)
	{
		// ResearchComponent가 없으면 초기화 시도
		BindToResearchDelegates();
		if (!CachedTechComponent)
		{
			// 여전히 없으면 연구 정보 초기화
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
	}

	// 현재 연구 상태 가져오기
	FResearchCurrentStat CurrentStat = CachedTechComponent->GetCurrentStat();

	// 연구 중이 아니면 NoSelect 데이터 표시
	FName TechNameToDisplay = CurrentStat.DevelopingName;
	if (TechNameToDisplay == NAME_None)
	{
		TechNameToDisplay = FName("NoSelect");
	}

	// 기술 데이터 가져오기
	FTechData TechData = CachedTechComponent->GetTechDataFromTable(TechNameToDisplay);
	
	if (TechData.ScienceCost == 0 && TechData.TechName.IsEmpty())
	{
		// 기술 데이터가 없으면 빈 값으로 설정
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

	// 기술 이름 표시
	if (DevelopingTxt)
	{
		DevelopingTxt->SetText(FText::FromString(TechData.TechName));
	}

	// 기술 아이콘 표시
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

	// 연구 진행도 바 업데이트 (DevelopingProgress / DevelopingCost)
	if (DevelopingBar)
	{
		// 실제 연구 중일 때만 진행도 표시, NoSelect일 때는 0%
		float ProgressPercent = 0.0f;
		if (CurrentStat.DevelopingName != NAME_None && CurrentStat.DevelopingCost > 0)
		{
			ProgressPercent = FMath::Clamp((float)CurrentStat.DevelopingProgress / (float)CurrentStat.DevelopingCost, 0.0f, 1.0f);
		}
		DevelopingBar->SetPercent(ProgressPercent);
	}
}

void UResearchUI::NativeDestruct()
{
	if (TechTreeScrollUIWidget)
	{
		TechTreeScrollUIWidget->OnExitClicked.RemoveDynamic(this, &UResearchUI::OnTechTreeExitClicked);
	}
	// 델리게이트 바인딩 해제
	UnbindFromResearchDelegates();
	UnbindFromFacilityDelegates();

	Super::NativeDestruct();
}

void UResearchUI::BindToResearchDelegates()
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

	// ResearchComponent 가져오기
	CachedTechComponent = PlayerState->GetResearchComponent();
	if (!CachedTechComponent)
	{
		return;
	}

	// 기존 바인딩 해제 후 새로 바인딩
	UnbindFromResearchDelegates();

	// 연구 시작 델리게이트 바인딩
	CachedTechComponent->OnTechResearchStarted.AddDynamic(this, &UResearchUI::OnTechResearchStarted);

	// 연구 완료 델리게이트 바인딩
	CachedTechComponent->OnTechResearchCompleted.AddDynamic(this, &UResearchUI::OnTechResearchCompleted);

	// 연구 진행도 변경 델리게이트 바인딩
	CachedTechComponent->OnTechResearchProgressChanged.AddDynamic(this, &UResearchUI::OnTechResearchProgressChanged);

	// 초기 연구 정보 업데이트
	UpdateResearchInfo();
}

void UResearchUI::BindToFacilityDelegates()
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
	FacilityManager->OnFacilityChanged.AddDynamic(this, &UResearchUI::OnFacilityChanged);
}

void UResearchUI::UnbindFromFacilityDelegates()
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
		FacilityManager->OnFacilityChanged.RemoveDynamic(this, &UResearchUI::OnFacilityChanged);
	}
}

void UResearchUI::OnFacilityChanged(FVector2D TileCoordinate)
{
	// 시설 변경 시 연구 데이터 업데이트 (시설로 인한 타일 생산량 변화 반영)
	UpdateResearchData();
}

void UResearchUI::UnbindFromResearchDelegates()
{
	if (CachedTechComponent)
	{
		// 델리게이트 바인딩 해제
		CachedTechComponent->OnTechResearchStarted.RemoveDynamic(this, &UResearchUI::OnTechResearchStarted);
		CachedTechComponent->OnTechResearchCompleted.RemoveDynamic(this, &UResearchUI::OnTechResearchCompleted);
		CachedTechComponent->OnTechResearchProgressChanged.RemoveDynamic(this, &UResearchUI::OnTechResearchProgressChanged);
	}
}

void UResearchUI::OnTechResearchStarted(FName TechID)
{
	// 연구 시작 시 연구 정보 업데이트
	UpdateResearchInfo();
}

void UResearchUI::OnTechResearchCompleted(FName TechID)
{
	// 연구 데이터 업데이트 (연구 완료로 인한 변화 반영)
	UpdateResearchData();
}

void UResearchUI::OnTechResearchProgressChanged()
{
	// 연구 진행도 변경 시 연구 정보 업데이트 (진행도 바 포함)
	UpdateResearchInfo();
}

