#pragma once

// clang-format off
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "ItemTypes.h"              // EItemType, EItemRarity, FItemDBRow
#include "PostApocInventoryTypes.h" // UPostApocInventoryComponent
#include "GercekCharacter.generated.h"
// clang-format on

// Etkileşim hedefi değiştiğinde veya kaybolduğunda yıyınlı (broadcast) giden
// delegate. Widget bu eventi dinleyerek metni güncelleyebilir — polling yerine
// event-driven yaklaşım.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractTargetChanged, FText,
                                            NewLabel);

// EItemType ve EItemRarity artik ItemTypes.h'de tanimli.
// ConsumeItem fonksiyonu EItemType::Food, EItemType::Med vb. kullanir.

UCLASS()
class GERCEK_API AGercekCharacter : public ACharacter {
  GENERATED_BODY()

public:
  AGercekCharacter();

protected:
  virtual void BeginPlay() override;

public:
  virtual void Tick(float DeltaTime) override;
  virtual void SetupPlayerInputComponent(
      class UInputComponent *PlayerInputComponent) override;
  virtual void Jump() override;

  // Etkileşim(E tuşu) etiketi değiştiğinde tüm dinleyicilere gönderilir.
  // WBP_Interact bunu dinleyerek anında güncellenebilir; polling gerekmez.
  UPROPERTY(BlueprintAssignable, Category = "Survival|UI")
  FOnInteractTargetChanged OnInteractTargetChanged;

  // Etkileşim fonksiyonu
  void Interact();

  // Bakışla Algılama (Looking At) HUD Yönetimi
  void CheckForInteractables();

  // Eşya Tüketim Fonksiyonları
  UFUNCTION(BlueprintCallable, Category = "Survival | Items")
  void ConsumeItem(EItemType Type, float Amount);

protected:
  void ApplyItemEffect(EItemType Type, float Amount);
  void ResetStaminaRecoveryBuff();

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

  // Etkileşim HUD Widget Referansı
  UPROPERTY(EditDefaultsOnly, Category = "Interaction")
  TSubclassOf<class UUserWidget> InteractWidgetClass;

  UPROPERTY()
  class UUserWidget *InteractWidget;

  // Şu anda bakılan etkileşimli nesnenin ekranda gösterilen adı
  // Widget bunu UPROPERTY yerine GetInteractLabelText() üzerinden okumalı
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
  FString CurrentInteractItemName;

  // Blueprint-safe getter: WBP_Interact bu fonksiyonu çağırarak etiket metnini
  // alır. UHT inline UFUNCTION'ları Blueprint'e expose etmez — implementasyon
  // .cpp'de.
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interaction")
  FText GetInteractLabelText() const;

  // Grid Tabanlı Envanter Bileşeni (Grid Inventory Component)
  // Eski liste-tabanlı UInventoryComponent kaldırıldı; yerine ızgara mantıklı
  // UPostApocInventoryComponent kullanılıyor.
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
  UPostApocInventoryComponent *InventoryComponent;

  AActor *CurrentInteractable;

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