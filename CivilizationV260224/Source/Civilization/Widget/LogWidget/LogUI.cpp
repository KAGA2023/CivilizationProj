// Fill out your copyright notice in the Description page of Project Settings.

#include "LogUI.h"
#include "Components/TextBlock.h"

void ULogUI::AppendRoundLine(int32 RoundNumber)
{
	if (!LogTxt)
	{
		return;
	}
	FString NewLine = FString::Printf(TEXT("----------Round: %d----------"), RoundNumber);
	FString Current = LogTxt->GetText().ToString();
	if (Current.IsEmpty())
	{
		LogTxt->SetText(FText::FromString(NewLine));
	}
	else
	{
		LogTxt->SetText(FText::FromString(Current + TEXT("\n") + NewLine));
	}
}

void ULogUI::AppendLine(const FString& Text)
{
	if (!LogTxt || Text.IsEmpty())
	{
		return;
	}
	FString Current = LogTxt->GetText().ToString();
	if (Current.IsEmpty())
	{
		LogTxt->SetText(FText::FromString(Text));
	}
	else
	{
		LogTxt->SetText(FText::FromString(Current + TEXT("\n") + Text));
	}
}
