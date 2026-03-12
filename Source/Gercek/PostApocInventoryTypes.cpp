// Fill out your copyright notice in the Description page of Project Settings.

#include "PostApocInventoryTypes.h"

// ============================================================
//  UPostApocInventoryComponent — Implementasyon
// ============================================================

UPostApocInventoryComponent::UPostApocInventoryComponent()
{
    // Tick kapalı: ızgara durumu sadece mutasyonlarda değişir,
    // her frame güncellenmesi gereksizdir.
    PrimaryComponentTick.bCanEverTick = false;
}

void UPostApocInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    // Başlangıçta ızgara boş; OccupiedSlots zaten TMap ile sıfırlanmış.
}

// ------------------------------------------------------------
//  CalculateBarterValue
//  Formül: InBaseValue * (InCondition * ConditionWeight)
// ------------------------------------------------------------
float UPostApocInventoryComponent::CalculateBarterValue(
    float InBaseValue, float InCondition, float ConditionWeight) const
{
    // Kondisyonu ve ağırlığı 0-1 aralığında sıkıştır; negatif değerleri engelle.
    const float SafeCondition = FMath::Clamp(InCondition, 0.0f, 1.0f);
    const float SafeWeight    = FMath::Max(ConditionWeight, 0.0f);

    return InBaseValue * (SafeCondition * SafeWeight);
}

// ------------------------------------------------------------
//  CheckSpace
//  TopLeftIndex'ten başlayarak ItemSize (döndürme hesaba katılarak)
//  kadar alanın müsait olup olmadığını kontrol eder.
// ------------------------------------------------------------
bool UPostApocInventoryComponent::CheckSpace(
    FIntPoint TopLeftIndex, FIntPoint ItemSize, bool bIsRotated) const
{
    // Döndürme aktifse X (genişlik) ve Y (yükseklik) yer değiştirir.
    const int32 EffectiveWidth  = bIsRotated ? ItemSize.Y : ItemSize.X;
    const int32 EffectiveHeight = bIsRotated ? ItemSize.X : ItemSize.Y;

    // Her eşyanın boyutu en az 1x1 olmalı.
    if (EffectiveWidth <= 0 || EffectiveHeight <= 0)
    {
        return false;
    }

    // Tüm hücreleri sola-üstten sağa-alta tara.
    for (int32 Row = 0; Row < EffectiveHeight; ++Row)
    {
        for (int32 Col = 0; Col < EffectiveWidth; ++Col)
        {
            const int32 CheckX = TopLeftIndex.X + Col;
            const int32 CheckY = TopLeftIndex.Y + Row;

            // --- Sınır Kontrolü ---
            // Hücre ızgara sınırlarının dışındaysa yer yok.
            if (CheckX < 0 || CheckX >= GridColumns ||
                CheckY < 0 || CheckY >= GridRows)
            {
                return false;
            }

            // --- Çakışma Kontrolü ---
            // Bu koordinatta zaten bir eşya varsa yer yok.
            const FIntPoint CellToCheck(CheckX, CheckY);
            if (OccupiedSlots.Contains(CellToCheck))
            {
                return false;
            }
        }
    }

    // Tüm hücreler boş ve sınır içi: yer var.
    return true;
}
