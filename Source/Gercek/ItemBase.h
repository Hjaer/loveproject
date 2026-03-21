#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "PostApocItemTypes.h"

// clang-format off
#include "ItemBase.generated.h"
// clang-format on

class UStaticMeshComponent;
class AGercekCharacter;

/**
 * AItemBase (Sistemin Babası)
 *
 * Tüm eşya verisi PostApocItems Data Table üzerinden okunur.
 * Editörde sadece satır adı (RowName) seçilir; tablo sabit: PostApocItems.
 * Zero-Pointer: Ham pointer saklanmaz; FDataTableRowHandle ile handle-based erişim.
 */
UCLASS()
class GERCEK_API AItemBase : public AActor, public IInteractable {
  GENERATED_BODY()

public:
  AItemBase();

protected:
  virtual void OnConstruction(const FTransform &Transform) override;

public:
  // ---------------------------------------------------------------
  // Bileşenler
  // ---------------------------------------------------------------
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item | Components")
  UStaticMeshComponent *ItemMesh;

  // ---------------------------------------------------------------
  // Veri: Sadece PostApocItems içindeki satır adı (RowName).
  // Tablo sabit: PostApocItems [cite: 2026-02-20]
  // ---------------------------------------------------------------
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Data",
            meta = (RowType = "PostApocItemRow"))
  FDataTableRowHandle ItemRowHandle;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Physics",
            meta = (ClampMin = "0.0"))
  float HeavyWeightThreshold = 5.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item | Data",
            meta = (ClampMin = "10", ClampMax = "100"))
  int32 ItemCondition = 100;

  // ---------------------------------------------------------------
  // Public API
  // ---------------------------------------------------------------
  UFUNCTION(BlueprintCallable, Category = "Item | Interaction")
  virtual void Interact(AGercekCharacter *Player);

  // === IInteractable (BlueprintNativeEvent → _Implementation) ===
  virtual void OnInteract_Implementation(AGercekCharacter *Player) override;
  virtual FText GetInteractionPrompt_Implementation(AGercekCharacter *Player) override;
  virtual FText GetInteractableName_Implementation() override;
  virtual FDataTableRowHandle GetItemData_Implementation() override;
};
