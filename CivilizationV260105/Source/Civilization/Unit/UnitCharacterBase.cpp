// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitCharacterBase.h"
#include "../Status/UnitStatusComponent.h"
#include "../Combat/UnitCombatComponent.h"
#include "../Status/UnitDataStruct.h"
#include "../AICon/UnitAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

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

    // ========== 무기 메시 컴포넌트 생성 ==========
    WeaponMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMeshComponent"));
    WeaponMeshComponent->SetupAttachment(GetMesh());  // 스켈레탈 메시에 부착 (소켓은 나중에 설정)
    
    // 무기 메시는 콜리전 비활성화 (별도 콜리전 컴포넌트 사용)
    WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // 인게임에서 무기 메시 표시
    WeaponMeshComponent->SetHiddenInGame(false);

    // ========== 무기 콜리전 컴포넌트 생성 (pre-Weapon 타입) ==========
    WeaponCollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponCollisionComp"));
    WeaponCollisionComp->SetupAttachment(WeaponMeshComponent);  // 무기 메시에 부착
    WeaponCollisionComp->SetCapsuleHalfHeight(20.0f);  // 무기 콜리전 높이
    WeaponCollisionComp->SetCapsuleRadius(5.0f);  // 무기 콜리전 반지름
    WeaponCollisionComp->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    
    // 무기 콜리전 설정 (pre-Weapon 프리셋 사용)
    // QueryOnly로 설정하여 오버랩만 감지
    WeaponCollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    WeaponCollisionComp->SetCollisionProfileName(FName(TEXT("pre-Weapon")));
    
    // 인게임에서 콜리전 시각화 (디버깅용)
    WeaponCollisionComp->SetHiddenInGame(false);

    // ========== Body 콜리전 컴포넌트 생성 (피격 감지용) ==========
    BodyCollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BodyCollisionComp"));
    BodyCollisionComp->SetupAttachment(RootComponent);
    BodyCollisionComp->SetCapsuleHalfHeight(90.0f);  // 기본 캐릭터 높이와 동일
    BodyCollisionComp->SetCapsuleRadius(34.0f);  // 기본 캐릭터 반지름과 동일
    BodyCollisionComp->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    
    // Body 콜리전 설정 (pre-Body 프리셋 사용)
    // 프리셋에 이미 모든 설정이 포함되어 있음 (QueryOnly, 모든 채널 무시, Weapon 채널 오버랩 등)
    BodyCollisionComp->SetCollisionProfileName(FName(TEXT("pre-Body")));
    
    // 인게임에서 콜리전 시각화 (디버깅용)
    BodyCollisionComp->SetHiddenInGame(false);
    
    // 오버랩 이벤트 바인딩
    BodyCollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AUnitCharacterBase::OnBodyCollisionBeginOverlap);

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
        MoveComp->JumpZVelocity = 1000.0f; // 점프 속도 설정
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
	SetupUnitWeapon(*UnitData);
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

void AUnitCharacterBase::SetupUnitWeapon(const FUnitData& UnitData)
{
    if (!WeaponMeshComponent)
    {
        return;
    }

    // 무기 메시 설정
    if (UnitData.WeaponMesh)
    {
        WeaponMeshComponent->SetStaticMesh(UnitData.WeaponMesh);
        
        // 소켓 이름이 지정되어 있으면 해당 소켓에 부착
        if (UnitData.WeaponSocketName != NAME_None && GetMesh())
        {
            WeaponMeshComponent->AttachToComponent(
                GetMesh(),
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                UnitData.WeaponSocketName
            );
        }
    }
    else
    {
        // 무기 메시가 없으면 컴포넌트 비활성화
        WeaponMeshComponent->SetStaticMesh(nullptr);
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

void AUnitCharacterBase::OnBodyCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                                      AActor* OtherActor,
                                                      UPrimitiveComponent* OtherComp,
                                                      int32 OtherBodyIndex,
                                                      bool bFromSweep,
                                                      const FHitResult& SweepResult)
{
    // 유효성 검사
    if (!OtherActor || !OtherComp)
    {
        return;
    }

    // OtherComp가 무기 콜리전인지 확인 (pre-Weapon 타입)
    // 블루프린트에서 설정한 무기 콜리전 컴포넌트와 충돌했는지 확인
    // 무기 콜리전은 유닛의 자식 컴포넌트이므로, OtherActor가 유닛이거나
    // OtherComp의 Owner가 유닛일 것입니다.
    
    // OtherActor가 AUnitCharacterBase인지 확인 (무기가 직접 유닛에 붙어있는 경우)
    AUnitCharacterBase* AttackerUnit = Cast<AUnitCharacterBase>(OtherActor);
    
    // OtherActor가 유닛이 아니면, OtherComp의 Owner를 확인 (무기가 별도 액터인 경우)
    if (!AttackerUnit && OtherComp)
    {
        AttackerUnit = Cast<AUnitCharacterBase>(OtherComp->GetOwner());
    }

    if (!AttackerUnit || AttackerUnit == this)
    {
        return;  // 자기 자신과의 충돌은 무시
    }

    // AIController를 통해 전투 중인지 확인
    AUnitAIController* ThisAIController = Cast<AUnitAIController>(GetController());
    AUnitAIController* AttackerAIController = Cast<AUnitAIController>(AttackerUnit->GetController());
    
    if (!ThisAIController || !AttackerAIController)
    {
        return;
    }

    // 전투 중인지 확인
    //if (!ThisAIController->IsInCombat() || !AttackerAIController->IsInCombat())
    //{
    //    return;
    //}

    // 전투 데이터 확인 (상대 유닛이 맞는지)
    AUnitCharacterBase* CombatAttacker = ThisAIController->GetCombatAttacker();
    AUnitCharacterBase* CombatDefender = ThisAIController->GetCombatDefender();
    
    // 현재 유닛이 방어자이고, 충돌한 유닛이 공격자인지 확인
    if (CombatDefender == this && CombatAttacker == AttackerUnit)
    {
        // 피격 이벤트 발생
        OnHitCollisionReceived.Broadcast();
    }
    // 반격 상황: 현재 유닛이 공격자이고, 충돌한 유닛이 방어자(반격 중)인지 확인
    else if (CombatAttacker == this && CombatDefender == AttackerUnit)
    {
        // 반격 피격 이벤트 발생
        OnHitCollisionReceived.Broadcast();
    }
}