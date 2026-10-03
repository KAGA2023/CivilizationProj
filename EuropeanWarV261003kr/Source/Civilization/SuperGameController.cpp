// Fill out your copyright notice in the Description page of Project Settings.

#include "SuperGameController.h"
#include "Camera/CameraComponent.h"
#include "SuperCameraPawn.h"
#include "SuperGameInstance.h"
#include "SuperGameModeBase.h"
#include "SuperGameState.h"
#include "SuperPlayerState.h"
#include "Diplomacy/DiplomacyManager.h"
#include "World/WorldComponent.h"
#include "Facility/FacilityManager.h"
#include "LobbyGameState.h"
#include "World/WorldStruct.h"
#include "Unit/UnitManager.h"
#include "Multiplayer/MultiplayerSessionSubsystem.h"

ASuperGameController::ASuperGameController()
{
	// Enhanced Input 초기화
	IMC_InGame = nullptr;
	IA_LMB = nullptr;
	IA_RMB = nullptr;
	IA_Wheel = nullptr;

	// 카메라 컨트롤 초기화
	bIsPanning = false;
	LastMousePosition = FVector2D::ZeroVector;
	CurrentMousePosition = FVector2D::ZeroVector;

	// 카메라 설정 기본값
	PanSpeed = 3.0f;
	ZoomSpeed = 200.0f; // 줌 속도 낮춰서 부드럽게
	MinZoomDistance = 500.0f;
	MaxZoomDistance = 5000.0f;

	PrimaryActorTick.bCanEverTick = true;
}

void ASuperGameController::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어인지 확인
	if (!IsLocalController()) return;

	// Enhanced Input 설정
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->ClearAllMappings();
			if (IMC_InGame)
			{
				Subsystem->AddMappingContext(IMC_InGame, 0);
			}
		}
	}

	// 마우스 커서 표시
	bShowMouseCursor = true;
	
	// 클릭 이벤트 활성화 (타일 클릭 감지용)
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	// 입력 모드 설정 (게임 + UI)
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	// 카메라 이동 범위 설정 (WorldConfig.WorldRadius 기준)
	UpdateCameraBoundsFromWorldConfig();
}

void ASuperGameController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Enhanced Input Component로 캐스팅
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// 마우스 입력 바인딩 (null 체크 추가)
		if (IA_LMB)
		{
			EnhancedInputComponent->BindAction(IA_LMB, ETriggerEvent::Started, this, &ASuperGameController::OnLMB_Pressed);
			EnhancedInputComponent->BindAction(IA_LMB, ETriggerEvent::Completed, this, &ASuperGameController::OnLMB_Released);
		}
		if (IA_RMB)
		{
			EnhancedInputComponent->BindAction(IA_RMB, ETriggerEvent::Started, this, &ASuperGameController::OnRMB_Pressed);
			EnhancedInputComponent->BindAction(IA_RMB, ETriggerEvent::Completed, this, &ASuperGameController::OnRMB_Released);
		}
		if (IA_Wheel)
		{
			EnhancedInputComponent->BindAction(IA_Wheel, ETriggerEvent::Triggered, this, &ASuperGameController::OnWheel_Triggered);
		}
	}
}

void ASuperGameController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 카메라 팬 업데이트
	if (bIsPanning)
	{
		UpdateCameraPan();
	}
}

void ASuperGameController::OnLMB_Pressed()
{
	// 좌클릭 시작 - 타일 클릭 전용 (WorldTileActor에서 처리)
}

void ASuperGameController::OnLMB_Released()
{
	// 좌클릭 종료
}

void ASuperGameController::OnRMB_Pressed()
{
	// 우클릭 시작 - 카메라 팬 시작
	bIsPanning = true;
	LastMousePosition = GetMousePosition();
}

void ASuperGameController::OnRMB_Released()
{
	// 우클릭 종료 - 카메라 팬 종료
	bIsPanning = false;
}

void ASuperGameController::OnWheel_Triggered(const FInputActionInstance& Instance)
{
	// 마우스 휠 - 카메라 줌
	float ZoomDelta = Instance.GetValue().Get<float>();
	UpdateCameraZoom(ZoomDelta);
}

void ASuperGameController::UpdateCameraPan()
{
	// 현재 마우스 위치 가져오기
	CurrentMousePosition = GetMousePosition();

	// 마우스 이동량 계산
	FVector2D MouseDelta = CurrentMousePosition - LastMousePosition;

	// 카메라 팬 계산
	if (APawn* ControlledPawn = GetPawn())
	{
		// 카메라의 현재 위치와 회전 가져오기
		FVector CurrentLocation = ControlledPawn->GetActorLocation();
		FRotator CurrentRotation = ControlledPawn->GetActorRotation();

		// 마우스 이동량을 월드 좌표로 변환
		FVector RightVector = CurrentRotation.RotateVector(FVector::RightVector);
		FVector ForwardVector = CurrentRotation.RotateVector(FVector::ForwardVector);

		// 팬 방향 계산 (마우스 X축 = 카메라 오른쪽, 마우스 Y축 = 카메라 앞쪽)
		FVector PanDirection = (RightVector * -MouseDelta.X + ForwardVector * MouseDelta.Y);
		PanDirection.Z = 0.0f; // Z축은 고정 (수평 이동만)

		// 새로운 카메라 위치 계산
		FVector NewLocation = CurrentLocation + PanDirection * PanSpeed;

		// 카메라 위치 설정
		SetCameraLocation(NewLocation);
	}

	// 마지막 마우스 위치 업데이트
	LastMousePosition = CurrentMousePosition;
}

void ASuperGameController::UpdateCameraZoom(float ZoomDelta)
{
	if (ASuperCameraPawn* CameraPawn = Cast<ASuperCameraPawn>(GetPawn()))
	{
		// 목표 줌 거리 설정 (Pawn의 Tick에서 보간됨)
		float CurrentTarget = CameraPawn->TargetZoomDistance;
		float NewTarget = CurrentTarget - (ZoomDelta * ZoomSpeed);
		
		// 줌 범위 제한
		NewTarget = FMath::Clamp(NewTarget, MinZoomDistance, MaxZoomDistance);
		
		CameraPawn->SetTargetZoomDistance(NewTarget);
	}
}

FVector2D ASuperGameController::GetMousePosition() const
{
	float MouseX, MouseY;
	if (APlayerController::GetMousePosition(MouseX, MouseY))
	{
		return FVector2D(MouseX, MouseY);
	}
	return FVector2D::ZeroVector;
}

void ASuperGameController::SetCameraLocation(FVector NewLocation)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		if (bCameraBoundsValid)
		{
			NewLocation.X = FMath::Clamp(NewLocation.X, CameraMinX, CameraMaxX);
			NewLocation.Y = FMath::Clamp(NewLocation.Y, CameraMinY, CameraMaxY);
		}
		ControlledPawn->SetActorLocation(NewLocation);
	}
}

void ASuperGameController::SetCameraRotation(FRotator NewRotation)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->SetActorRotation(NewRotation);
	}
}

void ASuperGameController::SetZoomLimits(float MinDistance, float MaxDistance)
{
	MinZoomDistance = FMath::Max(MinDistance, 100.0f);
	MaxZoomDistance = FMath::Max(MaxDistance, MinZoomDistance + 100.0f);
}

void ASuperGameController::UpdateCameraBoundsFromWorldConfig()
{
	UWorld* World = GetWorld();
	if (!World) return;

	USuperGameInstance* GameInst = Cast<USuperGameInstance>(World->GetGameInstance());
	if (!GameInst) return;

	const FWorldConfig& Config = GameInst->GetWorldConfig();
	int32 R = Config.WorldRadius;
	if (R <= 0) return;

	// WorldComponent::HexToWorld와 동일한 스케일
	// Unreal X = TILE_SIZE * (3/2 * r), Unreal Y = TILE_SIZE * sqrt(3) * (q + r/2)
	// 반지름 R인 육각형 AABB: X ∈ [-1.5*T*R, 1.5*T*R], Y ∈ [-sqrt(3)*T*R, sqrt(3)*T*R]
	const float HalfExtentX = 1.5f * TILE_SIZE * static_cast<float>(R);
	const float HalfExtentY = FMath::Sqrt(3.0f) * TILE_SIZE * static_cast<float>(R);
	const float Margin = 1.05f; // 가장자리 여유 5%

	CameraMinX = -HalfExtentX * Margin;
	CameraMaxX = HalfExtentX * Margin;
	CameraMinY = -HalfExtentY * Margin;
	CameraMaxY = HalfExtentY * Margin;
	bCameraBoundsValid = true;
}

void ASuperGameController::RequestSetLobbyCountry(FName CountryRowName)
{
	ServerSetLobbyCountry(CountryRowName.ToString());
}

void ASuperGameController::ServerSetLobbyCountry_Implementation(const FString& CountryRowName)
{
	if (UWorld* World = GetWorld())
	{
		if (ALobbyGameState* LobbyGameState = World->GetGameState<ALobbyGameState>())
		{
			LobbyGameState->SetClientCountry(FName(*CountryRowName));
		}
	}
}

void ASuperGameController::ClientCommitLobbyToGameInstance_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (ALobbyGameState* LobbyGameState = World->GetGameState<ALobbyGameState>())
		{
			LobbyGameState->ApplyLobbyToLocalGameInstance();
		}
	}

	ServerNotifyLobbyCommitReady();
}

void ASuperGameController::ServerNotifyLobbyCommitReady_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (ALobbyGameState* LobbyGameState = World->GetGameState<ALobbyGameState>())
		{
			LobbyGameState->NotifyClientCommitReady();
		}
	}
}

void ASuperGameController::ClientBeginNetworkWorld_Implementation(int32 TotalTiles, const TArray<FVector2D>& CityHexes)
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		GameInstance->BeginIncomingNetworkWorld(TotalTiles, CityHexes);
	}
}

void ASuperGameController::ClientReceiveNetworkWorldTiles_Implementation(const TArray<FWorldTileNetData>& Tiles)
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		GameInstance->AppendIncomingNetworkWorldTiles(Tiles);
	}
}

void ASuperGameController::ClientFinishNetworkWorld_Implementation()
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		GameInstance->ApplyNetworkWorldPayload();
	}

	ServerNotifyNetworkWorldReady();
}

void ASuperGameController::ServerNotifyNetworkWorldReady_Implementation()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UMultiplayerSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>())
		{
			SessionSubsystem->NotifyNetworkWorldClientReady();
		}
	}
}

int32 ASuperGameController::GetRequestingPlayerIndex() const
{
	return IsLocalController() ? 0 : 1;
}

void ASuperGameController::ServerRequestEndPlayerTurn_Implementation()
{
	if (UWorld* World = GetWorld())
	{
		if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
		{
			if (GameMode->RequestEndPlayerTurn(GetRequestingPlayerIndex()) && !IsLocalController())
			{
				ClientApplyEndPlayerTurn();
			}
		}
	}
}

void ASuperGameController::ClientApplyEndPlayerTurn_Implementation()
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		if (ASuperPlayerState* CivilizationPlayerState = GameInstance->GetLocalPlayerState())
		{
			CivilizationPlayerState->ProcessTurnResources();
		}
	}
}

void ASuperGameController::ServerRequestStartBuildingProduction_Implementation(FName BuildingRowName)
{
	if (UWorld* World = GetWorld())
	{
		if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
		{
			if (GameMode->RequestStartBuildingProduction(GetRequestingPlayerIndex(), BuildingRowName) && !IsLocalController())
			{
				ClientApplyStartBuildingProduction(BuildingRowName);
			}
		}
	}
}

void ASuperGameController::ServerRequestStartUnitProduction_Implementation(FName UnitName)
{
	if (UWorld* World = GetWorld())
	{
		if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
		{
			if (GameMode->RequestStartUnitProduction(GetRequestingPlayerIndex(), UnitName) && !IsLocalController())
			{
				ClientApplyStartUnitProduction(UnitName);
			}
		}
	}
}

void ASuperGameController::ServerRequestStartTechResearch_Implementation(FName TechRowName)
{
	if (UWorld* World = GetWorld())
	{
		if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
		{
			if (GameMode->RequestStartTechResearch(GetRequestingPlayerIndex(), TechRowName) && !IsLocalController())
			{
				ClientApplyStartTechResearch(TechRowName);
			}
		}
	}
}

void ASuperGameController::ClientApplyStartBuildingProduction_Implementation(FName BuildingRowName)
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		if (ASuperPlayerState* CivilizationPlayerState = GameInstance->GetLocalPlayerState())
		{
			CivilizationPlayerState->StartBuildingProduction(BuildingRowName);
		}
	}
}

void ASuperGameController::ClientApplyStartUnitProduction_Implementation(FName UnitName)
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		if (ASuperPlayerState* CivilizationPlayerState = GameInstance->GetLocalPlayerState())
		{
			CivilizationPlayerState->StartUnitProduction(UnitName);
		}
	}
}

void ASuperGameController::ClientApplyStartTechResearch_Implementation(FName TechRowName)
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		if (ASuperPlayerState* CivilizationPlayerState = GameInstance->GetLocalPlayerState())
		{
			CivilizationPlayerState->StartTechResearch(TechRowName);
		}
	}
}

void ASuperGameController::ServerRequestPurchaseBuilding_Implementation(FName BuildingRowName)
{
	if (UWorld* World = GetWorld())
	{
		if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
		{
			if (GameMode->RequestPurchaseBuilding(GetRequestingPlayerIndex(), BuildingRowName) && !IsLocalController())
			{
				ClientApplyPurchaseBuilding(BuildingRowName);
			}
		}
	}
}

void ASuperGameController::ServerRequestPurchaseUnit_Implementation(FName UnitName)
{
	if (UWorld* World = GetWorld())
	{
		if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
		{
			if (GameMode->RequestPurchaseUnit(GetRequestingPlayerIndex(), UnitName) && !IsLocalController())
			{
				ClientApplyPurchaseUnit(UnitName);
			}
		}
	}
}

void ASuperGameController::ClientApplyPurchaseBuilding_Implementation(FName BuildingRowName)
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		if (ASuperPlayerState* CivilizationPlayerState = GameInstance->GetLocalPlayerState())
		{
			CivilizationPlayerState->PurchaseBuildingWithGold(BuildingRowName);
		}
	}
}

void ASuperGameController::ClientApplyPurchaseUnit_Implementation(FName UnitName)
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		if (ASuperPlayerState* CivilizationPlayerState = GameInstance->GetLocalPlayerState())
		{
			CivilizationPlayerState->PurchaseUnitWithGold(UnitName);
		}
	}
}

void ASuperGameController::ClientApplySpawnedUnit_Implementation(int32 PlayerIndex, FName UnitName, FVector2D Hex)
{
	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	if (!GameInstance || UnitName.IsNone())
	{
		return;
	}

	UUnitManager* UnitManager = GameInstance->GetUnitManager();
	ASuperPlayerState* SlotState = GameInstance->GetPlayerState(PlayerIndex);
	if (!UnitManager || !SlotState)
	{
		return;
	}

	if (UnitManager->SpawnUnitAtHex(Hex, UnitName, PlayerIndex, true))
	{
		SlotState->AddSpawnedUnitPopulation();
	}
}

void ASuperGameController::ServerRequestMoveUnit_Implementation(FVector2D FromHex, FVector2D ToHex)
{
	if (UWorld* World = GetWorld())
	{
		if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
		{
			GameMode->RequestMoveUnit(GetRequestingPlayerIndex(), FromHex, ToHex);
		}
	}
}

void ASuperGameController::ServerRequestCombat_Implementation(FVector2D AttackerHex, FVector2D TargetHex)
{
	if (UWorld* World = GetWorld())
	{
		if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
		{
			GameMode->RequestCombat(GetRequestingPlayerIndex(), AttackerHex, TargetHex);
		}
	}
}

void ASuperGameController::ClientApplyMoveUnit_Implementation(FVector2D FromHex, FVector2D ToHex)
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		if (UUnitManager* UnitManager = GameInstance->GetUnitManager())
		{
			UnitManager->MoveUnitFromHexToHex(FromHex, ToHex);
		}
	}
}

void ASuperGameController::ClientApplyCombat_Implementation(FVector2D AttackerHex, FVector2D TargetHex)
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		if (UUnitManager* UnitManager = GameInstance->GetUnitManager())
		{
			UnitManager->CombatFromHexToHex(AttackerHex, TargetHex, false);
		}
	}
}

void ASuperGameController::ClientApplyResetUnitTurn_Implementation(int32 PlayerIndex)
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		if (UUnitManager* UnitManager = GameInstance->GetUnitManager())
		{
			UnitManager->ResetPlayerUnitTurn(PlayerIndex);
		}
	}
}

void ASuperGameController::ServerRequestBuildFacility_Implementation(FVector2D Hex, FName FacilityRowName)
{
	if (UWorld* World = GetWorld())
	{
		if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
		{
			GameMode->RequestBuildFacility(GetRequestingPlayerIndex(), Hex, FacilityRowName);
		}
	}
}

void ASuperGameController::ServerRequestRepairFacility_Implementation(FVector2D Hex)
{
	if (UWorld* World = GetWorld())
	{
		if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
		{
			GameMode->RequestRepairFacility(GetRequestingPlayerIndex(), Hex);
		}
	}
}

void ASuperGameController::ServerRequestDestroyFacility_Implementation(FVector2D Hex)
{
	if (UWorld* World = GetWorld())
	{
		if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
		{
			GameMode->RequestDestroyFacility(GetRequestingPlayerIndex(), Hex);
		}
	}
}

void ASuperGameController::ServerRequestPurchaseTile_Implementation(FVector2D Hex)
{
	if (UWorld* World = GetWorld())
	{
		if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
		{
			GameMode->RequestPurchaseTile(GetRequestingPlayerIndex(), Hex);
		}
	}
}

void ASuperGameController::ServerRequestDiplomacyAction_Implementation(EDiplomacyActionType ActionType, int32 TargetPlayerIndex)
{
	if (UWorld* World = GetWorld())
	{
		if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
		{
			GameMode->RequestDiplomacyAction(GetRequestingPlayerIndex(), ActionType, TargetPlayerIndex);
		}
	}
}

void ASuperGameController::ClientApplyBuildFacility_Implementation(FVector2D Hex, FName FacilityRowName)
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		if (UUnitManager* UnitManager = GameInstance->GetUnitManager())
		{
			UnitManager->RequestBuilderBuildFacility(Hex, FacilityRowName);
		}
	}
}

void ASuperGameController::ClientApplyRepairFacility_Implementation(FVector2D Hex)
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		if (UUnitManager* UnitManager = GameInstance->GetUnitManager())
		{
			UnitManager->RequestBuilderRepairFacility(Hex);
		}
	}
}

void ASuperGameController::ClientApplyDestroyFacility_Implementation(FVector2D Hex)
{
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance()))
	{
		if (UUnitManager* UnitManager = GameInstance->GetUnitManager())
		{
			UnitManager->RequestBuilderDestroyFacility(Hex);
		}
	}
}

void ASuperGameController::ClientApplyPillagedFacilities_Implementation(const TArray<FVector2D>& Hexes)
{
	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	UFacilityManager* FacilityManager = GameInstance->GetFacilityManager();
	UWorldComponent* WorldComponent = GameInstance->GetGeneratedWorldComponent();
	if (!FacilityManager || !WorldComponent)
	{
		return;
	}

	for (const FVector2D& Hex : Hexes)
	{
		FacilityManager->SetFacilityPillaged(Hex, true, WorldComponent);
	}
}

void ASuperGameController::ClientApplyPurchaseTile_Implementation(int32 PlayerIndex, FVector2D Hex)
{
	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	if (!GameInstance)
	{
		return;
	}

	ASuperPlayerState* SlotState = GameInstance->GetPlayerState(PlayerIndex);
	UWorldComponent* WorldComponent = GameInstance->GetGeneratedWorldComponent();
	if (SlotState && WorldComponent)
	{
		SlotState->PurchaseTile(Hex, WorldComponent);
	}
}

void ASuperGameController::ClientApplyDiplomacyAction_Implementation(int32 FromPlayerIndex, int32 TargetPlayerIndex, EDiplomacyActionType ActionType, int32 ActionId)
{
	USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetGameInstance());
	UDiplomacyManager* DiplomacyManager = GameInstance ? GameInstance->GetDiplomacyManager() : nullptr;
	if (!DiplomacyManager)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (const ASuperGameState* InGameState = World->GetGameState<ASuperGameState>())
		{
			if (UTurnComponent* Turn = InGameState->GetTurnComponent())
			{
				DiplomacyManager->OnRoundStarted(Turn->GetCurrentRoundNumber());
			}
		}
	}

	FDiplomacyAction Action;
	Action.Action = ActionType;
	Action.FromPlayerId = FromPlayerIndex;
	Action.ToPlayerId = TargetPlayerIndex;
	Action.ActionId = ActionId;
	DiplomacyManager->IssueAction(Action);
}
