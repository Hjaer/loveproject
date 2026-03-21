#pragma once

#include "Blueprint/DragDropOperation.h"
#include "CoreMinimal.h"
#include "PostApocGridDragDropOperation.generated.h"

class AGercekCharacter;
class ALootContainerBase;
class UPostApocInventoryComponent;

UENUM(BlueprintType)
enum class EPostApocInventoryGridRole : uint8 {
  PlayerInventory,
  LootContainer,
  MerchantInventory,
  TradePlayerOffer,
  TradeMerchantOffer
};

UCLASS()
class GERCEK_API UPostApocGridDragDropOperation : public UDragDropOperation {
  GENERATED_BODY()

public:
  UPROPERTY(BlueprintReadOnly, Category = "PostApoc Inventory")
  FGuid ItemInstanceId;

  UPROPERTY(BlueprintReadOnly, Category = "PostApoc Inventory")
  EPostApocInventoryGridRole SourceGridRole =
      EPostApocInventoryGridRole::PlayerInventory;

  UPROPERTY(BlueprintReadOnly, Category = "PostApoc Inventory")
  TObjectPtr<UPostApocInventoryComponent> SourceInventory = nullptr;

  UPROPERTY(BlueprintReadOnly, Category = "PostApoc Inventory")
  TObjectPtr<AGercekCharacter> OwningCharacter = nullptr;

  UPROPERTY(BlueprintReadOnly, Category = "PostApoc Inventory")
  TObjectPtr<ALootContainerBase> SourceLootContainer = nullptr;

  UPROPERTY(BlueprintReadOnly, Category = "PostApoc Inventory")
  FVector2D DragOffset = FVector2D::ZeroVector;
};
