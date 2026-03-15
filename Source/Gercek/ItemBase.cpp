#include "ItemBase.h"
#include "Components/StaticMeshComponent.h"
#include "GercekCharacter.h"
// #include "InventoryComponent.h" -- Eski liste sistemi (TradeComponent hâlâ kullanıyor)
#include "PostApocInventoryTypes.h" // Grid tabanlı yeni sistem

// ---------------------------------------------------------------------------
// PostApocItems Data Table — proje eşya tablosu [cite: 2026-02-20].
// Tüm FDataTableRowHandle aramaları bu tablo üzerinden yapılır.
// ---------------------------------------------------------------------------
static const FSoftObjectPath
    PostApocItemsPath(TEXT("/Game/Gercek/Datas/PostApocItems.PostApocItems"));

static FDataTableRowHandle BuildPostApocHandle(FName RowName) {
  FDataTableRowHandle Handle;
  Handle.RowName = RowName;
  Handle.DataTable =
      TSoftObjectPtr<UDataTable>(PostApocItemsPath).LoadSynchronous();
  return Handle;
}

AItemBase::AItemBase() {
  PrimaryActorTick.bCanEverTick = false;

  ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
  RootComponent = ItemMesh;

  ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  ItemMesh->SetCollisionResponseToAllChannels(ECR_Block);
  ItemMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
  ItemMesh->SetGenerateOverlapEvents(true);
  ItemMesh->SetSimulatePhysics(false);
}

void AItemBase::OnConstruction(const FTransform &Transform) {
  Super::OnConstruction(Transform);

  if (ItemRowHandle.RowName.IsNone()) {
    return;
  }

  const FDataTableRowHandle Handle = BuildPostApocHandle(ItemRowHandle.RowName);
  if (Handle.IsNull()) {
    return;
  }

  // Scope-local resolve only; no raw pointer stored (Zero-Pointer Policy).
  const FItemDBRow *Row =
      Handle.GetRow<FItemDBRow>(TEXT("ItemBase::OnConstruction"));
  if (!Row) {
    return;
  }

  if (!Row->PickupMesh.IsNull()) {
    UStaticMesh *LoadedMesh = Row->PickupMesh.LoadSynchronous();
    if (LoadedMesh) {
      ItemMesh->SetStaticMesh(LoadedMesh);
    }
  }

  const bool bIsHeavy = Row->ItemWeight > HeavyWeightThreshold;
  ItemMesh->SetSimulatePhysics(!bIsHeavy);
}

void AItemBase::Interact(AGercekCharacter *Player) {
  if (!IsValid(Player) || ItemRowHandle.RowName.IsNone()) {
    return;
  }

  const FDataTableRowHandle Handle = BuildPostApocHandle(ItemRowHandle.RowName);
  UPostApocInventoryComponent *Inventory =
      Player->FindComponentByClass<UPostApocInventoryComponent>();
  if (IsValid(Inventory) && Inventory->TryAddItem(Handle)) {
    Destroy();
  }
}

void AItemBase::OnInteract_Implementation(AGercekCharacter *Player) {
  Interact(Player);
}

FText AItemBase::GetInteractableName_Implementation() {
  if (ItemRowHandle.RowName.IsNone()) {
    return FText::GetEmpty();
  }
  const FDataTableRowHandle Handle = BuildPostApocHandle(ItemRowHandle.RowName);
  const FItemDBRow *Row =
      Handle.GetRow<FItemDBRow>(TEXT("ItemBase::GetInteractableName"));
  if (!Row || Row->ItemName.IsEmpty()) {
    return FText::GetEmpty();
  }
  return Row->ItemName;
}

FDataTableRowHandle AItemBase::GetItemData_Implementation() {
  return BuildPostApocHandle(ItemRowHandle.RowName);
}
