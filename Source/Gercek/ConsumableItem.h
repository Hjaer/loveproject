#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"

// clang-format off
#include "ConsumableItem.generated.h"
// clang-format on

/**
 * AConsumableItem (Tüketilebilirler)
 * ItemBase'den miras alır. İlaç, yemek ve benzeri nesnelerdir.
 * Oyuncu etkileşime girdiğinde "Kullan" veya "Çantaya At" akışını tetikler.
 */
UCLASS()
class GERCEK_API AConsumableItem : public AItemBase {
  GENERATED_BODY()

public:
  AConsumableItem();

  // Oyuncu E'ye bastığında çağrılır — tüketilebiliri kullanır veya çantaya atar
  virtual void Interact(AGercekCharacter *Player) override;

  // Eşyayı kullanma (yemek yeme, ilaç içme) — Blueprint'te de override
  // edilebilir
  UFUNCTION(BlueprintCallable, BlueprintNativeEvent,
            Category = "Consumable | Actions")
  void UseItem(AActor *UserActor);
};
