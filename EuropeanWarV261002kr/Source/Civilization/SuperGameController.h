// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "SaveLoad/SaveLoadStruct.h"
#include "SuperGameController.generated.h"

UCLASS()
class CIVILIZATION_API ASuperGameController : public APlayerController
{
	GENERATED_BODY()

public:
	ASuperGameController();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// ========== 입력 ==========

	// 인게임 Enhanced Input 매핑
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* IMC_InGame;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_LMB;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_RMB;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Wheel;

	void OnLMB_Pressed();
	void OnLMB_Released();
	void OnRMB_Pressed();
	void OnRMB_Released();
	void OnWheel_Triggered(const FInputActionInstance& Instance);

	// ========== 카메라 ==========

	bool bIsPanning;
	FVector2D LastMousePosition;
	FVector2D CurrentMousePosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float PanSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ZoomSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MinZoomDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxZoomDistance;

	// WorldConfig.WorldRadius로 계산한 카메라 이동 범위
	bool bCameraBoundsValid = false;
	float CameraMinX = 0.f;
	float CameraMaxX = 0.f;
	float CameraMinY = 0.f;
	float CameraMaxY = 0.f;

	void UpdateCameraBoundsFromWorldConfig();
	void UpdateCameraPan();
	void UpdateCameraZoom(float ZoomDelta);
	FVector2D GetMousePosition() const;
	void SetCameraLocation(FVector NewLocation);
	void SetCameraRotation(FRotator NewRotation);

public:
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetPanSpeed(float NewSpeed) { PanSpeed = NewSpeed; }

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetZoomSpeed(float NewSpeed) { ZoomSpeed = NewSpeed; }

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetZoomLimits(float MinDistance, float MaxDistance);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	bool IsCameraPanning() const { return bIsPanning; }

	// ========== 로비 RPC ==========

	// 로컬 UI가 고른 국가를 호스트 LobbyGameState에 요청합니다.
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void RequestSetLobbyCountry(FName CountryRowName);

	// 호스트가 시작 전, 로비 설정을 참가자 GI에 복사하라고 알립니다.
	UFUNCTION(Client, Reliable)
	void ClientCommitLobbyToGameInstance();

	UFUNCTION(Server, Reliable)
	void ServerSetLobbyCountry(const FString& CountryRowName);

	UFUNCTION(Server, Reliable)
	void ServerNotifyLobbyCommitReady();

	// ========== 월드 동기화 RPC ==========

	// 호스트가 보낼 타일 총개수와 시작 도시 hex를 받습니다.
	UFUNCTION(Client, Reliable)
	void ClientBeginNetworkWorld(int32 TotalTiles, const TArray<FVector2D>& CityHexes);

	// 타일 청크를 이어 붙입니다.
	UFUNCTION(Client, Reliable)
	void ClientReceiveNetworkWorldTiles(const TArray<FWorldTileNetData>& Tiles);

	// 수신이 끝나면 월드를 조립하고 호스트에 ack를 보냅니다.
	UFUNCTION(Client, Reliable)
	void ClientFinishNetworkWorld();

	UFUNCTION(Server, Reliable)
	void ServerNotifyNetworkWorldReady();

	// ========== 턴 RPC ==========

	// 내 턴 종료를 호스트에 요청합니다. 호스트가 지금 슬롯을 확인한 뒤 넘깁니다.
	UFUNCTION(Server, Reliable)
	void ServerRequestEndPlayerTurn();

	// 호스트가 승인한 턴 마무리를 참가자 월드에 적용합니다. 생산·연구 진행과 유닛 생성이 여기 있습니다.
	UFUNCTION(Client, Reliable)
	void ClientApplyEndPlayerTurn();

	// ========== 생산 / 연구 RPC ==========

	// 건물 생산 시작을 호스트에 요청합니다.
	UFUNCTION(Server, Reliable)
	void ServerRequestStartBuildingProduction(FName BuildingRowName);

	// 유닛 생산 시작을 호스트에 요청합니다.
	UFUNCTION(Server, Reliable)
	void ServerRequestStartUnitProduction(FName UnitName);

	// 기술 연구 시작을 호스트에 요청합니다.
	UFUNCTION(Server, Reliable)
	void ServerRequestStartTechResearch(FName TechRowName);

	// 호스트가 승인한 건물 생산을 참가자 월드에 적용합니다.
	UFUNCTION(Client, Reliable)
	void ClientApplyStartBuildingProduction(FName BuildingRowName);

	// 호스트가 승인한 유닛 생산을 참가자 월드에 적용합니다.
	UFUNCTION(Client, Reliable)
	void ClientApplyStartUnitProduction(FName UnitName);

	// 호스트가 승인한 기술 연구를 참가자 월드에 적용합니다.
	UFUNCTION(Client, Reliable)
	void ClientApplyStartTechResearch(FName TechRowName);

	// 골드 건물 구매를 호스트에 요청합니다.
	UFUNCTION(Server, Reliable)
	void ServerRequestPurchaseBuilding(FName BuildingRowName);

	// 골드 유닛 구매를 호스트에 요청합니다.
	UFUNCTION(Server, Reliable)
	void ServerRequestPurchaseUnit(FName UnitName);

	// 호스트가 승인한 건물 구매를 참가자 월드에 적용합니다.
	UFUNCTION(Client, Reliable)
	void ClientApplyPurchaseBuilding(FName BuildingRowName);

	// 호스트가 승인한 유닛 구매를 참가자 월드에 적용합니다.
	UFUNCTION(Client, Reliable)
	void ClientApplyPurchaseUnit(FName UnitName);

private:
	// 호스트에서 이 컨트롤러의 문명 슬롯을 정합니다. 호스트 0, 참가자 1.
	int32 GetRequestingPlayerIndex() const;
};
