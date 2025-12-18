// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DiplomacyStruct.generated.h"

// 외교 액션 타입 (요청/선언/제안 등)
UENUM(BlueprintType)
enum class EDiplomacyActionType : uint8
{
	DeclareWar			UMETA(DisplayName = "Declare War"),			// 전쟁 선포
	OfferPeace			UMETA(DisplayName = "Offer Peace"),			// 평화 제안
	Denounce			UMETA(DisplayName = "Denounce"),			// 국제적 비난
	Praise				UMETA(DisplayName = "Praise"),				// 국제적 찬사
	SendGift			UMETA(DisplayName = "Send Gift"),			// 외교 선물
	OfferAlliance		UMETA(DisplayName = "Offer Alliance"),		// 동맹 제안
	OfferNonAggression	UMETA(DisplayName = "Offer Non-Aggression"),// 불가침 조약 제안
	OfferOpenBorders	UMETA(DisplayName = "Offer Open Borders"),	// 국경 개방 제안
};

// 상태 타입 (턴에 제약없는 공통 상태)
UENUM(BlueprintType)
enum class EDiplomacyStatusType : uint8
{
	Peace	UMETA(DisplayName = "Peace"),	// 평화
	War		UMETA(DisplayName = "War"),		// 전쟁
};

// 조약 타입 (턴에 제약있는 공통 상태)
UENUM(BlueprintType)
enum class EDiplomacyTreatyType : uint8
{
	Alliance			UMETA(DisplayName = "Alliance"),		// 동맹
	NonAggression		UMETA(DisplayName = "NonAggression"),	// 불가침 조약
	OpenBorders			UMETA(DisplayName = "Open Borders"),	// 국경 개방
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

// 라운드에 제약있는 조약 데이터 (영구 조약 없음)
USTRUCT(BlueprintType)
struct CIVILIZATION_API FDiplomacyTreaty
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	EDiplomacyTreatyType Treaty = EDiplomacyTreatyType::OpenBorders;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 StartRound = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 EndRound = -1;

	// 조약이 현재 라운드에 활성화되어 있는지 여부
	bool IsActiveAtRound(int32 CurrentRound) const
	{
		if (StartRound < 0 || EndRound < 0)
		{
			return false;
		}
		if (CurrentRound < StartRound)
		{
			return false;
		}
		return CurrentRound <= EndRound;
	}
};

// 플레이어 쌍(A<->B)의 공통 상태/조약 데이터
USTRUCT(BlueprintType)
struct CIVILIZATION_API FDiplomacyPairState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	EDiplomacyStatusType Status = EDiplomacyStatusType::Peace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	TArray<FDiplomacyTreaty> Treaties;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	int32 LastStatusChangedRound = 0;

	// 현재 라운드에 활성화되어 있는 조약이 있는지 여부
	bool HasActiveTreaty(EDiplomacyTreatyType Treaty, int32 CurrentRound) const
	{
		for (const FDiplomacyTreaty& TreatyData : Treaties)
		{
			if (TreatyData.Treaty == Treaty && TreatyData.IsActiveAtRound(CurrentRound))
			{
				return true;
			}
		}
		return false;
	}
};

// 외교 액션(요청/선언/제안) 데이터
USTRUCT(BlueprintType)
struct CIVILIZATION_API FDiplomacyAction
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy")
	EDiplomacyActionType Action = EDiplomacyActionType::SendDelegation;

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
