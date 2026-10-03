// Fill out your copyright notice in the Description page of Project Settings.

#include "FindSessionUI.h"
#include "SessionItemUI.h"
#include "../../Multiplayer/MultiplayerSessionSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void UFindSessionUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (FindSessionBtn)
	{
		FindSessionBtn->OnClicked.AddDynamic(this, &UFindSessionUI::OnFindSessionButtonClicked);
	}

	if (SessionMessageTxt)
	{
		SessionMessageTxt->SetText(FText::GetEmpty());
		SessionMessageTxt->SetVisibility(ESlateVisibility::Hidden);
	}

	if (!SessionItemWidgetClass)
	{
		SessionItemWidgetClass = ResolveSessionItemWidgetClass();
	}

	if (UMultiplayerSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->OnFindSessionsComplete.AddDynamic(this, &UFindSessionUI::OnFindSessionsComplete);
	}
}

void UFindSessionUI::NativeDestruct()
{
	if (UMultiplayerSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->OnFindSessionsComplete.RemoveDynamic(this, &UFindSessionUI::OnFindSessionsComplete);
	}

	Super::NativeDestruct();
}

UMultiplayerSessionSubsystem* UFindSessionUI::GetSessionSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
	}
	return nullptr;
}

void UFindSessionUI::SetFindSessionButtonEnabled(bool bEnabled)
{
	if (FindSessionBtn)
	{
		FindSessionBtn->SetIsEnabled(bEnabled);
	}
}

void UFindSessionUI::SetSessionMessage(const FString& Message)
{
	if (!SessionMessageTxt)
	{
		return;
	}

	if (Message.IsEmpty())
	{
		SessionMessageTxt->SetText(FText::GetEmpty());
		SessionMessageTxt->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	SessionMessageTxt->SetText(FText::FromString(Message));
	SessionMessageTxt->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

TSubclassOf<USessionItemUI> UFindSessionUI::ResolveSessionItemWidgetClass()
{
	if (SessionItemWidgetClass)
	{
		return SessionItemWidgetClass;
	}

	return LoadClass<USessionItemUI>(nullptr, TEXT("/Game/Civilization/Widget/MultiplayerWidget/W_SessionItem.W_SessionItem_C"));
}

void UFindSessionUI::AddItemRenderer(const FBlueprintSessionResult& SearchResult)
{
	TSubclassOf<USessionItemUI> ItemClass = ResolveSessionItemWidgetClass();
	if (!SessionList || !ItemClass)
	{
		return;
	}

	USessionItemUI* SessionItem = CreateWidget<USessionItemUI>(this, ItemClass);
	if (!SessionItem)
	{
		return;
	}

	SessionItem->SetSearchResult(SearchResult);
	SessionItem->SetVisibility(ESlateVisibility::Visible);
	if (UVerticalBoxSlot* ItemSlot = SessionList->AddChildToVerticalBox(SessionItem))
	{
		ItemSlot->SetHorizontalAlignment(HAlign_Fill);
		ItemSlot->SetPadding(FMargin(0.f, 4.f));
	}
}

FString UFindSessionUI::GetSessionResultMessage(int32 SessionCount) const
{
	if (SessionCount <= 0)
	{
		return TEXT("찾은 세션이 없습니다");
	}

	return FString::Printf(TEXT("찾은 로비: %d개"), SessionCount);
}

void UFindSessionUI::OnFindSessionButtonClicked()
{
	UMultiplayerSessionSubsystem* SessionSubsystem = GetSessionSubsystem();
	if (!SessionSubsystem)
	{
		SetSessionMessage(TEXT("로비 시스템에 연결할 수 없습니다!"));
		return;
	}

	if (SessionList)
	{
		SessionList->ClearChildren();
	}

	SetFindSessionButtonEnabled(false);
	SetSessionMessage(TEXT("로비 검색 중..."));
	SessionSubsystem->FindSessions();
}

void UFindSessionUI::OnFindSessionsComplete(bool bWasSuccessful)
{
	SetFindSessionButtonEnabled(true);

	UMultiplayerSessionSubsystem* SessionSubsystem = GetSessionSubsystem();
	if (!bWasSuccessful || !SessionSubsystem)
	{
		SetSessionMessage(TEXT("로비 검색에 실패했습니다!"));
		return;
	}

	const TArray<FBlueprintSessionResult>& SessionResults = SessionSubsystem->GetLastSessionResults();
	SetSessionMessage(GetSessionResultMessage(SessionResults.Num()));

	if (SessionResults.Num() > 0 && !ResolveSessionItemWidgetClass())
	{
		SetSessionMessage(TEXT("로비는 찾았지만 목록 위젯이 없습니다. SessionItemWidgetClass를 지정하세요."));
		return;
	}

	if (SessionResults.Num() > 0 && !SessionList)
	{
		SetSessionMessage(TEXT("로비는 찾았지만 SessionList가 없습니다."));
		return;
	}

	for (const FBlueprintSessionResult& SearchResult : SessionResults)
	{
		AddItemRenderer(SearchResult);
	}
}
