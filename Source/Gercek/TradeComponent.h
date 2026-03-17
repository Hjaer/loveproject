// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// --- Standard Unreal includes first ---
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "PostApocInventoryTypes.h"
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
   * YENI TICARET SISTEMI (Server_ExecuteTrade):
   * 1. Oyuncunun teklif ettigi esyalari (PlayerOfferItems) envanterden siler.
   * 2. Tuccarin teklif ettigi esyalari (TraderOfferItems) oyuncu envanterine
   * eklemeye calisir. Eger yer yoksa (TryAddItem false donerse) esyayi oyuncunun
   * onunde (dunyada) Spawn eder.
   */
  UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Ticaret")
  void Server_ExecuteTrade(const TArray<FDataTableRowHandle>& PlayerOfferItems,
                           const TArray<FDataTableRowHandle>& TraderOfferItems,
                           class UPostApocInventoryComponent* PlayerInventory);

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
                             class UPostApocInventoryComponent *PlayerInventory);
};
