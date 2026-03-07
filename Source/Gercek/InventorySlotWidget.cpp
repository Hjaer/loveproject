// Fill out your copyright notice in the Description page of Project Settings.

#include "InventorySlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Styling/SlateColor.h"

// ============================================================
// Post-Apocalyptic Palette
// ============================================================
// Worn White  -- item names, military stencil feel
static const FLinearColor ColorItemName(0.85f, 0.82f, 0.72f, 1.0f);
// Irradiated Green -- stack quantity badge
static const FLinearColor ColorQuantity(0.18f, 0.95f, 0.35f, 1.0f);
// Scrap Gold -- barter/cap value
static const FLinearColor ColorValue(0.98f, 0.78f, 0.12f, 1.0f);

void UInventorySlotWidget::SetEmptySlot() {
  if (ItemNameText) {
    ItemNameText->SetText(FText::GetEmpty());
    ItemNameText->SetColorAndOpacity(
        FSlateColor(FLinearColor(0.45f, 0.45f, 0.45f, 1.0f)));
  }

  if (ItemQuantityText) {
    ItemQuantityText->SetVisibility(ESlateVisibility::Collapsed);
  }

  if (ItemValueText) {
    ItemNameText->SetText(FText::GetEmpty());
    ItemValueText->SetVisibility(ESlateVisibility::Collapsed);
  }

  if (ItemIcon) {
    ItemIcon->SetVisibility(ESlateVisibility::Hidden);
  }
}

void UInventorySlotWidget::UpdateSlot(const FItemDBRow &ItemData,
                                      int32 Quantity) {
  // ---- ItemNameText ----------------------------------------
  if (ItemNameText) {
    ItemNameText->SetText(ItemData.ItemName);
    ItemNameText->SetColorAndOpacity(FSlateColor(ColorItemName));
  }

  // ---- ItemIcon --------------------------------------------
  // TSoftObjectPtr oldugu icin once IsValid kontrolu yapilir,
  // ardindan LoadSynchronous() ile ham pointer alinir.
  // Bu desen hatanin bir daha tekrarlanmamasini onler.
  if (ItemIcon) {
    UTexture2D *LoadedIcon = nullptr;
    if (!ItemData.ItemIcon.IsNull() && ItemData.ItemIcon.IsValid()) {
      // Zaten bellekte -- direkt eris.
      LoadedIcon = ItemData.ItemIcon.Get();
    } else if (!ItemData.ItemIcon.IsNull()) {
      // Henuz bellekte degil -- senkron yukle (UI icin kabul edilebilir).
      LoadedIcon = ItemData.ItemIcon.LoadSynchronous();
    }

    if (IsValid(LoadedIcon)) {
      ItemIcon->SetBrushFromTexture(LoadedIcon, /*bMatchSize=*/true);
      ItemIcon->SetColorAndOpacity(FLinearColor::White);
      ItemIcon->SetVisibility(ESlateVisibility::Visible);
    } else {
      // Ikon atanmamis veya yuklenemedi -- widget Hidden.
      ItemIcon->SetVisibility(ESlateVisibility::Hidden);
    }
  }

  // ---- ItemQuantityText ------------------------------------
  // Miktar <= 1 ise etiketi tamamen kaldir (Collapsed = layout bosluk almaz).
  // Miktar > 1 ise yesil rozet: "x3"
  if (ItemQuantityText) {
    if (Quantity > 1) {
      ItemQuantityText->SetText(
          FText::Format(INVTEXT("x{0}"), FText::AsNumber(Quantity)));
      ItemQuantityText->SetColorAndOpacity(FSlateColor(ColorQuantity));
      ItemQuantityText->SetVisibility(ESlateVisibility::Visible);
    } else {
      ItemQuantityText->SetVisibility(ESlateVisibility::Collapsed);
    }
  }

  // ---- ItemValueText ---------------------------------------
  // Item degerini para birimiyle goster: "Deger: 50 Cap"
  if (ItemValueText) {
    ItemValueText->SetText(FText::Format(INVTEXT("Deger: {0} Cap"),
                                         FText::AsNumber(ItemData.ItemValue)));
    ItemValueText->SetColorAndOpacity(FSlateColor(ColorValue));
    ItemValueText->SetVisibility(ESlateVisibility::Visible);
  }
}
