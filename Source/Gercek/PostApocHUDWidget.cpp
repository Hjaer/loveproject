#include "PostApocHUDWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GercekCharacter.h"
#include "Kismet/GameplayStatics.h"

void UPostApocHUDWidget::RefreshCharacterBinding() { BindToCharacter(); }

void UPostApocHUDWidget::NativeConstruct() {
  Super::NativeConstruct();
  BindToCharacter();
}

void UPostApocHUDWidget::NativeDestruct() {
  UnbindFromCharacter();
  Super::NativeDestruct();
}

void UPostApocHUDWidget::BindToCharacter() {
  AGercekCharacter* CandidateCharacter = nullptr;

  if (APlayerController* OwningPlayerController = GetOwningPlayer()) {
    CandidateCharacter =
        Cast<AGercekCharacter>(OwningPlayerController->GetPawn());
  }

  if (!CandidateCharacter) {
    CandidateCharacter = Cast<AGercekCharacter>(GetOwningPlayerPawn());
  }

  if (!CandidateCharacter) {
    AGercekCharacter* FallbackCharacter =
        Cast<AGercekCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    if (FallbackCharacter && FallbackCharacter->IsLocallyControlled()) {
      CandidateCharacter = FallbackCharacter;
    }
  }

  if (BoundCharacter == CandidateCharacter && BoundCharacter) {
    if (UWorld* World = GetWorld()) {
      World->GetTimerManager().ClearTimer(DelayedBindRetryHandle);
    }
    BoundCharacter->BroadcastCurrentSurvivalStats(true);
    return;
  }

  if (BoundCharacter) {
    UnbindFromCharacter();
  }

  BoundCharacter = CandidateCharacter;

  if (!BoundCharacter) {
    if (UWorld *World = GetWorld()) {
      World->GetTimerManager().SetTimer(
          DelayedBindRetryHandle, this, &UPostApocHUDWidget::BindToCharacter,
          0.25f, false);
    }
    return;
  }

  if (UWorld *World = GetWorld()) {
    World->GetTimerManager().ClearTimer(DelayedBindRetryHandle);
  }

  BoundCharacter->OnHealthChanged.RemoveDynamic(
      this, &UPostApocHUDWidget::HandleHealthChanged);
  BoundCharacter->OnStaminaChanged.RemoveDynamic(
      this, &UPostApocHUDWidget::HandleStaminaChanged);
  BoundCharacter->OnHungerChanged.RemoveDynamic(
      this, &UPostApocHUDWidget::HandleHungerChanged);
  BoundCharacter->OnThirstChanged.RemoveDynamic(
      this, &UPostApocHUDWidget::HandleThirstChanged);

  BoundCharacter->OnHealthChanged.AddDynamic(
      this, &UPostApocHUDWidget::HandleHealthChanged);
  BoundCharacter->OnStaminaChanged.AddDynamic(
      this, &UPostApocHUDWidget::HandleStaminaChanged);
  BoundCharacter->OnHungerChanged.AddDynamic(
      this, &UPostApocHUDWidget::HandleHungerChanged);
  BoundCharacter->OnThirstChanged.AddDynamic(
      this, &UPostApocHUDWidget::HandleThirstChanged);

  BoundCharacter->BroadcastCurrentSurvivalStats(true);
}

void UPostApocHUDWidget::UnbindFromCharacter() {
  if (UWorld *World = GetWorld()) {
    World->GetTimerManager().ClearTimer(DelayedBindRetryHandle);
  }

  if (!BoundCharacter) {
    return;
  }

  BoundCharacter->OnHealthChanged.RemoveDynamic(
      this, &UPostApocHUDWidget::HandleHealthChanged);
  BoundCharacter->OnStaminaChanged.RemoveDynamic(
      this, &UPostApocHUDWidget::HandleStaminaChanged);
  BoundCharacter->OnHungerChanged.RemoveDynamic(
      this, &UPostApocHUDWidget::HandleHungerChanged);
  BoundCharacter->OnThirstChanged.RemoveDynamic(
      this, &UPostApocHUDWidget::HandleThirstChanged);

  BoundCharacter = nullptr;
}

void UPostApocHUDWidget::ApplyStatToWidgets(UProgressBar *ProgressBar,
                                            UTextBlock *TextBlock,
                                            float NewValue,
                                            float MaxValue) const {
  const float SafeMaxValue = MaxValue > 0.0f ? MaxValue : 1.0f;
  const float Ratio = FMath::Clamp(NewValue / SafeMaxValue, 0.0f, 1.0f);

  if (ProgressBar) {
    ProgressBar->SetPercent(Ratio);
  }

  if (TextBlock) {
    const int32 RoundedPercent = FMath::RoundToInt(Ratio * 100.0f);
    TextBlock->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), RoundedPercent)));
  }
}

void UPostApocHUDWidget::HandleHealthChanged(float NewValue, float MaxValue) {
  ApplyStatToWidgets(PB_Health, TXT_Health, NewValue, MaxValue);
}

void UPostApocHUDWidget::HandleStaminaChanged(float NewValue, float MaxValue) {
  ApplyStatToWidgets(PB_Stamina, TXT_Stamina, NewValue, MaxValue);
}

void UPostApocHUDWidget::HandleHungerChanged(float NewValue, float MaxValue) {
  ApplyStatToWidgets(PB_Hunger, TXT_Hunger, NewValue, MaxValue);
}

void UPostApocHUDWidget::HandleThirstChanged(float NewValue, float MaxValue) {
  ApplyStatToWidgets(PB_Thirst, TXT_Thirst, NewValue, MaxValue);
}
