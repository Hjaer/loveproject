// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "CoreMinimal.h"
#include "ItemData.h"

// clang-format off
#include "InventorySlotWidget.generated.h"
// clang-format on

/**
 * UInventorySlotWidget
 *
 * Represents a single item slot tile in the inventory grid.
 *
 * Blueprint setup (WBP_InventorySlot) MUST contain widgets with EXACT names:
 *   - UImage      "ItemIcon"
 *   - UTextBlock  "ItemNameText"
 *   - UTextBlock  "ItemQuantityText"
 *   - UTextBlock  "ItemValueText"
 *
 * Call UpdateSlot() from UInventoryWidget::RefreshInventory to populate.
 */
UCLASS(Abstract)
class GERCEK_API UInventorySlotWidget : public UUserWidget {
  GENERATED_BODY()

public:
  // ---- BindWidget: Blueprint widget names MUST match exactly ----

  // Eşya görseli
  UPROPERTY(meta = (BindWidget))
  class UImage *ItemIcon;

  // Eşya ismi
  UPROPERTY(meta = (BindWidget))
  class UTextBlock *ItemNameText;

  // Miktar (gizlenir eğer <=1)
  UPROPERTY(meta = (BindWidget))
  class UTextBlock *ItemQuantityText;

  // Para/takas değeri
  UPROPERTY(meta = (BindWidget))
  class UTextBlock *ItemValueText;

  // ---- Public API ----

  // DataTable satırından tüm görsel verileri widget'lara basar.
  UFUNCTION(BlueprintCallable, Category = "Survival | UI")
  void UpdateSlot(const FItemDBRow &ItemData, int32 Quantity);
};
