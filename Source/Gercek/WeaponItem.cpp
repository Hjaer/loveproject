#include "WeaponItem.h"
#include "GercekCharacter.h"
// InventoryComponent.h kaldırıldı — WeaponItem artık Super::Interact (ItemBase)
// üzerinden UPostApocInventoryComponent->TryAddItem kullanıyor.

AWeaponItem::AWeaponItem() {
  // Silah sınıfı için özel başlangıç ayarları
}

void AWeaponItem::Interact(AGercekCharacter *Player) {
  if (!IsValid(Player)) {
    return;
  }

  // Silaha özgü etkileşim: Eşyayı envanterе ekle ve "Equip" akışını tetikle.
  // Önce base sınıfın AddItem + Destroy mantığını çalıştır.
  Super::Interact(Player);

  // NOT: Destroy() base sınıfta çağrıldığı için bu noktadan sonra
  // bu Actor geçersiz olacak. Equip mantığını Super::Interact ÖNCE ekle.
  // Aşağıdaki log, Destroy öncesinde Super çağrısı tamamlanmadan ulaşılmaz
  // ama loglama için buraya bir mesaj eklenebilir (örn.
  // GEngine->AddOnScreenDebugMessage).
}

void AWeaponItem::Fire_Implementation() {
  // Silah ateş etme temeli
  // İleride: mermi spawn, line trace, hasar hesabı eklenebilir.
}

void AWeaponItem::Reload_Implementation() {
  // Şarjör değiştirme temeli
  // İleride: animasyon oynatma, mühimmat hesaplama eklenebilir.
}
