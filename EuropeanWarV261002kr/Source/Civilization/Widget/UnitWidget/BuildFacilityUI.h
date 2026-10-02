#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "BuildFacilityUI.generated.h"

class UWorldTile;
class ASuperPlayerState;
class UFacilityManager;
class UWorldComponent;
class UUnitManager;

UCLASS()
class CIVILIZATION_API UBuildFacilityUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========== BindWidget ==========

	// 농장을 짓는 버튼
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BuildFarmBtn = nullptr;

	// 목장을 짓는 버튼
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BuildPastureBtn = nullptr;

	// 광산을 짓는 버튼
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BuildMineBtn = nullptr;

	// 기존 시설을 철거하는 버튼
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* DestroyFacilityBtn = nullptr;

	// 플랜테이션을 짓는 버튼
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BuildPlantationBtn = nullptr;

	// 시장을 짓는 버튼
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BuildMarketBtn = nullptr;

	// 학교를 짓는 버튼
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BuildSchoolBtn = nullptr;

	// 마을을 짓는 버튼
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BuildVillageBtn = nullptr;

	// 제재소를 짓는 버튼
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BuildLumberMillBtn = nullptr;

	// 약탈된 시설을 복구하는 버튼. 블루프린트에 없으면 바인딩하지 않습니다.
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UButton* RepairFacilityBtn = nullptr;

	// 선택한 타일에서 지을 수 있는 시설로 버튼을 초기화합니다.
	UFUNCTION(BlueprintCallable, Category = "Facility")
	void SetupForTile(UWorldTile* InTile);

protected:
	virtual void NativeConstruct() override;

private:
	// ========== 캐시 ==========

	// 지금 시설 UI가 가리키는 타일
	UPROPERTY()
	UWorldTile* CachedTile = nullptr;

	// 지금 시설 UI가 가리키는 hex 좌표
	FVector2D CachedHex = FVector2D::ZeroVector;

	// 시설을 짓는 로컬 플레이어 스테이트
	UPROPERTY()
	ASuperPlayerState* CachedPlayerState = nullptr;

	// 시설 건설·철거를 담당하는 매니저
	UPROPERTY()
	UFacilityManager* CachedFacilityManager = nullptr;

	// 타일 조회에 쓰는 월드 컴포넌트
	UPROPERTY()
	UWorldComponent* CachedWorldComponent = nullptr;

	// 건설자 유닛 확인에 쓰는 유닛 매니저
	UPROPERTY()
	UUnitManager* CachedUnitManager = nullptr;

	// ========== 버튼 핸들러 ==========

	// 농장 버튼을 누르면 농장을 짓습니다.
	UFUNCTION()
	void OnClickedBuildFarm();

	// 목장 버튼을 누르면 목장을 짓습니다.
	UFUNCTION()
	void OnClickedBuildPasture();

	// 광산 버튼을 누르면 광산을 짓습니다.
	UFUNCTION()
	void OnClickedBuildMine();

	// 철거 버튼을 누르면 타일 시설을 없앱니다.
	UFUNCTION()
	void OnClickedDestroyFacility();

	// 플랜테이션 버튼을 누르면 플랜테이션을 짓습니다.
	UFUNCTION()
	void OnClickedBuildPlantation();

	// 시장 버튼을 누르면 시장을 짓습니다.
	UFUNCTION()
	void OnClickedBuildMarket();

	// 학교 버튼을 누르면 학교를 짓습니다.
	UFUNCTION()
	void OnClickedBuildSchool();

	// 마을 버튼을 누르면 마을을 짓습니다.
	UFUNCTION()
	void OnClickedBuildVillage();

	// 제재소 버튼을 누르면 제재소를 짓습니다.
	UFUNCTION()
	void OnClickedBuildLumberMill();

	// 복구 버튼을 누르면 약탈된 시설을 고칩니다.
	UFUNCTION()
	void OnClickedRepairFacility();

	// 시설 RowName으로 건설을 실행합니다.
	void BuildFacilityByRowName(const FName& RowName);

	// 지을 수 있는 시설만 버튼을 켭니다.
	void UpdateButtonStates(const TArray<FName>& AvailableFacilities);

	// 해당 타일 시설이 바뀌면 버튼 상태를 다시 그립니다.
	UFUNCTION()
	void OnFacilityChanged(FVector2D TileCoordinate);
};
