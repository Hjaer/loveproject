// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/WrapBox.h"
#include "InventorySlotWidget.h"

// ---------------------------------------------------------------------------
// NativeConstruct -- called when the widget is added to the viewport.
// We do NOT bind here because InventoryComponent may not be set yet.
// SetInventoryComponent() handles the bind on demand.
// ---------------------------------------------------------------------------
void UInventoryWidget::NativeConstruct() { Super::NativeConstruct(); }

// ---------------------------------------------------------------------------
// NativeDestruct -- safely unbind so we don't hold a stale callback.
// ---------------------------------------------------------------------------
void UInventoryWidget::NativeDestruct() {
  if (InventoryComponent) {
    // Dynamic multicast delegates use RemoveDynamic, not Remove(Handle).
    InventoryComponent->OnInventoryUpdated.RemoveDynamic(
        this, &UInventoryWidget::RefreshInventory);
  }
  Super::NativeDestruct();
}

// ---------------------------------------------------------------------------
// SetInventoryComponent
// Call this right after creating the widget (e.g. in GercekCharacter.cpp).
// ---------------------------------------------------------------------------
void UInventoryWidget::SetInventoryComponent(UInventoryComponent *InInventory) {
  // Unbind old component if we're swapping.
  if (InventoryComponent) {
    InventoryComponent->OnInventoryUpdated.RemoveDynamic(
        this, &UInventoryWidget::RefreshInventory);
  }

  InventoryComponent = InInventory;

  if (InventoryComponent) {
    // DYNAMIC multicast delegates require AddDynamic -- not AddUObject.
    InventoryComponent->OnInventoryUpdated.AddDynamic(
        this, &UInventoryWidget::RefreshInventory);

    // Immediately sync to current inventory state.
    RefreshInventory();
  }
}

// ---------------------------------------------------------------------------
// RefreshInventory -- always renders fixed-size slot list.
// Called automatically via delegate binding; can also be called from Blueprint.
// ---------------------------------------------------------------------------
void UInventoryWidget::RefreshInventory() {
  if (!InventoryGrid) {
    UE_LOG(LogTemp, Warning,
           TEXT("[InventoryWidget] InventoryGrid is null. Check BindWidget "
                "name in the Blueprint."));
    return;
  }

  // 1. Clear previous slots.
  InventoryGrid->ClearChildren();

  if (!SlotWidgetClass) {
    return;
  }

  TArray<FInventorySlot> Slots;
  if (InventoryComponent) {
    Slots = InventoryComponent->GetInventoryForUI();
  }

  const int32 SafeFixedSlotCount = FMath::Max(1, FixedSlotCount);
  const int32 OccupiedSlotCount = Slots.Num();

  // 2. Create fixed number of UI slots (default: 12).
  for (int32 SlotIndex = 0; SlotIndex < SafeFixedSlotCount; ++SlotIndex) {
    UInventorySlotWidget *SlotWidget =
        CreateWidget<UInventorySlotWidget>(GetOwningPlayer(), SlotWidgetClass);
    if (!SlotWidget) {
      continue;
    }

    if (SlotIndex < OccupiedSlotCount) {
      const FInventorySlot &CurrentSlot = Slots[SlotIndex];
      const FItemDBRow *Row = CurrentSlot.GetRow();

      if (CurrentSlot.IsValid() && Row) {
        SlotWidget->UpdateSlot(*Row, CurrentSlot.Quantity);
      } else {
        SlotWidget->SetEmptySlot();
      }
    } else {
      SlotWidget->SetEmptySlot();
    }

    InventoryGrid->AddChildToWrapBox(SlotWidget);
  }

  if (OccupiedSlotCount > SafeFixedSlotCount) {
    UE_LOG(LogTemp, Warning,
           TEXT("[InventoryWidget] %d item slot not shown (fixed UI slots: %d)."),
           OccupiedSlotCount - SafeFixedSlotCount, SafeFixedSlotCount);
  }
}
