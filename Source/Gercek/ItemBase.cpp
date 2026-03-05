#include "ItemBase.h"
#include "Components/StaticMeshComponent.h"
#include "GercekCharacter.h"

// Envantere eşya eklemek için
#include "InventoryComponent.h"

AItemBase::AItemBase() {
  PrimaryActorTick.bCanEverTick = false;

  ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
  RootComponent = ItemMesh;

  // Fizik simülasyonunu başlangıçta kapat; OnConstruction ağırlığa göre
  // ayarlar.
  ItemMesh->SetSimulatePhysics(false);
}

void AItemBase::OnConstruction(const FTransform &Transform) {
  Super::OnConstruction(Transform);

  // Seçilen bir DataTable ve Satır var mı diye kontrol ediyoruz
  if (!ItemRowHandle.DataTable || ItemRowHandle.RowName.IsNone()) {
    return;
  }

  FItemDBRow *RowData =
      ItemRowHandle.GetRow<FItemDBRow>(TEXT("ItemBase_OnConstruction"));
  if (!RowData) {
    return;
  }

  // ---------------------------------------------------------------
  // 1) Bukalemun: DataTable'dan mesh otomatik yüklenir
  // ---------------------------------------------------------------
  if (!RowData->PickupMesh.IsNull()) {
    UStaticMesh *LoadedMesh = RowData->PickupMesh.LoadSynchronous();
    if (LoadedMesh) {
      ItemMesh->SetStaticMesh(LoadedMesh);
    }
  }

  // ---------------------------------------------------------------
  // 2) Fizik entegrasyonu: Ağır eşyalar sabit durur, hafifler yuvarlanır
  // ---------------------------------------------------------------
  // ItemWeight > HeavyWeightThreshold (varsayılan: 5 kg) ise fizik kapalı.
  // Aksi hâlde simülasyon açılır — eşya düşerse döner/kayar.
  const bool bIsHeavy = (RowData->ItemWeight > HeavyWeightThreshold);
  ItemMesh->SetSimulatePhysics(!bIsHeavy);
}

bool AItemBase::GetItemData(FItemDBRow &OutItemData) const {
  if (ItemRowHandle.DataTable && !ItemRowHandle.RowName.IsNone()) {
    FItemDBRow *RowData =
        ItemRowHandle.GetRow<FItemDBRow>(TEXT("ItemBase_GetItemData"));
    if (RowData) {
      OutItemData = *RowData;
      return true;
    }
  }
  return false;
}

void AItemBase::Interact(AGercekCharacter *Player) {
  // Oyuncu geçerliliğini doğrula
  if (!IsValid(Player)) {
    return;
  }

  // DataTable verisini çek
  if (!ItemRowHandle.DataTable || ItemRowHandle.RowName.IsNone()) {
    return;
  }

  FItemDBRow *RowData =
      ItemRowHandle.GetRow<FItemDBRow>(TEXT("ItemBase_Interact"));
  if (!RowData) {
    return;
  }

  // Oyuncunun envanter bileşenini bul — handle-based add (Zero-Pointer Policy).
  UInventoryComponent *Inventory =
      Player->FindComponentByClass<UInventoryComponent>();
  if (IsValid(Inventory)) {
    Inventory->AddItem(ItemRowHandle, 1);
  }

  // Eşyayı dünyadan sil
  Destroy();
}
