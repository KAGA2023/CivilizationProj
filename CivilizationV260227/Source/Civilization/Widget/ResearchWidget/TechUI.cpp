// Fill out your copyright notice in the Description page of Project Settings.

#include "TechUI.h"
#include "Components/VerticalBox.h"
#include "../../SuperPlayerState.h"
#include "../../Research/ResearchComponent.h"
#include "../../Research/Research.h"
#include "../../World/WorldComponent.h"
#include "../../SuperGameInstance.h"
#include "../../Facility/FacilityManager.h"
#include "TechSlotUI.h"
#include "Engine/DataTable.h"

UTechUI::UTechUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CachedPlayerState = nullptr;
	CachedTechComponent = nullptr;
}

void UTechUI::NativeConstruct()
{
	Super::NativeConstruct();
}

void UTechUI::SetupTechUI(ASuperPlayerState* PlayerState)
{
	if (!PlayerState || !TechVB)
	{
		return;
	}

	CachedPlayerState = PlayerState;

	// ResearchComponent 가져오기
	CachedTechComponent = PlayerState->GetResearchComponent();
	if (!CachedTechComponent)
	{
		return;
	}

	// 연구 가능 목록 강제 갱신 (데이터 테이블 로드 및 목록 업데이트)
	CachedTechComponent->UpdateResearchableTechs();

	// 델리게이트 바인딩
	BindToResearchableTechsUpdated();
	BindToFacilityDelegates();

	// 초기 슬롯 생성
	TArray<FName> ResearchableTechs = CachedTechComponent->GetResearchableTechs();
	CreateTechSlots(ResearchableTechs);
}

void UTechUI::BindToResearchableTechsUpdated()
{
	if (!CachedTechComponent)
	{
		return;
	}

	// 기존 바인딩 해제
	CachedTechComponent->OnResearchableTechsUpdated.RemoveDynamic(this, &UTechUI::OnResearchableTechsUpdated);

	// 새로운 바인딩
	CachedTechComponent->OnResearchableTechsUpdated.AddDynamic(this, &UTechUI::OnResearchableTechsUpdated);
}

void UTechUI::OnResearchableTechsUpdated(TArray<FName> ResearchableTechs)
{
	if (!TechVB)
	{
		return;
	}

	// 기존 슬롯 제거
	ClearAllSlots();

	// 새로운 슬롯 생성
	CreateTechSlots(ResearchableTechs);
}

void UTechUI::CreateTechSlots(const TArray<FName>& TechNames)
{
	if (!TechVB || !CachedTechComponent)
	{
		return;
	}

	// TechSlotUI 클래스 로드
	UClass* SlotClass = LoadClass<UTechSlotUI>(nullptr, TEXT("/Game/Civilization/Widget/ResearchWidget/W_TechSlot.W_TechSlot_C"));
	if (!SlotClass)
	{
		return;
	}

	// 각 기술에 대해 슬롯 생성
	for (const FName& TechName : TechNames)
	{
		if (TechName == NAME_None)
		{
			continue;
		}

		// 기술 데이터 가져오기
		FTechData TechData = CachedTechComponent->GetTechDataFromTable(TechName);
		if (TechData.ScienceCost == 0 && TechData.TechName.IsEmpty())
		{
			continue;
		}

		// 위젯 생성
		UTechSlotUI* TechSlot = CreateWidget<UTechSlotUI>(this, SlotClass);
		if (TechSlot)
		{
			// TechRowName 설정
			TechSlot->TechRowName = TechName;

			// 이미지 및 텍스트 설정
			UTexture2D* TechIcon = nullptr;
			if (!TechData.TechIcon.IsNull())
			{
				TechIcon = TechData.TechIcon.LoadSynchronous();
			}
			TechSlot->SetTechImage(TechIcon);
			TechSlot->SetTechText(TechData.TechName);

			// 턴 수 계산 및 설정 (기술은 과학량 사용)
			int32 TurnScience = 0;
			if (CachedPlayerState)
			{
				if (UWorld* World = GetWorld())
				{
					if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
					{
						if (UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent())
						{
							TurnScience = CachedPlayerState->CalculateTotalScienceYield(WorldComponent);
						}
					}
				}
			}
			
			int32 Turns = 0;
			if (TurnScience > 0 && TechData.ScienceCost > 0)
			{
				Turns = FMath::CeilToInt((float)TechData.ScienceCost / (float)TurnScience);
			}
			else if (TurnScience <= 0)
			{
				Turns = 99;
			}
			TechSlot->SetTurnText(Turns);

			// VerticalBox에 추가
			TechVB->AddChild(TechSlot);

			// 슬롯 클릭 델리게이트 바인딩
			TechSlot->OnTechSlotClicked.AddDynamic(this, &UTechUI::OnTechSlotClicked);

			// 위젯 가시성 설정
			TechSlot->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UTechUI::ClearAllSlots()
{
	if (TechVB)
	{
		TechVB->ClearChildren();
	}
}

void UTechUI::OnTechSlotClicked(FName TechRowName)
{
	if (!CachedPlayerState || !CachedTechComponent || TechRowName == NAME_None)
	{
		return;
	}

	// 기술 연구 시작
	CachedPlayerState->StartTechResearch(TechRowName);
}

void UTechUI::OnFacilityChanged(FVector2D TileCoordinate)
{
	// 시설 변경 시 모든 슬롯의 TurnTxt 갱신 (시설로 인한 과학량 변화 반영)
	UpdateAllSlotsTurnText();
}

void UTechUI::BindToFacilityDelegates()
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
	FacilityManager->OnFacilityChanged.AddDynamic(this, &UTechUI::OnFacilityChanged);
}

void UTechUI::UnbindFromFacilityDelegates()
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
		FacilityManager->OnFacilityChanged.RemoveDynamic(this, &UTechUI::OnFacilityChanged);
	}
}

void UTechUI::UpdateAllSlotsTurnText()
{
	if (!CachedPlayerState || !CachedTechComponent)
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

	// 기술 슬롯의 TurnTxt 갱신
	if (TechVB)
	{
		for (int32 i = 0; i < TechVB->GetChildrenCount(); ++i)
		{
			UTechSlotUI* TechSlot = Cast<UTechSlotUI>(TechVB->GetChildAt(i));
			if (TechSlot && TechSlot->TechRowName != NAME_None)
			{
				// 기술 데이터 가져오기
				FTechData TechData = CachedTechComponent->GetTechDataFromTable(TechSlot->TechRowName);
				if (TechData.ScienceCost > 0 || !TechData.TechName.IsEmpty())
				{
					// 턴 수 계산 및 설정 (기술은 과학량 사용)
					int32 TurnScience = CachedPlayerState->CalculateTotalScienceYield(WorldComponent);
					
					int32 Turns = 0;
					if (TurnScience > 0 && TechData.ScienceCost > 0)
					{
						Turns = FMath::CeilToInt((float)TechData.ScienceCost / (float)TurnScience);
					}
					else if (TurnScience <= 0)
					{
						Turns = 99;
					}
					TechSlot->SetTurnText(Turns);
				}
			}
		}
	}
}

