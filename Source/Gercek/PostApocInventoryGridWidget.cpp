#include "PostApocInventoryGridWidget.h"

#include "Components/CanvasPanel.h"
#include "GercekCharacter.h"
#include "LootContainerBase.h"
#include "PostApocInventoryTypes.h"

void UPostApocInventoryGridWidget::InitializeGridContext(
    UPostApocInventoryComponent* InInventoryComponent,
    AGercekCharacter* InOwningCharacter, ALootContainerBase* InLootContainer,
    const EPostApocInventoryGridRole InGridRole) {
  BoundInventoryComponent = InInventoryComponent;
  OwningGercekCharacter = InOwningCharacter;
  BoundLootContainer = InLootContainer;
  GridRole = InGridRole;
}

void UPostApocInventoryGridWidget::HandleGridItemClicked(
    const FGuid ItemInstanceId) {
  if (!BoundInventoryComponent) {
    return;
  }

  FGridItemInstanceView ItemInstance;
  if (BoundInventoryComponent->GetItemInstanceView(ItemInstanceId,
                                                   ItemInstance)) {
    BP_OnGridItemClicked(ItemInstance);
  }
}

void UPostApocInventoryGridWidget::HandleGridItemActivated(
    const FGuid ItemInstanceId) {
  if (!BoundInventoryComponent || !OwningGercekCharacter) {
    return;
  }

  FGridItemInstanceView ItemInstance;
  if (!BoundInventoryComponent->GetItemInstanceView(ItemInstanceId,
                                                    ItemInstance)) {
    return;
  }

  BP_OnGridItemActivated(ItemInstance);

  if (!bUseDefaultItemActions) {
    if (GridRole == EPostApocInventoryGridRole::PlayerInventory ||
        GridRole == EPostApocInventoryGridRole::MerchantInventory) {
      OwningGercekCharacter->ToggleTradeOfferItem(GridRole, ItemInstanceId);
    }
    return;
  }

  switch (GridRole) {
  case EPostApocInventoryGridRole::LootContainer:
    OwningGercekCharacter->RequestTakeItemFromLootContainer(ItemInstanceId);
    break;
  case EPostApocInventoryGridRole::PlayerInventory:
    if (BoundLootContainer) {
      OwningGercekCharacter->RequestStoreItemInLootContainer(ItemInstanceId);
    } else {
      OwningGercekCharacter->UseItemInstanceFromInventory(ItemInstanceId);
    }
    break;
  default:
    break;
  }
}

bool UPostApocInventoryGridWidget::NativeOnDrop(
    const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation) {
  const UPostApocGridDragDropOperation* GridOperation =
      Cast<UPostApocGridDragDropOperation>(InOperation);
  if (!GridOperation || !GridOperation->ItemInstanceId.IsValid()) {
    return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
  }

  if (HandleInternalDrop(InGeometry, InDragDropEvent,
                         GridOperation->ItemInstanceId,
                         GridOperation->SourceInventory,
                         GridOperation->SourceGridRole)) {
    return true;
  }

  BP_OnGridDropRejected(GridOperation->ItemInstanceId);
  return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

bool UPostApocInventoryGridWidget::HandleInternalDrop(
    const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
    const FGuid ItemInstanceId, UPostApocInventoryComponent* SourceInventory,
    const EPostApocInventoryGridRole SourceRole) {
  if (!BoundInventoryComponent || !OwningGercekCharacter ||
      !ItemInstanceId.IsValid()) {
    return false;
  }

  if (SourceInventory == BoundInventoryComponent) {
    const FVector2D LocalPosition =
        InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
    const FIntPoint NewLocation(
        FMath::FloorToInt(LocalPosition.X / BoundInventoryComponent->TileSize),
        FMath::FloorToInt(LocalPosition.Y / BoundInventoryComponent->TileSize));
    return BoundInventoryComponent->HandleItemDropByInstanceId(ItemInstanceId,
                                                               NewLocation);
  }

  if (SourceRole == EPostApocInventoryGridRole::LootContainer &&
      GridRole == EPostApocInventoryGridRole::PlayerInventory) {
    OwningGercekCharacter->RequestTakeItemFromLootContainer(ItemInstanceId);
    return true;
  }

  if (SourceRole == EPostApocInventoryGridRole::PlayerInventory &&
      GridRole == EPostApocInventoryGridRole::LootContainer) {
    OwningGercekCharacter->RequestStoreItemInLootContainer(ItemInstanceId);
    return true;
  }

  if (!bUseDefaultItemActions &&
      (SourceRole == EPostApocInventoryGridRole::PlayerInventory ||
       SourceRole == EPostApocInventoryGridRole::MerchantInventory)) {
    OwningGercekCharacter->ToggleTradeOfferItem(SourceRole, ItemInstanceId);
    BP_OnGridItemDropped(ItemInstanceId, SourceRole, GridRole);
    return true;
  }

  BP_OnGridItemDropped(ItemInstanceId, SourceRole, GridRole);
  return true;
}
