#pragma once

// clang-format off
#include "CoreMinimal.h"
#include "GercekHostSaveGame.h"
#include "GameFramework/Character.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "ItemTypes.h"              // EItemType, EItemRarity
#include "PostApocItemTypes.h"      // ETradeKnowledge, EPostApocItemCategory vb.
#include "PostApocInventoryTypes.h" // UPostApocInventoryComponent
#include "GercekCharacter.generated.h"
// clang-format on

class AMerchantBase; // Ileriye donuk ticaret aktoru tanimlamasi
class ALootContainerBase;
class UPostApocHUDWidget;
class UPostApocTradeOfferPanelWidget;
class UTradeComponent;
// EItemType ve EItemRarity artik ItemTypes.h'de tanimli.
// ConsumeItem fonksiyonu EItemType::Food, EItemType::Med vb. kullanir.

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSurvivalStatChanged, float,
                                             NewValue, float, MaxValue);

UCLASS()
class GERCEK_API AGercekCharacter : public ACharacter {
  GENERATED_BODY()

public:
  // Meryem ve Hazar iÃ§in not:
  // BP tabanlÄ± UI eriÅŸimini garanti altÄ±na almak adÄ±na bu fonksiyon
  // Class'Ä±n en Ã¼st public: kÄ±smÄ±nda yer almalÄ±dÄ±r. (Visibility check)
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PostApoc - Knowledge")
  FText GetKnowledgeAdjustedValue(float BaseValue) const;

  AGercekCharacter();

  UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
  FOnSurvivalStatChanged OnHealthChanged;

  UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
  FOnSurvivalStatChanged OnStaminaChanged;

  UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
  FOnSurvivalStatChanged OnHungerChanged;

  UPROPERTY(BlueprintAssignable, Category = "Survival|Events")
  FOnSurvivalStatChanged OnThirstChanged;

  UFUNCTION(BlueprintCallable, Category = "Survival|Events")
  void BroadcastCurrentSurvivalStats(bool bForce = false);

  UFUNCTION(BlueprintPure, Category = "Survival")
  float GetCurrentHealth() const { return Health; }

  UFUNCTION(BlueprintPure, Category = "Survival")
  float GetCurrentStamina() const { return Stamina; }

  UFUNCTION(BlueprintPure, Category = "Survival")
  float GetCurrentHunger() const { return Hunger; }

  UFUNCTION(BlueprintPure, Category = "Survival")
  float GetCurrentThirst() const { return Thirst; }

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
  UFUNCTION(Client, Reliable)
  void ClientOpenTradeScreen(class AMerchantBase *TargetMerchant);
  UFUNCTION(Client, Reliable)
  void ClientOpenLootContainer(class ALootContainerBase *TargetContainer);
  // EtkileÅŸim fonksiyonu
  void Interact();

protected:
  // --- YENÄ° UZMAN (AAA) ETKÄ°LEÅÄ°M SÄ°STEMÄ° ---
  // Kamera aÃ§Ä±sÄ±ndan 300.0f uzunluÄŸunda Line Trace gerÃ§ekleÅŸtirir
  AActor* PerformInteractionTrace() const;

  // Optimizasyon: EtkileÅŸim kontrolÃ¼ Timer'Ä± ve Cache verisi
  FTimerHandle InteractionTimerHandle;
  UPROPERTY()
  AActor* CachedInteractableActor;
  FText CachedInteractionPrompt;
  void PerformInteractionTracePeriodic();

  // Optimizasyon: Kamera SarsÄ±ntÄ±sÄ± Timer'Ä±
  FTimerHandle CameraShakeTimerHandle;
  void PlayCameraShakePeriodic();

public:
  // HUD Ã¼zerinden her kare veya periyodik okunarak etkileÅŸim metnini Ã§eker (Zero-Blueprint Policy)
  UFUNCTION(BlueprintPure, Category = "Interaction")
  FText GetInteractionPrompt() const;

  // EÅŸya TÃ¼ketim FonksiyonlarÄ±
  UFUNCTION(BlueprintCallable, Category = "Survival | Items")
  void ConsumeItem(EItemType Type, float Amount);

protected:
  void ApplyItemEffect(EItemType Type, float Amount);
  float ResolveConsumableAmount(const FPostApocItemRow& ItemRow,
                                EConsumableFillState FillState) const;
  float GetEffectiveMaxStamina() const;
  void ResetStaminaRecoveryBuff();

  // Son hasar alÄ±nan zamanÄ± takip eden sayÄ±ÅŸ (Health regen lockout iÃ§in)
  UPROPERTY(Replicated)
  float LastDamageTakenTime;

  // HaÅŸar sistemi ile entegre: Son hasar zamanÄ±nÄ± gÃ¼nceller
  virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                            class AController* EventInstigator, AActor* DamageCauser) override;

  EItemType PendingConsumeType;
  float PendingConsumeAmount;

  FTimerHandle ConsumeTimerHandle;
  FTimerHandle StaminaBuffTimerHandle;

  // Hayatta Kalma Ã–zellikleri (Survival Attributes)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Survival")
  float Health;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
  float MaxHealth = 100.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Survival")
  float Radiation;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
  float MaxRadiation = 30.0f;

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

  // ==== NESNE / ETKÄ°LEÅÄ°M BÄ°LEÅENLERÄ° ====
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
  class UAudioComponent *BreathingAudioComponent;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
  class USoundBase *BreathingSound;

  // Hayatta Kalma OranlarÄ± (Survival Stats)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
  float StaminaDepletionRate;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
  float StaminaRecoveryRate;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
  float HungerDecreaseRate;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
  float ThirstDecreaseRate;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
  float HungerHealthDecayRate = 0.08f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
  float RadiationHealthDecayRate = 0.6f;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Survival Stats")
  float MovementSpeedMultiplier = 1.0f;

  // Kamera BileÅŸeni
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera,
            meta = (AllowPrivateAccess = "true"))
  class UCameraComponent *FpsCameraComponent;

  // Kamera FOV AyarlarÄ±
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
  float DefaultFOV;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
  float SprintFOV;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
  float CrouchFOV;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
  float InjuredFOV;

  // FOV GeÃ§iÅŸ HÄ±zÄ±
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
  float FOVInterpSpeed;

  // Kamera SarsÄ±ntÄ±sÄ± (Camera Shake)
  UPROPERTY(EditDefaultsOnly, Category = Camera)
  TSubclassOf<class UCameraShakeBase> WalkingCameraShakeClass;

  // HUD ReferansÄ±
  UPROPERTY(EditDefaultsOnly, Category = "HUD")
  TSubclassOf<class UPostApocHUDWidget> PlayerHUDClass;

  UPROPERTY()
  TObjectPtr<UPostApocHUDWidget> PlayerHUDWidget = nullptr;

  // Grid TabanlÄ± Envanter BileÅŸeni (Grid Inventory Component)
  // Eski liste-tabanlÄ± UInventoryComponent kaldÄ±rÄ±ldÄ±; yerine Ä±zgara mantÄ±klÄ±
  // UPostApocInventoryComponent kullanÄ±lÄ±yor.
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
  UPostApocInventoryComponent *InventoryComponent;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trade")
  UTradeComponent *TradeComponent;
  // ==== TÄ°CARET VE TECRÃœBE SÄ°STEMÄ° ====
public:
  
  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Trade Knowledge", Replicated)
  float TradeXP;

  UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Trade Knowledge", Replicated)
  ETradeKnowledge CurrentKnowledge;

  UFUNCTION(BlueprintCallable, Category = "Trade Knowledge")
  void AddTradeXP(float Amount);

  void BuildPersistentPlayerRecord(FGercekSavedPlayerRecord &OutRecord) const;
  void ApplyPersistentPlayerRecord(const FGercekSavedPlayerRecord &InRecord);

  // EditÃ¶rden IA_Interact ve IMC_Default seÃ§ebilmek iÃ§in
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputMappingContext *DefaultMappingContext;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputAction *InteractAction;

  // Envanteri AÃ§/Kapat eylemi
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputAction *ToggleInventoryAction;

  // Envanter ArayÃ¼zÃ¼ SÄ±nÄ±fÄ±
  UPROPERTY(EditDefaultsOnly, Category = "Inventory UI")
  TSubclassOf<class UUserWidget> InventoryWidgetClass;

  UPROPERTY()
  class UUserWidget *InventoryWidget;

  UFUNCTION(BlueprintCallable, Category = "Inventory UI")
  void ToggleInventory();

  // Envanter iÃ§erisindeki tÃ¼m eÅŸyalarÄ± ekrana ve log'a yazdÄ±rÄ±r
  UFUNCTION(BlueprintCallable, Category = "Inventory")
  void ShowInventoryDetails();

  UFUNCTION()
  void HandleGridInventoryUpdated();

  // ==== LOOT CONTAINER EKRANI (CONTAINER UI) YONETIMI ====
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Container UI")
  TSubclassOf<class UUserWidget> LootContainerWidgetClass;

  UPROPERTY()
  class UUserWidget *ActiveLootContainerWidget;

  UPROPERTY()
  class ALootContainerBase *ActiveLootContainer;

  UFUNCTION(BlueprintCallable, Category = "Container UI")
  void OpenLootContainer(class ALootContainerBase *TargetContainer);

  UFUNCTION(BlueprintCallable, Category = "Container UI")
  void CloseLootContainer();

  UFUNCTION(BlueprintCallable, Category = "Container UI")
  void RefreshLootContainerUI();

  UFUNCTION()
  void HandleActiveLootContainerUpdated();

  UFUNCTION(BlueprintCallable, Category = "Container UI")
  void RequestTakeItemFromLootContainer(FGuid ItemInstanceId);

  UFUNCTION(BlueprintCallable, Category = "Container UI")
  void RequestStoreItemInLootContainer(FGuid ItemInstanceId);

  UFUNCTION(Server, Reliable)
  void ServerTakeItemFromLootContainer(class ALootContainerBase *TargetContainer,
                                       FGuid ItemInstanceId);

  UFUNCTION(Server, Reliable)
  void ServerStoreItemInLootContainer(class ALootContainerBase *TargetContainer,
                                      FGuid ItemInstanceId);

  UFUNCTION(Server, Reliable)
  void ServerCloseLootContainer(class ALootContainerBase *TargetContainer);

  // ==== TÄ°CARET EKRANI (TRADE UI) YÃ–NETÄ°MÄ° ====
  
  // Blueprint'te seÃ§ilecek olan ticaret ekranÄ± arayÃ¼zÃ¼ sÄ±nÄ±fÄ± (WBP_TradeScreen_YENI vb.)
  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Trade UI")
  TSubclassOf<class UUserWidget> TradeScreenClass;

  // Åu anda ekranda aktif olan ticaret arayÃ¼zÃ¼ referansÄ±
  UPROPERTY()
  class UUserWidget* ActiveTradeWidget;

  // Ticaret arayÃ¼zÃ¼nÃ¼ hedef tÃ¼ccar verileriyle baÅŸlatÄ±r
  UFUNCTION(BlueprintCallable, Category = "Trade UI")
  void OpenTradeScreen(class AMerchantBase* TargetMerchant);

  // Ticaret arayÃ¼zÃ¼nÃ¼ gÃ¼venli ÅŸekilde kapatÄ±r ve oyun iÃ§i girdi moduna dÃ¶ner
  UFUNCTION(BlueprintCallable, Category = "Trade UI")
  void CloseTradeScreen();

  UFUNCTION(BlueprintCallable, Category = "Trade UI")
  void ToggleTradeOfferItem(EPostApocInventoryGridRole SourceRole,
                            FGuid ItemInstanceId);

  // Ticaret iÅŸlemini onaylar, deÄŸer kontrolÃ¼nÃ¼ yapar ve sunucu iÅŸlemini baÅŸlatÄ±r
  UFUNCTION()
  void ExecuteTrade();

  UFUNCTION(Client, Reliable)
  void ClientHandleTradeResult(bool bSuccess, const FString& Message);

  // Åu an etkileÅŸimde olunan tÃ¼ccar
  UPROPERTY()
  class AMerchantBase* ActiveMerchant;

  UFUNCTION()
  void HandleTradeInventoryUpdated();

  UFUNCTION(BlueprintPure, Category = "Trade Knowledge")
  FText GetKnowledgeAdjustedTradeValueText(int32 ActualValue) const;

  UFUNCTION(BlueprintPure, Category = "Trade Knowledge")
  int32 GetKnowledgePurchaseValue(int32 ActualValue) const;

  // ==== Ã‡ANTADAN ETKÄ°LEÅÄ°M YETENEKLERÄ° (USE & DROP) ====

  UFUNCTION(BlueprintCallable, Category = "Survival | Inventory Action")
  void UseItemFromInventory(const FDataTableRowHandle &ItemRowHandle);

  UFUNCTION(BlueprintCallable, Category = "Survival | Inventory Action")
  void UseItemInstanceFromInventory(FGuid ItemInstanceId);

  UFUNCTION(Server, Reliable)
  void ServerUseItemInstanceFromInventory(FGuid ItemInstanceId);

  UFUNCTION(BlueprintCallable, Category = "Survival | Inventory Action")
  void DropItemFromInventory(const FDataTableRowHandle &ItemRowHandle);

  UFUNCTION(BlueprintCallable, Category = "Survival | Inventory Action")
  void DropItemInstanceFromInventory(FGuid ItemInstanceId);

  UFUNCTION(Server, Reliable)
  void ServerDropItemInstanceFromInventory(FGuid ItemInstanceId);

  UFUNCTION(BlueprintCallable, Category = "Survival | Drops")
  AActor *SpawnItemInWorld(const FDataTableRowHandle &ItemRowHandle,
                           FVector SpawnLocation,
                           int32 ItemCondition = 100,
                           EConsumableFillState FillState =
                               EConsumableFillState::NotApplicable);

  // Blueprint-safe weight ratio getter.
  // WBP_PlayerHUD bu fonksiyonu kullanmalÄ±, WeightPercentage property'sini
  // deÄŸil.
  UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Survival | UI")
  float GetWeightRatio() const;

  // Delegate imzasÄ± FOnWeightChanged(float, float) ile eÅŸleÅŸmeli â€” TwoParams.
  UFUNCTION()
  void OnInventoryWeightChanged(float NewWeight, float MaxWeight);

  // Ã‡aÄŸrÄ±ldÄ±ÄŸÄ± an CurrentWeight > MaxWeight ise hÄ±zÄ± %50 dÃ¼ÅŸÃ¼rÃ¼r
  // (Overburdened). Delegate baÄŸlantÄ±sÄ± BeginPlay'de kurulur; Tick iÃ§inde
  // polling yapÄ±lmaz.
  void UpdateMovementSpeed(float CurrentWeight, float MaxWeight);

  // ZÄ±plama eylemi
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputAction *JumpAction;

  // Sprint ve Crouch iÃ§in
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputAction *SprintAction;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputAction *CrouchAction;

  // Hareket ve Kamera iÃ§in
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputAction *MoveAction;

  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
  class UInputAction *LookAction;

  // Hareket fonksiyonlarÄ±
  void StartSprint();
  void StopSprint();
  void StartCrouch();
  void StopCrouch();

  // Enhanced Input callback metotlarÄ±
  void Move(const struct FInputActionValue &Value);
  void Look(const struct FInputActionValue &Value);

private:
  void EmitSurvivalStatChangeEvents(bool bForce = false);
  bool ShouldBroadcastSurvivalValue(float CurrentValue, float LastBroadcastValue,
                                    bool bForce) const;
  void RefreshTradeKnowledgeTierFromXP();
  void RefreshTradeUI();
  void RefreshTradeOfferPanels();
  void SanitizeTradeOfferSelections();
  TArray<FGuid>* GetTradeOfferArray(EPostApocInventoryGridRole SourceRole);
  const TArray<FGuid>* GetTradeOfferArray(EPostApocInventoryGridRole SourceRole) const;
  bool BuildTradeOfferEntryData(UPostApocInventoryComponent* SourceInventory,
                                FGuid ItemInstanceId,
                                struct FPostApocTradeOfferEntryData& OutEntryData,
                                bool bApplyKnowledgePurchaseDiscount = false) const;
  void EnsureTradeOfferPanels();

  UPROPERTY()
  TArray<FGuid> PlayerTradeOfferItems;

  UPROPERTY()
  TArray<FGuid> MerchantTradeOfferItems;

  UPROPERTY()
  TObjectPtr<UPostApocTradeOfferPanelWidget> PlayerTradeOfferPanel = nullptr;

  UPROPERTY()
  TObjectPtr<UPostApocTradeOfferPanelWidget> MerchantTradeOfferPanel = nullptr;

protected:
  bool bIsSprinting;
  bool bIsFatigued;
  bool bIsExhausted;
  bool bIsConsuming;
  bool bIsRecovering;

  float OriginalStaminaRecoveryRate;
  float RecoveryDelayTimer;

  float LastJumpTime;

  // HÄ±z DeÄŸerleri
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Stats")
  float BaseWalkSpeed;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Stats")
  float SprintSpeed;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Stats")
  float InjuredSpeed;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Stats")
  float InjuredSprintSpeed;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
  float InteractionTraceDistance = 200.0f;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction")
  float InteractionTraceInterval = 0.1f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Effects")
  float StarvationAimSwayStrength = 0.15f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Effects")
  float StarvationAimSwaySpeed = 2.5f;

  float HungerAimSwayTime = 0.0f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Events",
            meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
  float SurvivalBroadcastThreshold = 0.5f;

  float LastBroadcastHealth = -1.0f;
  float LastBroadcastStamina = -1.0f;
  float LastBroadcastHunger = -1.0f;
  float LastBroadcastThirst = -1.0f;
};



