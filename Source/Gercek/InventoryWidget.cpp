// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/WrapBox.h"
#include "InventorySlotWidget.h"

// ---------------------------------------------------------------------------
// NativeConstruct — called when the widget is added to the viewport.
// We do NOT bind here because InventoryComponent may not be set yet.
// SetInventoryComponent() handles the bind on demand.
// ---------------------------------------------------------------------------
void UInventoryWidget::NativeConstruct() { Super::NativeConstruct(); }

// ---------------------------------------------------------------------------
// NativeDestruct — safely unbind so we don't hold a stale callback.
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
    // DYNAMIC multicast delegates require AddDynamic — not AddUObject.
    InventoryComponent->OnInventoryUpdated.AddDynamic(
        this, &UInventoryWidget::RefreshInventory);

    // Immediately sync to current inventory state.
    RefreshInventory();
  }
}

// ---------------------------------------------------------------------------
// RefreshInventory — rebuilds the grid from scratch.
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

  if (!InventoryComponent || !SlotWidgetClass) {
    return;
  }

  // 2. Iterate inventory and create one slot widget per entry.
  TArray<FInventorySlot> Slots = InventoryComponent->GetInventoryForUI();
  for (const FInventorySlot &CurrentSlot : Slots) {
    if (!CurrentSlot.IsValid())
      continue;

    const FItemDBRow *Row = CurrentSlot.GetRow();
    if (!Row)
      continue;

    // Create the slot widget using our designer-selected Blueprint subclass.
    UInventorySlotWidget *SlotWidget =
        CreateWidget<UInventorySlotWidget>(GetOwningPlayer(), SlotWidgetClass);
    if (!SlotWidget)
      continue;

    // Push data.
    SlotWidget->UpdateSlot(*Row, CurrentSlot.Quantity);

    // Add to grid.
    InventoryGrid->AddChildToWrapBox(SlotWidget);
  }

  UE_LOG(LogTemp, Log,
         TEXT("[InventoryWidget] Refreshed: %d slot(s) displayed."),
         Slots.Num());
}
