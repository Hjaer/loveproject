#include "TradeComponent.h"
#include "Engine/World.h"
#include "GercekCharacter.h"
#include "PostApocInventoryTypes.h"
#include "PostApocItemTypes.h"
#include "WorldItemActor.h"

UTradeComponent::UTradeComponent() {
  // Takas mantığı sadece ihtiyaç duyulduğunda çalışacağı için Tick kapalı
  PrimaryComponentTick.bCanEverTick = false;
}

bool UTradeComponent::Server_ExecuteTrade_Validate(
    const TArray<FDataTableRowHandle> &PlayerOffer,
    const TArray<FDataTableRowHandle> &TraderOffer,
    UPostApocInventoryComponent *PlayerInv,
    UPostApocInventoryComponent *TraderInv) {
  // Null işaretçi koruması (Zero-Pointer Policy Guard)
  // Envanter bileşenleri geçerli değilse RPC devreye girmez.
  return (PlayerInv != nullptr && TraderInv != nullptr);
}

void UTradeComponent::Server_ExecuteTrade_Implementation(
    const TArray<FDataTableRowHandle> &PlayerOffer,
    const TArray<FDataTableRowHandle> &TraderOffer,
    UPostApocInventoryComponent *PlayerInv,
    UPostApocInventoryComponent *TraderInv) {
  // Yalnızca sunucu yetkisine (Authority) sahipsek işlemleri yap
  if (!GetOwner()->HasAuthority()) {
    return;
  }

  float TotalTradeValue = 0.0f;
  AGercekCharacter *PlayerCharacter =
      Cast<AGercekCharacter>(PlayerInv->GetOwner());

  // 1. Oyuncunun teklif ettiği eşyaları (PlayerOffer) envanterden sil ve
  // tüccara aktar.
  for (const FDataTableRowHandle &OfferItem : PlayerOffer) {
    if (!OfferItem.IsNull()) {
      // Oyuncunun ızgara envanterinden eşyayı (satır adını baz alarak) kaldır
      bool bRemoved = PlayerInv->RemoveItemFromGrid(OfferItem.RowName);

      if (bRemoved) {
        // Başarılı şekilde çıkarıldıysa tüccarın envanterine eklemeyi dene
        TraderInv->TryAddItem(OfferItem);
      }
    }
  }

  // 2. Tüccarın teklif ettiği eşyaları (TraderOffer) tüccardan çıkar, değere
  // ekle ve oyuncuya ver.
  for (const FDataTableRowHandle &TraderItemHandle : TraderOffer) {
    if (!TraderItemHandle.IsNull()) {
      // FPostApocItemRow DataTable yapısından baz değeri (BaseValue) alıyoruz
      FPostApocItemRow *ItemRow = TraderItemHandle.GetRow<FPostApocItemRow>(
          TEXT("TradeExecuteContext"));
      if (ItemRow) {
        TotalTradeValue += ItemRow->BaseValue;
      }

      // Tüccarın ızgarasından eşyayı kaldır
      bool bTraderRemoved =
          TraderInv->RemoveItemFromGrid(TraderItemHandle.RowName);

      if (bTraderRemoved) {
        // Tüccardan başarıyla silindiyse oyuncunun envanterine eklemeyi dene
        bool bAddedToPlayer = PlayerInv->TryAddItem(TraderItemHandle);

        // Eğer oyuncu envanterinde yer yoksa TryAddItem false döner.
        // Kritik: Bu durumda eşyayı sızdırmayıp (overflow) oyuncunun önüne
        // spawn ediyoruz.
        if (!bAddedToPlayer && PlayerCharacter) {
          FVector SpawnLocation =
              PlayerCharacter->GetActorLocation() +
              (PlayerCharacter->GetActorForwardVector() * 100.0f);

          // Halihazırda var olan spawn metodunu kullanarak dünyada objeyi
          // (WorldItemActor) oluştur
          PlayerCharacter->SpawnItemInWorld(TraderItemHandle, SpawnLocation);
        }
      }
    }
  }

  // 3. Hesaplanan toplam takas değerini kullanarak oyuncuya (Trade XP) tecrübe
  // puanını yansıt
  if (PlayerCharacter && TotalTradeValue > 0.0f) {
    PlayerCharacter->AddTradeXP(TotalTradeValue);
  }
}
