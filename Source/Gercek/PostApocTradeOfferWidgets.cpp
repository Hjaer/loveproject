#include "PostApocTradeOfferWidgets.h"

#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/WrapBox.h"
#include "GercekCharacter.h"
#include "PostApocGridDragDropOperation.h"

void UPostApocTradeOfferEntryWidget::Configure(
    AGercekCharacter* InOwningCharacter,
    const EPostApocInventoryGridRole InSourceRole,
    const FPostApocTradeOfferEntryData& InEntryData) {
  OwningCharacter = InOwningCharacter;
  SourceRole = InSourceRole;
  EntryData = InEntryData;

  if (EntryText) {
    EntryText->SetText(FText::FromString(FString::Printf(
        TEXT("%s\n%s\nDeger: %s"), *EntryData.ItemName.ToString(),
        *EntryData.DisplayStateText.ToString(),
        *EntryData.DisplayValueText.ToString())));
  }
}

void UPostApocTradeOfferEntryWidget::NativeConstruct() {
  Super::NativeConstruct();
  EnsureWidgetTree();
  Configure(OwningCharacter, SourceRole, EntryData);
}

void UPostApocTradeOfferEntryWidget::HandleClicked() {
  if (OwningCharacter && EntryData.ItemInstanceId.IsValid()) {
    OwningCharacter->ToggleTradeOfferItem(SourceRole, EntryData.ItemInstanceId);
  }
}

void UPostApocTradeOfferEntryWidget::EnsureWidgetTree() {
  if (!WidgetTree || WidgetTree->RootWidget) {
    return;
  }

  EntryButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(),
                                                     TEXT("EntryButton"));
  EntryText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),
                                                      TEXT("EntryText"));
  EntryButton->AddChild(EntryText);
  EntryButton->OnClicked.AddDynamic(this,
                                    &UPostApocTradeOfferEntryWidget::HandleClicked);
  WidgetTree->RootWidget = EntryButton;
}

void UPostApocTradeOfferPanelWidget::Configure(
    AGercekCharacter* InOwningCharacter,
    const EPostApocInventoryGridRole InAcceptedRole,
    const FText& InPanelTitle) {
  OwningCharacter = InOwningCharacter;
  AcceptedRole = InAcceptedRole;
  PanelTitle = InPanelTitle;

  if (TitleText) {
    TitleText->SetText(PanelTitle);
  }
}

void UPostApocTradeOfferPanelWidget::RefreshEntries(
    const TArray<FPostApocTradeOfferEntryData>& Entries,
    const FText& TotalValueText) {
  EnsureWidgetTree();

  if (TitleText) {
    TitleText->SetText(PanelTitle);
  }
  if (TotalText) {
    TotalText->SetText(TotalValueText);
  }
  if (!EntryWrapBox) {
    return;
  }

  EntryWrapBox->ClearChildren();
  for (const FPostApocTradeOfferEntryData& Entry : Entries) {
    UPostApocTradeOfferEntryWidget* EntryWidget =
        CreateWidget<UPostApocTradeOfferEntryWidget>(
            GetWorld(), UPostApocTradeOfferEntryWidget::StaticClass());
    if (!EntryWidget) {
      continue;
    }

    EntryWidget->Configure(OwningCharacter, AcceptedRole, Entry);
    EntryWrapBox->AddChildToWrapBox(EntryWidget);
  }
}

void UPostApocTradeOfferPanelWidget::NativeConstruct() {
  Super::NativeConstruct();
  EnsureWidgetTree();
  Configure(OwningCharacter, AcceptedRole, PanelTitle);
}

bool UPostApocTradeOfferPanelWidget::NativeOnDrop(
    const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation) {
  const UPostApocGridDragDropOperation* GridOperation =
      Cast<UPostApocGridDragDropOperation>(InOperation);
  if (!GridOperation || !OwningCharacter || !GridOperation->ItemInstanceId.IsValid()) {
    return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
  }

  if (GridOperation->SourceGridRole != AcceptedRole) {
    return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
  }

  OwningCharacter->ToggleTradeOfferItem(AcceptedRole,
                                        GridOperation->ItemInstanceId);
  return true;
}

void UPostApocTradeOfferPanelWidget::EnsureWidgetTree() {
  if (!WidgetTree || WidgetTree->RootWidget) {
    return;
  }

  RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(),
                                                      TEXT("RootBox"));
  TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),
                                                      TEXT("TitleText"));
  TotalText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),
                                                      TEXT("TotalText"));
  EntryWrapBox = WidgetTree->ConstructWidget<UWrapBox>(UWrapBox::StaticClass(),
                                                       TEXT("EntryWrapBox"));

  RootBox->AddChildToVerticalBox(TitleText);
  RootBox->AddChildToVerticalBox(TotalText);
  RootBox->AddChildToVerticalBox(EntryWrapBox);
  WidgetTree->RootWidget = RootBox;
}
