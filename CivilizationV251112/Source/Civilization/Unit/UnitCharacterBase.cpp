// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitCharacterBase.h"
#include "../Status/UnitStatusComponent.h"
#include "UnitDataStruct.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"

AUnitCharacterBase::AUnitCharacterBase()
{
    PrimaryActorTick.bCanEverTick = false;

    // 기본 캐릭터 설정
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));

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

    // UnitDataTable 로드
    LoadUnitDataTable();
}

void AUnitCharacterBase::InitializeUnit(const FName& RowName)
{
	// UnitStatusComponent가 직접 스테이터스 데이터 로드 및 초기화
	if (UnitStatusComponent)
	{
		UnitStatusComponent->InitializeFromRowName(RowName);
	}

	// UnitDataTable 로드 확인
	if (!UnitDataTable)
	{
		LoadUnitDataTable();
		
		// 다시 확인
		if (!UnitDataTable)
		{
			return;
		}
	}

	// RowName으로 시각적 데이터 로딩
	FUnitData* UnitData = UnitDataTable->FindRow<FUnitData>(RowName, TEXT("InitializeUnit"));
	if (!UnitData)
	{
		return;
	}

	// 외형 설정
	SetupUnitMesh(*UnitData);
	SetupUnitSounds(*UnitData);
	SetupUnitUI(*UnitData);
}


void AUnitCharacterBase::LoadUnitDataTable()
{
	// SoftObjectPath를 사용한 로딩
	FSoftObjectPath UnitDataTablePath(TEXT("/Game/Civilization/Data/DT_UnitData.DT_UnitData"));
	UnitDataTable = Cast<UDataTable>(UnitDataTablePath.TryLoad());
}

void AUnitCharacterBase::SetupUnitMesh(const FUnitData& UnitData)
{
    if (UnitData.UnitMesh && GetMesh())
    {
        // 스켈레탈 메시 설정
        GetMesh()->SetSkeletalMesh(UnitData.UnitMesh);
        
        // 애니메이션 클래스 설정
        if (UnitData.AnimClass)
        {
            GetMesh()->SetAnimInstanceClass(UnitData.AnimClass);
        }
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