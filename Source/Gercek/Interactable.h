// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// clang-format off
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"
// clang-format on

class AGercekCharacter;

UINTERFACE(MinimalAPI, BlueprintType)
class UInteractable : public UInterface {
  GENERATED_BODY()
};

/**
 * IInteractable
 *
 * Zero-Pointer Policy: GetItemData() parametresiz FDataTableRowHandle döndürür.
 * BlueprintNativeEvent → C++ override: GetItemData_Implementation().
 * 24x24 World Partition için ham pointer taşınmaz.
 */
class GERCEK_API IInteractable {
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
  void OnInteract(class AGercekCharacter *Player);

  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
  FText GetInteractionPrompt(class AGercekCharacter *Player);

  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
  FText GetInteractableName();

  UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
  FDataTableRowHandle GetItemData();
};
