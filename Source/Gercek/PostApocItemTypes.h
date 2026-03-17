// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

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
  Valuable UMETA(DisplayName = "Valuable")
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
  FName MeshPath;

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

  // ---- Grid (Inventory) Alanları ----------------------------------------

  // Izgara üzerinde kaç hücre kapladığı: X=Genişlik, Y=Yükseklik.
  // Örn: FIntPoint(2,3) => 2 sütun geniş, 3 satır yüksek.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
  FIntPoint ItemSize = FIntPoint(1, 1);

  // Eşyanın envanter ızgarasında 90 derece döndürülüp döndürülemeyeceği.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
  bool bCanBeRotated = false;
};
