// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"

// GENERATED HEADER: MUST BE THE LAST INCLUDE. NON-NEGOTIABLE.
#include "PostApocItemTypes.generated.h"

/**
 * EPostApocItemCategory
 * Post-apocalyptic evrenindeki eşyaların mekaniksel ve mantıksal kırılımı.
 */
UENUM(BlueprintType)
enum class EPostApocItemCategory : uint8 {
  Junk UMETA(DisplayName = "Junk"),
  Medical UMETA(DisplayName = "Medical"),
  Food UMETA(DisplayName = "Food"),
  Drink UMETA(DisplayName = "Drink"),
  Crafting UMETA(DisplayName = "Crafting"),
  Gear UMETA(DisplayName = "Gear"),
  Weapon UMETA(DisplayName = "Weapon"),
  Ammo UMETA(DisplayName = "Ammo"),
  Tool UMETA(DisplayName = "Tool"),
  Valuable UMETA(DisplayName = "Valuable"),
  Quest UMETA(DisplayName = "Quest")
};

UENUM(BlueprintType)
enum class EPostApocConsumableEffectType : uint8 {
  None UMETA(DisplayName = "None"),
  Food UMETA(DisplayName = "Food"),
  Drink UMETA(DisplayName = "Drink"),
  Heal UMETA(DisplayName = "Heal"),
  AntiRad UMETA(DisplayName = "Anti-Rad")
};

UENUM(BlueprintType)
enum class EConsumableFillState : uint8 {
  NotApplicable UMETA(DisplayName = "Not Applicable"),
  HalfFull UMETA(DisplayName = "Half Full"),
  Full UMETA(DisplayName = "Full")
};

UENUM(BlueprintType)
enum class EConsumableLootFillRule : uint8 {
  None UMETA(DisplayName = "None"),
  StandardWorldLoot UMETA(DisplayName = "Standard World Loot"),
  SpecialZoneLoot UMETA(DisplayName = "Special Zone Loot"),
  AlwaysHalfFull UMETA(DisplayName = "Always Half Full"),
  AlwaysFull UMETA(DisplayName = "Always Full")
};

/**
 * ETradeKnowledge
 * Oyuncunun ticaret tecrübe seviyesini temsil eder. (Novice, Apprentice, Expert)
 */
UENUM(BlueprintType)
enum class ETradeKnowledge : uint8 {
  Novice UMETA(DisplayName = "Novice (0-1999 XP)"),
  Apprentice UMETA(DisplayName = "Apprentice (2000-4999 XP)"),
  Expert UMETA(DisplayName = "Expert (5000+ XP)")
};

/**
 * FPostApocItemRow
 * PostApocItems DataTable için production-friendly, genişletilebilir ve temiz
 * veri satırı.
 */
USTRUCT(BlueprintType)
struct GERCEK_API FPostApocItemRow : public FTableRowBase {
  GENERATED_BODY()

public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
  FName ItemID;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
  FText DisplayName;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
  FText Description;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
  EPostApocItemCategory Category = EPostApocItemCategory::Junk;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset References")
  TSoftObjectPtr<class UStaticMesh> PickupMesh;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset References")
  TSoftObjectPtr<class UTexture2D> ItemIcon;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
  FName ItemRarity;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
  int32 MaxStack = 1;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
  float Weight = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
  int32 BaseValue = 0;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flags")
  bool bCanConsume = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flags")
  bool bCanEquip = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flags")
  bool bCanStack = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flags")
  bool bQuestItem = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable")
  EPostApocConsumableEffectType ConsumeEffectType =
      EPostApocConsumableEffectType::None;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable",
            meta = (ClampMin = "0.0"))
  float ConsumeAmount = 0.0f;

  // ---- Grid (Inventory) Alanları ----------------------------------------

  // Izgara üzerinde kaç hücre kapladığı: X=Genişlik, Y=Yükseklik.
  // Örn: FIntPoint(2,3) => 2 sütun geniş, 3 satır yüksek.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
  FIntPoint ItemSize = FIntPoint(1, 1);

  // Eşyanın envanter ızgarasında 90 derece döndürülüp döndürülemeyeceği.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
  bool bCanBeRotated = false;
};
