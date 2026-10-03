#include "SuperGameState.h"

ASuperGameState::ASuperGameState()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	NetUpdateFrequency = 10.0f;

	TurnComponent = CreateDefaultSubobject<UTurnComponent>(TEXT("TurnComponent"));
	if (TurnComponent)
	{
		TurnComponent->SetIsReplicated(true);
	}
}

FTurnStruct ASuperGameState::GetCurrentTurn() const
{
	return TurnComponent ? TurnComponent->GetCurrentTurn() : FTurnStruct();
}

int32 ASuperGameState::GetCurrentPlayerIndex() const
{
	return TurnComponent ? TurnComponent->GetCurrentPlayerIndex() : 0;
}

int32 ASuperGameState::GetCurrentRoundNumber() const
{
	return TurnComponent ? TurnComponent->GetCurrentRoundNumber() : 1;
}
