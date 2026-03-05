// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// clang-format off
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ItemData.h"
#include "Interactable.generated.h"
// clang-format on

class AGercekCharacter;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractable : public UInterface {
  GENERATED_BODY()
};

/**
 *
 */
class GERCEK_API IInteractable {
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
  void OnInteract(class AGercekCharacter *Player);

  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
  FText GetInteractableName();

  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
  FItemDBRow GetItemData();
};
