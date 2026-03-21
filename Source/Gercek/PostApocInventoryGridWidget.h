#pragma once

#include "Blueprint/UserWidget.h"
#include "PostApocGridDragDropOperation.h"
#include "PostApocInventoryTypes.h"
#include "PostApocInventoryGridWidget.generated.h"

class AGercekCharacter;
class ALootContainerBase;
class UCanvasPanel;
class UPostApocInventoryComponent;

UCLASS(Abstract, Blueprintable)
class GERCEK_API UPostApocInventoryGridWidget : public UUserWidget {
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory")
  void InitializeGridContext(UPostApocInventoryComponent* InInventoryComponent,
                             AGercekCharacter* InOwningCharacter,
                             ALootContainerBase* InLootContainer,
                             EPostApocInventoryGridRole InGridRole);

  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory")
  void HandleGridItemClicked(FGuid ItemInstanceId);

  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory")
  void HandleGridItemActivated(FGuid ItemInstanceId);

  UFUNCTION(BlueprintPure, Category = "PostApoc Inventory")
  UPostApocInventoryComponent* GetBoundInventoryComponent() const {
    return BoundInventoryComponent;
  }

  UFUNCTION(BlueprintPure, Category = "PostApoc Inventory")
  AGercekCharacter* GetOwningGercekCharacter() const {
    return OwningGercekCharacter;
  }

  UFUNCTION(BlueprintPure, Category = "PostApoc Inventory")
  EPostApocInventoryGridRole GetGridRole() const { return GridRole; }

  UFUNCTION(BlueprintCallable, Category = "PostApoc Inventory")
  void SetUseDefaultItemActions(bool bInUseDefaultItemActions) {
    bUseDefaultItemActions = bInUseDefaultItemActions;
  }

  UFUNCTION(BlueprintImplementableEvent, Category = "PostApoc Inventory")
  void BP_OnGridItemClicked(const FGridItemInstanceView& ItemInstance);

  UFUNCTION(BlueprintImplementableEvent, Category = "PostApoc Inventory")
  void BP_OnGridItemActivated(const FGridItemInstanceView& ItemInstance);

  UFUNCTION(BlueprintImplementableEvent, Category = "PostApoc Inventory")
  void BP_OnGridDropRejected(FGuid ItemInstanceId);

  UFUNCTION(BlueprintImplementableEvent, Category = "PostApoc Inventory")
  void BP_OnGridItemDropped(FGuid ItemInstanceId,
                            EPostApocInventoryGridRole SourceGridRole,
                            EPostApocInventoryGridRole TargetGridRole);

protected:
  virtual bool NativeOnDrop(const FGeometry& InGeometry,
                            const FDragDropEvent& InDragDropEvent,
                            UDragDropOperation* InOperation) override;

  bool HandleInternalDrop(const FGeometry& InGeometry,
                          const FDragDropEvent& InDragDropEvent,
                          FGuid ItemInstanceId,
                          UPostApocInventoryComponent* SourceInventory,
                          EPostApocInventoryGridRole SourceRole);

  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional),
            Category = "PostApoc Inventory")
  TObjectPtr<UCanvasPanel> GridCanvas = nullptr;

  UPROPERTY(BlueprintReadOnly, Category = "PostApoc Inventory")
  TObjectPtr<UPostApocInventoryComponent> BoundInventoryComponent = nullptr;

  UPROPERTY(BlueprintReadOnly, Category = "PostApoc Inventory")
  TObjectPtr<AGercekCharacter> OwningGercekCharacter = nullptr;

  UPROPERTY(BlueprintReadOnly, Category = "PostApoc Inventory")
  TObjectPtr<ALootContainerBase> BoundLootContainer = nullptr;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory")
  EPostApocInventoryGridRole GridRole =
      EPostApocInventoryGridRole::PlayerInventory;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostApoc Inventory")
  bool bUseDefaultItemActions = true;
};
