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

  // Tüketilebilir etkileşim akışı:
  // ItemType'a göre ya doğrudan kullan (Food/Med), ya da çantaya ekle.
  FItemDBRow ItemData;
  if (GetItemData(ItemData)) {
    // Yiyecek ve İlaç tipindeki eşyalar anında kullanılabilir.
    // Diğer tüketilebilirler (Ammo vb.) envanterе gider.
    if (ItemData.ItemType == EItemType::Food ||
        ItemData.ItemType == EItemType::Med) {
      // Oynacunun consume fonksiyonunu tetikle
      // ItemWeight değerini etki miktarı olarak kullanıyoruz (geçici; ileride
      // FItemDBRow'a HealAmount alanı eklenince değiştir).
      Player->ConsumeItem(ItemData.ItemType, ItemData.ItemWeight);

      // Dünyadan sil — kullanıldı
      Destroy();
    } else {
      // Çantaya at (base sınıfın AddItem + Destroy akışı)
      Super::Interact(Player);
    }
  }
}

void AConsumableItem::UseItem_Implementation(AActor *UserActor) {
  // Tüketme temeli — Blueprint'te override edilerek görsel/ses efektleri
  // eklenebilir. İleride: can doldurma animasyonu, partikül efekti.
}
