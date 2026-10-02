#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Turn/TurnComponent.h"

class ASuperPlayerState;

#include "SuperGameModeBase.generated.h"

UCLASS(BlueprintType, Blueprintable)
class CIVILIZATION_API ASuperGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASuperGameModeBase();

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

public:
	// ========== 게임 상태 ==========

	// 게임을 처음부터 다시 시작합니다.
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void StartNewGame();

	// 게임을 종료하고 월드 컴포넌트를 정리합니다.
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void EndGame();

	// 게임을 일시정지합니다.
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void PauseGame();

	// 일시정지를 해제합니다.
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void ResumeGame();

	// 국가 RowName 배열을 설정합니다. 최대 8개로 자릅니다.
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void SetCountryNames(const TArray<FName>& InCountryNames);

	// ========== 턴 ==========

	// 인게임 GameState의 턴 컴포넌트를 반환합니다. 참가자는 GameMode가 없으니 SuperGameState를 직접 읽습니다.
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	UTurnComponent* GetTurnComponent() const;

	// AI 턴 콜백에서 다음 턴으로 진행합니다. 사람 슬롯이면 턴만 넘깁니다.
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	void NextTurn();

	// 현재 슬롯의 약탈·자원·이동력을 처리합니다.
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	void EndCurrentPlayerTurn();

	// 요청한 사람 슬롯이 지금 턴일 때만 종료합니다. 성공하면 true를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Turn Management")
	bool RequestEndPlayerTurn(int32 RequestingPlayerIndex);

	// ========== 생산 / 연구 명령 ==========

	// 지금 턴인 사람 슬롯의 건물 생산을 시작합니다.
	bool RequestStartBuildingProduction(int32 RequestingPlayerIndex, FName BuildingRowName);

	// 지금 턴인 사람 슬롯의 유닛 생산을 시작합니다. 자원이 빠져야 성공입니다.
	bool RequestStartUnitProduction(int32 RequestingPlayerIndex, FName UnitName);

	// 지금 턴인 사람 슬롯의 기술 연구를 시작합니다.
	bool RequestStartTechResearch(int32 RequestingPlayerIndex, FName TechRowName);

	// 지금 턴인 사람 슬롯이 골드로 건물을 구매합니다.
	bool RequestPurchaseBuilding(int32 RequestingPlayerIndex, FName BuildingRowName);

	// 지금 턴인 사람 슬롯이 골드로 유닛을 구매합니다. 그 슬롯의 도시 옆에 그 슬롯 유닛으로 나옵니다.
	bool RequestPurchaseUnit(int32 RequestingPlayerIndex, FName UnitName);

	// ========== 승리 ==========

	// 플레이어 0이 살아 있고 나머지가 모두 패배했으면 승리를 띄웁니다.
	UFUNCTION(BlueprintCallable, Category = "Victory")
	void CheckGameEndConditions();

protected:
	// ========== 상태 값 ==========

	// 인게임이 진행 중인지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
	bool bIsGameActive;

	// 일시정지 중인지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
	bool bIsGamePaused;

	// GameMode가 들고 있는 국가 RowName 복사. 진실은 GI CountryNames입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
	TArray<FName> CountryNames;

	// ========== 내부 ==========

	// 요청 슬롯이 사람이고 지금 턴이면 그 플레이어 스테이트를 반환합니다.
	ASuperPlayerState* GetPlayerStateIfTurn(int32 RequestingPlayerIndex) const;

	// 턴을 1라운드부터 시작합니다.
	void InitializeGame();

	// GI에 문명 플레이어 스테이트를 다시 만듭니다. 호스트/싱글 BeginPlay에서 호출합니다.
	void CreateAllPlayerStates();

	// 플레이어 0 승리 델리게이트를 띄웁니다.
	void OnPlayerVictory();

	// 라운드가 바뀌면 외교 매니저에 알립니다.
	UFUNCTION()
	void HandleRoundChanged(FTurnStruct NewTurn);

	// 턴이 사람 슬롯이 아니면 AI를 시작합니다.
	UFUNCTION()
	void HandleTurnChanged(FTurnStruct NewTurn);
};
