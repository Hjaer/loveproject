#include "ConsumableItem.h"
#include "GercekCharacter.h"
// InventoryComponent.h kaldırıldı — ConsumableItem artık Super::Interact (ItemBase)
// üzerinden UPostApocInventoryComponent->TryAddItem kullanıyor.

AConsumableItem::AConsumableItem() {
  // Tüketilebilir sınıfı için özel başlangıç ayarları
}

void AConsumableItem::Interact(AGercekCharacter *Player) {
  // Anında yutulma mantığı devredışı bırakıldı.
  // Oyuncu artık yiyecek/ilaçları önce çantasına (Inventory) toplayacak.
  Super::Interact(Player);
}

void AConsumableItem::UseItem_Implementation(AActor *UserActor) {
  // Tüketme temeli — Blueprint'te override edilerek görsel/ses efektleri
  // eklenebilir. İleride: can doldurma animasyonu, partikül efekti.
}
