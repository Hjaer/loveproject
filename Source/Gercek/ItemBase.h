#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "ItemTypes.h"

// clang-format off
#include "ItemBase.generated.h"
// clang-format on

class UStaticMeshComponent;
class AGercekCharacter;

/**
 * AItemBase (Sistemin Babası)
 * Tüm eşya sınıflarının atası.
 * İçine girilen DataTable verisine göre şekil değiştiren bukalemun mantığına
 * sahiptir. Fizik simülasyonu ağırlık verisine göre otomatik ayarlanır.
 * Interact() çağrıldığında eşyayı envantera aktarır ve dünyadan siler.
 */
UCLASS()
class GERCEK_API AItemBase : public AActor {
  GENERATED_BODY()

public:
  AItemBase();

protected:
  virtual void OnConstruction(const FTransform &Transform) override;

public:
  // ---------------------------------------------------------------
  // Bileşenler
  // ---------------------------------------------------------------

  // Eşyanın fiziksel dünyadaki görünümü
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item | Components")
  UStaticMeshComponent *ItemMesh;

  // ---------------------------------------------------------------
  // Veri Referansı
  // ---------------------------------------------------------------

  // Editörde Jules'un ItemRowHandle üzerinden veri seçebileceği alan.
  // RowType filtresi yalnızca FItemDBRow satırlarını listeler.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Data",
            meta = (RowType = "ItemDBRow"))
  FDataTableRowHandle ItemRowHandle;

  // ---------------------------------------------------------------
  // Ağırlık eşiği — Inspector'dan düzenlenebilir
  // ---------------------------------------------------------------

  // Bu değerin üstündeki eşyalar fizik simülasyonu olmadan durur (kaya gibi).
  // Daha hafif eşyalar fizik simülasyonuyla tepki verir (yuvarlanır, kayar).
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Physics",
            meta = (ClampMin = "0.0"))
  float HeavyWeightThreshold = 5.0f;

  // ---------------------------------------------------------------
  // Fonksiyonlar
  // ---------------------------------------------------------------

  // İlgili DataTable satırını çeker; yoksa false döner.
  UFUNCTION(BlueprintCallable, Category = "Item | Data")
  bool GetItemData(FItemDBRow &OutItemData) const;

  // Oyuncu E tuşuna bastığında çağrılır.
  // Veriyi envanterе aktarır ve eşyayı dünyadan siler.
  // Alt sınıflar bu fonksiyonu override ederek özel davranış ekleyebilir.
  UFUNCTION(BlueprintCallable, Category = "Item | Interaction")
  virtual void Interact(AGercekCharacter *Player);
};
