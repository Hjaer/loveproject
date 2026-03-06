#include "ConsumableItem.h"
#include "GercekCharacter.h"
#include "InventoryComponent.h"

AConsumableItem::AConsumableItem() {
  // Tüketilebilir sınıfı için özel başlangıç ayarları
}

void AConsumableItem::Interact(AGercekCharacter *Player) {
  if (!IsValid(Player)) {
    return;
  }

  const FDataTableRowHandle Handle = GetItemData_Implementation();
  if (Handle.IsNull()) {
    return;
  }

  const FItemDBRow *Row =
      Handle.GetRow<FItemDBRow>(TEXT("ConsumableItem::Interact"));
  if (!Row) {
    return;
  }

  if (Row->ItemType == EItemType::Food || Row->ItemType == EItemType::Med) {
    Player->ConsumeItem(Row->ItemType, Row->ItemWeight);
    Destroy();
  } else {
    Super::Interact(Player);
  }
}

void AConsumableItem::UseItem_Implementation(AActor *UserActor) {
  // Tüketme temeli — Blueprint'te override edilerek görsel/ses efektleri
  // eklenebilir. İleride: can doldurma animasyonu, partikül efekti.
}
