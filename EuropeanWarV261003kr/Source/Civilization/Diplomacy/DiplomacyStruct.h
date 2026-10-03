// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiplomacyStruct.generated.h"

// ========== 외교 액션 ==========

UENUM(BlueprintType)
enum class EDiplomacyActionType : uint8
{
	// 없음
	None			UMETA(DisplayName = "None"),
	// 전쟁 선포
	DeclareWar		UMETA(DisplayName = "Declare War"),
	// 평화 제안
	OfferPeace		UMETA(DisplayName = "Offer Peace"),
	// 동맹 제안
	OfferAlliance	UMETA(DisplayName = "Offer Alliance"),
	// 국제적 비난
	Denounce		UMETA(DisplayName = "Denounce"),
	// 외교 선물
	SendGift		UMETA(DisplayName = "Send Gift"),
};

// ========== 외교 상태 ==========

UENUM(BlueprintType)
enum class EDiplomacyStatusType : uint8
{
	// 없음
	None		UMETA(DisplayName = "None"),
	// 전쟁
	War			UMETA(DisplayName = "War"),
	// 평화
	Peace		UMETA(DisplayName = "Peace"),
	// 동맹
	Alliance	UMETA(DisplayName = "Alliance"),
};

// ========== 플레이어 쌍 키 ==========

USTRUCT(BlueprintType)
struct CIVILIZATION_API FDiplomacyPairKey
{
	GENERATED_BODY()

public:
	// 쌍의 작은 슬롯 인덱스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 PlayerA = 0;

	// 쌍의 큰 슬롯 인덱스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 PlayerB = 0;

	FDiplomacyPairKey() = default;

	// 두 슬롯을 정렬해 키를 만듭니다.
	FDiplomacyPairKey(int32 InPlayerA, int32 InPlayerB)
	{
		Set(InPlayerA, InPlayerB);
	}

	// PlayerA가 항상 PlayerB 이하가 되도록 정렬해 넣습니다.
	void Set(int32 InPlayerA, int32 InPlayerB)
	{
		if (InPlayerA <= InPlayerB)
		{
			PlayerA = InPlayerA;
			PlayerB = InPlayerB;
		}
		else
		{
			PlayerA = InPlayerB;
			PlayerB = InPlayerA;
		}
	}

	// 두 슬롯이 같으면 true입니다.
	bool operator==(const FDiplomacyPairKey& Other) const
	{
		return PlayerA == Other.PlayerA && PlayerB == Other.PlayerB;
	}
};

// 플레이어 쌍을 TMap 키로 쓰기 위한 해시입니다.
FORCEINLINE uint32 GetTypeHash(const FDiplomacyPairKey& Key)
{
	return HashCombine(::GetTypeHash(Key.PlayerA), ::GetTypeHash(Key.PlayerB));
}

// ========== 쌍 상태 ==========

USTRUCT(BlueprintType)
struct CIVILIZATION_API FDiplomacyPairState
{
	GENERATED_BODY()

public:
	// 두 슬롯 사이의 전쟁·평화·동맹 상태
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	EDiplomacyStatusType Status = EDiplomacyStatusType::None;

	// 마지막으로 전쟁 상태가 된 라운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 LastWarRound = 0;

	// 마지막으로 평화 상태가 된 라운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 LastPeaceRound = 0;

	// 마지막으로 동맹 상태가 된 라운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 LastAllianceRound = 0;

	// 마지막으로 비난한 라운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 LastDenounceRound = 0;

	// 마지막으로 선물한 라운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 LastGiftRound = 0;

	// 마지막으로 평화를 제안한 라운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 LastOfferPeaceRound = 0;

	// 마지막으로 동맹을 제안한 라운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 LastOfferAllianceRound = 0;
};

// ========== 외교 액션 데이터 ==========

USTRUCT(BlueprintType)
struct CIVILIZATION_API FDiplomacyAction
{
	GENERATED_BODY()

public:
	// 발행된 외교 액션 종류
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	EDiplomacyActionType Action = EDiplomacyActionType::None;

	// 액션을 보낸 문명 슬롯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 FromPlayerId = 0;

	// 액션을 받는 문명 슬롯
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 ToPlayerId = 0;

	// 요청·선언이 발생한 라운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 IssuedRound = 0;

	// 외교 액션 고유 ID. DiplomacyManager가 부여합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 ActionId = -1;
};
