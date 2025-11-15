// Fill out your copyright notice in the Description page of Project Settings.

#include "FacilityManager.h"

UFacilityManager::UFacilityManager()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UFacilityManager::BeginPlay()
{
    Super::BeginPlay();
}

void UFacilityManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
