// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// --- Standard Unreal includes first ---
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UObject/Interface.h"

// FItemDBRow ve FPostApocItemRow tanımları buradan geliyor.
#include "ItemTypes.h"
#include "PostApocItemTypes.h"

// --- GENERATED HEADER: MUST BE THE LAST INCLUDE. NON-NEGOTIABLE. ---
// clang-format off
#include "PostApocInventoryTypes.generated.h"
// clang-format on

// ============================================================
//  ARAYÜZ: IInventoryInterface
//  Widget-C++ köprüsü. Widget'lar Cast<IInventoryInterface>
//  ile bunu uygulayan herhangi bir Actor'a erişebilir.
// ============================================================

UINTERFACE(MinimalAPI, BlueprintType)
class UInventoryInterface : public UInterface {
  GENERATED_BODY()
};

/**
 * IInventoryInterface
 *
 * Widget'lar ile C++ kodunun haberleştiği sözleşme.
 * Arayüzü uygulamak isteyen her Actor, bu fonksiyonları
 * Blueprint veya C++ tarafında override etmelidir.
 */
class GERCEK_API IInventoryInterface {
  GENERATED_BODY()

public:
  /**
   * GetItemDetails
   *
   * Verilen ItemID'ye karşılık gelen satır verisini ve kondisyonu döndürür.
   * @param ItemID       - Sorgulanacak eşyanın Row adı.
   * @param OutItemData  - Doldurulacak FItemDBRow verisi.
   * @param OutCondition - 0.0-1.0 arası kondisyon değeri.
   */
  UFUNCTION(BlueprintCallable, BlueprintNativeEvent,
            Category = "PostApoc Inventory")
  void GetItemDetails(FName ItemID, FItemDBRow &OutItemData,
                      float &OutCondition);

  /**
   * MoveItem
   *
   * Bir eşyayı envanter içinde OldIndex'ten NewIndex'e taşır.
   * @return true başarılıysa, false çakışma/sınır dışı durumunda.
   */
  UFUNCTION(BlueprintCallable, BlueprintNativeEvent,
            Category = "PostApoc Inventory")
  bool MoveItem(int32 OldIndex, int32 NewIndex);

  /**
   * DropItem
   *
   * Eşyayı envanterdeki referansıyla dünyaya bırakır.
   * @param ItemID    - Bırakılacak eşyanın Row adı.
   * @param Condition - Eşyanın mevcut kondisyonu (0.0-1.0).
   */
  UFUNCTION(BlueprintCallable, BlueprintNativeEvent,
            Category = "PostApoc Inventory")
  void DropItem(FName ItemID, float Condition);
};

// ============================================================
//  COMPONENT: UInventoryComponent (Izgara tabanlı)
//  Mevcut UInventoryComponent'ten ayrı, grid-aware bileşen.
//  Aktöre eklenerek 2-boyutlu ızgara mantığını yönetir.
// ============================================================

UCLASS(ClassGroup = (PostApoc), meta = (BlueprintSpawnableComponent))
class GERCEK_API UPostApocInventoryComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UPostApocInventoryComponent();

public:
  virtual void BeginPlay() override;

  // Izgaranın sütun sayısı. Blueprint'ten düzenlenebilir.
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "PostApoc Inventory | Grid")
  int32 GridColumns = 10;

  // Izgaranın satır sayısı. Blueprint'ten düzenlenebilir.
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "PostApoc Inventory | Grid")
  int32 GridRows = 10;

  /**
   * OccupiedSlots
   *
   * Her dolu grid hücresinin sol-üst köşe koordinatı (FIntPoint) ->
   * İlgili eşyanın Row adı (FName). Eşya birden fazla hücre kaplıyorsa
   * her hücre için ayrı bir kayıt tutulur.
   */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
            Category = "PostApoc Inventory | Grid")
  TMap<FIntPoint, FName> OccupiedSlots;

public:
  /**
   * TryAddItem
   *
   * Eşyayı ızgarada uygun bir yere yerleştirmeyi dener.
   * @param ItemRowHandle - Eklenecek eşyanın DataTable satır referansı.
   * @return true yerleştirme başarılıysa, false alan yoksa.
   */
  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory | Grid")
  bool TryAddItem(FDataTableRowHandle ItemRowHandle);

  /**
   * RemoveItemFromGrid
   *
   * Izgara üzerindeki belirtilen satır adına (RowName) ait tüm hücre
   * kayıtlarını OccupiedSlots'tan siler.
   * @param ItemRowName - Kaldırılacak eşyanın DataTable satır adı.
   * @return true en az bir hücre silindiyse, false eşya ızgararada yoksa.
   */
  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory | Grid")
  bool RemoveItemFromGrid(FName ItemRowName);

  /**
   * GetItemCountInGrid
   *
   * Izgara üzerinde belirtilen RowName'e ait kaç hücre işgal edildiğini
   * döndürür. UI'da eşya sayımı, debug ve kontroller için kullanılır.
   * @param ItemRowName - Sorgulanacak eşyanın satır adı.
   * @return O eşyaya ait dolu hücre sayısı (0 ise ızgararada yok).
   */
  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Grid")
  int32 GetItemCountInGrid(FName ItemRowName) const;

  /**
   * FindEmptySpace
   *
   * Eşyanın boyutuna göre çantada (döndürülmüş veya normal) boş bir alan arar.
   * @param ItemSize          - Eşyanın (Genişlik, Yükseklik) hücre boyutu.
   * @param bCheckRotated     - true ise 90° döndürülmüş boyut da kontrol
   * edilir.
   * @param OutFoundLocation  - Başarılıysa bulunan sol-üst köşe koordinatı.
   * @return true uygun alan bulunduysa, false bulunamadıysa.
   */
  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory | Grid")
  bool FindEmptySpace(FIntPoint ItemSize, bool bCheckRotated,
                      FIntPoint &OutFoundLocation) const;

  /**
   * CalculateBarterValue
   *
   * Eşyanın kondisyona bağlı gerçek takas değerini hesaplar.
   * Formül: InBaseValue * (InCondition * ConditionWeight)
   *
   * @param InBaseValue      - FItemDBRow::BaseValue alanı.
   * @param InCondition      - 0.0 (kırık) – 1.0 (sıfır kusur).
   * @param ConditionWeight  - Kondisyonun ağırlık çarpanı (varsayılan 1.0).
   * @return Hesaplanan takas değeri.
   */
  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Economy")
  float CalculateBarterValue(float InBaseValue, float InCondition,
                             float ConditionWeight = 1.0f) const;

  /**
   * CheckSpace
   *
   * Belirtilen sol-üst köşeden başlayarak ItemSize kadar alan müsait mi?
   * bIsRotated = true ise X ve Y eksenleri yer değiştirilir (ItemSize.X <->
   * ItemSize.Y).
   *
   * @param TopLeftIndex - Kontrol edilecek sol-üst hücre koordinatı.
   * @param ItemSize     - Eşyanın (Genişlik, Yükseklik) boyutu.
   * @param bIsRotated   - true ise boyutlar döndürülmüş kabul edilir.
   * @return true alan müsaitse, false değilse.
   */
  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory | Grid")
  bool CheckSpace(FIntPoint TopLeftIndex, FIntPoint ItemSize,
                  bool bIsRotated) const;

  // -------------------------------------------------------
  /**
   * ExecuteTrade
   *
   * Oyuncu ile tüccar arasında Takas (Barter) gerçekleştirir.
   *
   * Akış:
   *   1. Oyuncunun teklif ettiği eşyaların tamamının envanterde
   *      mevcut olup olmadığını doğrular.
   *   2. Tüccarın teklif ettiği eşyalar için yeterli ızgara alanı
   *      bulunup bulunmadığını kontrol eder.
   *   3. Her iki koşul da sağlanıyorsa; oyuncu eşyalarını envanterden
   *      kaldırır, tüccar eşyalarını ekler ve takas tamamlanır.
   *
   * @param PlayerOfferItems  - Oyuncunun vereceği eşyaların DataTable satır referansları.
   * @param TraderOfferItems  - Oyuncunun alacağı eşyaların DataTable satır referansları.
   */
  UFUNCTION(BlueprintCallable, Category = "Ticaret")
  void ExecuteTrade(TArray<FDataTableRowHandle> PlayerOfferItems,
                    TArray<FDataTableRowHandle> TraderOfferItems);

  // --- Grid Erişim Yardımcıları ---

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Grid")
  int32 GetGridColumns() const { return GridColumns; }

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Grid")
  int32 GetGridRows() const { return GridRows; }

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Grid")
  const TMap<FIntPoint, FName> &GetOccupiedSlots() const {
    return OccupiedSlots;
  }
};
