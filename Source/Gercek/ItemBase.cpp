#include "ItemBase.h"
#include "Components/StaticMeshComponent.h"
#include "GercekCharacter.h"
#include "PlayerInventoryComponent.h"
// #include "InventoryComponent.h" -- Eski liste sistemi (TradeComponent hâlâ
// kullanıyor)
#include "PostApocInventoryTypes.h" // Grid tabanlı yeni sistem

// ---------------------------------------------------------------------------
// PostApocItems Data Table — proje eşya tablosu [cite: 2026-02-20].
// Tüm FDataTableRowHandle aramaları bu tablo üzerinden yapılır.
// ---------------------------------------------------------------------------
static const FSoftObjectPath
    PostApocItemsPath(TEXT("/Game/Gercek/Datas/PostApocItems.PostApocItems"));

static UDataTable *ResolvePostApocItemsDataTable() {
  static TWeakObjectPtr<UDataTable> CachedDataTable;
  if (CachedDataTable.IsValid()) {
    return CachedDataTable.Get();
  }

  UDataTable *LoadedDataTable =
      TSoftObjectPtr<UDataTable>(PostApocItemsPath).LoadSynchronous();
  CachedDataTable = LoadedDataTable;
  return LoadedDataTable;
}

static FDataTableRowHandle BuildPostApocHandle(
    const FDataTableRowHandle &SourceHandle) {
  FDataTableRowHandle Handle;
  Handle.RowName = SourceHandle.RowName;
  Handle.DataTable =
      SourceHandle.DataTable ? SourceHandle.DataTable.Get()
                             : ResolvePostApocItemsDataTable();
  return Handle;
}

AItemBase::AItemBase() {
  PrimaryActorTick.bCanEverTick = false;
  bReplicates = true;
  SetReplicateMovement(false);

  ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
  RootComponent = ItemMesh;

  ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  ItemMesh->SetCollisionResponseToAllChannels(ECR_Block);
  ItemMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
  ItemMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
  ItemMesh->SetGenerateOverlapEvents(true);
  ItemMesh->SetSimulatePhysics(false);
}

void AItemBase::OnConstruction(const FTransform &Transform) {
  Super::OnConstruction(Transform);

  if (ItemRowHandle.RowName.IsNone()) {
    return;
  }

  const FDataTableRowHandle Handle = BuildPostApocHandle(ItemRowHandle);
  if (Handle.IsNull()) {
    return;
  }

  // Scope-local resolve only; no raw pointer stored (Zero-Pointer Policy).
  const FPostApocItemRow *Row =
      Handle.GetRow<FPostApocItemRow>(TEXT("ItemBase::OnConstruction"));
  if (!Row) {
    return;
  }

  if (!Row->PickupMesh.IsNull()) {
    UStaticMesh *LoadedMesh = Row->PickupMesh.LoadSynchronous();
    if (LoadedMesh) {
      ItemMesh->SetStaticMesh(LoadedMesh);
    }
  }

  const bool bIsHeavy = Row->Weight > HeavyWeightThreshold;
  ItemMesh->SetSimulatePhysics(!bIsHeavy);
  SetReplicateMovement(!bIsHeavy);
}

void AItemBase::Interact(AGercekCharacter *Player) {
  if (!IsValid(Player) || ItemRowHandle.RowName.IsNone()) {
    return;
  }

  const FDataTableRowHandle Handle = BuildPostApocHandle(ItemRowHandle);
  UPlayerInventoryComponent *Inventory = Player->GetPlayerInventoryComponent();
  FGuid AddedInstanceId;
  if (IsValid(Inventory) &&
      Inventory->TryAddItem(Handle, AddedInstanceId, ItemCondition)) {
    Destroy();
  }
}

void AItemBase::OnInteract_Implementation(AGercekCharacter *Player) {
  Interact(Player);
}

FText AItemBase::GetInteractionPrompt_Implementation(AGercekCharacter *Player) {
  const FText ItemName = GetInteractableName_Implementation();
  if (ItemName.IsEmpty()) {
    return FText::GetEmpty();
  }

  return FText::FromString(
      FString::Printf(TEXT("E - Al %s"), *ItemName.ToString()));
}

FText AItemBase::GetInteractableName_Implementation() {
  if (ItemRowHandle.RowName.IsNone()) {
    return FText::GetEmpty();
  }
  const FDataTableRowHandle Handle = BuildPostApocHandle(ItemRowHandle);
  const FPostApocItemRow *Row =
      Handle.GetRow<FPostApocItemRow>(TEXT("ItemBase::GetInteractableName"));
  if (!Row || Row->DisplayName.IsEmpty()) {
    return FText::GetEmpty();
  }
  return Row->DisplayName;
}

FDataTableRowHandle AItemBase::GetItemData_Implementation() {
  return BuildPostApocHandle(ItemRowHandle);
}

