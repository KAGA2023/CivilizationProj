#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Turn/TurnComponent.h"
#include "SuperGameState.generated.h"

UCLASS()
class CIVILIZATION_API ASuperGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ASuperGameState();

	// ========== 턴 조회 ==========

	// 인게임 턴 컴포넌트를 반환합니다. 호스트·참가자 모두 읽을 수 있습니다.
	UFUNCTION(BlueprintCallable, Category = "Turn")
	UTurnComponent* GetTurnComponent() const { return TurnComponent; }

	// 현재 라운드·턴·슬롯을 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Turn")
	FTurnStruct GetCurrentTurn() const;

	// 지금 턴인 문명 슬롯을 반환합니다. 0=호스트/싱글, 1=멀티 참가자, 나머지는 AI.
	UFUNCTION(BlueprintCallable, Category = "Turn")
	int32 GetCurrentPlayerIndex() const;

	// 현재 라운드 번호를 반환합니다. 1부터 시작합니다.
	UFUNCTION(BlueprintCallable, Category = "Turn")
	int32 GetCurrentRoundNumber() const;

protected:
	// ========== 상태 ==========

	// 라운드·턴·슬롯. GameState에 있어 참가자에게도 복제됩니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn")
	TObjectPtr<UTurnComponent> TurnComponent;
};
