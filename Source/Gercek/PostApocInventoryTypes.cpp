#include "PostApocInventoryTypes.h"

#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/DataTable.h"
#include "Net/UnrealNetwork.h"
#include "PostApocGridItem.h"
#include "PostApocInventoryGridWidget.h"

namespace {
FIntPoint GetEffectiveItemSize(const FPostApocItemRow& ItemData, const bool bIsRotated) {
  return bIsRotated ? FIntPoint(ItemData.ItemSize.Y, ItemData.ItemSize.X)
                    : ItemData.ItemSize;
}

bool CompareLocations(const FGridSlotData& A, const FGridSlotData& B) {
  if (A.Location.Y != B.Location.Y) {
    return A.Location.Y < B.Location.Y;
  }
  return A.Location.X < B.Location.X;
}

bool UsesBinaryFillStateForCategory(const EPostApocItemCategory Category) {
  return Category == EPostApocItemCategory::Food ||
         Category == EPostApocItemCategory::Drink;
}

} // namespace

UPostApocInventoryComponent::UPostApocInventoryComponent() {
  PrimaryComponentTick.bCanEverTick = false;
  SetIsReplicatedByDefault(true);
}

void UPostApocInventoryComponent::BeginPlay() {
  Super::BeginPlay();
}

float UPostApocInventoryComponent::CalculateBarterValue(
    const float InBaseValue, const float InCondition,
    const float ConditionWeight) const {
  const float SafeCondition = FMath::Clamp(InCondition, 0.0f, 1.0f);
  const float SafeWeight = FMath::Max(ConditionWeight, 0.0f);
  return InBaseValue * (SafeCondition * SafeWeight);
}

bool UPostApocInventoryComponent::CheckSpaceInternal(
    const FIntPoint TopLeftIndex, const FIntPoint ItemSize,
    const bool bIsRotated, const FGuid* IgnoredInstanceId) const {
  const int32 EffectiveWidth = bIsRotated ? ItemSize.Y : ItemSize.X;
  const int32 EffectiveHeight = bIsRotated ? ItemSize.X : ItemSize.Y;

  if (EffectiveWidth <= 0 || EffectiveHeight <= 0) {
    return false;
  }

  for (int32 Row = 0; Row < EffectiveHeight; ++Row) {
    for (int32 Col = 0; Col < EffectiveWidth; ++Col) {
      const FIntPoint Cell(TopLeftIndex.X + Col, TopLeftIndex.Y + Row);
      if (Cell.X < 0 || Cell.X >= GridColumns || Cell.Y < 0 || Cell.Y >= GridRows) {
        return false;
      }

      for (const FGridSlotData& SlotData : OccupiedSlotsArray) {
        if (SlotData.Location != Cell) {
          continue;
        }

        if (IgnoredInstanceId && SlotData.ItemInstanceId == *IgnoredInstanceId) {
          continue;
        }

        return false;
      }
    }
  }

  return true;
}

bool UPostApocInventoryComponent::CheckSpace(
    const FIntPoint TopLeftIndex, const FIntPoint ItemSize,
    const bool bIsRotated) const {
  return CheckSpaceInternal(TopLeftIndex, ItemSize, bIsRotated, nullptr);
}

bool UPostApocInventoryComponent::FindEmptySpace(
    const FIntPoint ItemSize, const bool bCheckRotated,
    FIntPoint& OutFoundLocation) const {
  for (int32 Y = 0; Y < GridRows; ++Y) {
    for (int32 X = 0; X < GridColumns; ++X) {
      const FIntPoint CurrentCell(X, Y);
      if (CheckSpace(CurrentCell, ItemSize, bCheckRotated)) {
        OutFoundLocation = CurrentCell;
        return true;
      }
    }
  }

  return false;
}

bool UPostApocInventoryComponent::PlaceItemInstanceAt(
    const FDataTableRowHandle ItemRowHandle, const FGuid& ItemInstanceId,
    const FIntPoint TopLeftIndex, const bool bIsRotated,
    const int32 ItemCondition, const EConsumableFillState FillState) {
  if (ItemRowHandle.IsNull()) {
    return false;
  }

  const FPostApocItemRow* ItemData =
      ItemRowHandle.GetRow<FPostApocItemRow>(TEXT("PlaceItemInstanceAt"));
  if (!ItemData) {
    return false;
  }

  const EConsumableFillState NormalizedFillState =
      NormalizeFillStateForItem(*ItemData, FillState);
  if (!CheckSpace(TopLeftIndex, ItemData->ItemSize, bIsRotated)) {
    return false;
  }

  const FIntPoint EffectiveSize = GetEffectiveItemSize(*ItemData, bIsRotated);
  for (int32 Row = 0; Row < EffectiveSize.Y; ++Row) {
    for (int32 Col = 0; Col < EffectiveSize.X; ++Col) {
      FGridSlotData NewSlot;
      NewSlot.Location = FIntPoint(TopLeftIndex.X + Col, TopLeftIndex.Y + Row);
      NewSlot.ItemInstanceId = ItemInstanceId;
      NewSlot.ItemRowName = ItemRowHandle.RowName;
      NewSlot.bIsRotated = bIsRotated;
      NewSlot.Condition = FMath::Clamp(ItemCondition, 10, 100);
      NewSlot.FillState = NormalizedFillState;
      OccupiedSlotsArray.Add(NewSlot);
    }
  }

  return true;
}

bool UPostApocInventoryComponent::TryAddItem(FDataTableRowHandle ItemRowHandle) {
  FGuid IgnoredInstanceId;
  return TryAddItem(ItemRowHandle, IgnoredInstanceId, 100,
                    EConsumableFillState::NotApplicable);
}

bool UPostApocInventoryComponent::TryAddItem(
    FDataTableRowHandle ItemRowHandle, FGuid& OutItemInstanceId,
    const int32 ItemCondition, const EConsumableFillState FillState) {
  if (ItemRowHandle.IsNull()) {
    return false;
  }

  if (!ItemDataTable && ItemRowHandle.DataTable) {
    ItemDataTable = const_cast<UDataTable*>(ItemRowHandle.DataTable.Get());
  }

  const FPostApocItemRow* ItemData =
      ItemRowHandle.GetRow<FPostApocItemRow>(TEXT("TryAddItem"));
  if (!ItemData) {
    UE_LOG(LogTemp, Warning,
           TEXT("[Grid Inventory] TryAddItem failed for row '%s'."),
           *ItemRowHandle.RowName.ToString());
    return false;
  }

  FIntPoint FoundLocation;
  bool bFoundSpace = FindEmptySpace(ItemData->ItemSize, false, FoundLocation);
  bool bPlacedRotated = false;

  if (!bFoundSpace && ItemData->bCanBeRotated) {
    bFoundSpace = FindEmptySpace(ItemData->ItemSize, true, FoundLocation);
    bPlacedRotated = bFoundSpace;
  }

  if (!bFoundSpace) {
    UE_LOG(LogTemp, Warning,
           TEXT("[Grid Inventory] No space for '%s'."),
           *ItemRowHandle.RowName.ToString());
    return false;
  }

  if (!OutItemInstanceId.IsValid()) {
    OutItemInstanceId = FGuid::NewGuid();
  }

  if (!PlaceItemInstanceAt(ItemRowHandle, OutItemInstanceId, FoundLocation,
                           bPlacedRotated, ItemCondition, FillState)) {
    return false;
  }

  BroadcastGridChanged();
  return true;
}

bool UPostApocInventoryComponent::FindFirstItemInstanceByRowName(
    const FName ItemRowName, FGuid& OutItemInstanceId) const {
  if (ItemRowName.IsNone()) {
    return false;
  }

  TArray<FGridSlotData> SortedSlots = OccupiedSlotsArray;
  SortedSlots.Sort(CompareLocations);

  for (const FGridSlotData& SlotData : SortedSlots) {
    if (SlotData.ItemRowName == ItemRowName && SlotData.ItemInstanceId.IsValid()) {
      OutItemInstanceId = SlotData.ItemInstanceId;
      return true;
    }
  }

  return false;
}

bool UPostApocInventoryComponent::RemoveItemFromGrid(const FName ItemRowName) {
  FGuid ItemInstanceId;
  return FindFirstItemInstanceByRowName(ItemRowName, ItemInstanceId) &&
         RemoveItemByInstanceId(ItemInstanceId);
}

bool UPostApocInventoryComponent::RemoveItemByInstanceId(const FGuid ItemInstanceId) {
  if (!ItemInstanceId.IsValid()) {
    return false;
  }

  bool bRemovedAny = false;
  for (int32 Index = OccupiedSlotsArray.Num() - 1; Index >= 0; --Index) {
    if (OccupiedSlotsArray[Index].ItemInstanceId == ItemInstanceId) {
      OccupiedSlotsArray.RemoveAt(Index);
      bRemovedAny = true;
    }
  }

  if (bRemovedAny) {
    BroadcastGridChanged();
  }

  return bRemovedAny;
}

int32 UPostApocInventoryComponent::CalculateItemValue(
    const int32 BaseValue, const int32 ItemCondition) const {
  const int32 SafeCondition = FMath::Clamp(ItemCondition, 10, 100);
  float ValueMultiplier = 1.0f;

  if (BaseValue >= 100) {
    if (SafeCondition >= 85) {
      ValueMultiplier = 1.0f;
    } else if (SafeCondition >= 70) {
      ValueMultiplier = 0.85f;
    } else if (SafeCondition >= 49) {
      ValueMultiplier = 0.72f;
    } else if (SafeCondition >= 25) {
      ValueMultiplier = 0.52f;
    } else {
      ValueMultiplier = 0.20f;
    }
  } else if (SafeCondition < 70) {
    ValueMultiplier = 0.70f;
  }

  return FMath::RoundToInt(static_cast<float>(BaseValue) * ValueMultiplier);
}

int32 UPostApocInventoryComponent::CalculateItemValueForRow(
    const FPostApocItemRow& ItemData, const int32 ItemCondition,
    const EConsumableFillState FillState) const {
  if (UsesBinaryFillState(ItemData)) {
    const float FillMultiplier =
        FillState == EConsumableFillState::HalfFull ? 0.5f : 1.0f;
    return FMath::RoundToInt(static_cast<float>(ItemData.BaseValue) *
                             FillMultiplier);
  }

  return CalculateItemValue(ItemData.BaseValue, ItemCondition);
}

int32 UPostApocInventoryComponent::GetItemCountInGrid(const FName ItemRowName) const {
  if (ItemRowName.IsNone()) {
    return 0;
  }

  TSet<FGuid> UniqueItems;
  for (const FGridSlotData& SlotData : OccupiedSlotsArray) {
    if (SlotData.ItemRowName == ItemRowName && SlotData.ItemInstanceId.IsValid()) {
      UniqueItems.Add(SlotData.ItemInstanceId);
    }
  }

  return UniqueItems.Num();
}

float UPostApocInventoryComponent::GetInventoryValue() const {
  if (!ItemDataTable) {
    return 0.0f;
  }

  float TotalValue = 0.0f;
  TSet<FGuid> ProcessedItems;
  for (const FGridSlotData& SlotData : OccupiedSlotsArray) {
    if (!SlotData.ItemInstanceId.IsValid() ||
        ProcessedItems.Contains(SlotData.ItemInstanceId)) {
      continue;
    }

    const FPostApocItemRow* ItemData =
        ItemDataTable->FindRow<FPostApocItemRow>(SlotData.ItemRowName,
                                                 TEXT("GetInventoryValue"));
    if (ItemData) {
      TotalValue += CalculateItemValueForRow(*ItemData, SlotData.Condition,
                                             SlotData.FillState);
    }

    ProcessedItems.Add(SlotData.ItemInstanceId);
  }

  return TotalValue;
}

bool UPostApocInventoryComponent::GetItemPlacement(
    const FGuid ItemInstanceId, FGridItemInstanceView& OutInstance) const {
  if (!ItemInstanceId.IsValid() || !ItemDataTable) {
    return false;
  }

  TArray<FGridSlotData> ItemCells;
  for (const FGridSlotData& SlotData : OccupiedSlotsArray) {
    if (SlotData.ItemInstanceId == ItemInstanceId) {
      ItemCells.Add(SlotData);
    }
  }

  if (ItemCells.Num() == 0) {
    return false;
  }

  ItemCells.Sort(CompareLocations);
  const FGridSlotData& AnchorCell = ItemCells[0];
  const FPostApocItemRow* ItemData =
      ItemDataTable->FindRow<FPostApocItemRow>(AnchorCell.ItemRowName,
                                               TEXT("GetItemPlacement"));
  if (!ItemData) {
    return false;
  }

  OutInstance.ItemInstanceId = ItemInstanceId;
  OutInstance.ItemHandle.DataTable = ItemDataTable;
  OutInstance.ItemHandle.RowName = AnchorCell.ItemRowName;
  OutInstance.TopLeft = AnchorCell.Location;
  OutInstance.ItemSize = GetEffectiveItemSize(*ItemData, AnchorCell.bIsRotated);
  OutInstance.bIsRotated = AnchorCell.bIsRotated;
  OutInstance.Condition = AnchorCell.Condition;
  OutInstance.FillState = AnchorCell.FillState;
  return true;
}

TArray<FGridItemInstanceView> UPostApocInventoryComponent::GetItemInstances() const {
  TArray<FGridItemInstanceView> Instances;
  TSet<FGuid> ProcessedItems;

  for (const FGridSlotData& SlotData : OccupiedSlotsArray) {
    if (!SlotData.ItemInstanceId.IsValid() ||
        ProcessedItems.Contains(SlotData.ItemInstanceId)) {
      continue;
    }

    FGridItemInstanceView InstanceView;
    if (GetItemPlacement(SlotData.ItemInstanceId, InstanceView)) {
      Instances.Add(InstanceView);
      ProcessedItems.Add(SlotData.ItemInstanceId);
    }
  }

  Instances.Sort([](const FGridItemInstanceView& A, const FGridItemInstanceView& B) {
    if (A.TopLeft.Y != B.TopLeft.Y) {
      return A.TopLeft.Y < B.TopLeft.Y;
    }
    return A.TopLeft.X < B.TopLeft.X;
  });

  return Instances;
}

bool UPostApocInventoryComponent::GetItemInstanceView(
    const FGuid ItemInstanceId, FGridItemInstanceView& OutInstance) const {
  return GetItemPlacement(ItemInstanceId, OutInstance);
}

bool UPostApocInventoryComponent::GetItemHandleForInstance(
    const FGuid ItemInstanceId, FDataTableRowHandle& OutItemHandle) const {
  FGridItemInstanceView ItemInstance;
  if (!GetItemPlacement(ItemInstanceId, ItemInstance)) {
    return false;
  }

  OutItemHandle = ItemInstance.ItemHandle;
  return true;
}

bool UPostApocInventoryComponent::GetConditionForInstance(
    const FGuid ItemInstanceId, int32& OutCondition) const {
  FGridItemInstanceView ItemInstance;
  if (!GetItemPlacement(ItemInstanceId, ItemInstance)) {
    return false;
  }

  OutCondition = ItemInstance.Condition;
  return true;
}

bool UPostApocInventoryComponent::GetFillStateForInstance(
    const FGuid ItemInstanceId, EConsumableFillState& OutFillState) const {
  FGridItemInstanceView ItemInstance;
  if (!GetItemPlacement(ItemInstanceId, ItemInstance)) {
    return false;
  }

  OutFillState = ItemInstance.FillState;
  return true;
}

bool UPostApocInventoryComponent::GetItemDisplayStateForInstance(
    const FGuid ItemInstanceId, FText& OutStateText) const {
  FGridItemInstanceView ItemInstance;
  if (!GetItemPlacement(ItemInstanceId, ItemInstance) || !ItemDataTable) {
    return false;
  }

  const FPostApocItemRow* ItemData =
      ItemDataTable->FindRow<FPostApocItemRow>(ItemInstance.ItemHandle.RowName,
                                               TEXT("GetItemDisplayStateForInstance"));
  if (!ItemData) {
    return false;
  }

  OutStateText =
      GetDisplayStateText(*ItemData, ItemInstance.Condition, ItemInstance.FillState);
  return true;
}

int32 UPostApocInventoryComponent::GetItemValueForInstance(
    const FGuid ItemInstanceId) const {
  FGridItemInstanceView ItemInstance;
  if (!GetItemPlacement(ItemInstanceId, ItemInstance) || !ItemDataTable) {
    return 0;
  }

  const FPostApocItemRow* ItemData =
      ItemDataTable->FindRow<FPostApocItemRow>(ItemInstance.ItemHandle.RowName,
                                               TEXT("GetItemValueForInstance"));
  if (!ItemData) {
    return 0;
  }

  return CalculateItemValueForRow(*ItemData, ItemInstance.Condition,
                                  ItemInstance.FillState);
}

int32 UPostApocInventoryComponent::GetTotalValueForInstances(
    const TArray<FGuid>& ItemInstanceIds) const {
  int32 TotalValue = 0;
  TSet<FGuid> ProcessedItems;

  for (const FGuid& ItemInstanceId : ItemInstanceIds) {
    if (!ItemInstanceId.IsValid() || ProcessedItems.Contains(ItemInstanceId)) {
      continue;
    }

    TotalValue += GetItemValueForInstance(ItemInstanceId);
    ProcessedItems.Add(ItemInstanceId);
  }

  return TotalValue;
}

void UPostApocInventoryComponent::NativeRefreshUI(UUserWidget* GridWidget) {
  if (!IsValid(GridWidget) || !GridItemWidgetClass || !ItemDataTable) {
    return;
  }

  UCanvasPanel* GridCanvas =
      Cast<UCanvasPanel>(GridWidget->GetWidgetFromName(TEXT("GridCanvas")));
  if (!GridCanvas) {
    return;
  }

  GridCanvas->ClearChildren();
  UPostApocInventoryGridWidget* GridContextWidget =
      Cast<UPostApocInventoryGridWidget>(GridWidget);

  const TArray<FGridItemInstanceView> ItemInstances = GetItemInstances();
  for (const FGridItemInstanceView& ItemInstance : ItemInstances) {
    UPostApocGridItem* NewItemWidget =
        CreateWidget<UPostApocGridItem>(GridWidget, GridItemWidgetClass);
    if (!NewItemWidget) {
      continue;
    }

    NewItemWidget->ItemVerisi = ItemInstance.ItemHandle;
    NewItemWidget->ItemInstanceId = ItemInstance.ItemInstanceId;
    NewItemWidget->bIsRotated = ItemInstance.bIsRotated;
    NewItemWidget->OwningGridWidget = GridContextWidget;
    NewItemWidget->RefreshVisuals();

    if (UCanvasPanelSlot* CanvasSlot = GridCanvas->AddChildToCanvas(NewItemWidget)) {
      CanvasSlot->SetAutoSize(true);
      CanvasSlot->SetPosition(FVector2D(ItemInstance.TopLeft.X * TileSize,
                                        ItemInstance.TopLeft.Y * TileSize));
    }
  }
}

bool UPostApocInventoryComponent::HandleItemDrop(
    const FName ItemRowName, const FIntPoint NewLocation) {
  FGuid ItemInstanceId;
  return FindFirstItemInstanceByRowName(ItemRowName, ItemInstanceId) &&
         HandleItemDropByInstanceId(ItemInstanceId, NewLocation);
}

bool UPostApocInventoryComponent::HandleItemDropByInstanceId(
    const FGuid ItemInstanceId, const FIntPoint NewLocation) {
  FGridItemInstanceView ItemInstance;
  if (!GetItemPlacement(ItemInstanceId, ItemInstance)) {
    return false;
  }

  const FIntPoint OldLocation = ItemInstance.TopLeft;
  const FDataTableRowHandle ItemHandle = ItemInstance.ItemHandle;
  const bool bWasRotated = ItemInstance.bIsRotated;

  if (!RemoveItemByInstanceId(ItemInstanceId)) {
    return false;
  }

  if (PlaceItemInstanceAt(ItemHandle, ItemInstanceId, NewLocation, bWasRotated,
                          ItemInstance.Condition, ItemInstance.FillState)) {
    BroadcastGridChanged();
    return true;
  }

  PlaceItemInstanceAt(ItemHandle, ItemInstanceId, OldLocation, bWasRotated,
                      ItemInstance.Condition, ItemInstance.FillState);
  BroadcastGridChanged();
  return false;
}

void UPostApocInventoryComponent::OnRep_GridUpdated() {
  OnGridUpdated.Broadcast();
}

TMap<FIntPoint, FName> UPostApocInventoryComponent::GetOccupiedSlots() const {
  TMap<FIntPoint, FName> Slots;
  for (const FGridSlotData& SlotData : OccupiedSlotsArray) {
    Slots.Add(SlotData.Location, SlotData.ItemRowName);
  }
  return Slots;
}

TMap<FIntPoint, FGuid> UPostApocInventoryComponent::GetOccupiedSlotInstances() const {
  TMap<FIntPoint, FGuid> Slots;
  for (const FGridSlotData& SlotData : OccupiedSlotsArray) {
    Slots.Add(SlotData.Location, SlotData.ItemInstanceId);
  }
  return Slots;
}

void UPostApocInventoryComponent::ExportSaveData(
    TArray<FGridSlotData>& OutSlots, FString& OutDataTablePath) const {
  OutSlots = OccupiedSlotsArray;
  OutDataTablePath = ItemDataTable ? ItemDataTable->GetPathName() : FString();
}

void UPostApocInventoryComponent::MigrateLegacySlots() {
  if (OccupiedSlotsArray.Num() == 0) {
    return;
  }

  bool bNeedsMigration = false;
  for (const FGridSlotData& SlotData : OccupiedSlotsArray) {
    if (!SlotData.ItemInstanceId.IsValid()) {
      bNeedsMigration = true;
      break;
    }
  }

  if (!bNeedsMigration) {
    return;
  }

  TArray<FGridSlotData> SortedSlots = OccupiedSlotsArray;
  SortedSlots.Sort(CompareLocations);

  TMap<FIntPoint, int32> SlotIndexByLocation;
  for (int32 Index = 0; Index < OccupiedSlotsArray.Num(); ++Index) {
    SlotIndexByLocation.Add(OccupiedSlotsArray[Index].Location, Index);
  }

  TSet<FIntPoint> ClaimedCells;
  for (const FGridSlotData& SlotData : SortedSlots) {
    if (ClaimedCells.Contains(SlotData.Location)) {
      continue;
    }

    FGuid ItemInstanceId = FGuid::NewGuid();
    bool bIsRotated = false;
    FIntPoint EffectiveSize(1, 1);

    if (ItemDataTable) {
      if (const FPostApocItemRow* ItemData =
              ItemDataTable->FindRow<FPostApocItemRow>(SlotData.ItemRowName,
                                                       TEXT("MigrateLegacySlots"))) {
        EffectiveSize = ItemData->ItemSize;
      }
    }

    for (int32 Row = 0; Row < EffectiveSize.Y; ++Row) {
      for (int32 Col = 0; Col < EffectiveSize.X; ++Col) {
        const FIntPoint Cell(SlotData.Location.X + Col, SlotData.Location.Y + Row);
        if (ClaimedCells.Contains(Cell)) {
          continue;
        }

        const int32* ExistingIndex = SlotIndexByLocation.Find(Cell);
        if (!ExistingIndex) {
          continue;
        }

        FGridSlotData& MutableSlot = OccupiedSlotsArray[*ExistingIndex];
        if (MutableSlot.ItemRowName != SlotData.ItemRowName) {
          continue;
        }

        MutableSlot.ItemInstanceId = ItemInstanceId;
        MutableSlot.bIsRotated = bIsRotated;
        MutableSlot.Condition = FMath::Clamp(MutableSlot.Condition, 10, 100);
        if (ItemDataTable) {
          if (const FPostApocItemRow* ItemData =
                  ItemDataTable->FindRow<FPostApocItemRow>(
                      MutableSlot.ItemRowName,
                      TEXT("MigrateLegacySlots.FillState"))) {
            MutableSlot.FillState =
                NormalizeFillStateForItem(*ItemData, MutableSlot.FillState);
          }
        }
        ClaimedCells.Add(Cell);
      }
    }

    if (!ClaimedCells.Contains(SlotData.Location)) {
      const int32* ExistingIndex = SlotIndexByLocation.Find(SlotData.Location);
      if (ExistingIndex) {
        OccupiedSlotsArray[*ExistingIndex].ItemInstanceId = ItemInstanceId;
        OccupiedSlotsArray[*ExistingIndex].bIsRotated = false;
        OccupiedSlotsArray[*ExistingIndex].Condition =
            FMath::Clamp(OccupiedSlotsArray[*ExistingIndex].Condition, 10, 100);
        if (ItemDataTable) {
          if (const FPostApocItemRow* ItemData = ItemDataTable->FindRow<FPostApocItemRow>(
                  OccupiedSlotsArray[*ExistingIndex].ItemRowName,
                  TEXT("MigrateLegacySlots.AnchorFillState"))) {
            OccupiedSlotsArray[*ExistingIndex].FillState =
                NormalizeFillStateForItem(
                    *ItemData, OccupiedSlotsArray[*ExistingIndex].FillState);
          }
        }
        ClaimedCells.Add(SlotData.Location);
      }
    }
  }
}

void UPostApocInventoryComponent::ImportSaveData(
    const TArray<FGridSlotData>& InSlots, UDataTable* InDataTable) {
  ItemDataTable = InDataTable;
  OccupiedSlotsArray = InSlots;
  for (FGridSlotData& SlotData : OccupiedSlotsArray) {
    SlotData.Condition = FMath::Clamp(SlotData.Condition, 10, 100);
    if (ItemDataTable) {
      if (const FPostApocItemRow* ItemData = ItemDataTable->FindRow<FPostApocItemRow>(
              SlotData.ItemRowName, TEXT("ImportSaveData.FillState"))) {
        SlotData.FillState = NormalizeFillStateForItem(*ItemData, SlotData.FillState);
      }
    }
  }
  MigrateLegacySlots();
  BroadcastGridChanged();
}

EConsumableFillState UPostApocInventoryComponent::NormalizeFillStateForItem(
    const FPostApocItemRow& ItemData,
    const EConsumableFillState RequestedFillState) const {
  if (!UsesBinaryFillState(ItemData)) {
    return EConsumableFillState::NotApplicable;
  }

  if (RequestedFillState == EConsumableFillState::HalfFull ||
      RequestedFillState == EConsumableFillState::Full) {
    return RequestedFillState;
  }

  return EConsumableFillState::Full;
}

bool UPostApocInventoryComponent::UsesBinaryFillState(
    const FPostApocItemRow& ItemData) const {
  return UsesBinaryFillStateForCategory(ItemData.Category);
}

FText UPostApocInventoryComponent::GetDisplayStateText(
    const FPostApocItemRow& ItemData, const int32 ItemCondition,
    const EConsumableFillState FillState) const {
  if (UsesBinaryFillState(ItemData)) {
    switch (NormalizeFillStateForItem(ItemData, FillState)) {
    case EConsumableFillState::HalfFull:
      return FText::FromString(TEXT("Yari Dolu"));
    case EConsumableFillState::Full:
      return FText::FromString(TEXT("Tam Dolu"));
    default:
      break;
    }
  }

  return FText::FromString(
      FString::Printf(TEXT("Kondisyon: %d"), FMath::Clamp(ItemCondition, 10, 100)));
}

void UPostApocInventoryComponent::BroadcastGridChanged() {
  OnGridUpdated.Broadcast();
}

void UPostApocInventoryComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);
  DOREPLIFETIME(UPostApocInventoryComponent, OccupiedSlotsArray);
}
