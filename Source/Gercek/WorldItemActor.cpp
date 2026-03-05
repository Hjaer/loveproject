// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldItemActor.h"
#include "Components/StaticMeshComponent.h"
#include "GercekCharacter.h"
#include "InventoryComponent.h"

// Sets default values
AWorldItemActor::AWorldItemActor() {
  PrimaryActorTick.bCanEverTick = false;

  MeshComponent =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
  RootComponent = MeshComponent;

  // Collision: ECC_Visibility kanalını açıkça block et —
  // CheckForInteractables içindeki LineTraceSingleByChannel(ECC_Visibility)
  // bu ayar sayesinde nesneyi bulabilir.
  MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
  MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
  MeshComponent->SetGenerateOverlapEvents(true);
  MeshComponent->SetSimulatePhysics(true);
}

void AWorldItemActor::BeginPlay() { Super::BeginPlay(); }

// ---------------------------------------------------------------------------
// Helper: resolve the DataTable row for this actor.
// ---------------------------------------------------------------------------
static const FItemDBRow *ResolveRow(const FDataTableRowHandle &Handle) {
  if (Handle.IsNull())
    return nullptr;
  return Handle.GetRow<FItemDBRow>(TEXT("WorldItemActor::ResolveRow"));
}

// ---------------------------------------------------------------------------
// IInteractable — Player pressed E on this actor.
// ---------------------------------------------------------------------------
void AWorldItemActor::OnInteract_Implementation(AGercekCharacter *Player) {
  // Guard: row not configured.
  const FItemDBRow *Row = ResolveRow(ItemRowHandle);
  if (!Row) {
    UE_LOG(LogTemp, Warning,
           TEXT("[WorldItem] '%s' has no valid ItemRowHandle. Cannot pick up."),
           *GetName());
    return;
  }

  // Guard: no player.
  if (!Player) {
    UE_LOG(LogTemp, Error,
           TEXT("[WorldItem] OnInteract called with null Player!"));
    return;
  }

  // Fetch the player's inventory.
  UInventoryComponent *Inventory =
      Player->FindComponentByClass<UInventoryComponent>();
  if (!Inventory) {
    UE_LOG(LogTemp, Warning,
           TEXT("[WorldItem] Player '%s' has no InventoryComponent."),
           *Player->GetName());
    return;
  }

  // DataTable referansını null-safe şekilde al.
  // .Get() null dönebilir; açık kontrolle crash önlüyoruz.
  const UDataTable *DataTablePtr = ItemRowHandle.DataTable.Get();
  if (!DataTablePtr) {
    UE_LOG(LogTemp, Warning,
           TEXT("[WorldItem] '%s' DataTable referansı null. AddItem iptal."),
           *GetName());
    return;
  }

  // Delegate all weight/stack logic to InventoryComponent.
  if (Inventory->AddItem(ItemRowHandle.RowName, DataTablePtr, 1)) {
    // Success — the item is now in the pack. Remove it from the world.
    Destroy();
  } else {
    UE_LOG(LogTemp, Warning,
           TEXT("[WorldItem] Could not pick up '%s'. Pack is full or item is "
                "invalid."),
           *Row->ItemName.ToString());
  }
}

// ---------------------------------------------------------------------------
// IInteractable — Return the display name for the interaction prompt.
// Geçerliyse "[E] ItemName", geçersizse boş FText döner.
// ---------------------------------------------------------------------------
FText AWorldItemActor::GetInteractableName_Implementation() {
  const FItemDBRow *Row = ResolveRow(ItemRowHandle);
  if (!Row || Row->ItemName.IsEmpty()) {
    return FText::GetEmpty();
  }
  // Sadece ham eşya adını döndür.
  // GercekCharacter::CheckForInteractables zaten "[E] Al - " ön ekini ekler;
  // burada tekrar eklersek ekranda çift prefix görünür.
  return Row->ItemName;
}

// ---------------------------------------------------------------------------
// IInteractable — Return the full data row for external systems.
// ---------------------------------------------------------------------------
FItemDBRow AWorldItemActor::GetItemData_Implementation() {
  const FItemDBRow *Row = ResolveRow(ItemRowHandle);
  if (Row) {
    return *Row;
  }
  return FItemDBRow{}; // default/empty row
}
