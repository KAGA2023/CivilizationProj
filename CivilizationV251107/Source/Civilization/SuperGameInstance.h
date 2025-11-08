// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "WorldStruct.h"
#include "SuperGameInstance.generated.h"
#define GameInst Cast<USuperGameInstance>(GetWorld()->GetGameInstance())

UCLASS()
class CIVILIZATION_API USuperGameInstance : public UGameInstance
{
	GENERATED_BODY()
	TSoftObjectPtr<UWorld> TargetLevel{}; // 이동할 타겟 레벨
	TSoftObjectPtr<UWorld> OldLevel{}; // 이전 레벨 (언로드용)
public:
	USuperGameInstance(); // 생성자
	virtual void Init() override; // 게임 인스턴스 초기화

	UFUNCTION(BlueprintCallable)
	void OpenLevel(TSoftObjectPtr<UWorld> newLevel);  // 다른 레벨로 이동합니다

	FName GetTargetLevelName() // 현재 타겟 레벨의 이름을 반환합니다
	{
		return *TargetLevel.GetAssetName();
	}
	
	FString GetTargetLevelPackageName() const // 현재 타겟 레벨의 패키지 경로를 반환합니다
	{
		if (TargetLevel.IsNull())
		{
			return FString();
		}
		return TargetLevel.ToSoftObjectPath().GetLongPackageName();
	}

	// 월드 설정 관리
	UPROPERTY(BlueprintReadWrite, Category = "World Settings") // 현재 월드 설정
	FWorldConfig CurrentWorldConfig;

	UFUNCTION(BlueprintCallable, Category = "World Settings") // 월드 설정 설정
	void SetWorldConfig(const FWorldConfig& NewSettings);

	UFUNCTION(BlueprintCallable, Category = "World Settings") // 월드 설정 가져오기
	FWorldConfig GetWorldConfig() const { return CurrentWorldConfig; }

	UFUNCTION(BlueprintCallable, Category = "World Settings") // 기본 월드 설정으로 리셋
	void ResetWorldConfigToDefault();

	// 월드 컴포넌트 관리 (로딩에서 생성된 월드 컴포넌트 저장)
	UPROPERTY(BlueprintReadWrite, Category = "World Management") // 생성된 월드 컴포넌트
	class UWorldComponent* GeneratedWorldComponent = nullptr;

	UFUNCTION(BlueprintCallable, Category = "World Management") // 월드 컴포넌트 설정
	void SetGeneratedWorldComponent(class UWorldComponent* WorldComponent);

	UFUNCTION(BlueprintCallable, Category = "World Management") // 월드 컴포넌트 가져오기
	class UWorldComponent* GetGeneratedWorldComponent() const { return GeneratedWorldComponent; }

	UFUNCTION(BlueprintCallable, Category = "World Management") // 월드 컴포넌트 정리 (메모리 해제)
	void ClearGeneratedWorldComponent();

	// 유닛 매니저 관리
	UPROPERTY(BlueprintReadWrite, Category = "Unit Management") // 유닛 매니저
	class UUnitManager* UnitManager = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Unit Management") // 유닛 매니저 설정
	void SetUnitManager(class UUnitManager* InUnitManager);

	UFUNCTION(BlueprintCallable, Category = "Unit Management") // 유닛 매니저 가져오기
	class UUnitManager* GetUnitManager() const { return UnitManager; }

	UFUNCTION(BlueprintCallable, Category = "Unit Management") // 유닛 매니저 정리 (메모리 해제)
	void ClearUnitManager();

	// 플레이어 스테이트 관리
	UPROPERTY(BlueprintReadWrite, Category = "Player Management") // 모든 플레이어 스테이트
	TArray<class ASuperPlayerState*> PlayerStates;

	UFUNCTION(BlueprintCallable, Category = "Player Management") // 플레이어 스테이트 추가
	void AddPlayerState(class ASuperPlayerState* PlayerState);

	UFUNCTION(BlueprintCallable, Category = "Player Management") // 플레이어 스테이트 가져오기
	class ASuperPlayerState* GetPlayerState(int32 PlayerIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Player Management") // 총 플레이어 수
	int32 GetPlayerStateCount() const { return PlayerStates.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Player Management") // 모든 플레이어 스테이트 정리
	void ClearAllPlayerStates();
};
