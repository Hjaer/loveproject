#include "PostApocGridItem.h"

#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "GercekCharacter.h"
#include "PostApocGridDragDropOperation.h"
#include "PostApocInventoryGridWidget.h"

static constexpr float GRID_CELL_SIZE = 50.0f;

void UPostApocGridItem::NativePreConstruct() {
  Super::NativePreConstruct();
  RefreshVisuals();
}

FReply UPostApocGridItem::NativeOnMouseButtonDown(
    const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) {
  if (OwningGridWidget && ItemInstanceId.IsValid() &&
      InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
    OwningGridWidget->HandleGridItemClicked(ItemInstanceId);
    return UWidgetBlueprintLibrary::DetectDragIfPressed(
        InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
  }

  return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UPostApocGridItem::NativeOnMouseButtonDoubleClick(
    const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) {
  if (OwningGridWidget && ItemInstanceId.IsValid() &&
      InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
    OwningGridWidget->HandleGridItemActivated(ItemInstanceId);
    return FReply::Handled();
  }

  return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

void UPostApocGridItem::NativeOnDragDetected(
    const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
    UDragDropOperation*& OutOperation) {
  Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

  if (!OwningGridWidget || !ItemInstanceId.IsValid()) {
    return;
  }

  UPostApocGridDragDropOperation* DragOperation =
      NewObject<UPostApocGridDragDropOperation>();
  if (!DragOperation) {
    return;
  }

  DragOperation->ItemInstanceId = ItemInstanceId;
  DragOperation->SourceGridRole = OwningGridWidget->GetGridRole();
  DragOperation->SourceInventory = OwningGridWidget->GetBoundInventoryComponent();
  DragOperation->DragOffset = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

  UPostApocGridItem* DragVisual = CreateWidget<UPostApocGridItem>(GetWorld(), GetClass());
  if (DragVisual) {
    DragVisual->ItemVerisi = ItemVerisi;
    DragVisual->ItemInstanceId = ItemInstanceId;
    DragVisual->bIsRotated = bIsRotated;
    DragVisual->RefreshVisuals();
    DragOperation->DefaultDragVisual = DragVisual;
  }

  OutOperation = DragOperation;
}

void UPostApocGridItem::RefreshVisuals() {
  if (!ItemSizeBox || !ItemIcon || ItemVerisi.IsNull()) {
    return;
  }

  const FPostApocItemRow* ItemData =
      ItemVerisi.GetRow<FPostApocItemRow>(TEXT("PostApocGridItem::RefreshVisuals"));
  if (!ItemData) {
    return;
  }

  float CalculatedWidth = static_cast<float>(ItemData->ItemSize.X) * GRID_CELL_SIZE;
  float CalculatedHeight = static_cast<float>(ItemData->ItemSize.Y) * GRID_CELL_SIZE;
  if (bIsRotated) {
    Swap(CalculatedWidth, CalculatedHeight);
  }

  ItemSizeBox->SetWidthOverride(CalculatedWidth);
  ItemSizeBox->SetHeightOverride(CalculatedHeight);

  if (Txt_ItemName) {
    Txt_ItemName->SetVisibility(ESlateVisibility::Collapsed);
  }
  if (Txt_ItemCondition) {
    Txt_ItemCondition->SetVisibility(ESlateVisibility::Collapsed);
  }
  if (Txt_ItemValue) {
    Txt_ItemValue->SetVisibility(ESlateVisibility::Collapsed);
  }

  const AGercekCharacter* CharacterOwner =
      OwningGridWidget ? OwningGridWidget->GetOwningGercekCharacter() : nullptr;

  if (ItemData->ItemIcon.IsNull()) {
    ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
    if (Txt_ItemName) {
      Txt_ItemName->SetVisibility(ESlateVisibility::Visible);
      Txt_ItemName->SetText(ItemData->DisplayName);
    }
    if (Txt_ItemCondition || Txt_ItemValue || OwningGridWidget) {
      int32 Condition = 100;
      int32 EffectiveValue = 0;
      FText StateText = FText::FromString(TEXT("Kondisyon: 100"));
      if (OwningGridWidget && OwningGridWidget->GetBoundInventoryComponent()) {
        OwningGridWidget->GetBoundInventoryComponent()->GetConditionForInstance(
            ItemInstanceId, Condition);
        OwningGridWidget->GetBoundInventoryComponent()->GetItemDisplayStateForInstance(
            ItemInstanceId, StateText);
        EffectiveValue =
            OwningGridWidget->GetBoundInventoryComponent()->GetItemValueForInstance(
                ItemInstanceId);
      }

      if (Txt_ItemCondition) {
        Txt_ItemCondition->SetVisibility(ESlateVisibility::Visible);
        Txt_ItemCondition->SetText(StateText);
      }
      if (Txt_ItemValue) {
        Txt_ItemValue->SetVisibility(ESlateVisibility::Visible);
        const FText ValueText = CharacterOwner
                                    ? CharacterOwner->GetKnowledgeAdjustedTradeValueText(
                                          EffectiveValue)
                                    : FText::AsNumber(EffectiveValue);
        Txt_ItemValue->SetText(
            FText::FromString(FString::Printf(TEXT("Deger: %s"),
                                              *ValueText.ToString())));
      }
      SetToolTipText(FText::FromString(FString::Printf(
          TEXT("%s\n%s\nDeger: %s"),
          *ItemData->DisplayName.ToString(), *StateText.ToString(),
          *(CharacterOwner
                ? CharacterOwner->GetKnowledgeAdjustedTradeValueText(EffectiveValue)
                      .ToString()
                : FText::AsNumber(EffectiveValue).ToString()))));
    }
    return;
  }

  ItemIcon->SetVisibility(ESlateVisibility::Visible);
  if (UTexture2D* LoadedIcon = ItemData->ItemIcon.LoadSynchronous()) {
    ItemIcon->SetBrushFromTexture(LoadedIcon, true);
  }

  int32 Condition = 100;
  int32 EffectiveValue = ItemData->BaseValue;
  FText StateText = FText::FromString(TEXT("Kondisyon: 100"));
  if (OwningGridWidget && OwningGridWidget->GetBoundInventoryComponent()) {
    OwningGridWidget->GetBoundInventoryComponent()->GetConditionForInstance(
        ItemInstanceId, Condition);
    OwningGridWidget->GetBoundInventoryComponent()->GetItemDisplayStateForInstance(
        ItemInstanceId, StateText);
    EffectiveValue =
        OwningGridWidget->GetBoundInventoryComponent()->GetItemValueForInstance(
            ItemInstanceId);
  }

  if (Txt_ItemName) {
    Txt_ItemName->SetText(ItemData->DisplayName);
  }
  if (Txt_ItemCondition) {
    Txt_ItemCondition->SetText(StateText);
  }
  if (Txt_ItemValue) {
    const FText ValueText = CharacterOwner
                                ? CharacterOwner->GetKnowledgeAdjustedTradeValueText(
                                      EffectiveValue)
                                : FText::AsNumber(EffectiveValue);
    Txt_ItemValue->SetText(
        FText::FromString(FString::Printf(TEXT("Deger: %s"),
                                          *ValueText.ToString())));
  }

  const FText TooltipValueText =
      CharacterOwner
          ? CharacterOwner->GetKnowledgeAdjustedTradeValueText(EffectiveValue)
          : FText::AsNumber(EffectiveValue);
  SetToolTipText(FText::FromString(FString::Printf(
      TEXT("%s\n%s\nDeger: %s"), *ItemData->DisplayName.ToString(),
      *StateText.ToString(), *TooltipValueText.ToString())));
}
