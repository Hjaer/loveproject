#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "PostApocHUDWidget.generated.h"

class AGercekCharacter;
class UProgressBar;
class UTextBlock;

UCLASS()
class GERCEK_API UPostApocHUDWidget : public UUserWidget {
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintCallable, Category = "HUD")
  void RefreshCharacterBinding();

protected:
  virtual void NativeConstruct() override;
  virtual void NativeDestruct() override;

  UFUNCTION()
  void HandleHealthChanged(float NewValue, float MaxValue);

  UFUNCTION()
  void HandleStaminaChanged(float NewValue, float MaxValue);

  UFUNCTION()
  void HandleHungerChanged(float NewValue, float MaxValue);

  UFUNCTION()
  void HandleThirstChanged(float NewValue, float MaxValue);

  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
  TObjectPtr<UProgressBar> PB_Health = nullptr;

  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
  TObjectPtr<UProgressBar> PB_Stamina = nullptr;

  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
  TObjectPtr<UProgressBar> PB_Hunger = nullptr;

  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
  TObjectPtr<UProgressBar> PB_Thirst = nullptr;

  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
  TObjectPtr<UTextBlock> TXT_Health = nullptr;

  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
  TObjectPtr<UTextBlock> TXT_Stamina = nullptr;

  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
  TObjectPtr<UTextBlock> TXT_Hunger = nullptr;

  UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
  TObjectPtr<UTextBlock> TXT_Thirst = nullptr;

private:
  void BindToCharacter();
  void UnbindFromCharacter();
  void ApplyStatToWidgets(UProgressBar *ProgressBar, UTextBlock *TextBlock,
                          float NewValue, float MaxValue) const;

  UPROPERTY()
  TObjectPtr<AGercekCharacter> BoundCharacter = nullptr;

  FTimerHandle DelayedBindRetryHandle;
};
