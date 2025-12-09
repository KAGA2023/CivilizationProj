// Fill out your copyright notice in the Description page of Project Settings.

#include "MainHUD.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "../SuperGameInstance.h"
#include "../SuperPlayerState.h"
#include "../World/WorldComponent.h"
#include "../SuperGameModeBase.h"
#include "../Turn/TurnComponent.h"
#include "../Turn/TurnStruct.h"
#include "../World/WorldTileActor.h"
#include "../Facility/FacilityManager.h"
#include "../World/WorldStruct.h"
#include "StrategicResourceSlotUI.h"
#include "UnitWidget/BuildFacilityUI.h"
#include "CombatWidget/UnitCombatUI.h"
#include "../Unit/UnitManager.h"
#include "../Unit/UnitCharacterBase.h"
#include "../Combat/UnitCombatComponent.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"

void UMainHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// 초기 텍스트 설정
	if (FaithTxt)
	{
		FaithTxt->SetText(FText::FromString(TEXT("0")));
	}

	if (GoldTxt)
	{
		GoldTxt->SetText(FText::FromString(TEXT("0")));
	}

	if (PopulationTxt)
	{
		PopulationTxt->SetText(FText::FromString(TEXT("0/0")));
	}

	if (ScienceTxt)
	{
		ScienceTxt->SetText(FText::FromString(TEXT("+0")));
	}

	// GetGameMode -> CastToSuperGameModeBase -> GetTurnComponent의 OnTurnChanged 델리게이트 바인딩
	if (GetWorld())
	{
		if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
		{
			if (ASuperGameModeBase* SuperGameMode = Cast<ASuperGameModeBase>(GameMode))
			{
				if (UTurnComponent* TurnComponent = SuperGameMode->GetTurnComponent())
				{
					TurnComponent->OnTurnChanged.AddDynamic(this, &UMainHUD::OnTurnChanged);
				}
			}
		}
	}

	// 0.5초 후 모든 WorldTileActor의 OnPlayerCityTileClicked 델리게이트 바인딩
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(BindCityTileTimerHandle, this, &UMainHUD::BindCityTileClickedDelegates, 0.5f, false);
		GetWorld()->GetTimerManager().SetTimer(BindBuilderTileTimerHandle, this, &UMainHUD::BindBuilderTileClickedDelegates, 0.5f, false);
		GetWorld()->GetTimerManager().SetTimer(BindGeneralTileTimerHandle, this, &UMainHUD::BindGeneralTileClickedDelegates, 0.5f, false);
		GetWorld()->GetTimerManager().SetTimer(BindCombatTileHoverTimerHandle, this, &UMainHUD::BindCombatTileHoverDelegates, 0.5f, false);
	}

	// BuildFacilityUI 위젯 초기화 (Hidden으로 설정)
	if (BuildFacilityUIWidget)
	{
		BuildFacilityUIWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	// UnitCombatUI 위젯 초기화 (Hidden으로 설정)
	if (UnitCombatUIWidget)
	{
		UnitCombatUIWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	// PlayerState의 골드/인구 변경 델리게이트 바인딩
	BindPlayerStateDelegates();

	// FacilityManager의 시설 변경 델리게이트 바인딩
	BindFacilityDelegates();

	// UnitManager의 전투 실행 완료 델리게이트 바인딩
	BindCombatExecutedDelegate();

	// 전략 자원 슬롯 초기화
	UpdateStrategicResourceSlots();
}

void UMainHUD::UpdateHUDData()
{
	// GetGameInstance -> CastToSuperGameInstance -> GetPlayerState(0)
	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0);
	if (!PlayerState)
	{
		return;
	}

	// Gold를 GoldTxt에 표시
	if (GoldTxt)
	{
		FString GoldString = FString::Printf(TEXT("%d"), PlayerState->Gold);
		GoldTxt->SetText(FText::FromString(GoldString));
	}

	// Faith를 FaithTxt에 표시
	if (FaithTxt)
	{
		FString FaithString = FString::Printf(TEXT("%d"), PlayerState->Faith);
		FaithTxt->SetText(FText::FromString(FaithString));
	}

	// CalculateTotalScienceYield를 +x 형식으로 ScienceTxt에 표시
	if (ScienceTxt)
	{
		UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent();
		if (WorldComponent)
		{
			int32 ScienceYield = PlayerState->CalculateTotalScienceYield(WorldComponent);
			FString ScienceString = FString::Printf(TEXT("+%d"), ScienceYield);
			ScienceTxt->SetText(FText::FromString(ScienceString));
		}
		else
		{
			ScienceTxt->SetText(FText::FromString(TEXT("+0")));
		}
	}

	// GetPopulation()과 GetLimitPopulation()을 x/y 형식으로 PopulationTxt에 표시
	if (PopulationTxt)
	{
		int32 Population = PlayerState->GetPopulation();
		int32 LimitPopulation = PlayerState->GetLimitPopulation();
		FString PopulationString = FString::Printf(TEXT("%d/%d"), Population, LimitPopulation);
		PopulationTxt->SetText(FText::FromString(PopulationString));
	}
}

void UMainHUD::OnTurnChanged(FTurnStruct NewTurn)
{
	// 턴이 변경될 때마다 HUD 데이터 업데이트
	UpdateHUDData();
	
	// 턴 종료 시 전투 UI 및 시설 UI 닫기
	CloseCombatUI();
	CloseFacilityUI();
}

void UMainHUD::OnPlayerCityTileClicked()
{
	// 도시 타일 클릭 시 UI가 열려있으면 닫기
	if (bIsFacilityUIOpen)
	{
		CloseFacilityUI();
	}
	
	// 도시 타일 클릭 시 OnCityTileClicked 델리게이트 브로드캐스트
	OnCityTileClicked.Broadcast();
}

void UMainHUD::OnBuilderTileClickedHandler(UWorldTile* Tile, FVector2D TileCoordinate)
{
	if (!BuildFacilityUIWidget || !Tile)
	{
		return;
	}

	// 이미 열려있고 같은 타일이면 아무것도 안함 (중복 방지)
	if (bIsFacilityUIOpen && CurrentOpenFacilityTile == TileCoordinate)
	{
		return;
	}

	// 다른 타일이거나 새로 열어야 하는 경우: 기존 UI 닫고 새로 열기
	if (bIsFacilityUIOpen)
	{
		CloseFacilityUI();
	}

	// 새 타일로 UI 열기
	BuildFacilityUIWidget->SetupForTile(Tile);
	BuildFacilityUIWidget->SetVisibility(ESlateVisibility::Visible);
	CurrentOpenFacilityTile = TileCoordinate;
	bIsFacilityUIOpen = true;
}

void UMainHUD::BindCityTileClickedDelegates()
{
	if (!GetWorld())
	{
		return;
	}
	
	// 모든 WorldTileActor 찾기
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWorldTileActor::StaticClass(), FoundActors);

	// 각 WorldTileActor의 OnPlayerCityTileClicked 델리게이트 바인딩
	for (AActor* Actor : FoundActors)
	{
		if (AWorldTileActor* TileActor = Cast<AWorldTileActor>(Actor))
		{
			TileActor->OnPlayerCityTileClicked.AddDynamic(this, &UMainHUD::OnPlayerCityTileClicked);
		}
	}
}

void UMainHUD::BindBuilderTileClickedDelegates()
{
	if (!GetWorld())
	{
		return;
	}

	// 모든 WorldTileActor 찾기
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWorldTileActor::StaticClass(), FoundActors);

	// 각 WorldTileActor의 OnBuilderTileClicked 델리게이트 바인딩
	for (AActor* Actor : FoundActors)
	{
		if (AWorldTileActor* TileActor = Cast<AWorldTileActor>(Actor))
		{
			TileActor->OnBuilderTileClicked.AddDynamic(this, &UMainHUD::OnBuilderTileClickedHandler);
		}
	}
}

void UMainHUD::BindGeneralTileClickedDelegates()
{
	if (!GetWorld())
	{
		return;
	}

	// 모든 WorldTileActor 찾기
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWorldTileActor::StaticClass(), FoundActors);

	// 각 WorldTileActor의 OnGeneralTileClicked 델리게이트 바인딩
	for (AActor* Actor : FoundActors)
	{
		if (AWorldTileActor* TileActor = Cast<AWorldTileActor>(Actor))
		{
			TileActor->OnGeneralTileClicked.AddDynamic(this, &UMainHUD::OnGeneralTileClickedHandler);
		}
	}
}

void UMainHUD::BindCombatTileHoverDelegates()
{
	if (!GetWorld())
	{
		return;
	}

	// 모든 WorldTileActor 찾기
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWorldTileActor::StaticClass(), FoundActors);

	// 각 WorldTileActor의 전투 호버 델리게이트 바인딩
	for (AActor* Actor : FoundActors)
	{
		if (AWorldTileActor* TileActor = Cast<AWorldTileActor>(Actor))
		{
			TileActor->OnCombatTileHoverBegin.AddDynamic(this, &UMainHUD::OnCombatTileHoverBeginHandler);
			TileActor->OnCombatTileHoverEnd.AddDynamic(this, &UMainHUD::OnCombatTileHoverEndHandler);
		}
	}
}

void UMainHUD::BindCombatExecutedDelegate()
{
	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	UUnitManager* UnitManager = SuperGameInst->GetUnitManager();
	if (!UnitManager)
	{
		return;
	}

	// 기존 바인딩 해제
	UnitManager->OnCombatExecuted.RemoveDynamic(this, &UMainHUD::OnCombatExecutedHandler);

	// 새로운 바인딩
	UnitManager->OnCombatExecuted.AddDynamic(this, &UMainHUD::OnCombatExecutedHandler);
}

void UMainHUD::BindPlayerStateDelegates()
{
	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0);
	if (!PlayerState)
	{
		return;
	}

	// 기존 바인딩 해제
	PlayerState->OnGoldChanged.RemoveDynamic(this, &UMainHUD::OnGoldChanged);
	PlayerState->OnPopulationChanged.RemoveDynamic(this, &UMainHUD::OnPopulationChanged);
	PlayerState->OnStrategicResourceStockChanged.RemoveDynamic(this, &UMainHUD::OnStrategicResourceStockChanged);

	// 새로운 바인딩
	PlayerState->OnGoldChanged.AddDynamic(this, &UMainHUD::OnGoldChanged);
	PlayerState->OnPopulationChanged.AddDynamic(this, &UMainHUD::OnPopulationChanged);
	PlayerState->OnStrategicResourceStockChanged.AddDynamic(this, &UMainHUD::OnStrategicResourceStockChanged);
}

void UMainHUD::OnGoldChanged(int32 NewGold)
{
	// 골드 변경 시 HUD 업데이트
	UpdateHUDData();
}

void UMainHUD::OnPopulationChanged(int32 NewPopulation)
{
	// 인구 변경 시 HUD 업데이트
	UpdateHUDData();
}

void UMainHUD::BindFacilityDelegates()
{
	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	UFacilityManager* FacilityManager = SuperGameInst->GetFacilityManager();
	if (!FacilityManager)
	{
		return;
	}

	// 기존 바인딩 해제 후 새로 바인딩
	UnbindFacilityDelegates();

	// 시설 변경 델리게이트 바인딩
	FacilityManager->OnFacilityChanged.AddDynamic(this, &UMainHUD::OnFacilityChanged);
}

void UMainHUD::UnbindFacilityDelegates()
{
	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	UFacilityManager* FacilityManager = SuperGameInst->GetFacilityManager();
	if (FacilityManager)
	{
		// 델리게이트 바인딩 해제
		FacilityManager->OnFacilityChanged.RemoveDynamic(this, &UMainHUD::OnFacilityChanged);
	}
}

void UMainHUD::OnFacilityChanged(FVector2D TileCoordinate)
{
	// 시설 변경 시 HUD 업데이트 (시설로 인한 타일 생산량 변화 반영)
	UpdateHUDData();
	
	// 현재 열린 타일에서 시설이 변경되었으면 UI 닫기 (건설 완료 후 닫기)
	if (bIsFacilityUIOpen && CurrentOpenFacilityTile == TileCoordinate)
	{
		CloseFacilityUI();
	}
}

void UMainHUD::OnGeneralTileClickedHandler(FVector2D TileCoordinate)
{
	// 일반 타일 클릭 시: UI가 열려있으면 닫기 (마지막 클릭한 타일이 건설자 타일이 아니면 닫기)
	if (bIsFacilityUIOpen)
	{
		CloseFacilityUI();
	}
}

void UMainHUD::OnCombatTileHoverBeginHandler(UWorldTile* Tile)
{
	if (!Tile || !UnitCombatUIWidget)
	{
		return;
	}

	// 플레이어 0의 턴인지 확인
	if (UWorld* World = GetWorld())
	{
		if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
		{
			if (UTurnComponent* TurnComponent = GameMode->GetTurnComponent())
			{
				if (TurnComponent->GetCurrentPlayerIndex() != 0)
				{
					return; // 플레이어 0의 턴이 아니면 무시
				}
			}
		}
	}

	// UnitManager 가져오기
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			if (UUnitManager* UnitManager = SuperGameInst->GetUnitManager())
			{
				// 첫 번째 전투 선택이 있는지 확인
				if (!UnitManager->HasCombatFirstSelection())
				{
					return; // 첫 번째 전투 선택이 없으면 무시
				}

				FVector2D HoverHexPos = Tile->GetGridPosition();
				
				// 호버한 타일에 유닛이 있는지 확인
				AUnitCharacterBase* Defender = UnitManager->GetUnitAtHex(HoverHexPos);
				if (!Defender)
				{
					return; // 유닛이 없으면 무시
				}

				// 첫 번째 선택된 타일의 공격자 가져오기
				UWorldTile* FirstSelectedTile = UnitManager->GetCombatFirstSelectedTile();
				if (!FirstSelectedTile)
				{
					return;
				}

				FVector2D AttackerHexPos = FirstSelectedTile->GetGridPosition();
				AUnitCharacterBase* Attacker = UnitManager->GetUnitAtHex(AttackerHexPos);
				if (!Attacker)
				{
					return;
				}

				// 공격 가능한 유닛인지 확인 (FUnitBaseStat::CanAttack)
				if (UUnitStatusComponent* AttackerStatus = Attacker->GetUnitStatusComponent())
				{
					FUnitBaseStat BaseStat = AttackerStatus->GetBaseStat();
					if (!BaseStat.CanAttack)
					{
						return; // 공격 불가능한 유닛
					}
				}
				else
				{
					return; // 상태 컴포넌트가 없으면 무시
				}

				// UnitCombatComponent를 통한 통합 검증 (단순화!)
				if (UUnitCombatComponent* CombatComp = Attacker->GetUnitCombatComponent())
				{
					if (!CombatComp->CanExecuteCombat(Attacker, Defender, AttackerHexPos, HoverHexPos))
					{
						return; // 공격 불가능
					}
				}
				else
				{
					return; // 전투 컴포넌트가 없으면 무시
				}

				// 같은 타일을 호버한 경우 무시 (중복 방지)
				if (bIsCombatUIOpen && CurrentCombatHoverTile == HoverHexPos)
				{
					return;
				}

				// 다른 타일이거나 새로 열어야 하는 경우: 기존 UI 닫고 새로 열기
				if (bIsCombatUIOpen)
				{
					CloseCombatUI();
				}

				// 전투 UI 설정 및 표시
				UnitCombatUIWidget->SetupForCombat(Attacker, Defender, AttackerHexPos, HoverHexPos);
				UnitCombatUIWidget->SetVisibility(ESlateVisibility::Visible);
				CurrentCombatHoverTile = HoverHexPos;
				bIsCombatUIOpen = true;
			}
		}
	}
}

void UMainHUD::OnCombatTileHoverEndHandler(UWorldTile* Tile)
{
	// 호버 종료 시 UI 닫기
	CloseCombatUI();
}

void UMainHUD::OnCombatExecutedHandler()
{
	// 전투 실행 완료 시 UI 닫기
	CloseCombatUI();
}

void UMainHUD::CloseCombatUI()
{
	if (UnitCombatUIWidget)
	{
		UnitCombatUIWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	CurrentCombatHoverTile = FVector2D::ZeroVector;
	bIsCombatUIOpen = false;
}

void UMainHUD::CloseFacilityUI()
{
	if (BuildFacilityUIWidget)
	{
		BuildFacilityUIWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	CurrentOpenFacilityTile = FVector2D::ZeroVector;
	bIsFacilityUIOpen = false;
}

void UMainHUD::OnStrategicResourceStockChanged(EStrategicResource Resource, int32 NewStock)
{
	// 전략 자원 보유량 변경 시 슬롯 업데이트
	UpdateStrategicResourceSlots();
}

void UMainHUD::NativeDestruct()
{
	// 타이머 해제
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(BindCityTileTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(BindBuilderTileTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(BindGeneralTileTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(BindCombatTileHoverTimerHandle);
	}

	// 델리게이트 바인딩 해제
	if (GetWorld())
	{
		// TurnComponent 델리게이트 바인딩 해제
		if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
		{
			if (ASuperGameModeBase* SuperGameMode = Cast<ASuperGameModeBase>(GameMode))
			{
				if (UTurnComponent* TurnComponent = SuperGameMode->GetTurnComponent())
				{
					TurnComponent->OnTurnChanged.RemoveDynamic(this, &UMainHUD::OnTurnChanged);
				}
			}
		}

		// 모든 WorldTileActor의 OnPlayerCityTileClicked 델리게이트 바인딩 해제
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWorldTileActor::StaticClass(), FoundActors);

		for (AActor* Actor : FoundActors)
		{
			if (AWorldTileActor* TileActor = Cast<AWorldTileActor>(Actor))
			{
				TileActor->OnPlayerCityTileClicked.RemoveDynamic(this, &UMainHUD::OnPlayerCityTileClicked);
				TileActor->OnBuilderTileClicked.RemoveDynamic(this, &UMainHUD::OnBuilderTileClickedHandler);
				TileActor->OnGeneralTileClicked.RemoveDynamic(this, &UMainHUD::OnGeneralTileClickedHandler);
				TileActor->OnCombatTileHoverBegin.RemoveDynamic(this, &UMainHUD::OnCombatTileHoverBeginHandler);
				TileActor->OnCombatTileHoverEnd.RemoveDynamic(this, &UMainHUD::OnCombatTileHoverEndHandler);
			}
		}

		// PlayerState 델리게이트 바인딩 해제
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance()))
		{
			if (ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0))
			{
				PlayerState->OnGoldChanged.RemoveDynamic(this, &UMainHUD::OnGoldChanged);
				PlayerState->OnPopulationChanged.RemoveDynamic(this, &UMainHUD::OnPopulationChanged);
				PlayerState->OnStrategicResourceStockChanged.RemoveDynamic(this, &UMainHUD::OnStrategicResourceStockChanged);
			}
		}

		// FacilityManager 델리게이트 바인딩 해제
		UnbindFacilityDelegates();

		// UnitManager의 전투 실행 완료 델리게이트 바인딩 해제
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance()))
		{
			if (UUnitManager* UnitManager = SuperGameInst->GetUnitManager())
			{
				UnitManager->OnCombatExecuted.RemoveDynamic(this, &UMainHUD::OnCombatExecutedHandler);
			}
		}
	}

	// 전략 자원 슬롯 정리
	ClearStrategicResourceSlots();

	Super::NativeDestruct();
}

void UMainHUD::UpdateStrategicResourceSlots()
{
	if (!StrategicResourceHB)
	{
		return;
	}

	if (!GetWorld())
	{
		return;
	}

	USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
	if (!SuperGameInst)
	{
		return;
	}

	ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0);
	if (!PlayerState)
	{
		return;
	}

	UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent();
	if (!WorldComponent)
	{
		return;
	}

	UDataTable* StrategicResourceDataTable = WorldComponent->GetStrategicResourceDataTable();
	if (!StrategicResourceDataTable)
	{
		return;
	}

	// 기존 슬롯 제거
	ClearStrategicResourceSlots();

	// OwnedStrategicResourceStocks 순회
	const TMap<EStrategicResource, int32>& Stocks = PlayerState->OwnedStrategicResourceStocks;
	
	bool bHasAnyStock = false;
	for (const auto& Pair : Stocks)
	{
		EStrategicResource Resource = Pair.Key;
		int32 Stock = Pair.Value;

		// 보유량이 0보다 큰 경우만 슬롯 생성
		if (Stock > 0)
		{
			bHasAnyStock = true;

			// 전략 자원 데이터 가져오기
			FString EnumString = UEnum::GetValueAsString(Resource);
			FString ResourceName = EnumString.RightChop(EnumString.Find(TEXT("::")) + 2);
			FStrategicResourceData* ResourceData = StrategicResourceDataTable->FindRow<FStrategicResourceData>(FName(*ResourceName), TEXT("StrategicResourceData"));

			if (ResourceData)
			{
				// StrategicResourceSlotUI 클래스 로드
				UClass* SlotClass = LoadClass<UStrategicResourceSlotUI>(nullptr, TEXT("/Game/Civilization/Widget/W_StrategicResourceSlot.W_StrategicResourceSlot_C"));
				if (SlotClass)
				{
					// 위젯 생성
					UStrategicResourceSlotUI* ResourceSlot = CreateWidget<UStrategicResourceSlotUI>(this, SlotClass);
					if (ResourceSlot)
					{
						// 아이콘 설정
						UTexture2D* ResourceIcon = nullptr;
						if (!ResourceData->ResourceIcon.IsNull())
						{
							ResourceIcon = ResourceData->ResourceIcon.LoadSynchronous();
						}
						ResourceSlot->SetResourceIcon(ResourceIcon);

						// 수량 설정
						ResourceSlot->SetResourceStockText(Stock);

						// HorizontalBox에 추가
						StrategicResourceHB->AddChild(ResourceSlot);

						// 위젯 가시성 설정
						ResourceSlot->SetVisibility(ESlateVisibility::Visible);
					}
				}
			}
		}
	}

	// 보유량이 하나도 없으면 HorizontalBox 숨김
	if (bHasAnyStock)
	{
		StrategicResourceHB->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		StrategicResourceHB->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMainHUD::ClearStrategicResourceSlots()
{
	if (StrategicResourceHB)
	{
		StrategicResourceHB->ClearChildren();
	}
}

