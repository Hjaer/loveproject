// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// --- Standard Unreal includes first ---
#include "CoreMinimal.h"
#include "Engine/DataTable.h"

// --- GENERATED HEADER: MUST BE THE LAST INCLUDE. NON-NEGOTIABLE. ---
// clang-format off
#include "ItemTypes.generated.h"
// clang-format on

// ============================================================
//  ENUMS
// ============================================================

/**
 * EItemType
 *
 * Bir eşyanın oyun-mekanik kategorisi.
 * Loot tabloları, UI filtreleri ve tüketim mantığı bu enum'a göre ayrışır.
 */
UENUM(BlueprintType)
enum class EItemType : uint8 {
  Junk UMETA(DisplayName = "Junk (Hurda)"),
  Food UMETA(DisplayName = "Food (Yiyecek)"),
  Drink UMETA(DisplayName = "Drink (Icecek)"),
  Med UMETA(DisplayName = "Medical (Tibbi)"),
  Ammo UMETA(DisplayName = "Ammo (Muhimmat)"),
  Weapon UMETA(DisplayName = "Weapon (Silah)"),
  Quest UMETA(DisplayName = "Quest (Gorev)"),
  Backpack UMETA(DisplayName = "Backpack (Sirt Cantasi)"),
  QuestItem UMETA(DisplayName = "Quest"),
  Gear UMETA(DisplayName = "Gear (Ekipman)"),
  Valuable UMETA(DisplayName = "Valuable (Degerli)"),
  Tool UMETA(DisplayName = "Tool (Alet)"),
  AntiRad UMETA(DisplayName = "Anti-Radiation (Radyasyon Ilaci)")
};

/**
 * EItemRarity
 *
 * Eşyanın nadirlik seviyesi.
 * UI renk kodlaması ve loot ağırlıkları bu enum üzerinden yürütülür.
 */
UENUM(BlueprintType)
enum class EItemRarity : uint8 {
  Common UMETA(DisplayName = "Common"),
  Uncommon UMETA(DisplayName = "Uncommon"),
  Rare UMETA(DisplayName = "Rare"),
  Epic UMETA(DisplayName = "Epic"),
  Legendary UMETA(DisplayName = "Legendary")
};

/**
 * EItemCategory
 *
 * Eşyaların genel sınıflandırma kategorisi. Özellikle 'Quest' kategorisindeki
 * eşyaların atılamaz veya satılamaz olmasını kontrol etmek için kullanılır.
 */
UENUM(BlueprintType)
enum class EItemCategory : uint8 {
  None UMETA(DisplayName = "None (Yok)"),
  Consumable UMETA(DisplayName = "Consumable (Tuketilebilir)"),
  Equipment UMETA(DisplayName = "Equipment (Ekipman)"),
  Weapon UMETA(DisplayName = "Weapon (Silah)"),
  Material UMETA(DisplayName = "Material (Malzeme)"),
  Quest UMETA(DisplayName = "Quest (Gorev)"),
  Valuable UMETA(DisplayName = "Valuable (Degerli)")
};

