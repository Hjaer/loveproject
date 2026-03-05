// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// --- Standard Unreal includes first ---
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemData.h"

// --- GENERATED HEADER: MUST BE THE LAST INCLUDE. NON-NEGOTIABLE. ---
// clang-format off
#include "InventoryComponent.generated.h"
// clang-format on

// Post-Apocalyptic Note: These delegates are the heartbeat of the scavenger
// economy. When inventory changes, the world reacts.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeightChanged, float, NewWeight,
                                             float, MaxWeight);

/**
 * FInventorySlot
 *
 * A single slot in the survivor's pack.
 * RowName points to the row in the shared DataTable — no duplication of data.
 * The DataTable reference is shared so we can look up weight, value, etc.
 */
USTRUCT(BlueprintType)
struct GERCEK_API FInventorySlot {
  GENERATED_BODY()

  // The DataTable row name that identifies this item.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival | Inventory")
  FName RowName = NAME_None;

  // The DataTable that owns this row.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival | Inventory")
  const UDataTable *Table = nullptr;

  // How many units are stacked in this slot?
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival | Inventory")
  int32 Quantity = 1;

  // Is this slot occupied?
  bool IsValid() const {
    return RowName != NAME_None && Table != nullptr && Quantity > 0;
  }

  // Fetch the underlying row data. Returns nullptr if invalid.
  const FItemDBRow *GetRow() const {
    if (!IsValid())
      return nullptr;
    return Table->FindRow<FItemDBRow>(RowName, TEXT("InventorySlot::GetRow"));
  }
};

/**
 * UInventoryComponent
 *
 * Post-Apocalyptic Note: This is the survivor's burden — their entire worldly
 * possessions. Every slot filled is a choice. Every kilogram carried is a tax
 * on survival speed. MaxCapacity is not arbitrary; it is the line between life
 * and death.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GERCEK_API UInventoryComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UInventoryComponent();

protected:
  virtual void BeginPlay() override;

  // The survivor's entire worldly possessions.
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
            Category = "Survival | Inventory")
  TArray<FInventorySlot> InventoryItems;

  // The current encumbrance. This number kills sprints and drains stamina.
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
            Category = "Survival | Inventory")
  float TotalWeight = 0.0f;

  // The hard ceiling. Exceed this and you're a sitting target.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival | Inventory")
  float MaxCapacity = 15.0f;

  // Internal recalculation — call after any inventory mutation.
  UFUNCTION(BlueprintCallable, Category = "Survival | Inventory")
  void CalculateTotalWeight();

public:
  // --- Core Inventory Operations ---

  // Try to add an item by DataTable row reference. Returns true on success.
  UFUNCTION(BlueprintCallable, Category = "Survival | Inventory")
  bool AddItem(FName RowName, const UDataTable *Table, int32 Qty = 1);

  // Remove Qty units of RowName from the inventory. Returns true if removed.
  UFUNCTION(BlueprintCallable, Category = "Survival | Inventory")
  bool RemoveItem(FName RowName, int32 Qty = 1);

  // --- UI Query Functions ---

  // Returns the raw inventory slot array for UI rendering.
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival | UI")
  TArray<FInventorySlot> GetInventoryForUI() const;

  // Fills all inventory-related out-params in a single Blueprint call.
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival | UI")
  void GetInventoryDetailsForUI(TArray<FInventorySlot> &OutItems,
                                float &OutTotalWeight,
                                float &OutMaxCapacity) const;

  // Returns a formatted capacity string: "Encumbrance: 8.5 / 15.0 kg"
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival | UI")
  FText GetCapacityText() const;

  // Returns the current total weight of all carried items.
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival | UI")
  float GetTotalWeight() const { return TotalWeight; }

  // Returns the maximum weight the owner can carry.
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival | UI")
  float GetMaxWeight() const { return MaxCapacity; }

  // Returns encumbrance as a 0.0-1.0 ratio. Bind this to a UI progress bar.
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival | UI")
  float GetCapacityRatio() const;

  // --- Weight Metadata ---

  // Weight as a 0.0-1.0 ratio. Kept as a UPROPERTY for Blueprint easy-access.
  UPROPERTY(BlueprintReadOnly, Category = "Survival | UI")
  float WeightPercentage = 0.0f;

  // --- Events ---

  // Fires whenever an item is added or removed. Bind to UI refresh.
  UPROPERTY(BlueprintAssignable, Category = "Survival | Events")
  FOnInventoryUpdated OnInventoryUpdated;

  // Fires whenever TotalWeight meaningfully changes. Bind to movement/stamina.
  UPROPERTY(BlueprintAssignable, Category = "Survival | Events")
  FOnWeightChanged OnWeightChanged;

  virtual void
  TickComponent(float DeltaTime, ELevelTick TickType,
                FActorComponentTickFunction *ThisTickFunction) override;
};
