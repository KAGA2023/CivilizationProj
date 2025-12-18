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

	// 플레이어 쌍(A <-> B)의 기본 외교 상태(평화) 설정
	for (int32 PlayerA = 0; PlayerA < NumPlayers; ++PlayerA)
	{
		for (int32 PlayerB = PlayerA + 1; PlayerB < NumPlayers; ++PlayerB)
		{
			FDiplomacyPairKey PairKey(PlayerA, PlayerB);
			FDiplomacyPairState PairState;

			PairState.Status = EDiplomacyStatusType::Peace;
			PairState.Treaties.Reset();
			PairState.LastStatusChangedRound = 0;

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
	// 유효하지 않은 플레이어 인덱스면 기본값(평화) 반환
	if (PlayerA == PlayerB || PlayerA < 0 || PlayerB < 0 || PlayerA >= PlayerCount || PlayerB >= PlayerCount)
	{
		return EDiplomacyStatusType::Peace;
	}

	FDiplomacyPairKey PairKey(PlayerA, PlayerB);

	if (const FDiplomacyPairState* FoundState = PairStates.Find(PairKey))
	{
		return FoundState->Status;
	}

	// 맵에 없으면 기본적으로 평화 상태로 간주
	return EDiplomacyStatusType::Peace;
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
		NewState.Status = EDiplomacyStatusType::Peace;
		NewState.Treaties.Reset();
		NewState.LastStatusChangedRound = 0;

		State = &PairStates.Add(PairKey, NewState);
	}

	// 이미 전쟁 상태면 변경 없음
	if (State->Status == EDiplomacyStatusType::War)
	{
		return false;
	}

	State->Status = EDiplomacyStatusType::War;
	State->LastStatusChangedRound = CurrentRound;

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
		NewState.Status = EDiplomacyStatusType::Peace;
		NewState.Treaties.Reset();
		NewState.LastStatusChangedRound = 0;

		State = &PairStates.Add(PairKey, NewState);
	}

	// 이미 평화 상태면 변경 없음
	if (State->Status == EDiplomacyStatusType::Peace)
	{
		return false;
	}

	State->Status = EDiplomacyStatusType::Peace;
	State->LastStatusChangedRound = CurrentRound;

	// 평화 상태 변경 델리게이트 브로드캐스트
	OnDiplomacyStatusChanged.Broadcast(PlayerA, PlayerB, EDiplomacyStatusType::Peace);

	return true;
}

bool UDiplomacyManager::AddTreaty(int32 PlayerA, int32 PlayerB, EDiplomacyTreatyType TreatyType, int32 StartRound, int32 DurationRounds)
{
	// 인덱스 및 입력 값 검증
	if (PlayerA == PlayerB || PlayerA < 0 || PlayerB < 0 || PlayerA >= PlayerCount || PlayerB >= PlayerCount)
	{
		return false;
	}
	if (DurationRounds <= 0 || StartRound < 0)
	{
		return false;
	}

	FDiplomacyPairKey PairKey(PlayerA, PlayerB);
	FDiplomacyPairState* State = PairStates.Find(PairKey);

	if (!State)
	{
		FDiplomacyPairState NewState;
		NewState.Status = EDiplomacyStatusType::Peace;
		NewState.Treaties.Reset();
		NewState.LastStatusChangedRound = 0;

		State = &PairStates.Add(PairKey, NewState);
	}

	// 중복 조약 방지: 동일 타입의 활성 조약이 이미 있으면 추가 금지
	for (const FDiplomacyTreaty& Existing : State->Treaties)
	{
		if (Existing.Treaty == TreatyType && Existing.IsActiveAtRound(StartRound))
		{
			return false;
		}
	}

	FDiplomacyTreaty NewTreaty;
	NewTreaty.Treaty = TreatyType;
	NewTreaty.StartRound = StartRound;
	NewTreaty.EndRound = StartRound + DurationRounds - 1;

	State->Treaties.Add(NewTreaty);

	// 조약 추가 델리게이트 브로드캐스트
	OnDiplomacyTreatyChanged.Broadcast(PlayerA, PlayerB, TreatyType, true);

	return true;
}

bool UDiplomacyManager::CancelTreaty(int32 PlayerA, int32 PlayerB, EDiplomacyTreatyType TreatyType, int32 CurrentRound)
{
	if (PlayerA == PlayerB || PlayerA < 0 || PlayerB < 0 || PlayerA >= PlayerCount || PlayerB >= PlayerCount)
	{
		return false;
	}

	FDiplomacyPairKey PairKey(PlayerA, PlayerB);
	FDiplomacyPairState* State = PairStates.Find(PairKey);

	if (!State)
	{
		return false;
	}

	bool bRemovedAny = false;

	// 뒤에서부터 순회하며 제거
	for (int32 Index = State->Treaties.Num() - 1; Index >= 0; --Index)
	{
		const FDiplomacyTreaty& Treaty = State->Treaties[Index];
		if (Treaty.Treaty == TreatyType && Treaty.IsActiveAtRound(CurrentRound))
		{
			State->Treaties.RemoveAtSwap(Index);
			bRemovedAny = true;
		}
	}

	// 하나 이상의 조약이 제거되었다면 델리게이트 브로드캐스트
	if (bRemovedAny)
	{
		OnDiplomacyTreatyChanged.Broadcast(PlayerA, PlayerB, TreatyType, false);
	}

	return bRemovedAny;
}

bool UDiplomacyManager::HasActiveTreaty(int32 PlayerA, int32 PlayerB, EDiplomacyTreatyType TreatyType, int32 CurrentRound) const
{
	if (PlayerA == PlayerB || PlayerA < 0 || PlayerB < 0 || PlayerA >= PlayerCount || PlayerB >= PlayerCount)
	{
		return false;
	}

	FDiplomacyPairKey PairKey(PlayerA, PlayerB);
	const FDiplomacyPairState* State = PairStates.Find(PairKey);

	if (!State)
	{
		return false;
	}

	return State->HasActiveTreaty(TreatyType, CurrentRound);
}

void UDiplomacyManager::OnRoundStarted(int32 NewRound)
{
	// 이미 처리한 라운드면 중복 처리 방지
	if (NewRound <= LastProcessedRound)
	{
		return;
	}
	LastProcessedRound = NewRound;
	CurrentRound = NewRound;

	// 만료된 조약 정리
	for (TPair<FDiplomacyPairKey, FDiplomacyPairState>& Pair : PairStates)
	{
		TArray<FDiplomacyTreaty>& Treaties = Pair.Value.Treaties;
		const FDiplomacyPairKey& Key = Pair.Key;

		for (int32 Index = Treaties.Num() - 1; Index >= 0; --Index)
		{
			const FDiplomacyTreaty& Treaty = Treaties[Index];

			// 현재 라운드 기준으로 더 이상 활성화되지 않으면 제거
			if (!Treaty.IsActiveAtRound(NewRound) && Treaty.EndRound < NewRound)
			{
				const EDiplomacyTreatyType ExpiredType = Treaty.Treaty;

				// 먼저 실제로 제거
				Treaties.RemoveAtSwap(Index);

				// 그 다음 델리게이트 브로드캐스트 (상태가 이미 반영된 뒤)
				OnDiplomacyTreatyChanged.Broadcast(Key.PlayerA, Key.PlayerB, ExpiredType, false);
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

	// 로컬 복사본 생성
	FDiplomacyAction NewAction = Action;

	// 액션 ID 부여 (아직 설정되지 않은 경우에만)
	if (NewAction.ActionId <= 0)
	{
		NewAction.ActionId = NextActionId++;
	}

	// 항상 현재 라운드를 기록
	NewAction.IssuedRound = CurrentRound;

	// 큐에 추가
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

	// 액션 타입에 따른 실제 효과 적용
	switch (Action.Action)
	{
	case EDiplomacyActionType::DeclareWar:
		if (bAccepted)
		{
			DeclareWar(Action.FromPlayerId, Action.ToPlayerId, CurrentRound);
		}
		break;

	case EDiplomacyActionType::DeclareAlliance:
		if (bAccepted)
		{
			// 예시: 30라운드 동안 동맹
			AddTreaty(Action.FromPlayerId, Action.ToPlayerId, EDiplomacyTreatyType::Alliance, CurrentRound, 10);
		}
		else
		{
			// 거절 패널티 예시: 호감도 감소
			AddAttitude(Action.ToPlayerId, Action.FromPlayerId, -15);
		}
		break;

	case EDiplomacyActionType::OpenBorders:
		if (bAccepted)
		{
			// 예시: 20라운드 동안 국경 개방
			AddTreaty(Action.FromPlayerId, Action.ToPlayerId, EDiplomacyTreatyType::OpenBorders, CurrentRound, 10);
		}
		else
		{
			// 거절 패널티 예시
			AddAttitude(Action.ToPlayerId, Action.FromPlayerId, -5);
		}
		break;

	case EDiplomacyActionType::SendDelegation:
		// 대표단 파견은 단방향이므로, Issue 시점에 바로 처리하는 식으로 설계할 수도 있음.
		// 여기서는 별도 처리를 하지 않음.
		break;

	case EDiplomacyActionType::Denounce:
		// 비난은 단방향이므로, Issue 시점에 바로 처리하는 식으로 설계할 수도 있음.
		// 여기서는 별도 처리를 하지 않음.
		break;

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