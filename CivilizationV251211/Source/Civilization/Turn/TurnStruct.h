// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TurnStruct.generated.h"

/**
 * 라운드 및 턴 정보 구조체
 * 라운드와 라운드 내에서 각 플레이어의 턴을 나타냅니다.
 * 
 * 예시:
 * - Round 1, Turn 1: Player 0의 턴
 * - Round 1, Turn 2: AI 1의 턴
 * - Round 1, Turn 3: AI 2의 턴
 * - Round 1, Turn 4: AI 3의 턴
 * - Round 2, Turn 1: Player 0의 턴
 */
USTRUCT(BlueprintType)
struct CIVILIZATION_API FTurnStruct
{
	GENERATED_BODY()

	// 라운드 번호 (1부터 시작)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Info")
	int32 RoundNumber = 1;

	// 라운드 내에서의 턴 번호 (1~4: Player0, AI1, AI2, AI3 순서)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Info")
	int32 TurnNumber = 1;

	// 현재 턴의 플레이어 인덱스 (0=Player, 1~3=AI)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Info")
	int32 PlayerIndex = 0;
};

