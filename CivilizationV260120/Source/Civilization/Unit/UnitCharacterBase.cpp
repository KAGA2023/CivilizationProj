// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitCharacterBase.h"
#include "../Status/UnitStatusComponent.h"
#include "../Status/UnitStatusStruct.h"
#include "../Combat/UnitCombatComponent.h"
#include "UnitVisualizationComponent.h"
#include "../Status/UnitDataStruct.h"
#include "../AICon/UnitAIController.h"
#include "../Widget/UnitWidget/SmallUnitUI.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/DecalComponent.h"
#include "Blueprint/UserWidget.h"

AUnitCharacterBase::AUnitCharacterBase()
{
    PrimaryActorTick.bCanEverTick = false;

    // AI Controller 클래스 설정
    AIControllerClass = AUnitAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    // 기본 캐릭터 설정
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.0f));
    // 메시가 반대 방향을 보고 있다면 180도 추가 회전 (90 + 180 = 270)
    GetMesh()->SetRelativeRotation(FRotator(0.f, 270.f, 0.f));

    // 유닛 상태 컴포넌트 생성
    UnitStatusComponent = CreateDefaultSubobject<UUnitStatusComponent>(TEXT("UnitStatusComponent"));

    // 유닛 전투 컴포넌트 생성
    UnitCombatComponent = CreateDefaultSubobject<UUnitCombatComponent>(TEXT("UnitCombatComponent"));

    // 유닛 시각화 컴포넌트 생성
    UnitVisualizationComponent = CreateDefaultSubobject<UUnitVisualizationComponent>(TEXT("UnitVisualizationComponent"));

    // 유닛 머리 위 UI 위젯 컴포넌트 생성
    SmallUnitWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("SmallUnitWidgetComponent"));
    SmallUnitWidgetComponent->SetupAttachment(RootComponent);
    // 머리 위 위치 설정 (캡슐 컴포넌트 위)
    SmallUnitWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 130.0f));
    // 위젯 회전을 절대값으로 설정 (부모 회전 무시)
    SmallUnitWidgetComponent->SetUsingAbsoluteRotation(true);
    // 위젯을 수평으로 고정 (Pitch=0, Yaw=0, Roll=0)
    SmallUnitWidgetComponent->SetWorldRotation(FRotator(60.0f, 180.0f, 0.0f));
    // 위젯을 월드 공간에 배치 (카메라 거리에 따라 크기 변경)
    SmallUnitWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
    // 위젯 크기 설정
    SmallUnitWidgetComponent->SetDrawSize(FVector2D(100.0f, 50.0f));
    // 그림자 비활성화
    SmallUnitWidgetComponent->SetCastShadow(false);
    // 위젯 가시성 설정
    SmallUnitWidgetComponent->SetVisibility(true);

    // 유닛 선택 데칼 생성
    SelectionDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("SelectionDecal"));
    SelectionDecal->SetupAttachment(RootComponent);
    // 데칼을 바닥으로 투영 (Pitch -90도 = 아래 방향)
    SelectionDecal->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
    SelectionDecal->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    // 데칼 크기 설정 (X: 높이, Y/Z: 가로/세로)
    SelectionDecal->DecalSize = FVector(100.0f, 80.0f, 80.0f);
    // 기본적으로 숨김
    SelectionDecal->SetVisibility(false);
    // 데칼 정렬 순서 (높을수록 위에 표시)
    SelectionDecal->SortOrder = 1;
    
    // 데칼 머티리얼 로드
    static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialFinder(TEXT("/Game/Civilization/Materials/M_UnitSelection"));
    if (DecalMaterialFinder.Succeeded())
    {
        SelectionDecal->SetDecalMaterial(DecalMaterialFinder.Object);
    }

    // 유닛끼리 충돌하지 않도록 설정
    // CapsuleComponent 충돌 설정
    if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
    {
        // Pawn 채널에 대해 충돌 무시 (유닛끼리 통과 가능)
        CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    }

    // SkeletalMeshComponent 충돌 설정
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        // Pawn 채널에 대해 충돌 무시 (유닛끼리 통과 가능)
        MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    }

    // 캐릭터 이동 설정
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        // 이동 속도 설정
        MoveComp->MaxWalkSpeed = 500.0f;
        
        // 가속도 없이 즉시 최대 속도에 도달하도록 설정
        MoveComp->MaxAcceleration = 999999.0f; // 매우 큰 값으로 설정하여 즉시 최대 속도 도달
        MoveComp->BrakingDecelerationWalking = 999999.0f; // 감속도 매우 크게 설정
        
        // 이동 방향으로 회전
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.f, 720.f, 0.f); // 회전 속도 (초당 720도)
        
        // 평면 제약 해제 (회전이 제대로 작동하도록)
        MoveComp->bConstrainToPlane = false;
        MoveComp->bSnapToPlaneAtStart = false;
        
        // 회전을 더 부드럽게
        MoveComp->bUseControllerDesiredRotation = false; // MovementComponent가 회전 제어
        
        // 점프 설정
        MoveComp->JumpZVelocity = 1300.0f; // 점프 속도 설정
        MoveComp->bCanWalkOffLedges = true; // 절벽에서 떨어질 수 있도록
        MoveComp->bCanWalkOffLedgesWhenCrouching = true;
        
        // 중력 설정
        MoveComp->GravityScale = 5.0f; // 중력 영향 5배
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
	// RowName 저장 (나중에 FUnitData 접근용)
	UnitDataRowName = RowName;

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
    if (!SmallUnitWidgetComponent)
    {
        return;
    }

    // W_SmallUnit 위젯 클래스 로드
    FSoftClassPath WidgetClassPath(TEXT("/Game/Civilization/Widget/UnitWidget/W_SmallUnit.W_SmallUnit_C"));
    UClass* WidgetClass = WidgetClassPath.TryLoadClass<UUserWidget>();
    
    if (WidgetClass)
    {
        // 위젯 클래스 설정
        SmallUnitWidgetComponent->SetWidgetClass(WidgetClass);
        
        // UI 초기화는 UnitManager에서 스폰 시 처리됨
    }
}

USmallUnitUI* AUnitCharacterBase::GetSmallUnitUI() const
{
    if (!SmallUnitWidgetComponent)
    {
        return nullptr;
    }
    
    return Cast<USmallUnitUI>(SmallUnitWidgetComponent->GetUserWidgetObject());
}

void AUnitCharacterBase::SetUnitSelected(bool bSelected)
{
    bIsSelected = bSelected;
    
    if (SelectionDecal)
    {
        SelectionDecal->SetVisibility(bSelected);
    }
}

void AUnitCharacterBase::SetSelectionColor(FLinearColor Color)
{
    if (SelectionDecal)
    {
        // 동적 머티리얼 인스턴스 생성 (런타임에 색상 변경 가능)
        UMaterialInstanceDynamic* DynMaterial = SelectionDecal->CreateDynamicMaterialInstance();
        
        if (DynMaterial)
        {
            DynMaterial->SetVectorParameterValue(TEXT("TintColor"), Color);
        }
    }
}

const FUnitData* AUnitCharacterBase::GetUnitData() const
{
    // UnitDataTable이 없거나 RowName이 없으면 nullptr 반환
    if (!UnitDataTable || UnitDataRowName == NAME_None)
    {
        return nullptr;
    }

    // RowName으로 FUnitData 가져오기
    return UnitDataTable->FindRow<FUnitData>(UnitDataRowName, TEXT("GetUnitData"));
}