#pragma once

#include "Blueprint/UserWidget.h"
#include "PostApocItemTypes.h"
#include "PostApocGridDragDropOperation.h"
#include "PostApocTradeOfferWidgets.generated.h"

class AGercekCharacter;
class UButton;
class UTextBlock;
class UVerticalBox;
class UWrapBox;

USTRUCT(BlueprintType)
struct FPostApocTradeOfferEntryData {
  GENERATED_BODY()

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Trade")
  FGuid ItemInstanceId;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Trade")
  FText ItemName;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Trade")
  int32 Condition = 100;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Trade")
  EConsumableFillState FillState = EConsumableFillState::NotApplicable;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Trade")
  int32 EffectiveValue = 0;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Trade")
  FText DisplayValueText;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Trade")
  FText DisplayStateText;
};

UCLASS()
class GERCEK_API UPostApocTradeOfferEntryWidget : public UUserWidget {
  GENERATED_BODY()

public:
  void Configure(AGercekCharacter* InOwningCharacter,
                 EPostApocInventoryGridRole InSourceRole,
                 const FPostApocTradeOfferEntryData& InEntryData);

protected:
  virtual void NativeConstruct() override;

  UFUNCTION()
  void HandleClicked();

private:
  void EnsureWidgetTree();

  UPROPERTY()
  TObjectPtr<AGercekCharacter> OwningCharacter = nullptr;

  EPostApocInventoryGridRole SourceRole =
      EPostApocInventoryGridRole::PlayerInventory;

  FPostApocTradeOfferEntryData EntryData;

  UPROPERTY()
  TObjectPtr<UButton> EntryButton = nullptr;

  UPROPERTY()
  TObjectPtr<UTextBlock> EntryText = nullptr;
};

UCLASS()
class GERCEK_API UPostApocTradeOfferPanelWidget : public UUserWidget {
  GENERATED_BODY()

public:
  void Configure(AGercekCharacter* InOwningCharacter,
                 EPostApocInventoryGridRole InAcceptedRole,
                 const FText& InPanelTitle);
  void RefreshEntries(const TArray<FPostApocTradeOfferEntryData>& Entries,
                      const FText& TotalValueText);

protected:
  virtual void NativeConstruct() override;
  virtual bool NativeOnDrop(const FGeometry& InGeometry,
                            const FDragDropEvent& InDragDropEvent,
                            UDragDropOperation* InOperation) override;

private:
  void EnsureWidgetTree();

  UPROPERTY()
  TObjectPtr<AGercekCharacter> OwningCharacter = nullptr;

  EPostApocInventoryGridRole AcceptedRole =
      EPostApocInventoryGridRole::PlayerInventory;

  FText PanelTitle;

  UPROPERTY()
  TObjectPtr<UVerticalBox> RootBox = nullptr;

  UPROPERTY()
  TObjectPtr<UTextBlock> TitleText = nullptr;

  UPROPERTY()
  TObjectPtr<UTextBlock> TotalText = nullptr;

  UPROPERTY()
  TObjectPtr<UWrapBox> EntryWrapBox = nullptr;
};
