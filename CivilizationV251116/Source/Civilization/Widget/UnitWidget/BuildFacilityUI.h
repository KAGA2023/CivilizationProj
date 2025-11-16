#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "BuildFacilityUI.generated.h"

class UWorldTile;
class ASuperPlayerState;
class UFacilityManager;
class UWorldComponent;

UCLASS()
class CIVILIZATION_API UBuildFacilityUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BuildFarmBtn = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BuildPastureBtn = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BuildCampBtn = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BuildMineBtn = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* DestroyFacilityBtn = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BuildQuarryBtn = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BuildPlantationBtn = nullptr;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UButton* BuildLumberMillBtn = nullptr;

	// 현재 선택된 타일 기준으로 UI 초기화
	UFUNCTION(BlueprintCallable, Category = "Facility")
	void SetupForTile(UWorldTile* InTile);

protected:
	virtual void NativeConstruct() override;

private:
	// 캐시된 컨텍스트
	UPROPERTY()
	UWorldTile* CachedTile = nullptr;

	FVector2D CachedHex = FVector2D::ZeroVector;

	UPROPERTY()
	ASuperPlayerState* CachedPlayerState = nullptr;

	UPROPERTY()
	UFacilityManager* CachedFacilityManager = nullptr;

	UPROPERTY()
	UWorldComponent* CachedWorldComponent = nullptr;

	// 버튼 클릭 핸들러들
	UFUNCTION()
	void OnClickedBuildFarm();

	UFUNCTION()
	void OnClickedBuildPasture();

	UFUNCTION()
	void OnClickedBuildCamp();

	UFUNCTION()
	void OnClickedBuildMine();

	UFUNCTION()
	void OnClickedDestroyFacility();

	UFUNCTION()
	void OnClickedBuildQuarry();

	UFUNCTION()
	void OnClickedBuildPlantation();

	UFUNCTION()
	void OnClickedBuildLumberMill();

	// 공통 실행 헬퍼
	void BuildFacilityByRowName(const FName& RowName);

	// 버튼 활성/비활성 갱신
	void UpdateButtonStates(const TArray<FName>& AvailableFacilities);
};

