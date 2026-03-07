// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/WrapBox.h"
#include "CoreMinimal.h"
#include "InventoryComponent.h"
#include "InventorySlotWidget.h"

// clang-format off
#include "InventoryWidget.generated.h"
// clang-format on

/**
 * UInventoryWidget
 *
 * The main inventory screen widget.
 *
 * Blueprint setup requirements:
 *   1. Create "WBP_Inventory" based on this class.
 *   2. Add a UWrapBox named exactly "InventoryGrid" to the hierarchy.
 *   3. Set SlotWidgetClass to your WBP_InventorySlot asset.
 *
 * C++ automatically binds to InventoryComponent::OnInventoryUpdated
 * and rebuilds the grid on every change -- no Blueprint event graph needed.
 */
UCLASS(Abstract)
class GERCEK_API UInventoryWidget : public UUserWidget {
  GENERATED_BODY()

public:
  // The grid container. Name in Blueprint MUST be "InventoryGrid".
  UPROPERTY(meta = (BindWidget))
  class UWrapBox *InventoryGrid;

  // UI'da her zaman gosterilecek sabit slot sayisi.
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survival | UI",
            meta = (ClampMin = "1"))
  int32 FixedSlotCount = 12;

  // Set this in the Blueprint CDO to the WBP_InventorySlot asset.
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Survival | UI")
  TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

  // The owning character's inventory that this widget mirrors.
  // Call SetInventoryComponent() after creating the widget.
  UFUNCTION(BlueprintCallable, Category = "Survival | UI")
  void SetInventoryComponent(UInventoryComponent *InInventory);

  // Clears InventoryGrid, then rebuilds fixed-size slot list.
  // Must be UFUNCTION for AddDynamic binding.
  UFUNCTION(BlueprintCallable, Category = "Survival | UI")
  void RefreshInventory();

protected:
  virtual void NativeConstruct() override;
  virtual void NativeDestruct() override;

private:
  // Cached reference -- bound during SetInventoryComponent.
  UPROPERTY()
  UInventoryComponent *InventoryComponent = nullptr;
  // Note: AddDynamic does not return a FDelegateHandle.
  // RemoveDynamic is used in NativeDestruct for cleanup.
};
