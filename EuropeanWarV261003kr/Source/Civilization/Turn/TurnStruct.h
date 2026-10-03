// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TurnStruct.generated.h"

USTRUCT(BlueprintType)
struct CIVILIZATION_API FTurnStruct
{
	GENERATED_BODY()

	// ========== 턴 정보 ==========

	// 라운드 번호. 1부터 시작합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Info")
	int32 RoundNumber = 1;

	// 라운드 안의 턴 번호. 1=슬롯0, 2=슬롯1, ... 순서입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Info")
	int32 TurnNumber = 1;

	// 지금 턴인 문명 슬롯. 0=호스트/싱글, 1=멀티 참가자, 나머지는 AI.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Info")
	int32 PlayerIndex = 0;
};
