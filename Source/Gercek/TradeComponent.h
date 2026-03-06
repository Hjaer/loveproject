// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// --- Standard Unreal includes first ---
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "InventoryComponent.h"
#include "ItemTypes.h"
#include "Net/UnrealNetwork.h"

// --- GENERATED HEADER: MUST BE THE LAST INCLUDE. NON-NEGOTIABLE. ---
// clang-format off
#include "TradeComponent.generated.h"
// clang-format on

/**
 * TradeComponent: Hem oyuncu hem de tüccar karakterlerinde bulunabilen ticaret
 * bileşeni. Bu bileşen; eşya alış-satış, para yönetimi ve çanta yükseltme
 * mantığını yürütür. Zero-Pointer kuralına uygun olarak tüm eşya verileri
 * FDataTableRowHandle üzerinden taşınır.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GERCEK_API UTradeComponent : public UActorComponent {
  GENERATED_BODY()

public:
  UTradeComponent();

  // Çok oyunculu (Co-Op) için değişkenlerin eşitlenmesini sağlayan fonksiyon
  virtual void GetLifetimeReplicatedProps(
      TArray<FLifetimeProperty> &OutLifetimeProps) const override;

  // Oyuncunun mevcut para miktarı. Sunucu tarafında yönetilir ve istemcilere
  // eşitlenir.
  UPROPERTY(Replicated, BlueprintReadWrite, Category = "Ekonomi")
  float CurrentMoney;

  /**
   * EŞYA SATIŞI (Server_SellItem):
   * Oyuncunun envanterinden bir eşyayı siler ve 'ItemValue' değerini parasına
   * ekler.
   * @param ItemToSell: Satılacak eşyanın veri tablosu satır verisi (Handle).
   * @param PlayerInventory: İşlemin yapılacağı envanter bileşeni.
   */
  UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Ticaret")
  void Server_SellItem(FDataTableRowHandle ItemToSell,
                       class UInventoryComponent *PlayerInventory);

  /**
   * EŞYA ALIMI (Server_BuyItem):
   * Parayı düşer, ağırlık kontrolü yapar ve uygunsa eşyayı envantere ekler.
   * @param ItemToBuy: Alınacak eşyanın veri tablosu satır verisi (Handle).
   * @param PlayerInventory: Eşyanın ekleneceği envanter bileşeni.
   */
  UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Ticaret")
  void Server_BuyItem(FDataTableRowHandle ItemToBuy,
                      class UInventoryComponent *PlayerInventory);

  /**
   * PARA EKLEME/ÇIKARMA:
   * Oyun içi ödüller veya harcamalar için kullanılabilir.
   */
  UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Ekonomi")
  void Server_AdjustMoney(float Amount);

protected:
  /**
   * ÇANTA YÜKSELTMESİ:
   * Eğer alınan eşya 'Backpack' türündeyse kapasiteyi artırır.
   * (İsim bazlı veya EItemType üzerinden kontrol sağlanabilir)
   */
  void HandleBackpackUpgrade(FDataTableRowHandle BackpackItem,
                             class UInventoryComponent *PlayerInventory);
};
