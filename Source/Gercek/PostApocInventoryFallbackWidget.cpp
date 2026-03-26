#include "PostApocInventoryFallbackWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Widgets/SWidget.h"

UPostApocInventoryFallbackWidget::UPostApocInventoryFallbackWidget(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) {
  SetIsFocusable(false);
}

TSharedRef<SWidget> UPostApocInventoryFallbackWidget::RebuildWidget() {
  EnsureRuntimeLayout();
  return Super::RebuildWidget();
}

void UPostApocInventoryFallbackWidget::NativeConstruct() {
  Super::NativeConstruct();
  EnsureRuntimeLayout();
}

void UPostApocInventoryFallbackWidget::EnsureRuntimeLayout() {
  if (!WidgetTree) {
    return;
  }

  UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
  if (!RootCanvas) {
    RootCanvas =
        WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(),
                                                  TEXT("InventoryRootCanvas"));
    WidgetTree->RootWidget = RootCanvas;
  }

  if (!RootCanvas) {
    return;
  }

  RootCanvas->SetVisibility(ESlateVisibility::Visible);
  RootCanvas->SetRenderOpacity(1.0f);

  UBorder* GridBackdrop =
      Cast<UBorder>(WidgetTree->FindWidget(TEXT("InventoryGridBackdrop")));
  if (!GridBackdrop) {
    GridBackdrop = WidgetTree->ConstructWidget<UBorder>(
        UBorder::StaticClass(), TEXT("InventoryGridBackdrop"));
    GridBackdrop->SetBrushColor(FLinearColor(0.10f, 0.12f, 0.14f, 0.88f));

    if (UCanvasPanelSlot* GridBackdropSlot =
            RootCanvas->AddChildToCanvas(GridBackdrop)) {
      GridBackdropSlot->SetAutoSize(false);
      GridBackdropSlot->SetPosition(FVector2D(56.0f, 96.0f));
      GridBackdropSlot->SetSize(FVector2D(520.0f, 520.0f));
      GridBackdropSlot->SetZOrder(-1);
    }
  }

  UBorder* Background =
      Cast<UBorder>(WidgetTree->FindWidget(TEXT("InventoryBackground")));
  if (!Background) {
    Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(),
                                                      TEXT("InventoryBackground"));
    Background->SetBrushColor(FLinearColor(0.04f, 0.05f, 0.07f, 0.78f));

    if (UCanvasPanelSlot* BackgroundSlot = RootCanvas->AddChildToCanvas(Background)) {
      BackgroundSlot->SetAutoSize(false);
      BackgroundSlot->SetPosition(FVector2D(40.0f, 40.0f));
      BackgroundSlot->SetSize(FVector2D(620.0f, 600.0f));
      BackgroundSlot->SetZOrder(-20);
    }
  }

  UTextBlock* TitleText =
      Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("InventoryTitle")));
  if (!TitleText) {
    TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),
                                                        TEXT("InventoryTitle"));
    TitleText->SetText(FText::FromString(TEXT("ENVANTER")));
    TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.96f, 0.97f, 0.98f, 1.0f)));

    if (UCanvasPanelSlot* TitleSlot = RootCanvas->AddChildToCanvas(TitleText)) {
      TitleSlot->SetAutoSize(true);
      TitleSlot->SetPosition(FVector2D(64.0f, 52.0f));
      TitleSlot->SetZOrder(10);
    }
  }

  if (!GridCanvas) {
    GridCanvas =
        Cast<UCanvasPanel>(WidgetTree->FindWidget(TEXT("GridCanvas")));
  }

  if (!GridCanvas) {
    GridCanvas =
        WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(),
                                                  TEXT("GridCanvas"));
    GridCanvas->SetVisibility(ESlateVisibility::Visible);
    GridCanvas->SetRenderOpacity(1.0f);

    if (UCanvasPanelSlot* GridSlot = RootCanvas->AddChildToCanvas(GridCanvas)) {
      GridSlot->SetAutoSize(false);
      GridSlot->SetPosition(FVector2D(60.0f, 90.0f));
      GridSlot->SetSize(FVector2D(500.0f, 500.0f));
      GridSlot->SetZOrder(5);
    }
  }
}
