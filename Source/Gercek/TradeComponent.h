#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TradeComponent.generated.h"

// Ileriye dönük tanımlamalar (Forward declarations)
class UPostApocInventoryComponent;
class AMerchantBase;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GERCEK_API UTradeComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UTradeComponent();

public:
  /**
   * Oyuncu ve tüccar arasında takas işlemini sunucu üzerinde güvenli bir
   * şekilde gerçekleştirir.
   *
   * @param PlayerOffer Oyuncunun takas için teklif ettiği eşya satır
   * referansları
   * @param TraderOffer Tüccarın takas için teklif ettiği eşya satır
   * referansları
   * @param PlayerInv Oyuncunun ızgara envanter bileşeni
   * @param TraderInv Tüccarın ızgara envanter bileşeni
   */
  UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable,
            Category = "Trade System")
  void Server_ExecuteTrade(const TArray<FDataTableRowHandle> &PlayerOffer,
                           const TArray<FDataTableRowHandle> &TraderOffer,
                           UPostApocInventoryComponent *PlayerInv,
                           UPostApocInventoryComponent *TraderInv);
};
