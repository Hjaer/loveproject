#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "TradeComponent.generated.h"

class UPostApocInventoryComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GERCEK_API UTradeComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UTradeComponent();

  UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable,
            Category = "Trade System")
  void Server_ExecuteTrade(const TArray<FGuid> &PlayerOffer,
                           const TArray<FGuid> &TraderOffer,
                           UPostApocInventoryComponent *PlayerInv,
                           UPostApocInventoryComponent *TraderInv);
};
