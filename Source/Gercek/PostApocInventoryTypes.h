#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UObject/Interface.h"

#include "ItemTypes.h"
#include "PostApocItemTypes.h"

#include "PostApocInventoryTypes.generated.h"

class UUserWidget;

UINTERFACE(MinimalAPI, BlueprintType)
class UInventoryInterface : public UInterface {
  GENERATED_BODY()
};

class GERCEK_API IInventoryInterface {
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintCallable, BlueprintNativeEvent,
            Category = "PostApoc Inventory")
  void GetItemDetails(FName ItemID, FItemDBRow &OutItemData,
                      float &OutCondition);

  UFUNCTION(BlueprintCallable, BlueprintNativeEvent,
            Category = "PostApoc Inventory")
  bool MoveItem(int32 OldIndex, int32 NewIndex);

  UFUNCTION(BlueprintCallable, BlueprintNativeEvent,
            Category = "PostApoc Inventory")
  void DropItem(FName ItemID, float Condition);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGridUpdatedDelegate);

USTRUCT(BlueprintType)
struct FGridSlotData {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory")
  FIntPoint Location = FIntPoint::ZeroValue;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory")
  FGuid ItemInstanceId;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory")
  FName ItemRowName = NAME_None;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory")
  bool bIsRotated = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory")
  int32 Condition = 100;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory")
  EConsumableFillState FillState = EConsumableFillState::NotApplicable;

  bool operator==(const FGridSlotData& Other) const {
    return Location == Other.Location &&
           ItemInstanceId == Other.ItemInstanceId &&
           ItemRowName == Other.ItemRowName &&
           bIsRotated == Other.bIsRotated &&
           Condition == Other.Condition &&
           FillState == Other.FillState;
  }
};

USTRUCT(BlueprintType)
struct FGridItemInstanceView {
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory")
  FGuid ItemInstanceId;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory")
  FDataTableRowHandle ItemHandle;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory")
  FIntPoint TopLeft = FIntPoint::ZeroValue;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory")
  FIntPoint ItemSize = FIntPoint(1, 1);

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory")
  bool bIsRotated = false;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory")
  int32 Condition = 100;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory")
  EConsumableFillState FillState = EConsumableFillState::NotApplicable;
};

UCLASS(ClassGroup = (PostApoc), meta = (BlueprintSpawnableComponent))
class GERCEK_API UPostApocInventoryComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UPostApocInventoryComponent();

  virtual void BeginPlay() override;
  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty>& OutLifetimeProps) const override;

  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "PostApoc Inventory | Grid")
  int32 GridColumns = 10;

  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "PostApoc Inventory | Grid")
  int32 GridRows = 10;

  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "PostApoc Inventory | Grid")
  float TileSize = 50.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "PostApoc Inventory | Grid")
  TSubclassOf<class UUserWidget> GridItemWidgetClass;

  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            Category = "PostApoc Inventory | Grid")
  class UDataTable* ItemDataTable = nullptr;

  UPROPERTY(ReplicatedUsing = OnRep_GridUpdated)
  TArray<FGridSlotData> OccupiedSlotsArray;

  UPROPERTY(BlueprintAssignable, Category = "PostApoc Inventory | Events")
  FOnGridUpdatedDelegate OnGridUpdated;

  UFUNCTION()
  void OnRep_GridUpdated();

  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory | Grid")
  bool TryAddItem(FDataTableRowHandle ItemRowHandle);

  bool TryAddItem(FDataTableRowHandle ItemRowHandle, FGuid& OutItemInstanceId,
                  int32 ItemCondition = 100,
                  EConsumableFillState FillState =
                      EConsumableFillState::NotApplicable);

  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory | Grid")
  bool RemoveItemFromGrid(FName ItemRowName);

  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory | Grid")
  bool RemoveItemByInstanceId(FGuid ItemInstanceId);

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Grid")
  int32 GetItemCountInGrid(FName ItemRowName) const;

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Grid")
  bool FindFirstItemInstanceByRowName(FName ItemRowName,
                                      FGuid& OutItemInstanceId) const;

  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory | Grid")
  bool FindEmptySpace(FIntPoint ItemSize, bool bCheckRotated,
                      FIntPoint &OutFoundLocation) const;

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Economy")
  float CalculateBarterValue(float InBaseValue, float InCondition,
                             float ConditionWeight = 1.0f) const;

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Economy")
  int32 CalculateItemValue(int32 BaseValue, int32 ItemCondition) const;

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Economy")
  int32 CalculateItemValueForRow(const FPostApocItemRow& ItemData,
                                 int32 ItemCondition,
                                 EConsumableFillState FillState) const;

  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory | Grid")
  bool CheckSpace(FIntPoint TopLeftIndex, FIntPoint ItemSize,
                  bool bIsRotated) const;

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Grid")
  float GetInventoryValue() const;

  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory | Grid")
  void NativeRefreshUI(UUserWidget* GridWidget);

  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory | Grid")
  bool HandleItemDrop(FName ItemRowName, FIntPoint NewLocation);

  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory | Grid")
  bool HandleItemDropByInstanceId(FGuid ItemInstanceId, FIntPoint NewLocation);

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Grid")
  int32 GetGridColumns() const { return GridColumns; }

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Grid")
  int32 GetGridRows() const { return GridRows; }

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Grid")
  TMap<FIntPoint, FName> GetOccupiedSlots() const;

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Grid")
  TMap<FIntPoint, FGuid> GetOccupiedSlotInstances() const;

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Grid")
  TArray<FGridItemInstanceView> GetItemInstances() const;

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Grid")
  bool GetItemInstanceView(FGuid ItemInstanceId,
                           FGridItemInstanceView& OutInstance) const;

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Grid")
  bool GetItemHandleForInstance(FGuid ItemInstanceId,
                                FDataTableRowHandle& OutItemHandle) const;

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Grid")
  bool GetConditionForInstance(FGuid ItemInstanceId, int32& OutCondition) const;

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Grid")
  bool GetFillStateForInstance(FGuid ItemInstanceId,
                               EConsumableFillState& OutFillState) const;

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Grid")
  bool GetItemDisplayStateForInstance(FGuid ItemInstanceId,
                                      FText& OutStateText) const;

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Economy")
  int32 GetItemValueForInstance(FGuid ItemInstanceId) const;

  UFUNCTION(BlueprintCallable, BlueprintPure,
            Category = "PostApoc Inventory | Economy")
  int32 GetTotalValueForInstances(const TArray<FGuid>& ItemInstanceIds) const;

  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory | Save")
  void ExportSaveData(TArray<FGridSlotData>& OutSlots, FString& OutDataTablePath) const;

  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory | Save")
  void ImportSaveData(const TArray<FGridSlotData>& InSlots, class UDataTable* InDataTable);

private:
  bool CheckSpaceInternal(FIntPoint TopLeftIndex, FIntPoint ItemSize,
                          bool bIsRotated, const FGuid* IgnoredInstanceId) const;
  bool PlaceItemInstanceAt(FDataTableRowHandle ItemRowHandle, const FGuid& ItemInstanceId,
                           FIntPoint TopLeftIndex, bool bIsRotated,
                           int32 ItemCondition,
                           EConsumableFillState FillState);
  bool GetItemPlacement(FGuid ItemInstanceId, FGridItemInstanceView& OutInstance) const;
  EConsumableFillState NormalizeFillStateForItem(
      const FPostApocItemRow& ItemData,
      EConsumableFillState RequestedFillState) const;
  bool UsesBinaryFillState(const FPostApocItemRow& ItemData) const;
  FText GetDisplayStateText(const FPostApocItemRow& ItemData,
                            int32 ItemCondition,
                            EConsumableFillState FillState) const;
  void MigrateLegacySlots();
  void BroadcastGridChanged();
};
