#pragma once

#include "CoreMinimal.h"
#include "PostApocInventoryGridWidget.h"
#include "PostApocInventoryFallbackWidget.generated.h"

class UCanvasPanel;

UCLASS()
class GERCEK_API UPostApocInventoryFallbackWidget
    : public UPostApocInventoryGridWidget {
  GENERATED_BODY()

public:
  UPostApocInventoryFallbackWidget(const FObjectInitializer& ObjectInitializer);

protected:
  virtual TSharedRef<SWidget> RebuildWidget() override;
  virtual void NativeConstruct() override;

private:
  void EnsureRuntimeLayout();
};
