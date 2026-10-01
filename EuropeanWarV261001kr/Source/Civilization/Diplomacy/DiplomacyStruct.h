// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiplomacyStruct.generated.h"

// 외교 액션 타입 (요청/선언/제안 등)
UENUM(BlueprintType)
enum class EDiplomacyActionType : uint8
{
	None			UMETA(DisplayName = "None"),			// 없음
	DeclareWar		UMETA(DisplayName = "Declare War"),		// 전쟁 선포
	OfferPeace		UMETA(DisplayName = "Offer Peace"),		// 평화 제안
	OfferAlliance	UMETA(DisplayName = "Offer Alliance"),	// 동맹 제안
	Denounce		UMETA(DisplayName = "Denounce"),		// 국제적 비난
	SendGift		UMETA(DisplayName = "Send Gift"),		// 외교 선물
};

// 상태 타입 (턴에 제약없는 공통 상태)
UENUM(BlueprintType)
enum class EDiplomacyStatusType : uint8
{
	None		UMETA(DisplayName = "None"),		// 없음
	War			UMETA(DisplayName = "War"),			// 전쟁
	Peace		UMETA(DisplayName = "Peace"),		// 평화
	Alliance	UMETA(DisplayName = "Alliance"),	// 동맹
};

// 플레이어 쌍 키
USTRUCT(BlueprintType)
struct CIVILIZATION_API FDiplomacyPairKey
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 PlayerA = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 PlayerB = 0;

	FDiplomacyPairKey() = default;
	FDiplomacyPairKey(int32 InPlayerA, int32 InPlayerB)
	{
		Set(InPlayerA, InPlayerB);
	}

	// 항상 PlayerA <= PlayerB가 되도록 정렬
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

	bool operator==(const FDiplomacyPairKey& Other) const
	{
		return PlayerA == Other.PlayerA && PlayerB == Other.PlayerB;
	}
};

// 플레이어 쌍을 TMap의 키로 사용하기 위해 해시 함수 정의
FORCEINLINE uint32 GetTypeHash(const FDiplomacyPairKey& Key)
{
	return HashCombine(::GetTypeHash(Key.PlayerA), ::GetTypeHash(Key.PlayerB));
}

// 플레이어 쌍(A<->B)의 공통 상태 데이터
USTRUCT(BlueprintType)
struct CIVILIZATION_API FDiplomacyPairState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	EDiplomacyStatusType Status = EDiplomacyStatusType::None;

	// 마지막 전쟁상태 된 라운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 LastWarRound = 0;

	// 마지막 평화상태 된 라운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 LastPeaceRound = 0;

	// 마지막 동맹상태 된 라운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 LastAllianceRound = 0;

	// 마지막 비난한 라운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 LastDenounceRound = 0;

	// 마지막 선물한 라운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 LastGiftRound = 0;

	// 마지막 평화 제안한 라운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 LastOfferPeaceRound = 0;

	// 마지막 동맹 제안한 라운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 LastOfferAllianceRound = 0;
};

// 외교 액션(요청/선언/제안) 데이터
USTRUCT(BlueprintType)
struct CIVILIZATION_API FDiplomacyAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	EDiplomacyActionType Action = EDiplomacyActionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 FromPlayerId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 ToPlayerId = 0;

	// 요청/선언이 발생한 라운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 IssuedRound = 0;

	// 외교 액션 고유 ID (UDiplomacyManager에서 부여)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 ActionId = -1;
};
