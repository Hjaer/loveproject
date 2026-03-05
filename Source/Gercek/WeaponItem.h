#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"

// clang-format off
#include "WeaponItem.generated.h"
// clang-format on

/**
 * AWeaponItem (Silahlar)
 * ItemBase'den miras alır. Şekil değiştirme ve fizik özelliklerini korur.
 * Oyuncu etkileşime girdiğinde silahı doğrudan "Kuşanma" (Equip) akışına
 * yönlendirir. Ateş etme ve şarjör fonksiyonları Blueprint'te
 * zenginleştirilebilir.
 */
UCLASS()
class GERCEK_API AWeaponItem : public AItemBase {
  GENERATED_BODY()

public:
  AWeaponItem();

  // Oyuncu E'ye bastığında çağrılır — silahı kuşandırır, sonra Destroy
  virtual void Interact(AGercekCharacter *Player) override;

  // Silahı ateşleme — BlueprintNativeEvent: Blueprint üzerinden da override
  // edilebilir
  UFUNCTION(BlueprintCallable, BlueprintNativeEvent,
            Category = "Weapon | Actions")
  void Fire();

  // Şarjör değiştirme
  UFUNCTION(BlueprintCallable, BlueprintNativeEvent,
            Category = "Weapon | Actions")
  void Reload();
};
