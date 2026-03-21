#include "TradeComponent.h"

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
    const TArray<FGuid> &PlayerOffer, const TArray<FGuid> &TraderOffer,
    UPostApocInventoryComponent *PlayerInv,
    UPostApocInventoryComponent *TraderInv) {
  if (!PlayerInv || !TraderInv) {
    return false;
  }

  const AGercekCharacter *PlayerCharacter = Cast<AGercekCharacter>(GetOwner());
  const AMerchantBase *MerchantOwner = Cast<AMerchantBase>(TraderInv->GetOwner());

  return PlayerCharacter != nullptr && PlayerInv->GetOwner() == PlayerCharacter &&
         MerchantOwner != nullptr &&
         PlayerCharacter->GetDistanceTo(MerchantOwner) <= 300.0f &&
         PlayerOffer.Num() > 0 && TraderOffer.Num() > 0;
}

void UTradeComponent::Server_ExecuteTrade_Implementation(
    const TArray<FGuid> &PlayerOffer, const TArray<FGuid> &TraderOffer,
    UPostApocInventoryComponent *PlayerInv,
    UPostApocInventoryComponent *TraderInv) {
  if (!GetOwner() || !GetOwner()->HasAuthority() || !PlayerInv || !TraderInv) {
    return;
  }

  AGercekCharacter *PlayerCharacter = Cast<AGercekCharacter>(PlayerInv->GetOwner());
  if (!PlayerCharacter) {
    return;
  }

  TArray<FGridItemInstanceView> PlayerOfferViews;
  TArray<FGridItemInstanceView> TraderOfferViews;
  TSet<FGuid> UniquePlayerItems;
  TSet<FGuid> UniqueTraderItems;

  for (const FGuid& ItemInstanceId : PlayerOffer) {
    if (!ItemInstanceId.IsValid() || UniquePlayerItems.Contains(ItemInstanceId)) {
      continue;
    }

    FGridItemInstanceView ItemView;
    if (!PlayerInv->GetItemInstanceView(ItemInstanceId, ItemView)) {
      PlayerCharacter->ClientHandleTradeResult(
          false, TEXT("Takas basarisiz. Oyuncu teklifi guncel degil."));
      return;
    }

    PlayerOfferViews.Add(ItemView);
    UniquePlayerItems.Add(ItemInstanceId);
  }

  for (const FGuid& ItemInstanceId : TraderOffer) {
    if (!ItemInstanceId.IsValid() || UniqueTraderItems.Contains(ItemInstanceId)) {
      continue;
    }

    FGridItemInstanceView ItemView;
    if (!TraderInv->GetItemInstanceView(ItemInstanceId, ItemView)) {
      PlayerCharacter->ClientHandleTradeResult(
          false, TEXT("Takas basarisiz. Tuccar envanteri degisti."));
      return;
    }

    TraderOfferViews.Add(ItemView);
    UniqueTraderItems.Add(ItemInstanceId);
  }

  const int32 PlayerOfferValue =
      PlayerInv->GetTotalValueForInstances(PlayerOffer);
  int32 TraderOfferValue = 0;
  for (const FGridItemInstanceView& ItemView : TraderOfferViews) {
    TraderOfferValue += PlayerCharacter->GetKnowledgePurchaseValue(
        TraderInv->GetItemValueForInstance(ItemView.ItemInstanceId));
  }
  if (PlayerOfferValue <= 0 || PlayerOfferValue < TraderOfferValue) {
    PlayerCharacter->ClientHandleTradeResult(
        false, TEXT("Takas basarisiz. Teklif degeri yetersiz."));
    return;
  }

  TArray<FGridItemInstanceView> RemovedPlayerItems;
  for (const FGridItemInstanceView& ItemView : PlayerOfferViews) {
    if (!PlayerInv->RemoveItemByInstanceId(ItemView.ItemInstanceId)) {
      for (const FGridItemInstanceView& RemovedItem : RemovedPlayerItems) {
        FGuid RestoredInstanceId = RemovedItem.ItemInstanceId;
        PlayerInv->TryAddItem(RemovedItem.ItemHandle, RestoredInstanceId,
                              RemovedItem.Condition, RemovedItem.FillState);
      }

      PlayerCharacter->ClientHandleTradeResult(
          false, TEXT("Takas basarisiz. Oyuncu teklifi guncel degil."));
      return;
    }

    RemovedPlayerItems.Add(ItemView);
  }

  TArray<FGridItemInstanceView> AddedToTraderItems;
  for (const FGridItemInstanceView& ItemView : PlayerOfferViews) {
    FGuid AddedInstanceId = ItemView.ItemInstanceId;
    if (!TraderInv->TryAddItem(ItemView.ItemHandle, AddedInstanceId,
                               ItemView.Condition, ItemView.FillState)) {
      for (const FGridItemInstanceView& AddedItem : AddedToTraderItems) {
        TraderInv->RemoveItemByInstanceId(AddedItem.ItemInstanceId);
      }
      for (const FGridItemInstanceView& RemovedItem : RemovedPlayerItems) {
        FGuid RestoredInstanceId = RemovedItem.ItemInstanceId;
        PlayerInv->TryAddItem(RemovedItem.ItemHandle, RestoredInstanceId,
                              RemovedItem.Condition, RemovedItem.FillState);
      }

      PlayerCharacter->ClientHandleTradeResult(
          false, TEXT("Takas basarisiz. Tuccar yeni esyalari kabul edemedi."));
      return;
    }

    AddedToTraderItems.Add(ItemView);
  }

  for (const FGridItemInstanceView& ItemView : TraderOfferViews) {
    if (!TraderInv->RemoveItemByInstanceId(ItemView.ItemInstanceId)) {
      for (const FGridItemInstanceView& AddedItem : AddedToTraderItems) {
        TraderInv->RemoveItemByInstanceId(AddedItem.ItemInstanceId);
      }
      for (const FGridItemInstanceView& RemovedItem : RemovedPlayerItems) {
        FGuid RestoredInstanceId = RemovedItem.ItemInstanceId;
        PlayerInv->TryAddItem(RemovedItem.ItemHandle, RestoredInstanceId,
                              RemovedItem.Condition, RemovedItem.FillState);
      }

      PlayerCharacter->ClientHandleTradeResult(
          false, TEXT("Takas basarisiz. Tuccar envanteri degisti."));
      return;
    }

    FGuid AddedInstanceId = ItemView.ItemInstanceId;
    const bool bAddedToPlayer =
        PlayerInv->TryAddItem(ItemView.ItemHandle, AddedInstanceId,
                              ItemView.Condition, ItemView.FillState);
    if (!bAddedToPlayer) {
      const FVector SpawnLocation =
          PlayerCharacter->GetActorLocation() +
          (PlayerCharacter->GetActorForwardVector() * 100.0f);
      PlayerCharacter->SpawnItemInWorld(ItemView.ItemHandle, SpawnLocation,
                                        ItemView.Condition, ItemView.FillState);
    }
  }

  PlayerCharacter->AddTradeXP(static_cast<float>(TraderOfferValue));
  PlayerCharacter->ClientHandleTradeResult(true,
                                           TEXT("Takas basariyla tamamlandi."));
}
