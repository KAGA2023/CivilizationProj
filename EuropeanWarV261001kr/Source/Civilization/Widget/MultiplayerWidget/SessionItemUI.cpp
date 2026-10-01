// Fill out your copyright notice in the Description page of Project Settings.

#include "SessionItemUI.h"
#include "../../Multiplayer/MultiplayerSessionSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void USessionItemUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (JoinBtn)
	{
		JoinBtn->OnClicked.AddDynamic(this, &USessionItemUI::OnJoinButtonClicked);
	}

	if (UMultiplayerSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->OnJoinSessionComplete.AddDynamic(this, &USessionItemUI::OnJoinSessionComplete);
	}

	RefreshSessionInfo();
}

void USessionItemUI::NativeDestruct()
{
	if (UMultiplayerSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->OnJoinSessionComplete.RemoveDynamic(this, &USessionItemUI::OnJoinSessionComplete);
	}

	Super::NativeDestruct();
}

UMultiplayerSessionSubsystem* USessionItemUI::GetSessionSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
	}
	return nullptr;
}

void USessionItemUI::SetSearchResult(const FBlueprintSessionResult& InSearchResult)
{
	SearchResult = InSearchResult;
	RefreshSessionInfo();
}

void USessionItemUI::RefreshSessionInfo()
{
	const FOnlineSessionSearchResult& OnlineResult = SearchResult.OnlineResult;
	FString ServerName = OnlineResult.Session.OwningUserName;
	if (ServerName.IsEmpty())
	{
		ServerName = TEXT("알 수 없는 방");
	}

	const int32 MaxPlayers = OnlineResult.Session.SessionSettings.NumPublicConnections;
	const int32 OpenSlots = OnlineResult.Session.NumOpenPublicConnections;
	const int32 CurrentPlayers = FMath::Max(0, MaxPlayers - OpenSlots);

	if (ServerNameTxt)
	{
		ServerNameTxt->SetText(FText::FromString(ServerName));
	}

	if (PlayerCountTxt)
	{
		PlayerCountTxt->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentPlayers, MaxPlayers)));
	}
}

void USessionItemUI::OnJoinButtonClicked()
{
	UMultiplayerSessionSubsystem* SessionSubsystem = GetSessionSubsystem();
	if (!SessionSubsystem)
	{
		return;
	}

	if (JoinBtn)
	{
		JoinBtn->SetIsEnabled(false);
	}

	SessionSubsystem->JoinSession(GetOwningPlayer(), SearchResult);
}

void USessionItemUI::OnJoinSessionComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		return;
	}

	if (JoinBtn)
	{
		JoinBtn->SetIsEnabled(true);
	}
}
