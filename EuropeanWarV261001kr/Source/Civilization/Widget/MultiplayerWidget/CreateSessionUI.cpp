// Fill out your copyright notice in the Description page of Project Settings.

#include "CreateSessionUI.h"
#include "../../Multiplayer/MultiplayerSessionSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UCreateSessionUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (CreateSessionBtn)
	{
		CreateSessionBtn->OnClicked.AddDynamic(this, &UCreateSessionUI::OnCreateSessionButtonClicked);
	}

	if (WarningTxt)
	{
		WarningTxt->SetVisibility(ESlateVisibility::Hidden);
	}
}

UMultiplayerSessionSubsystem* UCreateSessionUI::GetSessionSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
	}
	return nullptr;
}

void UCreateSessionUI::SetCreateSessionButtonEnabled(bool bEnabled)
{
	if (CreateSessionBtn)
	{
		CreateSessionBtn->SetIsEnabled(bEnabled);
	}
}

void UCreateSessionUI::SetWarningText(const FString& Message)
{
	if (!WarningTxt)
	{
		return;
	}

	if (Message.IsEmpty())
	{
		WarningTxt->SetText(FText::GetEmpty());
		WarningTxt->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	WarningTxt->SetText(FText::FromString(Message));
	WarningTxt->SetVisibility(ESlateVisibility::Visible);
}

void UCreateSessionUI::OnCreateSessionButtonClicked()
{
	UMultiplayerSessionSubsystem* SessionSubsystem = GetSessionSubsystem();
	if (!SessionSubsystem)
	{
		SetWarningText(TEXT("세션 시스템에 연결할 수 없습니다!"));
		return;
	}

	SetWarningText(FString());
	SetCreateSessionButtonEnabled(false);

	// Null OSS는 메뉴 월드에서 만든 세션이 맵 이동 후 안 보이는 경우가 있어
	// listen 로비로 먼저 이동한 뒤, 로비에서 세션을 만듭니다.
	if (!SessionSubsystem->TravelToLobbyAsHost())
	{
		SetCreateSessionButtonEnabled(true);
		SetWarningText(TEXT("로비로 이동하지 못했습니다!"));
	}
}
