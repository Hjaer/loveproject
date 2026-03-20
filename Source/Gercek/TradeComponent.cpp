#include "TradeComponent.h"
#include "Engine/World.h"
#include "GercekCharacter.h"
#include "MerchantBase.h"
#include "PostApocInventoryTypes.h"
#include "PostApocItemTypes.h"
#include "WorldItemActor.h"

UTradeComponent::UTradeComponent() {
  PrimaryComponentTick.bCanEverTick = false;
  SetIsReplicatedByDefault(true);
}

bool UTradeComponent::Server_ExecuteTrade_Validate(
    const TArray<FDataTableRowHandle> &PlayerOffer,
    const TArray<FDataTableRowHandle> &TraderOffer,
    UPostApocInventoryComponent *PlayerInv,
    UPostApocInventoryComponent *TraderInv) {
  if (!PlayerInv || !TraderInv) {
    return false;
  }

  const AGercekCharacter *PlayerCharacter =
      Cast<AGercekCharacter>(GetOwner());
  const AMerchantBase *MerchantOwner =
      Cast<AMerchantBase>(TraderInv->GetOwner());

  return PlayerCharacter != nullptr && PlayerInv->GetOwner() == PlayerCharacter &&
         MerchantOwner != nullptr &&
         PlayerCharacter->GetDistanceTo(MerchantOwner) <= 300.0f &&
         PlayerOffer.Num() > 0 &&
         TraderOffer.Num() > 0;
}

void UTradeComponent::Server_ExecuteTrade_Implementation(
    const TArray<FDataTableRowHandle> &PlayerOffer,
    const TArray<FDataTableRowHandle> &TraderOffer,
    UPostApocInventoryComponent *PlayerInv,
    UPostApocInventoryComponent *TraderInv) {
  if (!GetOwner() || !GetOwner()->HasAuthority() || !PlayerInv || !TraderInv) {
    return;
  }

  float TotalTradeValue = 0.0f;
  AGercekCharacter *PlayerCharacter =
      Cast<AGercekCharacter>(PlayerInv->GetOwner());

  if (!PlayerCharacter) {
    return;
  }

  for (const FDataTableRowHandle &OfferItem : PlayerOffer) {
    if (OfferItem.IsNull()) {
      continue;
    }

    if (PlayerInv->RemoveItemFromGrid(OfferItem.RowName)) {
      TraderInv->TryAddItem(OfferItem);
    }
  }

  for (const FDataTableRowHandle &TraderItemHandle : TraderOffer) {
    if (TraderItemHandle.IsNull()) {
      continue;
    }

    const FPostApocItemRow *ItemRow =
        TraderItemHandle.GetRow<FPostApocItemRow>(TEXT("TradeExecuteContext"));
    if (ItemRow) {
      TotalTradeValue += ItemRow->BaseValue;
    }

    if (TraderInv->RemoveItemFromGrid(TraderItemHandle.RowName)) {
      const bool bAddedToPlayer = PlayerInv->TryAddItem(TraderItemHandle);
      if (!bAddedToPlayer) {
        const FVector SpawnLocation =
            PlayerCharacter->GetActorLocation() +
            (PlayerCharacter->GetActorForwardVector() * 100.0f);
        PlayerCharacter->SpawnItemInWorld(TraderItemHandle, SpawnLocation);
      }
    }
  }

  if (TotalTradeValue > 0.0f) {
    PlayerCharacter->AddTradeXP(TotalTradeValue);
  }
}
