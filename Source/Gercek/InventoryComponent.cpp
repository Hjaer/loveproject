// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent() {
  // Tick is disabled — a resting survivor still has a full pack.
  PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay() { Super::BeginPlay(); }

void UInventoryComponent::TickComponent(
    float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction *ThisTickFunction) {
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UInventoryComponent::AddItem(FName RowName, const UDataTable *Table,
                                  int32 Qty) {
  // Guard: bad args
  if (RowName == NAME_None || !Table || Qty <= 0) {
    UE_LOG(LogTemp, Warning,
           TEXT("[Inventory] Rejected: invalid row name, null table, or "
                "zero quantity."));
    return false;
  }

  // Fetch the row to validate the item and read its properties.
  const FItemDBRow *Row =
      Table->FindRow<FItemDBRow>(RowName, TEXT("InventoryComponent::AddItem"));
  if (!Row) {
    UE_LOG(LogTemp, Warning,
           TEXT("[Inventory] Rejected: row '%s' not found in DataTable."),
           *RowName.ToString());
    return false;
  }

  // Weight check before committing.
  float AddedWeight = Row->ItemWeight * static_cast<float>(Qty);
  if (TotalWeight + AddedWeight > MaxCapacity) {
    UE_LOG(LogTemp, Warning,
           TEXT("[Inventory] Encumbered. Cannot add '%s' (%.2f kg would exceed "
                "%.2f kg limit)."),
           *Row->ItemName.ToString(), AddedWeight, MaxCapacity);
    return false;
  }

  int32 Remaining = Qty;

  // Try to merge into an existing stack if stackable (MaxStackSize > 1).
  if (Row->IsStackable()) {
    for (FInventorySlot &Slot : InventoryItems) {
      if (Slot.RowName != RowName || Slot.Table != Table)
        continue;

      const int32 SpaceLeft = Row->MaxStackSize - Slot.Quantity;
      if (SpaceLeft <= 0)
        continue;

      if (Remaining <= SpaceLeft) {
        Slot.Quantity += Remaining;
        Remaining = 0;
        break;
      } else {
        Slot.Quantity = Row->MaxStackSize;
        Remaining -= SpaceLeft;
      }
    }
  }

  // Overflow or non-stackable — claim a new slot.
  if (Remaining > 0) {
    FInventorySlot NewSlot;
    NewSlot.RowName = RowName;
    NewSlot.Table = Table;
    NewSlot.Quantity = Remaining;
    InventoryItems.Add(NewSlot);
  }

  // Recalculate the burden and fire delegates.
  CalculateTotalWeight();
  OnInventoryUpdated.Broadcast();

  UE_LOG(LogTemp, Log,
         TEXT("[Inventory] Scavenged: '%s' x%d (%.2f kg each). Total: %.2f / "
              "%.2f kg"),
         *Row->ItemName.ToString(), Qty, Row->ItemWeight, TotalWeight,
         MaxCapacity);

  if (GetCapacityRatio() >= 0.9f) {
    UE_LOG(LogTemp, Warning,
           TEXT("[Inventory] Warning: Backpack is getting heavy. %.0f%% "
                "capacity used. You're a slow target."),
           GetCapacityRatio() * 100.0f);
  }

  return true;
}

bool UInventoryComponent::RemoveItem(FName RowName, int32 Qty) {
  if (RowName == NAME_None || Qty <= 0) {
    UE_LOG(LogTemp, Warning,
           TEXT("[Inventory] RemoveItem: invalid RowName or zero Qty."));
    return false;
  }

  int32 Remaining = Qty;
  for (int32 i = InventoryItems.Num() - 1; i >= 0; --i) {
    FInventorySlot &Slot = InventoryItems[i];
    if (Slot.RowName != RowName)
      continue;

    if (Slot.Quantity <= Remaining) {
      Remaining -= Slot.Quantity;
      InventoryItems.RemoveAt(i);
    } else {
      Slot.Quantity -= Remaining;
      Remaining = 0;
    }

    if (Remaining == 0)
      break;
  }

  if (Remaining > 0) {
    UE_LOG(LogTemp, Warning,
           TEXT("[Inventory] RemoveItem: could not remove full Qty of '%s'. "
                "Short by %d."),
           *RowName.ToString(), Remaining);
    // Still recalculate in case we removed part of it.
  }

  CalculateTotalWeight();
  OnInventoryUpdated.Broadcast();
  return Remaining == 0;
}

void UInventoryComponent::CalculateTotalWeight() {
  float NewWeight = 0.0f;
  for (const FInventorySlot &Slot : InventoryItems) {
    if (!Slot.IsValid())
      continue;
    const FItemDBRow *Row = Slot.GetRow();
    if (Row) {
      NewWeight += Row->ItemWeight * static_cast<float>(Slot.Quantity);
    }
  }

  if (FMath::Abs(TotalWeight - NewWeight) > KINDA_SMALL_NUMBER) {
    TotalWeight = NewWeight;
    WeightPercentage = FMath::Clamp(TotalWeight / MaxCapacity, 0.0f, 1.0f);
    OnWeightChanged.Broadcast(TotalWeight, MaxCapacity);
  }
}

float UInventoryComponent::GetCapacityRatio() const {
  if (MaxCapacity <= 0.0f)
    return 0.0f;
  return FMath::Clamp(TotalWeight / MaxCapacity, 0.0f, 1.0f);
}

TArray<FInventorySlot> UInventoryComponent::GetInventoryForUI() const {
  return InventoryItems;
}

void UInventoryComponent::GetInventoryDetailsForUI(
    TArray<FInventorySlot> &OutItems, float &OutTotalWeight,
    float &OutMaxCapacity) const {
  OutItems = InventoryItems;
  OutTotalWeight = TotalWeight;
  OutMaxCapacity = MaxCapacity;
}

FText UInventoryComponent::GetCapacityText() const {
  FString Formatted = FString::Printf(TEXT("Encumbrance: %.1f / %.1f kg"),
                                      TotalWeight, MaxCapacity);
  return FText::FromString(Formatted);
}
