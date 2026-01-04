// Fill out your copyright notice in the Description page of Project Settings.

#include "AIPlayerStruct.h"
#include "../SuperPlayerState.h"

// ================= FReservedTiles 구현 =================

void FReservedTiles::Add(const FVector2D& Tile)
{
    ReservedTileSet.Add(Tile);
}

void FReservedTiles::Remove(const FVector2D& Tile)
{
    ReservedTileSet.Remove(Tile);
}

bool FReservedTiles::Contains(const FVector2D& Tile) const
{
    return ReservedTileSet.Contains(Tile);
}

int32 FReservedTiles::Num() const
{
    return ReservedTileSet.Num();
}

void FReservedTiles::Clear()
{
    ReservedTileSet.Empty();
}

bool FReservedTiles::IsEmpty() const
{
    return ReservedTileSet.Num() == 0;
}