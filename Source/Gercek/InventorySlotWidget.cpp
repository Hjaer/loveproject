// Fill out your copyright notice in the Description page of Project Settings.

#include "InventorySlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Styling/SlateColor.h"

// ============================================================
// Post-Apocalyptic Palette
// ============================================================
// Worn White  — item names, military stencil feel
static const FLinearColor ColorItemName(0.85f, 0.82f, 0.72f, 1.0f);
// Irradiated Green — stack quantity badge
static const FLinearColor ColorQuantity(0.18f, 0.95f, 0.35f, 1.0f);
// Scrap Gold — barter/cap value
static const FLinearColor ColorValue(0.98f, 0.78f, 0.12f, 1.0f);

void UInventorySlotWidget::UpdateSlot(const FItemDBRow &ItemData,
                                      int32 Quantity) {
  // ---- ItemNameText ----------------------------------------
  if (ItemNameText) {
    ItemNameText->SetText(ItemData.ItemName);
    ItemNameText->SetColorAndOpacity(FSlateColor(ColorItemName));
  }

  // ---- ItemIcon --------------------------------------------
  // TSoftObjectPtr olduğu için önce IsValid kontrolü yapılır,
  // ardından LoadSynchronous() ile ham pointer alınır.
  // Bu desen hatanın bir daha tekrarlanmasını önler.
  if (ItemIcon) {
    UTexture2D *LoadedIcon = nullptr;
    if (!ItemData.ItemIcon.IsNull() && ItemData.ItemIcon.IsValid()) {
      // Zaten bellekte — direkt eriş.
      LoadedIcon = ItemData.ItemIcon.Get();
    } else if (!ItemData.ItemIcon.IsNull()) {
      // Henüz bellekte değil — senkron yükle (UI için kabul edilebilir).
      LoadedIcon = ItemData.ItemIcon.LoadSynchronous();
    }

    if (IsValid(LoadedIcon)) {
      ItemIcon->SetBrushFromTexture(LoadedIcon, /*bMatchSize=*/true);
      ItemIcon->SetColorAndOpacity(FLinearColor::White);
      ItemIcon->SetVisibility(ESlateVisibility::Visible);
    } else {
      // İkon atanmamış veya yüklenemedi — widget Hidden.
      ItemIcon->SetVisibility(ESlateVisibility::Hidden);
    }
  }

  // ---- ItemQuantityText ------------------------------------
  // Miktar <= 1 ise etiketi tamamen kaldır (Collapsed = layout boşluk almaz).
  // Miktar > 1 ise yeşil rozet: "x3"
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
  // Değer > 0 ise altın sarısı "50 cap" formatında göster.
  // Değersiz eşyalarda yer kaplamaz (Collapsed).
  if (ItemValueText) {
    if (ItemData.ItemValue > 0) {
      ItemValueText->SetText(FText::Format(
          INVTEXT("{0} cap"), FText::AsNumber(ItemData.ItemValue)));
      ItemValueText->SetColorAndOpacity(FSlateColor(ColorValue));
      ItemValueText->SetVisibility(ESlateVisibility::Visible);
    } else {
      ItemValueText->SetVisibility(ESlateVisibility::Collapsed);
    }
  }
}
