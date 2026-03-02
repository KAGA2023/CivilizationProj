// Fill out your copyright notice in the Description page of Project Settings.

#include "DiplomacyUI.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "DiplomacyCountryUI.h"
#include "../../SuperPlayerState.h"
#include "../../SuperGameInstance.h"
#include "../../SuperGameModeBase.h"
#include "../../Turn/TurnComponent.h"
#include "../../Diplomacy/DiplomacyManager.h"
#include "../../Diplomacy/DiplomacyStruct.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"

void UDiplomacyUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 모든 버튼 클릭 이벤트 바인딩
	if (SendGiftBtn)
	{
		SendGiftBtn->OnClicked.AddDynamic(this, &UDiplomacyUI::OnSendGiftBtnClicked);
	}

	if (OfferAllianceBtn)
	{
		OfferAllianceBtn->OnClicked.AddDynamic(this, &UDiplomacyUI::OnOfferAllianceBtnClicked);
	}

	if (OfferPeaceBtn)
	{
		OfferPeaceBtn->OnClicked.AddDynamic(this, &UDiplomacyUI::OnOfferPeaceBtnClicked);
	}

	if (DenounceBtn)
	{
		DenounceBtn->OnClicked.AddDynamic(this, &UDiplomacyUI::OnDenounceBtnClicked);
	}

	if (DeclareWarBtn)
	{
		DeclareWarBtn->OnClicked.AddDynamic(this, &UDiplomacyUI::OnDeclareWarBtnClicked);
	}

	if (CloseBtn)
	{
		CloseBtn->OnClicked.AddDynamic(this, &UDiplomacyUI::OnCloseBtnClicked);
	}
}

void UDiplomacyUI::SetupForPlayer(int32 TargetPlayerIndex, ASuperPlayerState* TargetPlayerState)
{
	if (!TargetPlayerState)
	{
		return;
	}

	// 플레이어 인덱스 저장
	CurrentTargetPlayerIndex = TargetPlayerIndex;

	// CountryTxt 설정
	if (CountryTxt)
	{
		FString CountryName = TargetPlayerState->GetCountryName();
		CountryTxt->SetText(FText::FromString(CountryName));
	}

	// CountryImg 설정 (큰 이미지 사용)
	if (CountryImg)
	{
		UTexture2D* Texture = TargetPlayerState->GetCountryLargeImg();
		if (Texture)
		{
			CountryImg->SetBrushFromTexture(Texture);
		}
	}

	// KingImg 설정 (왕 이미지 사용)
	if (KingImg)
	{
		UTexture2D* KingTexture = TargetPlayerState->GetCountryKingImg();
		if (KingTexture)
		{
			KingImg->SetBrushFromTexture(KingTexture);
		}
	}

	// ========== LoseImg, DefeatBrd 표시 처리 (패배한 플레이어인 경우) ==========
	bool bIsDefeated = TargetPlayerState->bIsDefeated;
	if (LoseImg)
	{
		if (bIsDefeated)
		{
			LoseImg->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			LoseImg->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	if (DefeatBrd)
	{
		DefeatBrd->SetVisibility(bIsDefeated ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	// 버튼 상태 업데이트
	UpdateButtonStates();

	// 해당 국가의 외교 관계(Good/Bad/War/Ally) 박스 갱신
	UpdateRelationBoxes();
}

UDiplomacyManager* UDiplomacyUI::GetDiplomacyManager() const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return nullptr;
	}

	return SuperGameInst->GetDiplomacyManager();
}

int32 UDiplomacyUI::GetCurrentRound() const
{
	if (!GetWorld())
	{
		return 1;
	}

	if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
	{
		if (ASuperGameModeBase* SuperGameMode = Cast<ASuperGameModeBase>(GameMode))
		{
			if (UTurnComponent* TurnComponent = SuperGameMode->GetTurnComponent())
			{
				return TurnComponent->GetCurrentRoundNumber();
			}
		}
	}

	return 1;
}

void UDiplomacyUI::UpdateRelationBoxes()
{
	UDiplomacyManager* DiplomacyManager = GetDiplomacyManager();
	if (!DiplomacyManager || CurrentTargetPlayerIndex < 0 || !GetWorld())
	{
		return;
	}
	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	// DiplomacyCountryUI 블루프린트 클래스 로드 (없으면 C++ 클래스 사용)
	UClass* CountryUIClass = LoadClass<UDiplomacyCountryUI>(nullptr, TEXT("/Game/Civilization/Widget/DiplomacyWidget/W_DiplomacyCountry.W_DiplomacyCountry_C"));
	if (!CountryUIClass)
	{
		CountryUIClass = UDiplomacyCountryUI::StaticClass();
	}

	const int32 Target = CurrentTargetPlayerIndex;
	const int32 NumPlayers = SuperGameInst->PlayerStates.Num();

	auto AddCountryToBox = [this, SuperGameInst, CountryUIClass, NumPlayers, Target, DiplomacyManager](UHorizontalBox* Box, int32 OtherIndex) -> void
	{
		if (!Box) return;
		ASuperPlayerState* OtherState = SuperGameInst->GetPlayerState(OtherIndex);
		if (!OtherState || OtherState->bIsDefeated) return;
		UDiplomacyCountryUI* CountryWidget = CreateWidget<UDiplomacyCountryUI>(this, CountryUIClass);
		if (!CountryWidget || !CountryWidget->CountryImg) return;
		UTexture2D* Tex = OtherState->GetCountryLargeImg();
		if (Tex)
		{
			CountryWidget->CountryImg->SetBrushFromTexture(Tex);
		}
		Box->AddChild(CountryWidget);
	};

	// WarHB: 전쟁 중인 국가
	if (WarHB)
	{
		WarHB->ClearChildren();
		for (int32 i = 0; i < NumPlayers; ++i)
		{
			if (i == Target) continue;
			if (DiplomacyManager->GetStatus(Target, i) == EDiplomacyStatusType::War)
			{
				AddCountryToBox(WarHB, i);
			}
		}
	}

	// AllyHB: 동맹인 국가
	if (AllyHB)
	{
		AllyHB->ClearChildren();
		for (int32 i = 0; i < NumPlayers; ++i)
		{
			if (i == Target) continue;
			if (DiplomacyManager->GetStatus(Target, i) == EDiplomacyStatusType::Alliance)
			{
				AddCountryToBox(AllyHB, i);
			}
		}
	}

	// GoodHB: 우호적(전쟁 제외, 호감도 20 이상)
	if (GoodHB)
	{
		GoodHB->ClearChildren();
		for (int32 i = 0; i < NumPlayers; ++i)
		{
			if (i == Target) continue;
			EDiplomacyStatusType Status = DiplomacyManager->GetStatus(Target, i);
			int32 Attitude = DiplomacyManager->GetAttitude(Target, i);
			if (Status != EDiplomacyStatusType::War && Attitude >= 20)
			{
				AddCountryToBox(GoodHB, i);
			}
		}
	}

	// BadHB: 적대적(전쟁 제외, 호감도 -20 이하)
	if (BadHB)
	{
		BadHB->ClearChildren();
		for (int32 i = 0; i < NumPlayers; ++i)
		{
			if (i == Target) continue;
			EDiplomacyStatusType Status = DiplomacyManager->GetStatus(Target, i);
			int32 Attitude = DiplomacyManager->GetAttitude(Target, i);
			if (Status != EDiplomacyStatusType::War && Attitude <= -20)
			{
				AddCountryToBox(BadHB, i);
			}
		}
	}
}

void UDiplomacyUI::OnSendGiftBtnClicked()
{
	if (CurrentTargetPlayerIndex < 0)
	{
		return;
	}

	UDiplomacyManager* DiplomacyManager = GetDiplomacyManager();
	if (!DiplomacyManager)
	{
		return;
	}

	// 외교 액션 생성
	FDiplomacyAction Action;
	Action.Action = EDiplomacyActionType::SendGift;
	Action.FromPlayerId = 0; // 플레이어 0
	Action.ToPlayerId = CurrentTargetPlayerIndex;

	// 액션 발행
	int32 ActionId = DiplomacyManager->IssueAction(Action);
	
	if (ActionId == -1)
	{
	}
	else
	{
		// 버튼 상태 업데이트 (쿨다운 반영)
		UpdateButtonStates();
		// 외교 관계 박스 갱신 (GoodHB/BadHB 등)
		UpdateRelationBoxes();
	}
}

void UDiplomacyUI::OnOfferAllianceBtnClicked()
{
	if (CurrentTargetPlayerIndex < 0)
	{
		return;
	}

	UDiplomacyManager* DiplomacyManager = GetDiplomacyManager();
	if (!DiplomacyManager)
	{
		return;
	}

	// 외교 액션 생성
	FDiplomacyAction Action;
	Action.Action = EDiplomacyActionType::OfferAlliance;
	Action.FromPlayerId = 0; // 플레이어 0
	Action.ToPlayerId = CurrentTargetPlayerIndex;

	// 액션 발행
	int32 ActionId = DiplomacyManager->IssueAction(Action);
	
	if (ActionId == -1)
	{
	}
	else
	{
		// 버튼 상태 업데이트
		UpdateButtonStates();
	}
}

void UDiplomacyUI::OnOfferPeaceBtnClicked()
{
	if (CurrentTargetPlayerIndex < 0)
	{
		return;
	}

	UDiplomacyManager* DiplomacyManager = GetDiplomacyManager();
	if (!DiplomacyManager)
	{
		return;
	}

	// 외교 액션 생성
	FDiplomacyAction Action;
	Action.Action = EDiplomacyActionType::OfferPeace;
	Action.FromPlayerId = 0; // 플레이어 0
	Action.ToPlayerId = CurrentTargetPlayerIndex;

	// 액션 발행
	int32 ActionId = DiplomacyManager->IssueAction(Action);
	
	if (ActionId == -1)
	{
	}
	else
	{
		// 버튼 상태 업데이트
		UpdateButtonStates();
	}
}

void UDiplomacyUI::OnDenounceBtnClicked()
{
	if (CurrentTargetPlayerIndex < 0)
	{
		return;
	}

	UDiplomacyManager* DiplomacyManager = GetDiplomacyManager();
	if (!DiplomacyManager)
	{
		return;
	}

	// 외교 액션 생성
	FDiplomacyAction Action;
	Action.Action = EDiplomacyActionType::Denounce;
	Action.FromPlayerId = 0; // 플레이어 0
	Action.ToPlayerId = CurrentTargetPlayerIndex;

	// 액션 발행
	int32 ActionId = DiplomacyManager->IssueAction(Action);
	
	if (ActionId == -1)
	{
	}
	else
	{
		// 버튼 상태 업데이트 (쿨다운 반영)
		UpdateButtonStates();
		// 외교 관계 박스 갱신 (GoodHB/BadHB 등)
		UpdateRelationBoxes();
	}
}

void UDiplomacyUI::OnDeclareWarBtnClicked()
{
	if (CurrentTargetPlayerIndex < 0)
	{
		return;
	}

	UDiplomacyManager* DiplomacyManager = GetDiplomacyManager();
	if (!DiplomacyManager)
	{
		return;
	}

	// 외교 액션 생성
	FDiplomacyAction Action;
	Action.Action = EDiplomacyActionType::DeclareWar;
	Action.FromPlayerId = 0; // 플레이어 0
	Action.ToPlayerId = CurrentTargetPlayerIndex;

	// 액션 발행
	int32 ActionId = DiplomacyManager->IssueAction(Action);
	
	if (ActionId == -1)
	{
	}
	else
	{
		// 버튼 상태 업데이트 (외교 상태 변경 반영)
		UpdateButtonStates();
		// 외교 관계 박스 갱신 (WarHB/AllyHB 등)
		UpdateRelationBoxes();
	}
}

void UDiplomacyUI::UpdateButtonStates()
{
	if (CurrentTargetPlayerIndex < 0)
	{
		return;
	}

	// ========== 패배한 플레이어는 모든 버튼 비활성화 ==========
	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (GameInstance)
	{
		ASuperPlayerState* TargetPlayerState = GameInstance->GetPlayerState(CurrentTargetPlayerIndex);
		if (TargetPlayerState && TargetPlayerState->bIsDefeated)
		{
			// 패배한 플레이어와는 외교 불가
			if (SendGiftBtn) SendGiftBtn->SetIsEnabled(false);
			if (OfferAllianceBtn) OfferAllianceBtn->SetIsEnabled(false);
			if (OfferPeaceBtn) OfferPeaceBtn->SetIsEnabled(false);
			if (DenounceBtn) DenounceBtn->SetIsEnabled(false);
			if (DeclareWarBtn) DeclareWarBtn->SetIsEnabled(false);

			// StateWithMeTxt 업데이트
			if (StateWithMeTxt)
			{
				StateWithMeTxt->SetText(FText::FromString(TEXT("敗北")));
				StateWithMeTxt->SetColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f)); // 회색
			}

			// AttitudeWithMeTxt 숨김
			if (AttitudeWithMeTxt)
			{
				AttitudeWithMeTxt->SetText(FText::FromString(TEXT("-")));
				AttitudeWithMeTxt->SetColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f, 1.0f)); // 회색
			}

			return;
		}
	}

	UDiplomacyManager* DiplomacyManager = GetDiplomacyManager();
	if (!DiplomacyManager)
	{
		// DiplomacyManager가 없으면 모든 버튼 비활성화
		if (SendGiftBtn) SendGiftBtn->SetIsEnabled(false);
		if (OfferAllianceBtn) OfferAllianceBtn->SetIsEnabled(false);
		if (OfferPeaceBtn) OfferPeaceBtn->SetIsEnabled(false);
		if (DenounceBtn) DenounceBtn->SetIsEnabled(false);
		if (DeclareWarBtn) DeclareWarBtn->SetIsEnabled(false);
		return;
	}

	// 현재 라운드 가져오기
	int32 CurrentRound = GetCurrentRound();

	// PairState 정보 가져오기 (쿨다운 확인용)
	FDiplomacyPairKey PairKey(0, CurrentTargetPlayerIndex);
	const FDiplomacyPairState* State = DiplomacyManager->PairStates.Find(PairKey);

	// DeclareWarBtn 활성화 조건
	bool bCanDeclareWar = true;
	if (State)
	{
		// 동맹 상태면 전쟁 선포 불가
		if (State->Status == EDiplomacyStatusType::Alliance)
		{
			bCanDeclareWar = false;
		}
		// 마지막 전쟁/평화 상태 변경 후 10라운드가 지나지 않았으면 불가
		const int32 LastStatusRound = FMath::Max(State->LastWarRound, State->LastPeaceRound);
		if (LastStatusRound > 0 && (CurrentRound - LastStatusRound) < 10)
		{
			bCanDeclareWar = false;
		}
	}
	if (DeclareWarBtn)
	{
		DeclareWarBtn->SetIsEnabled(bCanDeclareWar);
	}

	// OfferPeaceBtn 활성화 조건
	bool bCanOfferPeace = false;
	if (State)
	{
		// 전쟁 중이어야 함
		if (State->Status == EDiplomacyStatusType::War)
		{
			// 마지막 전쟁/평화 상태 변경 후 10라운드가 지나야 함
			const int32 LastStatusRound = FMath::Max(State->LastWarRound, State->LastPeaceRound);
			if (LastStatusRound == 0 || (CurrentRound - LastStatusRound) >= 10)
			{
				// 마지막 평화 제안 후 5라운드가 지나야 함
				if (State->LastOfferPeaceRound == 0 || (CurrentRound - State->LastOfferPeaceRound) >= 5)
				{
					bCanOfferPeace = true;
				}
			}
		}
	}
	if (OfferPeaceBtn)
	{
		OfferPeaceBtn->SetIsEnabled(bCanOfferPeace);
	}

	// OfferAllianceBtn 활성화 조건: 동맹이 아니면서 전쟁 중이 아니어야 함
	bool bCanOfferAlliance = true;
	if (State)
	{
		if (State->Status == EDiplomacyStatusType::Alliance)
		{
			bCanOfferAlliance = false;
		}
		if (State->Status == EDiplomacyStatusType::War)
		{
			bCanOfferAlliance = false;
		}
		// 마지막 동맹 제안 후 5라운드가 지나지 않았으면 불가
		if (State->LastOfferAllianceRound > 0 && (CurrentRound - State->LastOfferAllianceRound) < 5)
		{
			bCanOfferAlliance = false;
		}
	}
	if (OfferAllianceBtn)
	{
		OfferAllianceBtn->SetIsEnabled(bCanOfferAlliance);
	}

	// DenounceBtn 활성화 조건
	bool bCanDenounce = true;
	if (State && State->LastDenounceRound > 0)
	{
		// 마지막 비난 후 3라운드가 지나지 않았으면 불가
		if ((CurrentRound - State->LastDenounceRound) < 3)
		{
			bCanDenounce = false;
		}
	}
	if (DenounceBtn)
	{
		DenounceBtn->SetIsEnabled(bCanDenounce);
	}

	// SendGiftBtn 활성화 조건
	bool bCanSendGift = true;
	if (State && State->LastGiftRound > 0)
	{
		// 마지막 선물 후 3라운드가 지나지 않았으면 불가
		if ((CurrentRound - State->LastGiftRound) < 3)
		{
			bCanSendGift = false;
		}
	}
	if (SendGiftBtn)
	{
		SendGiftBtn->SetIsEnabled(bCanSendGift);
	}

	// StateWithMeTxt 업데이트 (외교 상태 표시)
	if (StateWithMeTxt)
	{
		FString StateText = TEXT("");
		FLinearColor StateColor = FLinearColor::White;
		EDiplomacyStatusType Status = DiplomacyManager->GetStatus(0, CurrentTargetPlayerIndex);
		
		if (Status == EDiplomacyStatusType::War)
		{
			StateText = TEXT("戦争中");
			StateColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // 빨간색
		}
		else if (Status == EDiplomacyStatusType::Alliance)
		{
			StateText = TEXT("同盟");
			StateColor = FLinearColor(0.5f, 0.8f, 1.0f, 1.0f); // 하늘색
		}
		else
		{
			// 호감도에 따라 표시 (20 이상 우호, -20 이하 적대, 그 외 중립)
			int32 Attitude = DiplomacyManager->GetAttitude(0, CurrentTargetPlayerIndex);
			if (Attitude <= -20)
			{
				StateText = TEXT("敵対的");
				StateColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // 빨간색
			}
			else if (Attitude >= -19 && Attitude <= 19)
			{
				StateText = TEXT("中立的");
				StateColor = FLinearColor(1.0f, 0.65f, 0.0f, 1.0f); // 주황색
			}
			else if (Attitude >= 20)
			{
				StateText = TEXT("友好的");
				StateColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // 초록색
			}
			else
			{
				StateText = TEXT("中立的");
				StateColor = FLinearColor(1.0f, 0.65f, 0.0f, 1.0f); // 주황색
			}
		}
		
		StateWithMeTxt->SetText(FText::FromString(StateText));
		StateWithMeTxt->SetColorAndOpacity(StateColor);
	}

	// AttitudeWithMeTxt 업데이트 (호감도 숫자 표시)
	if (AttitudeWithMeTxt)
	{
		int32 Attitude = DiplomacyManager->GetAttitude(0, CurrentTargetPlayerIndex);
		FString AttitudeText = FString::Printf(TEXT("%d"), Attitude);
		
		FLinearColor AttitudeColor = FLinearColor::White;
		if (Attitude <= -20)
		{
			AttitudeColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // 빨간색
		}
		else if (Attitude >= -19 && Attitude <= 19)
		{
			AttitudeColor = FLinearColor(1.0f, 0.65f, 0.0f, 1.0f); // 주황색
		}
		else if (Attitude >= 20)
		{
			AttitudeColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // 초록색
		}
		
		AttitudeWithMeTxt->SetText(FText::FromString(AttitudeText));
		AttitudeWithMeTxt->SetColorAndOpacity(AttitudeColor);
	}
}

void UDiplomacyUI::OnCloseBtnClicked()
{
	// UI 숨기기 (델리게이트가 바인딩되지 않아도 작동하도록)
	SetVisibility(ESlateVisibility::Hidden);
	
	// 델리게이트 브로드캐스트 (MainHUD에서 상태 플래그 리셋)
	OnDiplomacyUICloseButtonClicked.Broadcast();
}

