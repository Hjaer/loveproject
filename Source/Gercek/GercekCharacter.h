#pragma once

// clang-format off
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "ItemTypes.h"              // EItemType, EItemRarity, FItemDBRow
#include "PostApocItemTypes.h"      // ETradeKnowledge, EPostApocItemCategory vb.
#include "PostApocInventoryTypes.h" // UPostApocInventoryComponent
#include "GercekCharacter.generated.h"
// clang-format on

class AMerchantBase; // Ileriye donuk ticaret aktoru tanimlamasi
class UTradeComponent;
// EItemType ve EItemRarity artik ItemTypes.h'de tanimli.
// ConsumeItem fonksiyonu EItemType::Food, EItemType::Med vb. kullanir.

UCLASS()
class GERCEK_API AGercekCharacter : public ACharacter {
  GENERATED_BODY()

public:
  // Meryem ve Hazar için not:
  // BP tabanlı UI erişimini garanti altına almak adına bu fonksiyon
  // Class'ın en üst public: kısmında yer almalıdır. (Visibility check)
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PostApoc - Knowledge")
  FText GetKnowledgeAdjustedValue(float BaseValue) const;

  AGercekCharacter();

protected:
  virtual void BeginPlay() override;
  virtual void PawnClientRestart() override;
  virtual void
  GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

public:
  virtual void Tick(float DeltaTime) override;
  virtual void SetupPlayerInputComponent(
      class UInputComponent *PlayerInputComponent) override;
  virtual void Jump() override;
  UFUNCTION(Server, Reliable)
  void ServerInteract(AActor *TargetActor);
  // Etkileşim fonksiyonu
  void Interact();

protected:
  // --- YENİ UZMAN (AAA) ETKİLEŞİM SİSTEMİ ---
  // Kamera açısından 300.0f uzunluğunda Line Trace gerçekleştirir
  AActor* PerformInteractionTrace() const;

  // Optimizasyon: Etkileşim kontrolü Timer'ı ve Cache verisi
  FTimerHandle InteractionTimerHandle;
  UPROPERTY()
  AActor* CachedInteractableActor;
  FText CachedInteractionPrompt;
  void PerformInteractionTracePeriodic();

  // Optimizasyon: Kamera Sarsıntısı Timer'ı
  FTimerHandle CameraShakeTimerHandle;
  void PlayCameraShakePeriodic();

public:
  // HUD üzerinden her kare veya periyodik okunarak etkileşim metnini çeker (Zero-Blueprint Policy)
  UFUNCTION(BlueprintPure, Category = "Interaction")
  FText GetInteractionPrompt() const;

  // Eşya Tüketim Fonksiyonları
  UFUNCTION(BlueprintCallable, Category = "Survival | Items")
  void ConsumeItem(EItemType Type, float Amount);

protected:
  void ApplyItemEffect(EItemType Type, float Amount);
  void ResetStaminaRecoveryBuff();

  // Son hasar alınan zamanı takip eden sayış (Health regen lockout için)
  UPROPERTY(Replicated)
  float LastDamageTakenTime;

  // Haşar sistemi ile entegre: Son hasar zamanını günceller
  virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                            class AController* EventInstigator, AActor* DamageCauser) override;

  EItemType PendingConsumeType;
  float PendingConsumeAmount;

  FTimerHandle ConsumeTimerHandle;
  FTimerHandle StaminaBuffTimerHandle;

  // Hayatta Kalma Özellikleri (Survival Attributes)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Survival")
  float Health;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
  float MaxHealth = 100.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
  float Radiation;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
  float MaxRadiation = 100.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Survival")
  float Hunger;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
  float MaxHunger = 100.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Survival")
  float Thirst;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
  float MaxThirst = 100.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Survival")
  float Stamina;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
  float MaxStamina;

  // ==== NESNE / ETKİLEŞİM BİLEŞENLERİ ====
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
  class UAudioComponent *BreathingAudioComponent;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
  class USoundBase *BreathingSound;

  // Hayatta Kalma Oranları (Survival Stats)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
  float StaminaDepletionRate;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
  float StaminaRecoveryRate;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
  float HungerDecreaseRate;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
  float ThirstDecreaseRate;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival Stats")
  float MovementSpeedMultiplier = 1.0f;

  // Kamera Bileşeni
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera,
            meta = (AllowPrivateAccess = "true"))
  class UCameraComponent *FpsCameraComponent;

  // Kamera FOV Ayarları
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
  float DefaultFOV;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
  float SprintFOV;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
  float CrouchFOV;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
  float InjuredFOV;

  // FOV Geçiş Hızı
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
  float FOVInterpSpeed;

  // Kamera Sarsıntısı (Camera Shake)
  UPROPERTY(EditDefaultsOnly, Category = Camera)
  TSubclassOf<class UCameraShakeBase> WalkingCameraShakeClass;

  // HUD Referansı
  UPROPERTY(EditDefaultsOnly, Category = "HUD")
  TSubclassOf<class UUserWidget> PlayerHUDClass;

  // Grid Tabanlı Envanter Bileşeni (Grid Inventory Component)
  // Eski liste-tabanlı UInventoryComponent kaldırıldı; yerine ızgara mantıklı
  // UPostApocInventoryComponent kullanılıyor.
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
  UPostApocInventoryComponent *InventoryComponent;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trade")
  UTradeComponent *TradeComponent;
  // ==== TİCARET VE TECRÜBE SİSTEMİ ====
public:
  
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Trade Knowledge", Replicated)
  float TradeXP;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Trade Knowledge", Replicated)
  ETradeKnowledge CurrentKnowledge;

  UFUNCTION(BlueprintCallable, Category = "Trade Knowledge")
  void AddTradeXP(float Amount);

  // Editörden IA_Interact ve IMC_Default seçebilmek için
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputMappingContext *DefaultMappingContext;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputAction *InteractAction;

  // Envanteri Aç/Kapat eylemi
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputAction *ToggleInventoryAction;

  // Envanter Arayüzü Sınıfı
  UPROPERTY(EditDefaultsOnly, Category = "Inventory UI")
  TSubclassOf<class UUserWidget> InventoryWidgetClass;

  UPROPERTY()
  class UUserWidget *InventoryWidget;

  UFUNCTION(BlueprintCallable, Category = "Inventory UI")
  void ToggleInventory();

  // Envanter içerisindeki tüm eşyaları ekrana ve log'a yazdırır
  UFUNCTION(BlueprintCallable, Category = "Inventory")
  void ShowInventoryDetails();

  UFUNCTION()
  void HandleGridInventoryUpdated();

  // ==== TİCARET EKRANI (TRADE UI) YÖNETİMİ ====
  
  // Blueprint'te seçilecek olan ticaret ekranı arayüzü sınıfı (WBP_TradeScreen_YENI vb.)
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trade UI")
  TSubclassOf<class UUserWidget> TradeScreenClass;

  // Şu anda ekranda aktif olan ticaret arayüzü referansı
  UPROPERTY()
  class UUserWidget* ActiveTradeWidget;

  // Ticaret arayüzünü hedef tüccar verileriyle başlatır
  UFUNCTION(BlueprintCallable, Category = "Trade UI")
  void OpenTradeScreen(class AMerchantBase* TargetMerchant);

  // Ticaret arayüzünü güvenli şekilde kapatır ve oyun içi girdi moduna döner
  UFUNCTION(BlueprintCallable, Category = "Trade UI")
  void CloseTradeScreen();

  // Ticaret işlemini onaylar, değer kontrolünü yapar ve sunucu işlemini başlatır
  UFUNCTION()
  void ExecuteTrade();

  // Şu an etkileşimde olunan tüccar
  UPROPERTY()
  class AMerchantBase* ActiveMerchant;

  // ==== ÇANTADAN ETKİLEŞİM YETENEKLERİ (USE & DROP) ====

  UFUNCTION(BlueprintCallable, Category = "Survival | Inventory Action")
  void UseItemFromInventory(const FDataTableRowHandle &ItemRowHandle);

  UFUNCTION(BlueprintCallable, Category = "Survival | Inventory Action")
  void DropItemFromInventory(const FDataTableRowHandle &ItemRowHandle);

  UFUNCTION(BlueprintCallable, Category = "Survival | Drops")
  AActor *SpawnItemInWorld(const FDataTableRowHandle &ItemRowHandle,
                           FVector SpawnLocation);

  // Blueprint-safe weight ratio getter.
  // WBP_PlayerHUD bu fonksiyonu kullanmalı, WeightPercentage property'sini
  // değil.
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival | UI")
  float GetWeightRatio() const;

  // Delegate imzası FOnWeightChanged(float, float) ile eşleşmeli — TwoParams.
  UFUNCTION()
  void OnInventoryWeightChanged(float NewWeight, float MaxWeight);

  // Çağrıldığı an CurrentWeight > MaxWeight ise hızı %50 düşürür
  // (Overburdened). Delegate bağlantısı BeginPlay'de kurulur; Tick içinde
  // polling yapılmaz.
  void UpdateMovementSpeed(float CurrentWeight, float MaxWeight);

  // Zıplama eylemi
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputAction *JumpAction;

  // Sprint ve Crouch için
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputAction *SprintAction;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputAction *CrouchAction;

  // Hareket ve Kamera için
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputAction *MoveAction;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputAction *LookAction;

  // Hareket fonksiyonları
  void StartSprint();
  void StopSprint();
  void StartCrouch();
  void StopCrouch();

  // Enhanced Input callback metotları
  void Move(const struct FInputActionValue &Value);
  void Look(const struct FInputActionValue &Value);

protected:
  bool bIsSprinting;
  bool bIsFatigued;
  bool bIsExhausted;
  bool bIsConsuming;
  bool bIsRecovering;

  float OriginalStaminaRecoveryRate;
  float RecoveryDelayTimer;

  float LastJumpTime;

  // Hız Değerleri
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Stats")
  float BaseWalkSpeed;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Stats")
  float SprintSpeed;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Stats")
  float InjuredSpeed;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Stats")
  float InjuredSprintSpeed;
};

