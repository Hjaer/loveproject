// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "ItemData.h"
#include "PostApocItemTypes.h"

// clang-format off
#include "WorldItemActor.generated.h"
// clang-format on

UCLASS()
class GERCEK_API AWorldItemActor : public AActor, public IInteractable {
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  AWorldItemActor();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

public:
  // --- IInteractable interface ---
  virtual void
  OnInteract_Implementation(class AGercekCharacter *Player) override;
  virtual FText GetInteractionPrompt_Implementation(class AGercekCharacter *Player) override;
  virtual FText GetInteractableName_Implementation() override;
  virtual FDataTableRowHandle GetItemData_Implementation() override;

  // Çantadan yere atıldığında (Spawning) Mesh ve Fiziklerin derhal kurulması için çalışır.
  UFUNCTION(BlueprintCallable, Category = "Survival Item")
  void InitializeItemData(
      const FDataTableRowHandle& InHandle, int32 InCondition = 100,
      EConsumableFillState InFillState = EConsumableFillState::NotApplicable);

  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Persistence")
  bool IsPersistentWorldDrop() const { return bPersistentWorldDrop; }

  UFUNCTION(BlueprintCallable, Category = "Persistence")
  void SetPersistentWorldDrop(bool bInPersistentWorldDrop) {
    bPersistentWorldDrop = bInPersistentWorldDrop;
  }

  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Persistence")
  int32 GetItemCondition() const { return ItemCondition; }

  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Persistence")
  EConsumableFillState GetItemFillState() const { return ItemFillState; }

protected:
  // Physical mesh in the world
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  class UStaticMeshComponent *MeshComponent;

  // Which row in the shared DataTable defines this item?
  // Editörden hem tabloyu hem satırı seçebilirsiniz.
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival Item")
  FDataTableRowHandle ItemRowHandle;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival Item",
            meta = (ClampMin = "10", ClampMax = "100"))
  int32 ItemCondition = 100;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival Item")
  EConsumableFillState ItemFillState = EConsumableFillState::NotApplicable;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Persistence")
  bool bPersistentWorldDrop = false;
};
