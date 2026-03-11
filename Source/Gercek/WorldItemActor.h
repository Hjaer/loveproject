// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "ItemData.h"

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
  virtual FText GetInteractableName_Implementation() override;
  virtual FDataTableRowHandle GetItemData_Implementation() override;

  // Çantadan yere atıldığında (Spawning) Mesh ve Fiziklerin derhal kurulması için çalışır.
  UFUNCTION(BlueprintCallable, Category = "Survival Item")
  void InitializeItemData(const FDataTableRowHandle& InHandle);

protected:
  // Physical mesh in the world
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  class UStaticMeshComponent *MeshComponent;

  // Which row in the shared DataTable defines this item?
  // Editörden hem tabloyu hem satırı seçebilirsiniz.
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival Item")
  FDataTableRowHandle ItemRowHandle;
};
