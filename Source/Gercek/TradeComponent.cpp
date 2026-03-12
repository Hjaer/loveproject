// Fill out your copyright notice in the Description page of Project Settings.

#include "TradeComponent.h"
#include "Net/UnrealNetwork.h"

UTradeComponent::UTradeComponent() {
  PrimaryComponentTick.bCanEverTick = false;
  SetIsReplicatedByDefault(true);
  CurrentMoney = 0.0f; // Başlangıç parası
}

void UTradeComponent::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(UTradeComponent, CurrentMoney);
}

void UTradeComponent::Server_AdjustMoney_Implementation(float Amount) {
  CurrentMoney += Amount;
  // Negatif değerlere düşme kontrolü eklenebilir
  CurrentMoney = FMath::Max(0.0f, CurrentMoney);
  UE_LOG(LogTemp, Log, TEXT("[TRADE] Money adjusted by %f. New Total: %f"),
         Amount, CurrentMoney);
}

bool UTradeComponent::Server_AdjustMoney_Validate(float Amount) { return true; }

// EŞYA SATMA MANTIĞI
void UTradeComponent::Server_SellItem_Implementation(
    FDataTableRowHandle ItemToSell, UInventoryComponent *PlayerInventory) {

  if (!PlayerInventory || ItemToSell.IsNull())
    return;

  // Eşya verisine bak, eğer Quest (Görev) eşyası ise satışı reddet
  const FItemDBRow *ItemRowCheck =
      ItemToSell.GetRow<FItemDBRow>(TEXT("TradeComponent::Server_SellItem_Check"));
  if (ItemRowCheck && (ItemRowCheck->ItemType == EItemType::QuestItem || ItemRowCheck->ItemType == EItemType::Quest)) {
      UE_LOG(LogTemp, Warning, TEXT("Görev eşyası satılamaz."));
      return;
  }

  // 1. ADIM: Eşyayı oyuncudan al (Envanterden sil)
  bool bRemoved = PlayerInventory->RemoveItem(ItemToSell, 1);

  // 2. ADIM: Eğer eşya başarıyla silindiyse parasını öde
  if (bRemoved) {
    const FItemDBRow *ItemRow =
        ItemToSell.GetRow<FItemDBRow>(TEXT("TradeComponent::Server_SellItem"));
    if (ItemRow) {
      CurrentMoney += ItemRow->ItemValue;
    }
  }
}

bool UTradeComponent::Server_SellItem_Validate(
    FDataTableRowHandle ItemToSell, UInventoryComponent *PlayerInventory) {
  return true;
}

// EŞYA SATIN ALMA MANTIĞI
void UTradeComponent::Server_BuyItem_Implementation(
    FDataTableRowHandle ItemToBuy, UInventoryComponent *PlayerInventory) {

  if (!PlayerInventory || ItemToBuy.IsNull())
    return;

  const FItemDBRow *ItemRow =
      ItemToBuy.GetRow<FItemDBRow>(TEXT("TradeComponent::Server_BuyItem"));
  if (!ItemRow)
    return;

  // 1. KONTROL: Para yetiyor mu?
  if (CurrentMoney < ItemRow->ItemValue) {
    // Burada Blueprint'e "Yetersiz Para" mesajı gönderebilirsin.
    return;
  }

  // 2. KONTROL: Çantada yer/ağırlık var mı?
  // Not: Envanterindeki 'GetTotalWeight' ve 'GetMaxWeight' fonksiyonlarını
  // kullanıyoruz.
  float NewWeight = PlayerInventory->GetTotalWeight() + ItemRow->ItemWeight;
  if (NewWeight > PlayerInventory->GetMaxWeight())
    return;

  // 3. İŞLEM: Parayı düş ve eşyayı ver
  CurrentMoney -= ItemRow->ItemValue;
  PlayerInventory->AddItem(ItemToBuy, 1);

  // 4. ÖZEL DURUM: Eğer bu bir çanta ise kapasiteyi kalıcı olarak artır
  if (ItemRow->ItemType == EItemType::Backpack) {
    HandleBackpackUpgrade(ItemToBuy, PlayerInventory);
  }
}

bool UTradeComponent::Server_BuyItem_Validate(
    FDataTableRowHandle ItemToBuy, UInventoryComponent *PlayerInventory) {
  return true;
}

// ÇANTA YÜKSELTME SİSTEMİ
void UTradeComponent::HandleBackpackUpgrade(
    FDataTableRowHandle BackpackItem, UInventoryComponent *PlayerInventory) {

  if (PlayerInventory && !BackpackItem.IsNull()) {
    const FItemDBRow *ItemRow = BackpackItem.GetRow<FItemDBRow>(
        TEXT("TradeComponent::HandleBackpackUpgrade"));
    if (ItemRow) {
      // Eşya verisindeki 'ExtraCapacity' değerini envanterin limitine
      // ekliyoruz.
      PlayerInventory->SetMaxWeight(PlayerInventory->GetMaxWeight() +
                                    ItemRow->ExtraCapacity);
    }
  }
}
