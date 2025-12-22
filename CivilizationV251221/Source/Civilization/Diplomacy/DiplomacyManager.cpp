// Fill out your copyright notice in the Description page of Project Settings.

#include "DiplomacyManager.h"

UDiplomacyManager::UDiplomacyManager()
{
}

void UDiplomacyManager::Initialize(int32 NumPlayers)
{
	PlayerCount = NumPlayers;

	// 기존 데이터 초기화
	PairStates.Empty();
	Attitudes.Empty();

	// 플레이어 쌍(A <-> B)의 기본 외교 상태 설정
	for (int32 PlayerA = 0; PlayerA < NumPlayers; ++PlayerA)
	{
		for (int32 PlayerB = PlayerA + 1; PlayerB < NumPlayers; ++PlayerB)
		{
			FDiplomacyPairKey PairKey(PlayerA, PlayerB);
			FDiplomacyPairState PairState;

			PairState.Status = EDiplomacyStatusType::None;
			PairState.LastWarRound = 0;
			PairState.LastPeaceRound = 0;
			PairState.LastAllianceRound = 0;
			PairState.LastDenounceRound = 0;
			PairState.LastGiftRound = 0;

			PairStates.Add(PairKey, PairState);
		}
	}

	// 방향성 호감도 초기화 (자기 자신은 제외, 기본 0)
	for (int32 FromPlayer = 0; FromPlayer < NumPlayers; ++FromPlayer)
	{
		TMap<int32, int32>& ToMap = Attitudes.FindOrAdd(FromPlayer);

		for (int32 ToPlayer = 0; ToPlayer < NumPlayers; ++ToPlayer)
		{
			if (FromPlayer == ToPlayer)
			{
				continue;
			}

			ToMap.Add(ToPlayer, 0);
		}
	}
}

EDiplomacyStatusType UDiplomacyManager::GetStatus(int32 PlayerA, int32 PlayerB) const
{
	// 유효하지 않은 플레이어 인덱스면 기본값(None) 반환
	if (PlayerA == PlayerB || PlayerA < 0 || PlayerB < 0 || PlayerA >= PlayerCount || PlayerB >= PlayerCount)
	{
		return EDiplomacyStatusType::None;
	}

	FDiplomacyPairKey PairKey(PlayerA, PlayerB);

	if (const FDiplomacyPairState* FoundState = PairStates.Find(PairKey))
	{
		return FoundState->Status;
	}

	// 맵에 없으면 기본적으로 None 상태로 간주
	return EDiplomacyStatusType::None;
}

bool UDiplomacyManager::IsAtWar(int32 PlayerA, int32 PlayerB) const
{
	return GetStatus(PlayerA, PlayerB) == EDiplomacyStatusType::War;
}

bool UDiplomacyManager::DeclareWar(int32 PlayerA, int32 PlayerB, int32 CurrentRound)
{
	// 자기 자신에게 전쟁 선포 불가 및 인덱스 검증
	if (PlayerA == PlayerB || PlayerA < 0 || PlayerB < 0 || PlayerA >= PlayerCount || PlayerB >= PlayerCount)
	{
		return false;
	}

	FDiplomacyPairKey PairKey(PlayerA, PlayerB);
	FDiplomacyPairState* State = PairStates.Find(PairKey);

	if (!State)
	{
		// 존재하지 않으면 기본 값으로 생성
		FDiplomacyPairState NewState;
		NewState.Status = EDiplomacyStatusType::None;
		NewState.LastWarRound = 0;
		NewState.LastPeaceRound = 0;
		NewState.LastAllianceRound = 0;
		NewState.LastDenounceRound = 0;
		NewState.LastGiftRound = 0;

		State = &PairStates.Add(PairKey, NewState);
	}

	// 동맹 상태면 전쟁 선포 불가
	if (State->Status == EDiplomacyStatusType::Alliance)
	{
		return false;
	}

	// 이미 전쟁 상태면 변경 없음
	if (State->Status == EDiplomacyStatusType::War)
	{
		return false;
	}

	State->Status = EDiplomacyStatusType::War;
	State->LastWarRound = CurrentRound;

	// 전쟁 상태 변경 델리게이트 브로드캐스트
	OnDiplomacyStatusChanged.Broadcast(PlayerA, PlayerB, EDiplomacyStatusType::War);

	return true;
}

bool UDiplomacyManager::MakePeace(int32 PlayerA, int32 PlayerB, int32 CurrentRound)
{
	// 자기 자신과의 평화 체결은 의미 없으므로 무시
	if (PlayerA == PlayerB || PlayerA < 0 || PlayerB < 0 || PlayerA >= PlayerCount || PlayerB >= PlayerCount)
	{
		return false;
	}

	FDiplomacyPairKey PairKey(PlayerA, PlayerB);
	FDiplomacyPairState* State = PairStates.Find(PairKey);

	if (!State)
	{
		// 존재하지 않으면 기본 값으로 생성
		FDiplomacyPairState NewState;
		NewState.Status = EDiplomacyStatusType::None;
		NewState.LastWarRound = 0;
		NewState.LastPeaceRound = 0;
		NewState.LastAllianceRound = 0;
		NewState.LastDenounceRound = 0;
		NewState.LastGiftRound = 0;

		State = &PairStates.Add(PairKey, NewState);
	}

	// 이미 평화 상태면 변경 없음
	if (State->Status == EDiplomacyStatusType::Peace)
	{
		return false;
	}

	State->Status = EDiplomacyStatusType::Peace;
	State->LastPeaceRound = CurrentRound;

	// 평화 상태 변경 델리게이트 브로드캐스트
	OnDiplomacyStatusChanged.Broadcast(PlayerA, PlayerB, EDiplomacyStatusType::Peace);

	return true;
}

bool UDiplomacyManager::MakeAlliance(int32 PlayerA, int32 PlayerB, int32 CurrentRound)
{
	// 자기 자신과의 동맹은 의미 없으므로 무시
	if (PlayerA == PlayerB || PlayerA < 0 || PlayerB < 0 || PlayerA >= PlayerCount || PlayerB >= PlayerCount)
	{
		return false;
	}

	FDiplomacyPairKey PairKey(PlayerA, PlayerB);
	FDiplomacyPairState* State = PairStates.Find(PairKey);

	if (!State)
	{
		// 존재하지 않으면 기본 값으로 생성
		FDiplomacyPairState NewState;
		NewState.Status = EDiplomacyStatusType::None;
		NewState.LastWarRound = 0;
		NewState.LastPeaceRound = 0;
		NewState.LastAllianceRound = 0;
		NewState.LastDenounceRound = 0;
		NewState.LastGiftRound = 0;

		State = &PairStates.Add(PairKey, NewState);
	}

	// 이미 동맹 상태면 변경 없음
	if (State->Status == EDiplomacyStatusType::Alliance)
	{
		return false;
	}

	State->Status = EDiplomacyStatusType::Alliance;
	State->LastAllianceRound = CurrentRound;

	// 동맹 상태 변경 델리게이트 브로드캐스트
	OnDiplomacyStatusChanged.Broadcast(PlayerA, PlayerB, EDiplomacyStatusType::Alliance);

	return true;
}

void UDiplomacyManager::OnRoundStarted(int32 CurrentRound)
{
	// 이미 처리한 라운드면 중복 처리 방지
	if (CurrentRound <= LastProcessedRound)
	{
		return;
	}
	LastProcessedRound = CurrentRound;
	CachedCurrentRound = CurrentRound;

	// 동맹 만료 정리 (10라운드 지난 동맹은 자동으로 만료)
	for (TPair<FDiplomacyPairKey, FDiplomacyPairState>& Pair : PairStates)
	{
		FDiplomacyPairState& State = Pair.Value;
		const FDiplomacyPairKey& Key = Pair.Key;

		// 동맹 상태이고 10라운드가 지났으면 만료
		if (State.Status == EDiplomacyStatusType::Alliance && State.LastAllianceRound > 0)
		{
			const int32 AllianceDuration = CurrentRound - State.LastAllianceRound;
			if (AllianceDuration >= 10)
			{
				// 동맹 만료: 평화 상태로 변경
				State.Status = EDiplomacyStatusType::Peace;
				State.LastPeaceRound = CurrentRound;
				OnDiplomacyStatusChanged.Broadcast(Key.PlayerA, Key.PlayerB, EDiplomacyStatusType::Peace);
			}
		}
	}
}

int32 UDiplomacyManager::GetAttitude(int32 FromPlayerId, int32 ToPlayerId) const
{
	// 자기 자신에 대한 호감도는 0으로 고정
	if (FromPlayerId == ToPlayerId)
	{
		return 0;
	}

	// 인덱스 범위 검증
	if (FromPlayerId < 0 || ToPlayerId < 0 || FromPlayerId >= PlayerCount || ToPlayerId >= PlayerCount)
	{
		return 0;
	}

	const TMap<int32, int32>* ToMapPtr = Attitudes.Find(FromPlayerId);
	if (!ToMapPtr)
	{
		return 0;
	}

	const int32* ScorePtr = ToMapPtr->Find(ToPlayerId);
	if (!ScorePtr)
	{
		return 0;
	}

	// 안전하게 -100 ~ +100 범위로 클램프
	return FMath::Clamp(*ScorePtr, -100, 100);
}

void UDiplomacyManager::SetAttitude(int32 FromPlayerId, int32 ToPlayerId, int32 NewScore)
{
	// 자기 자신에 대한 호감도는 변경하지 않음
	if (FromPlayerId == ToPlayerId)
	{
		return;
	}

	// 인덱스 범위 검증
	if (FromPlayerId < 0 || ToPlayerId < 0 || FromPlayerId >= PlayerCount || ToPlayerId >= PlayerCount)
	{
		return;
	}

	// -100 ~ +100 범위로 클램프
	const int32 ClampedScore = FMath::Clamp(NewScore, -100, 100);

	// Attitudes[FromPlayerId][ToPlayerId]에 직접 설정
	TMap<int32, int32>& ToMap = Attitudes.FindOrAdd(FromPlayerId);
	int32& ScoreRef = ToMap.FindOrAdd(ToPlayerId);
	ScoreRef = ClampedScore;
}

void UDiplomacyManager::AddAttitude(int32 FromPlayerId, int32 ToPlayerId, int32 Delta)
{
	// 자기 자신에 대한 호감도는 변경하지 않음
	if (FromPlayerId == ToPlayerId)
	{
		return;
	}

	// 인덱스 범위 검증
	if (FromPlayerId < 0 || ToPlayerId < 0 || FromPlayerId >= PlayerCount || ToPlayerId >= PlayerCount)
	{
		return;
	}

	// Attitudes[FromPlayerId][ToPlayerId]에 직접 접근
	TMap<int32, int32>& ToMap = Attitudes.FindOrAdd(FromPlayerId);
	int32& ScoreRef = ToMap.FindOrAdd(ToPlayerId);
	ScoreRef = FMath::Clamp(ScoreRef + Delta, -100, 100);
}

int32 UDiplomacyManager::IssueAction(const FDiplomacyAction& Action)
{
	// 플레이어 인덱스 유효성 검사
	if (Action.FromPlayerId < 0 || Action.ToPlayerId < 0 ||
		Action.FromPlayerId >= PlayerCount || Action.ToPlayerId >= PlayerCount)
	{
		return -1;
	}

	// 자기 자신에게 보내는 액션은 허용하지 않음 (필요 시 변경 가능)
	if (Action.FromPlayerId == Action.ToPlayerId)
	{
		return -1;
	}

	FDiplomacyPairKey PairKey(Action.FromPlayerId, Action.ToPlayerId);
	FDiplomacyPairState* State = PairStates.Find(PairKey);

	// 쿨다운 체크
	switch (Action.Action)
	{
	case EDiplomacyActionType::DeclareWar:
	{
		// 동맹 상태면 전쟁 선포 불가
		if (State && State->Status == EDiplomacyStatusType::Alliance)
		{
			return -1;
		}
		// 마지막 전쟁/평화 상태 변경 후 10라운드가 지나지 않았으면 불가
		if (State)
		{
			const int32 LastStatusRound = FMath::Max(State->LastWarRound, State->LastPeaceRound);
			if (LastStatusRound > 0 && (CachedCurrentRound - LastStatusRound) < 10)
			{
				return -1;
			}
		}
		// 즉시 실행
		DeclareWar(Action.FromPlayerId, Action.ToPlayerId, CachedCurrentRound);
		break;
	}

	case EDiplomacyActionType::OfferPeace:
	{
		// 전쟁 중이 아니면 평화 제안 불가
		if (!State || State->Status != EDiplomacyStatusType::War)
		{
			return -1;
		}
		// 마지막 전쟁/평화 상태 변경 후 10라운드가 지나지 않았으면 불가
		const int32 LastStatusRound = FMath::Max(State->LastWarRound, State->LastPeaceRound);
		if (LastStatusRound > 0 && (CachedCurrentRound - LastStatusRound) < 10)
		{
			return -1;
		}
		// 큐에 추가 (허락 필요)
		break;
	}

	case EDiplomacyActionType::Denounce:
	{
		// 마지막 비난 후 5라운드가 지나지 않았으면 불가
		if (State && State->LastDenounceRound > 0 && (CachedCurrentRound - State->LastDenounceRound) < 5)
		{
			return -1;
		}
		// 즉시 실행
		if (State)
		{
			State->LastDenounceRound = CachedCurrentRound;
		}
		AddAttitude(Action.ToPlayerId, Action.FromPlayerId, -30);
		break;
	}

	case EDiplomacyActionType::SendGift:
	{
		// 마지막 선물 후 5라운드가 지나지 않았으면 불가
		if (State && State->LastGiftRound > 0 && (CachedCurrentRound - State->LastGiftRound) < 5)
		{
			return -1;
		}
		// 즉시 실행
		if (State)
		{
			State->LastGiftRound = CachedCurrentRound;
		}
		AddAttitude(Action.ToPlayerId, Action.FromPlayerId, +25);
		break;
	}

	case EDiplomacyActionType::OfferAlliance:
	{
		// 동맹 상태면 동맹 제안 불가
		if (State && State->Status == EDiplomacyStatusType::Alliance)
		{
			return -1;
		}
		// 큐에 추가 (허락 필요)
		break;
	}

	default:
		return -1;
	}

	// 로컬 복사본 생성
	FDiplomacyAction NewAction = Action;

	// 액션 ID 부여 (아직 설정되지 않은 경우에만)
	if (NewAction.ActionId <= 0)
	{
		NewAction.ActionId = NextActionId++;
	}

	// 항상 현재 라운드를 기록
	NewAction.IssuedRound = CachedCurrentRound;

	// 즉시 실행 액션은 큐에 추가하지 않음
	if (Action.Action == EDiplomacyActionType::DeclareWar || 
		Action.Action == EDiplomacyActionType::Denounce || 
		Action.Action == EDiplomacyActionType::SendGift)
	{
		OnDiplomacyActionIssued.Broadcast(NewAction);
		return NewAction.ActionId;
	}

	// 큐에 추가 (OfferPeace, OfferAlliance)
	PendingActions.Add(NewAction);

	// 델리게이트 브로드캐스트
	OnDiplomacyActionIssued.Broadcast(NewAction);

	return NewAction.ActionId;
}

void UDiplomacyManager::ResolveAction(int32 ActionId, bool bAccepted)
{
	// 해당 ActionId를 가진 액션 찾기
	int32 FoundIndex = INDEX_NONE;
	for (int32 i = 0; i < PendingActions.Num(); ++i)
	{
		if (PendingActions[i].ActionId == ActionId)
		{
			FoundIndex = i;
			break;
		}
	}

	if (FoundIndex == INDEX_NONE)
	{
		// 이미 처리되었거나 존재하지 않는 액션
		return;
	}

	const FDiplomacyAction Action = PendingActions[FoundIndex];

	FDiplomacyPairKey PairKey(Action.FromPlayerId, Action.ToPlayerId);
	FDiplomacyPairState* State = PairStates.Find(PairKey);

	// 액션 타입에 따른 처리
	switch (Action.Action)
	{
	case EDiplomacyActionType::OfferPeace:
	{
		if (!State || State->Status != EDiplomacyStatusType::War)
		{
			// 이미 전쟁 상태가 아니면 무시
			break;
		}
		if (bAccepted)
		{
			MakePeace(Action.FromPlayerId, Action.ToPlayerId, CachedCurrentRound);
			AddAttitude(Action.ToPlayerId, Action.FromPlayerId, +15);
			AddAttitude(Action.FromPlayerId, Action.ToPlayerId, +10);
		}
		else
		{
			AddAttitude(Action.ToPlayerId, Action.FromPlayerId, -10);
			AddAttitude(Action.FromPlayerId, Action.ToPlayerId, -5);
		}
		break;
	}

	case EDiplomacyActionType::OfferAlliance:
	{
		if (State && State->Status == EDiplomacyStatusType::Alliance)
		{
			// 이미 동맹 상태면 무시
			break;
		}
		if (bAccepted)
		{
			MakeAlliance(Action.FromPlayerId, Action.ToPlayerId, CachedCurrentRound);
			AddAttitude(Action.ToPlayerId, Action.FromPlayerId, +20);
			AddAttitude(Action.FromPlayerId, Action.ToPlayerId, +20);
		}
		else
		{
			AddAttitude(Action.ToPlayerId, Action.FromPlayerId, -15);
		}
		break;
	}

	default:
		break;
	}

	// 결과 델리게이트 브로드캐스트
	OnDiplomacyActionResolved.Broadcast(Action, bAccepted);

	// 큐에서 제거
	PendingActions.RemoveAtSwap(FoundIndex);
}

TArray<FDiplomacyAction> UDiplomacyManager::GetPendingActionsForPlayer(int32 ToPlayerId) const
{
	TArray<FDiplomacyAction> Result;

	// 인덱스 범위 검증
	if (ToPlayerId < 0 || ToPlayerId >= PlayerCount)
	{
		return Result;
	}

	for (const FDiplomacyAction& Action : PendingActions)
	{
		if (Action.ToPlayerId == ToPlayerId)
		{
			Result.Add(Action);
		}
	}

	return Result;
}