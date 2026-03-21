#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h"
#include "PostApocItemTypes.h"
#include "PostApocGridItem.generated.h"

class UDragDropOperation;
class UImage;
class UPostApocInventoryGridWidget;
class USizeBox;
class UTextBlock;

UCLASS()
class GERCEK_API UPostApocGridItem : public UUserWidget {
  GENERATED_BODY()

public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            meta = (ExposeOnSpawn = "true"),
            Category = "PostApoc Grid Item")
  FDataTableRowHandle ItemVerisi;

  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            meta = (ExposeOnSpawn = "true"),
            Category = "PostApoc Grid Item")
  FGuid ItemInstanceId;

  UPROPERTY(EditAnywhere, BlueprintReadWrite,
            meta = (ExposeOnSpawn = "true"),
            Category = "PostApoc Grid Item")
  bool bIsRotated = false;

  UPROPERTY(BlueprintReadWrite, Category = "PostApoc Grid Item")
  TObjectPtr<UPostApocInventoryGridWidget> OwningGridWidget = nullptr;

protected:
  UPROPERTY(BlueprintReadOnly, meta = (BindWidget),
            Category = "PostApoc Grid Item | Widgets")
  TObjectPtr<USizeBox> ItemSizeBox;

  UPROPERTY(BlueprintReadOnly, meta = (BindWidget),
            Category = "PostApoc Grid Item | Widgets")
  TObjectPtr<UImage> ItemIcon;

  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional),
            Category = "PostApoc Grid Item | Widgets")
  TObjectPtr<UTextBlock> Txt_ItemName;

  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional),
            Category = "PostApoc Grid Item | Widgets")
  TObjectPtr<UTextBlock> Txt_ItemCondition;

  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional),
            Category = "PostApoc Grid Item | Widgets")
  TObjectPtr<UTextBlock> Txt_ItemValue;

  virtual void NativePreConstruct() override;
  virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry,
                                         const FPointerEvent& InMouseEvent) override;
  virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry,
                                                const FPointerEvent& InMouseEvent) override;
  virtual void NativeOnDragDetected(const FGeometry& InGeometry,
                                    const FPointerEvent& InMouseEvent,
                                    UDragDropOperation*& OutOperation) override;

public:
  void RefreshVisuals();
};
