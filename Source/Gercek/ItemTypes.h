// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// --- Standard Unreal includes first ---
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"

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
  Med UMETA(DisplayName = "Medical (Tibbi)"),
  Ammo UMETA(DisplayName = "Ammo (Muhimmat)"),
  Weapon UMETA(DisplayName = "Weapon (Silah)"),
  Quest UMETA(DisplayName = "Quest (Gorev)"),
  Backpack UMETA(DisplayName = "Backpack (Sirt Cantasi)"),
  QuestItem UMETA(DisplayName = "Quest"),
  Gear UMETA(DisplayName = "Gear (Ekipman)"),
  Valuable UMETA(DisplayName = "Valuable (Degerli)"),
  Tool UMETA(DisplayName = "Tool (Alet)")
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

// ============================================================
//  MASTER DATA STRUCT
// ============================================================

/**
 * FItemDBRow
 *
 * Her eşya için tek gerçek kaynak.
 * UDataTable asset'inden okunur — C++ recompile gerekmez.
 *
 * TSoftObjectPtr ile Lazy Loading:
 *   ItemIcon ve PickupMesh yalnızca gerçekten ihtiyaç duyulduğunda yüklenir.
 *   Bu, harita başlangıcında yaşanabilecek bellek yükünü önler.
 */
USTRUCT(BlueprintType)
struct GERCEK_API FItemDBRow : public FTableRowBase {
  GENERATED_BODY()

public:
  // ---- Görsel & Metin ----------------------------------------

  // Oyuncunun envanterinde ve dünyada gördüğü ad.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Display")
  FText ItemName;

  // Eşyayı tanımlayan kısa açıklama. Tooltip / inspect paneli için.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Display")
  FText ItemDescription;

  // Envanter slot ikonları için Lazy-Loaded texture.
  // Yalnızca UI açıkken yüklenir; kapat = serbest kalır.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Display")
  TSoftObjectPtr<UTexture2D> ItemIcon;

  // Dünyada bırakıldığında / tutulduğunda kullanılan mesh.
  // Lazy-Loaded: sahneye girilene kadar RAM'e taşınmaz.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Display")
  TSoftObjectPtr<UStaticMesh> PickupMesh;

  // ---- Oyun Mekanikleri ---------------------------------------

  // Eşyanın işlevsel kategorisi (loot tablosu, tüketim, filtre).
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Mechanics")
  EItemType ItemType = EItemType::Junk;

  // Nadirlik seviyesi (UI rengi, drop ağırlığı).
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Mechanics")
  EItemRarity Rarity = EItemRarity::Common;

  // Bir slotta en fazla kaç adet yığılabilir?
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Mechanics",
            meta = (ClampMin = "1"))
  int32 MaxStackSize = 1;

  // bIsStackable bu alanın derivası; MaxStackSize > 1 ise stackable kabul et.
  bool IsStackable() const { return MaxStackSize > 1; }

  // ---- Ekonomi & Fizik ----------------------------------------

  // Kilogram cinsinden ağırlık. Taşıma kapasitesini etkiler.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Economy",
            meta = (ClampMin = "0.0"))
  float ItemWeight = 0.0f;

  // Takas / satış değeri (caps, puan, vb.).
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Economy",
            meta = (ClampMin = "0"))
  int32 ItemValue = 0;

  // Çanta türü eşyalar için ekstra kapasite değeri.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Upgrade",
            meta = (ClampMin = "0.0"))
  float ExtraCapacity = 0.0f;

  // ---- Flags --------------------------------------------------

  // Kullanılabilir / Tüketilebilir (Yemek, İlaç vb.)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Flags")
  bool bCanConsume = false;

  // Kuşanılabilir (Zırh, Silah vb.)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Flags")
  bool bCanEquip = false;

  // ---- PostApoc Envanter Sistemi ---------------------------

  // Envanterde kaç slot kaplayacağı (X = Genişlik, Y = Yükseklik).
  // Örnek: FIntPoint(2,3) => 2 sütun, 3 satır.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory")
  FIntPoint ItemSize = FIntPoint(1, 1);

  // Eşyanın %100 kondisyondaki ham takas değeri (caps, puan vb.).
  // Gerçek değer CalculateBarterValue() ile kondisyona göre hesaplanır.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory",
            meta = (ClampMin = "0.0"))
  float BaseValue = 0.0f;

  // true ise eşya envanter ızgarasında 90 derece döndürülebilir.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory")
  bool bCanBeRotated = false;

  // Eşyanın kilogram cinsinden ağırlığı.
  // UI'da "X.X kg" formatında gösterilir; taşıma kapasitesini etkiler.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory",
            meta = (ClampMin = "0.0"))
  float Weight = 0.0f;
};
