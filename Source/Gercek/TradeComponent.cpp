// Fill out your copyright notice in the Description page of Project Settings.

#include "TradeComponent.h"
#include "Net/UnrealNetwork.h"
#include "WorldItemActor.h"
#include "GercekCharacter.h"

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
  CurrentMoney = FMath::Max(0.0f, CurrentMoney);
  UE_LOG(LogTemp, Log, TEXT("[TRADE] Para değişti: %f | Yeni Toplam: %f"),
         Amount, CurrentMoney);
}

bool UTradeComponent::Server_AdjustMoney_Validate(float Amount) { return true; }

// ============================================================
//  YENI TICARET SISTEMI — Server_ExecuteTrade
//  1. Oyuncunun teklif ettigi eşyaları (PlayerOfferItems) envanterden siler.
//  2. Tüccarın teklif ettigi eşyaları (TraderOfferItems) ekler.
//  3. Eğer çanta doluysa (TryAddItem = false), eşyayı yere spawn eder.
// ============================================================
void UTradeComponent::Server_ExecuteTrade_Implementation(
    const TArray<FDataTableRowHandle>& PlayerOfferItems,
    const TArray<FDataTableRowHandle>& TraderOfferItems,
    UPostApocInventoryComponent* PlayerInventory) {

  if (!PlayerInventory) {
    UE_LOG(LogTemp, Warning, TEXT("[TRADE] Server_ExecuteTrade: PlayerInventory null."));
    return;
  }

  // 1. OYUNCUNUN VERDIKLERINI SIL (Çantadan çıkar)
  for (const FDataTableRowHandle& OfferItem : PlayerOfferItems) {
    if (OfferItem.IsNull()) continue;
    
    // Grid sisteminden esyayi kaldir
    bool bRemoved = PlayerInventory->RemoveItemFromGrid(OfferItem.RowName);
    if (bRemoved) {
      UE_LOG(LogTemp, Log, TEXT("[TRADE] Teklif edilen '%s' envanterden silindi."), *OfferItem.RowName.ToString());
    } else {
      UE_LOG(LogTemp, Warning, TEXT("[TRADE] Teklif edilen '%s' envanterde bulunamadi!"), *OfferItem.RowName.ToString());
    }
  }

  // Sahibi olan Karakteri bul (Yere eşya atma hesabi icin)
  AGercekCharacter* OwnerCharacter = Cast<AGercekCharacter>(GetOwner());
  FVector SpawnLocation = FVector::ZeroVector;
  if (OwnerCharacter) {
    SpawnLocation = OwnerCharacter->GetActorLocation() + (OwnerCharacter->GetActorForwardVector() * 100.0f);
  } else {
    // Controller vs ise GetOwner()->GetActorLocation()
    SpawnLocation = GetOwner()->GetActorLocation() + (GetOwner()->GetActorForwardVector() * 100.0f);
  }

  // 2. TÜCCARDAN ALINANLARI ÇANTAYA YERLEŞTİR VEYA YERE AT
  for (const FDataTableRowHandle& RequestItem : TraderOfferItems) {
    if (RequestItem.IsNull()) continue;

    // TryAddItem, eşyayı Tetris çantasına yerleştirmeye çalışır
    bool bSuccess = PlayerInventory->TryAddItem(RequestItem);
    
    if (bSuccess) {
      UE_LOG(LogTemp, Display, TEXT("[TRADE] Alinan '%s' basariyla Tetris cantaya eklendi."), *RequestItem.RowName.ToString());
      
      // Çanta yükseltmesi (Backpack) kontrolü
      const FItemDBRow* ItemRowCheck = RequestItem.GetRow<FItemDBRow>(TEXT("TradeComponent::BackpackCheck"));
      if (ItemRowCheck && ItemRowCheck->ItemType == EItemType::Backpack) {
        HandleBackpackUpgrade(RequestItem, PlayerInventory);
      }

    } else {
      UE_LOG(LogTemp, Error, TEXT("[TRADE] Cantada yer yok! '%s' yere atiliyor (Drop)."), *RequestItem.RowName.ToString());
      
      // Yere Atma Islemi (Spawn AWorldItemActor)
      if (GetWorld()) {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AWorldItemActor* SpawnedItem = GetWorld()->SpawnActor<AWorldItemActor>(
            AWorldItemActor::StaticClass(), 
            SpawnLocation, 
            FRotator::ZeroRotator, 
            SpawnParams
        );

        if (SpawnedItem) {
          SpawnedItem->InitializeItemData(RequestItem);
        }
      }
    }
  }
}

bool UTradeComponent::Server_ExecuteTrade_Validate(
    const TArray<FDataTableRowHandle>& PlayerOfferItems,
    const TArray<FDataTableRowHandle>& TraderOfferItems,
    UPostApocInventoryComponent* PlayerInventory) {
  return true;
}

// ============================================================
//  ÇANTA YÜKSELTME — HandleBackpackUpgrade
//  Satın alınan çantanın ExtraCapacity değeri kadar ızgara
//  satır sayısını (GridRows) artırır.
//  Tarkov mantığı: büyük çanta = daha fazla ızgara alanı.
// ============================================================
void UTradeComponent::HandleBackpackUpgrade(
    FDataTableRowHandle BackpackItem,
    UPostApocInventoryComponent *PlayerInventory) {

  if (!PlayerInventory || BackpackItem.IsNull())
    return;

  const FItemDBRow *ItemRow =
      BackpackItem.GetRow<FItemDBRow>(TEXT("TradeComponent::HandleBackpackUpgrade"));

  if (ItemRow && ItemRow->ExtraCapacity > 0) {
    // ExtraCapacity'yi ızgara satır sayısına ekle
    // (Her satır = 1 birim yükseklik = GridColumns adet ekstra slot)
    PlayerInventory->GridRows += ItemRow->ExtraCapacity;

    UE_LOG(LogTemp, Log,
           TEXT("[TRADE] Çanta yükseltmesi yapıldı. Yeni ızgara satır sayısı: %d"),
           PlayerInventory->GridRows);
  }
}
