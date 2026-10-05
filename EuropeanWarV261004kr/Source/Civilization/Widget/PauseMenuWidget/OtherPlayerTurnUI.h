// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OtherPlayerTurnUI.generated.h"

// 다른 플레이어 턴임을 알려주는 위젯입니다.
UCLASS()
class CIVILIZATION_API UOtherPlayerTurnUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
};
