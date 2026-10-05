// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "UnitAIController.generated.h"


UCLASS()
class CIVILIZATION_API AUnitAIController : public AAIController
{
    GENERATED_BODY()

public:
    // 유닛용 AI 컨트롤러를 만듭니다.
    AUnitAIController();

protected:
    // 플레이 시작 시 부모 BeginPlay를 호출합니다.
    virtual void BeginPlay() override;
};
