// Fill out your copyright notice in the Description page of Project Settings.

#include "BorderManager.h"
#include "../World/WorldComponent.h"
#include "../World/WorldStruct.h"
#include "ProceduralMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "../SuperGameInstance.h"
#include "../SuperPlayerState.h"
#include "UObject/ConstructorHelpers.h"

UBorderManager::UBorderManager()
{
	BorderThickness = 10.0f;
	BorderHeightOffset = 10.0f;
	EmissiveIntensity = 20.0f; // 라이팅 영향 최소화를 위해 높게 설정
	SphereScale = 0.5f;

	// 기본 보더 머티리얼 로드 (EmissiveColor를 사용하여 라이팅 영향 최소화)
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (MaterialAsset.Succeeded())
	{
		DefaultBorderMaterial = MaterialAsset.Object;
	}
	else
	{
		DefaultBorderMaterial = nullptr;
	}

	// 꼭짓점 구용 스피어 메시 로드 (국경선과 동일 BasicShapeMaterial 사용)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		DefaultSphereMesh = SphereMeshAsset.Object;
	}
	else
	{
		DefaultSphereMesh = nullptr;
	}
}

void UBorderManager::Initialize(UWorldComponent* InWorldComponent, AActor* InParentActor)
{
	WorldComponent = InWorldComponent;
	ParentActor = InParentActor;
}

void UBorderManager::UpdatePlayerBorder(int32 PlayerIndex, const TArray<FVector2D>& OwnedTileCoordinates)
{
	if (!WorldComponent)
	{
		return;
	}

	// SuperPlayerState에서 BorderColor 가져와서 설정
	if (GetWorld())
	{
		if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetWorld()->GetGameInstance()))
		{
			if (ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(PlayerIndex))
			{
				SetPlayerColor(PlayerIndex, PlayerState->BorderColor);
			}
		}
	}

	// 각 소유 타일의 육각형 외곽선 그리기
	TArray<FVector> AllVertices;
	TArray<int32> AllTriangles;
	TArray<FVector> AllNormals;
	TArray<FVector2D> AllUVs;
	TArray<FVector> BorderCornerPositions; // 꼭짓점 구 배치용 (중복 제거)

	// 각 소유 타일의 꼭짓점 확인
	for (const FVector2D& TileCoord : OwnedTileCoordinates)
	{
		// 해당 타일의 6개 꼭짓점 가져오기
		TArray<FVector> HexVertices = GetHexVertices(TileCoord, BorderHeightOffset);

		// 이웃한 꼭짓점끼리 엣지로 연결 (0-1, 1-2, 2-3, 3-4, 4-5, 5-0)
		for (int32 i = 0; i < 6; i++)
		{
			// 외곽 엣지인지 확인 (같은 플레이어 소유 타일과 맞닿아 있지 않은 엣지만 그리기)
			if (!IsOuterEdge(TileCoord, i, OwnedTileCoordinates))
			{
				continue; // 내부 엣지는 건너뛰기
			}

			int32 StartIndex = i;
			int32 EndIndex = (i + 1) % 6; // 다음 꼭짓점 (5 다음은 0)

			FVector StartVertex = HexVertices[StartIndex];
			FVector EndVertex = HexVertices[EndIndex];

			// 구를 놓을 꼭짓점 목록에 추가 (이미 가까운 위치가 있으면 건너뜀)
			AddBorderCornerIfNew(BorderCornerPositions, StartVertex);
			AddBorderCornerIfNew(BorderCornerPositions, EndVertex);

			// 엣지 방향 벡터
			FVector EdgeDirection = (EndVertex - StartVertex).GetSafeNormal();
			if (EdgeDirection.IsNearlyZero())
			{
				continue;
			}

			FVector UpVector = FVector(0.0f, 0.0f, 1.0f);
			FVector RightVector = FVector::CrossProduct(EdgeDirection, UpVector).GetSafeNormal();

			// 두께만큼 오프셋
			FVector Offset = RightVector * (BorderThickness * 0.5f);

			// 4개 버텍스 생성 (사각형)
			int32 VertexOffset = AllVertices.Num();
			AllVertices.Add(StartVertex - Offset); // 왼쪽 아래
			AllVertices.Add(StartVertex + Offset); // 왼쪽 위
			AllVertices.Add(EndVertex + Offset);   // 오른쪽 위
			AllVertices.Add(EndVertex - Offset);   // 오른쪽 아래

			// 트라이앵글 인덱스 (2개 트라이앵글로 사각형 구성)
			AllTriangles.Add(VertexOffset + 0);
			AllTriangles.Add(VertexOffset + 2);
			AllTriangles.Add(VertexOffset + 1);
			AllTriangles.Add(VertexOffset + 0);
			AllTriangles.Add(VertexOffset + 3);
			AllTriangles.Add(VertexOffset + 2);

			// 노말 벡터 (위쪽 - 카메라를 향하도록)
			FVector UpVectorNormal = FVector(0.0f, 0.0f, 1.0f);
			for (int32 j = 0; j < 4; j++)
			{
				AllNormals.Add(UpVectorNormal);
			}

			// UV 좌표
			AllUVs.Add(FVector2D(0.0f, 0.0f));
			AllUVs.Add(FVector2D(0.0f, 1.0f));
			AllUVs.Add(FVector2D(1.0f, 1.0f));
			AllUVs.Add(FVector2D(1.0f, 0.0f));
		}
	}

	// ProceduralMeshComponent 생성/업데이트
	if (AllVertices.Num() > 0)
	{
		GenerateBorderMesh(PlayerIndex, AllVertices, AllTriangles, AllNormals, AllUVs);
		CreateSpheresAtVertices(PlayerIndex, BorderCornerPositions);
	}
	else
	{
		// 버텍스가 없으면 메시 제거
		RemovePlayerBorder(PlayerIndex);
	}
}

void UBorderManager::UpdateAllBorders()
{
	if (!WorldComponent || !GetWorld())
	{
		return;
	}

	// SuperGameInstance에서 모든 PlayerState 가져오기
	if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetWorld()->GetGameInstance()))
	{
		for (int32 i = 0; i < GameInstance->GetPlayerStateCount(); i++)
		{
			if (ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(i))
			{
				TArray<FVector2D> OwnedTiles = PlayerState->GetOwnedTileCoordinates();
				UpdatePlayerBorder(i, OwnedTiles);
			}
		}
	}
}

void UBorderManager::RemovePlayerBorder(int32 PlayerIndex)
{
	// 꼭짓점 구 제거
	if (TArray<UStaticMeshComponent*>* SphereList = PlayerBorderSphereComponents.Find(PlayerIndex))
	{
		for (UStaticMeshComponent* SphereComp : *SphereList)
		{
			if (SphereComp)
			{
				SphereComp->DestroyComponent();
			}
		}
		PlayerBorderSphereComponents.Remove(PlayerIndex);
	}

	if (TObjectPtr<UProceduralMeshComponent>* MeshCompPtr = PlayerBorderMeshes.Find(PlayerIndex))
	{
		if (UProceduralMeshComponent* MeshComp = MeshCompPtr->Get())
		{
			MeshComp->ClearAllMeshSections();
			MeshComp->DestroyComponent();
		}
		PlayerBorderMeshes.Remove(PlayerIndex);
	}
	
	// MaterialInstanceDynamic도 제거
	PlayerMaterials.Remove(PlayerIndex);
}

void UBorderManager::ClearAllBorders()
{
	for (auto& Pair : PlayerBorderSphereComponents)
	{
		for (UStaticMeshComponent* SphereComp : Pair.Value)
		{
			if (SphereComp)
			{
				SphereComp->DestroyComponent();
			}
		}
	}
	PlayerBorderSphereComponents.Empty();

	for (auto& Pair : PlayerBorderMeshes)
	{
		if (Pair.Value)
		{
			Pair.Value->ClearAllMeshSections();
			Pair.Value->DestroyComponent();
		}
	}
	PlayerBorderMeshes.Empty();
	PlayerMaterials.Empty();
}

void UBorderManager::SetPlayerColor(int32 PlayerIndex, FLinearColor Color)
{
	// 메인 메뉴에서만 호출됨 (게임 중 색상 변경 불가)
	// 레벨 진입 전에 설정되어야 하며, 메시 생성 시점에 적용됨
	PlayerColors.Add(PlayerIndex, Color);
}

FLinearColor UBorderManager::GetPlayerColor(int32 PlayerIndex) const
{
	if (const FLinearColor* Color = PlayerColors.Find(PlayerIndex))
	{
		return *Color;
	}
	return FLinearColor::White; // 기본값
}

bool UBorderManager::IsOuterEdge(const FVector2D& TileCoord, int32 EdgeIndex, const TArray<FVector2D>& OwnedTileCoordinates)
{
	// 인접 타일 좌표 가져오기
	FVector2D NeighborCoord = GetNeighborHexAtEdge(TileCoord, EdgeIndex);

	// 인접 타일이 같은 플레이어 소유가 아니면 외곽 엣지
	return !OwnedTileCoordinates.Contains(NeighborCoord);
}

FVector2D UBorderManager::GetNeighborHexAtEdge(const FVector2D& HexCoord, int32 EdgeIndex)
{
	// WorldComponent의 GetHexNeighbors를 사용하여 Pointy top 좌표계에서 올바른 인접 타일 가져오기
	if (!WorldComponent)
	{
		return HexCoord;
	}

	TArray<FVector2D> Neighbors = WorldComponent->GetHexNeighbors(HexCoord);
	
	// Pointy top 헥스에서 엣지 인덱스와 인접 타일 인덱스 매핑
	// GetHexNeighbors 순서: 0=오른쪽, 1=왼쪽, 2=오른쪽 아래, 3=왼쪽 위, 4=오른쪽 위, 5=왼쪽 아래
	// 엣지 인덱스는 GetHexVertices의 꼭짓점 순서와 일치 (0-1, 1-2, 2-3, 3-4, 4-5, 5-0)
	// 각 엣지가 가리키는 방향에 해당하는 인접 타일을 올바르게 매핑
	int32 NeighborIndex = -1;
	switch (EdgeIndex)
	{
	case 0: // 좌상 엣지 → 왼쪽 아래 인접 타일 (Neighbors[5])
		NeighborIndex = 5;
		break;
	case 1: // 우상 엣지 → 오른쪽 아래 인접 타일 (Neighbors[2])
		NeighborIndex = 2;
		break;
	case 2: // 우 엣지 → 오른쪽 인접 타일 (Neighbors[0])
		NeighborIndex = 0;
		break;
	case 3: // 우하 엣지 → 오른쪽 위 인접 타일 (Neighbors[4])
		NeighborIndex = 4;
		break;
	case 4: // 좌하 엣지 → 왼쪽 위 인접 타일 (Neighbors[3])
		NeighborIndex = 3;
		break;
	case 5: // 좌 엣지 → 왼쪽 인접 타일 (Neighbors[1])
		NeighborIndex = 1;
		break;
	default:
		return HexCoord; // 실패 시 원래 좌표 반환
	}
	
	if (NeighborIndex >= 0 && NeighborIndex < Neighbors.Num())
	{
		return Neighbors[NeighborIndex];
	}
	
	return HexCoord;
}

FVector UBorderManager::HexToWorldPosition(const FVector2D& HexCoord, float HeightOffset)
{
	if (WorldComponent)
	{
		FVector WorldPos = WorldComponent->HexToWorld(HexCoord);
		
		// 타일의 지형 높이에 따른 Z 오프셋 추가
		if (UWorldTile* Tile = WorldComponent->GetTileAtHex(HexCoord))
		{
			ELandType LandType = Tile->GetLandType();
			float TileZOffset = 0.0f;
			
			switch (LandType)
			{
			case ELandType::Plains:
				TileZOffset = 71.0f; // 평지
				break;
			case ELandType::Hills:
				TileZOffset = 142.0f; // 언덕
				break;
			case ELandType::Mountains:
				TileZOffset = 213.0f; // 산
				break;
			default:
				TileZOffset = 71.0f; // 기본값
				break;
			}
			
			WorldPos.Z = TileZOffset + HeightOffset;
		}
		else
		{
			// 타일이 없으면 기본 높이 + 오프셋
			WorldPos.Z = 74.0f + HeightOffset;
		}
		
		return WorldPos;
	}
	return FVector::ZeroVector;
}

TArray<FVector> UBorderManager::GetHexVertices(const FVector2D& HexCoord, float HeightOffset)
{
	TArray<FVector> Vertices;
	
	if (!WorldComponent)
	{
		return Vertices;
	}
	
	// 헥스 중심점 가져오기
	FVector Center = HexToWorldPosition(HexCoord, HeightOffset);
	
	// 국경 육각형 크기 (Pointy top 헥스)
	const float TileSize = 175.0f;
	
	// Pointy top 헥스의 6개 꼭짓점 계산
	// 각 꼭짓점은 중심점에서 -60도, 0도, 60도, 120도, 180도, 240도 방향에 위치 (-90도 회전)
	for (int32 i = 0; i < 6; i++)
	{
		float Angle = FMath::DegreesToRadians(30.0f + i * 60.0f - 90.0f); // Pointy top은 30도에서 시작, -90도 회전
		
		// 방향 벡터 계산 (X, Y 평면)
		// 언리얼 좌표계: X=앞뒤, Y=좌우
		FVector Direction = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f);
		
		// 꼭짓점 위치 = 중심점 + 방향 * 반지름
		FVector Vertex = Center + Direction * TileSize;
		Vertices.Add(Vertex);
	}
	
	return Vertices;
}


UProceduralMeshComponent* UBorderManager::GetOrCreateBorderMesh(int32 PlayerIndex)
{
	// 이미 존재하면 반환
	if (TObjectPtr<UProceduralMeshComponent>* ExistingMeshPtr = PlayerBorderMeshes.Find(PlayerIndex))
	{
		if (UProceduralMeshComponent* ExistingMesh = ExistingMeshPtr->Get())
		{
			return ExistingMesh;
		}
	}

	// 새로 생성 (ParentActor에 부착)
	if (ParentActor)
	{
		UProceduralMeshComponent* NewMesh = NewObject<UProceduralMeshComponent>(ParentActor);
		if (NewMesh)
		{
			NewMesh->SetupAttachment(ParentActor->GetRootComponent());
			NewMesh->RegisterComponent();

		// 머티리얼 설정 (MaterialInstanceDynamic으로 색상 직접 설정)
		if (DefaultBorderMaterial)
		{
			// MaterialInstanceDynamic을 생성하여 플레이어 색상을 직접 설정
			UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(DefaultBorderMaterial, ParentActor);
			if (DynamicMaterial)
			{
				// SuperPlayerState에서 BorderColor 가져오기
				FLinearColor PlayerColor = FLinearColor::White; // 기본값
				if (GetWorld())
				{
					if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(GetWorld()->GetGameInstance()))
					{
						if (ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(PlayerIndex))
						{
							PlayerColor = PlayerState->BorderColor;
						}
					}
				}
				
				// PlayerColors 맵에도 저장
				PlayerColors.Add(PlayerIndex, PlayerColor);
				
				// EmissiveColor를 매우 밝게 설정하여 라이팅 영향 최소화
				// 라이팅이 어둡게 만들기 때문에 EmissiveColor를 매우 밝게 설정
				FLinearColor BrightColor = PlayerColor * EmissiveIntensity;
				
				// 여러 파라미터 이름 시도 (머티리얼에 따라 다름)
				// EmissiveColor를 매우 밝게 설정하여 라이팅 영향 무시
				DynamicMaterial->SetVectorParameterValue(FName("EmissiveColor"), BrightColor);
				DynamicMaterial->SetVectorParameterValue(FName("Emissive"), BrightColor);
				// Emissive Intensity를 별도로 설정 (일부 머티리얼에서 필요)
				DynamicMaterial->SetScalarParameterValue(FName("EmissiveIntensity"), EmissiveIntensity);
				DynamicMaterial->SetScalarParameterValue(FName("EmissivePower"), EmissiveIntensity);
				DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), PlayerColor);
				DynamicMaterial->SetVectorParameterValue(FName("Color"), PlayerColor);
				DynamicMaterial->SetVectorParameterValue(FName("DiffuseColor"), PlayerColor);
				
				NewMesh->SetMaterial(0, DynamicMaterial);
				
				// MaterialInstanceDynamic 저장
				PlayerMaterials.Add(PlayerIndex, DynamicMaterial);
			}
			else
			{
				// Dynamic Material 생성 실패 시 기본 머티리얼 사용
				NewMesh->SetMaterial(0, DefaultBorderMaterial);
			}
		}

		// CastShadow 비활성화
		NewMesh->SetCastShadow(false);
		
		// Visibility 설정
		NewMesh->SetVisibility(true);

		PlayerBorderMeshes.Add(PlayerIndex, NewMesh);
			return NewMesh;
		}
	}

	return nullptr;
}

void UBorderManager::GenerateBorderMesh(int32 PlayerIndex, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, const TArray<FVector>& Normals, const TArray<FVector2D>& UVs)
{
	UProceduralMeshComponent* MeshComp = GetOrCreateBorderMesh(PlayerIndex);
	if (!MeshComp)
	{
		return;
	}

	// 기존 메시 제거
	MeshComp->ClearAllMeshSections();

	// 색상은 GetOrCreateBorderMesh()에서 처음 생성될 때만 설정됨
	// 게임 중 색상 변경이 없으므로 여기서 업데이트할 필요 없음

	// FLinearColor를 FColor로 변환 (흰색으로 설정 - MaterialInstanceDynamic의 색상이 적용됨)
	TArray<FColor> VertexColors;
	FColor WhiteColor = FColor::White;
	for (int32 i = 0; i < Vertices.Num(); i++)
	{
		VertexColors.Add(WhiteColor); // 모든 버텍스를 흰색으로 설정
	}

	// 새 메시 생성
	MeshComp->CreateMeshSection(
		0,
		Vertices,
		Triangles,
		Normals,
		UVs,
		VertexColors,
		TArray<FProcMeshTangent>(),
		false // Collision 비활성화
	);
}

void UBorderManager::CreateSpheresAtVertices(int32 PlayerIndex, const TArray<FVector>& CornerPositions)
{
	if (!ParentActor || !DefaultSphereMesh || CornerPositions.Num() == 0)
	{
		return;
	}

	// 기존 꼭짓점 구 제거
	if (TArray<UStaticMeshComponent*>* ExistingList = PlayerBorderSphereComponents.Find(PlayerIndex))
	{
		for (UStaticMeshComponent* SphereComp : *ExistingList)
		{
			if (SphereComp)
			{
				SphereComp->DestroyComponent();
			}
		}
		PlayerBorderSphereComponents.Remove(PlayerIndex);
	}

	// 국경선과 동일 머티리얼 사용
	UMaterialInterface* SphereMaterial = nullptr;
	if (TObjectPtr<UMaterialInstanceDynamic>* MatPtr = PlayerMaterials.Find(PlayerIndex))
	{
		SphereMaterial = MatPtr->Get();
	}
	if (!SphereMaterial && DefaultBorderMaterial)
	{
		SphereMaterial = DefaultBorderMaterial;
	}

	// 엔진 스피어 반지름 50 기준, 원하는 월드 반지름 = BorderThickness * SphereScale
	const float EngineSphereRadius = 50.0f;
	const float WorldRadius = BorderThickness * SphereScale;
	const float Scale = WorldRadius / EngineSphereRadius;

	TArray<UStaticMeshComponent*> NewSpheres;
	for (const FVector& CornerPos : CornerPositions)
	{
		UStaticMeshComponent* SphereComp = NewObject<UStaticMeshComponent>(ParentActor);
		if (!SphereComp)
		{
			continue;
		}
		SphereComp->SetupAttachment(ParentActor->GetRootComponent());
		SphereComp->SetStaticMesh(DefaultSphereMesh);
		if (SphereMaterial)
		{
			SphereComp->SetMaterial(0, SphereMaterial);
		}
		SphereComp->SetWorldLocation(CornerPos);
		SphereComp->SetWorldScale3D(FVector(Scale));
		SphereComp->SetCastShadow(false);
		SphereComp->SetVisibility(true);
		SphereComp->RegisterComponent();

		NewSpheres.Add(SphereComp);
	}

	if (NewSpheres.Num() > 0)
	{
		PlayerBorderSphereComponents.Add(PlayerIndex, MoveTemp(NewSpheres));
	}
}

void UBorderManager::AddBorderCornerIfNew(TArray<FVector>& InOutCornerList, const FVector& NewPosition)
{
	const float Tolerance = 2.0f; // 이 거리 안에 있으면 같은 꼭짓점으로 봄

	for (const FVector& Existing : InOutCornerList)
	{
		if (FVector::Dist(Existing, NewPosition) < Tolerance)
		{
			return; // 이미 비슷한 위치가 있으면 추가하지 않음
		}
	}

	InOutCornerList.Add(NewPosition);
}

