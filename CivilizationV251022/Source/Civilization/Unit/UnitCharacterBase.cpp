// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitCharacterBase.h"
#include "../Status/UnitStatusComponent.h"
#include "../Status/UnitStatusStruct.h"
#include "UnitDataStruct.h"
#include "UnitSpawnStruct.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"

AUnitCharacterBase::AUnitCharacterBase()
{
    PrimaryActorTick.bCanEverTick = false;

    // 기본 캐릭터 설정
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

    // 유닛 상태 컴포넌트 생성
    UnitStatusComponent = CreateDefaultSubobject<UUnitStatusComponent>(TEXT("UnitStatusComponent"));

    // 캐릭터 이동 설정
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.f, 720.f, 0.f);
        MoveComp->bConstrainToPlane = true;
        MoveComp->bSnapToPlaneAtStart = true;
    }
}

void AUnitCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    // 데이터 테이블 로드
    LoadDataTables();
}

void AUnitCharacterBase::InitializeUnit(const FName& UnitID)
{
    if (!UnitSpawnTable || !UnitDataTable || !UnitStatusTable)
    {
        return;
    }

    FUnitSpawnData* SpawnData = UnitSpawnTable->FindRow<FUnitSpawnData>(UnitID, TEXT("InitializeUnit"));
    if (!SpawnData)
    {
        return;
    }

    FUnitData* UnitData = UnitDataTable->FindRow<FUnitData>(SpawnData->DataTableRowName, TEXT("InitializeUnit"));
    if (!UnitData)
    {
        return;
    }

    FUnitBaseStat* UnitStat = UnitStatusTable->FindRow<FUnitBaseStat>(SpawnData->DataTableRowName, TEXT("InitializeUnit"));
    if (!UnitStat)
    {
        return;
    }

    InitializeUnitFromData(*SpawnData, *UnitData, *UnitStat);
}

void AUnitCharacterBase::InitializeUnitFromData(const FUnitSpawnData& SpawnData, const FUnitData& UnitData, const FUnitBaseStat& UnitStat)
{
    // 스테이터스 컴포넌트 초기화
    if (UnitStatusComponent)
    {
        UnitStatusComponent->InitFromBaseStat(UnitStat);
    }

    // 외형 설정
    SetupUnitMesh(UnitData);
    SetupUnitSounds(UnitData);
    SetupUnitUI(UnitData);

    // 스폰 위치 설정
    if (SpawnData.DefaultSpawnLocation != FVector::ZeroVector)
    {
        SetActorLocation(SpawnData.DefaultSpawnLocation);
    }

    if (SpawnData.DefaultSpawnRotation != FRotator::ZeroRotator)
    {
        SetActorRotation(SpawnData.DefaultSpawnRotation);
    }

}

void AUnitCharacterBase::LoadDataTables()
{
    UnitSpawnTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Civilization/Data/DT_UnitSpawn.DT_UnitSpawn"));
    UnitDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Civilization/Data/DT_UnitData.DT_UnitData"));
    UnitStatusTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Civilization/Data/DT_UnitStatus.DT_UnitStatus"));
}

void AUnitCharacterBase::SetupUnitMesh(const FUnitData& UnitData)
{
    if (UnitData.UnitMesh && GetMesh())
    {
        // 스켈레탈 메시 설정
        GetMesh()->SetSkeletalMesh(UnitData.UnitMesh);
    }
}

void AUnitCharacterBase::SetupUnitSounds(const FUnitData& UnitData)
{
    // 사운드 설정은 나중에 오디오 시스템과 연동하여 구현
    // 현재는 데이터만 저장 (필요시 AudioComponent 추가 가능)
}

void AUnitCharacterBase::SetupUnitUI(const FUnitData& UnitData)
{
    // UI 위젯 설정은 나중에 UI 시스템과 연동하여 구현
    // 현재는 데이터만 저장 (필요시 WidgetComponent 추가 가능)
}