// Fill out your copyright notice in the Description page of Project Settings.

#include "GercekCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "CollisionQueryParams.h"
#include "Components/AudioComponent.h"
#include "Components/Button.h"
#include "Components/InputComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/TextBlock.h"
#include "DrawDebugHelpers.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interactable.h"
#include "Kismet/KismetTextLibrary.h"
#include "MerchantBase.h"
#include "Net/UnrealNetwork.h"
#include "PostApocInventoryTypes.h"
#include "PostApocItemTypes.h"
#include "TimerManager.h"
#include "TradeComponent.h"
#include "WorldItemActor.h"


// Sets default values
AGercekCharacter::AGercekCharacter() {
  // Set this character to call Tick() every frame.  You can turn this off to
  // improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

  // Kamera oluşturma
  FpsCameraComponent =
      CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
  FpsCameraComponent->SetupAttachment(RootComponent);
  FpsCameraComponent->bUsePawnControlRotation = true;

  // FPS Karakter Rotasyon Kilitleri (Kritik)
  bUseControllerRotationPitch = false;
  bUseControllerRotationYaw = true;
  bUseControllerRotationRoll = false;

  // Ses Bileşeni Oluşturma
  BreathingAudioComponent =
      CreateDefaultSubobject<UAudioComponent>(TEXT("BreathingAudioComponent"));
  BreathingAudioComponent->SetupAttachment(RootComponent);
  BreathingAudioComponent->bAutoActivate = false; // Başlangıçta çalmasın

  // Grid (Tetris) Tabanlı Envanter Bileşeni Başlatma
  InventoryComponent = CreateDefaultSubobject<UPostApocInventoryComponent>(
      TEXT("InventoryComponent"));
  if (InventoryComponent) {
    InventoryComponent->GridColumns = 10;
    InventoryComponent->GridRows = 10; // Daha geniş ızgara
    InventoryComponent->TileSize = 50.0f;
  }

  TradeComponent =
      CreateDefaultSubobject<UTradeComponent>(TEXT("TradeComponent"));

  DefaultFOV = 90.0f;
  SprintFOV = 110.0f;
  CrouchFOV = 80.0f;
  InjuredFOV = 70.0f; // Can %25 altındayken daralacak FOV
  FOVInterpSpeed = 10.0f;

  Health = 100.0f;
  Radiation = 0.0f;
  Hunger = 100.0f;
  Thirst = 100.0f;
  MaxStamina = 100.0f;
  Stamina = MaxStamina;

  StaminaDepletionRate = 12.5f;
  StaminaRecoveryRate = 10.0f;
  HungerDecreaseRate = 0.05f;
  ThirstDecreaseRate = 0.07f;

  bIsSprinting = false;
  bIsFatigued = false;
  bIsExhausted = false;
  bIsConsuming = false;
  bIsRecovering = false;

  RecoveryDelayTimer = 0.0f;

  OriginalStaminaRecoveryRate = 10.0f;

  LastJumpTime =
      -10.0f; // Başlangıçta zıplama beklemesini sıfırlamak için eksi bir değer

  // Eğilme (Crouch) özelliğini aktif et
  if (GetCharacterMovement()) {
    GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
    GetCharacterMovement()->MaxWalkSpeed = 300.0f; // Normal yürüme hızı
    GetCharacterMovement()->bOrientRotationToMovement =
        false; // FPS için kapalı olmalı

    // Başlangıç hız limitleri
    BaseWalkSpeed = 300.0f;
    SprintSpeed = 600.0f;
    InjuredSpeed = 150.0f;       // Can azken walk hızı
    InjuredSprintSpeed = 300.0f; // Can azken sprint hızı
  }

  // --- CO-OP ALTYAPISI (EKLEME) ---
  bReplicates = true;
  SetReplicateMovement(true);

  // Ticaret / XP Başlangıç
  TradeXP = 0.0f;
  CurrentKnowledge = ETradeKnowledge::Novice;

  // Son hasar alma zamanını başlangıçta çok küçük bir değere ayarla (hemen
  // regen başlasın)
  LastDamageTakenTime = -999.0f;
}

// --- REPLICATION KAYIT FONKSİYONU (EKLEME) ---
void AGercekCharacter::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(AGercekCharacter, Health);
  DOREPLIFETIME(AGercekCharacter, Stamina);
  DOREPLIFETIME(AGercekCharacter, Hunger);
  DOREPLIFETIME(AGercekCharacter, Thirst);
  DOREPLIFETIME(AGercekCharacter, LastDamageTakenTime);

  // Ticaret XP ve Bilgi Seviyesi
  DOREPLIFETIME(AGercekCharacter, TradeXP);
  DOREPLIFETIME(AGercekCharacter, CurrentKnowledge);
}

// ==== TİCARET VE BİLGİ SEVİYESİ (TRADE KNOWLEDGE) ====

void AGercekCharacter::AddTradeXP(float Amount) {
  // Sadece yetkili sunucuda çalışır
  if (!HasAuthority())
    return;

  TradeXP += Amount;

  // Seviye (Knowledge) güncellemesi
  if (TradeXP >= 5000.0f) {
    CurrentKnowledge = ETradeKnowledge::Expert;
  } else if (TradeXP >= 2000.0f) {
    CurrentKnowledge = ETradeKnowledge::Apprentice;
  } else {
    CurrentKnowledge = ETradeKnowledge::Novice;
  }
}

// Meryem ve Hazar için not:
// Bu fonksiyon "const" olarak işaretlenmiştir, çünkü karakterin state (durum)
// bilgisini değiştirmez; sadece girilen BaseValue argümanına ve oyuncunun
// sahip olduğu Trade Knowledge XP seviyesine bağlı olarak okuma (read-only)
// yapar.
FText AGercekCharacter::GetKnowledgeAdjustedValue(float BaseValue) const {
  switch (CurrentKnowledge) {
  case ETradeKnowledge::Novice:
    // Acemi: Değeri hiç bilemez, soru işareti döneriz.
    return FText::FromString(TEXT("???"));

  case ETradeKnowledge::Apprentice: {
    // Çırak: %33 hata/yanılma payı ile tahmin verebilir.
    int32 MinVal = FMath::RoundToInt(BaseValue * 0.67f);
    int32 MaxVal = FMath::RoundToInt(BaseValue * 1.33f);

    // Tahmin aralığı (Min - Max) örneğin: "~ 75 - 125" formatı.
    return FText::FromString(
        FString::Printf(TEXT("~ %d - %d"), MinVal, MaxVal));
  }

  case ETradeKnowledge::Expert: {
    // Uzman: Tam değeri noktası virgülüne hatasız bilir.
    return FText::FromString(FString::FromInt(FMath::RoundToInt(BaseValue)));
  }

  default:
    // Fallback
    return FText::FromString(TEXT("???"));
  }
}

// Called when the game starts or when spawned
void AGercekCharacter::BeginPlay() {
  Super::BeginPlay();

  if (InventoryComponent) {
    InventoryComponent->OnGridUpdated.AddDynamic(
        this, &AGercekCharacter::HandleGridInventoryUpdated);
  }

  if (!BreathingAudioComponent) {
    BreathingAudioComponent = FindComponentByClass<UAudioComponent>();
  }

  // Oyun başında nefes sesinin çalmamasını garantiye alıyoruz
  if (BreathingAudioComponent) {
    BreathingAudioComponent->Stop();
  }

  if (IsLocallyControlled()) {
    // Enhanced Input Mapping Context Ekleme
    if (APlayerController *PlayerController =
            Cast<APlayerController>(Controller)) {
      if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
              ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                  PlayerController->GetLocalPlayer())) {
        if (DefaultMappingContext) {
          Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
      }
    }

    // Sadece yerel oyuncuda etkileşim ve kamera timer'ları çalışsın.
    GetWorld()->GetTimerManager().SetTimer(
        InteractionTimerHandle, this,
        &AGercekCharacter::PerformInteractionTracePeriodic, 0.1f, true);
    GetWorld()->GetTimerManager().SetTimer(
        CameraShakeTimerHandle, this, &AGercekCharacter::PlayCameraShakePeriodic,
        0.35f, true);

    // HUD sadece yerel sahipte oluşturulmalı.
    if (PlayerHUDClass) {
      UUserWidget *HUDWidget =
          CreateWidget<UUserWidget>(GetWorld(), PlayerHUDClass);
      if (HUDWidget) {
        HUDWidget->AddToViewport();
      }
    }
  }
}

void AGercekCharacter::PawnClientRestart() {
  Super::PawnClientRestart();

  if (APlayerController *PlayerController =
          Cast<APlayerController>(GetController())) {

    // Oyun/UI odak sorunlarını engellemek için fare ve girdi modunu zorla
    // GameOnly yap.
    FInputModeGameOnly GameMode;
    PlayerController->SetInputMode(GameMode);
    PlayerController->bShowMouseCursor = false;

    if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                PlayerController->GetLocalPlayer())) {
      // Sıkışmış ve eski tuş atamalarını (stuck states) temizle
      Subsystem->ClearAllMappings();

      if (DefaultMappingContext) {
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
        UE_LOG(LogTemp, Display,
               TEXT("[EnhancedInput] DefaultMappingContext forcibly injected "
                    "and UI Input cleared via C++."));
      }
    }
  }
}

// Called every frame
void AGercekCharacter::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // Debug HUD spam'ini sadece yerel karakterde göster.
  if (IsLocallyControlled() && GEngine) {
    GEngine->AddOnScreenDebugMessage(
        -1, 0.0f, FColor::Yellow,
        FString::Printf(TEXT("STAMINA: %.1f"), Stamina));
  }

  // Sunucu (Server) yetkili stat güncellemeleri
  if (HasAuthority()) {
    // Zamanla açlık ve susuzluk azalması
    if (Hunger > 0.0f) {
      Hunger -= DeltaTime * HungerDecreaseRate;
      Hunger = FMath::Clamp(Hunger, 0.0f, 100.0f);
    } else {
      // Açlık 0'a ulaştığında, can (Health) her 12 saniyede 1 birim azalmalıdır
      // (Dakikada 5 can).
      Health -= (1.0f / 12.0f) * DeltaTime;
      Health = FMath::Clamp(Health, 0.0f, MaxHealth);
    }

    if (Thirst > 0.0f) {
      Thirst -= DeltaTime * ThirstDecreaseRate;
      Thirst = FMath::Clamp(Thirst, 0.0f, 100.0f);
    }

    // Radyasyon: Sunucuda hesaplanan radyasyon seviyesi 50'yi geçtiğinde can
    // kademeli azalmalıdır.
    if (Radiation > 50.0f) {
      // Kademeli azalma hızı - Saniyede 0.5 can
      Health -= 0.5f * DeltaTime;
      Health = FMath::Clamp(Health, 0.0f, MaxHealth);
    }

    // ==== SAĞLIK YENİLENMESİ (KADEMELİ DİZAYN) ====
    // Kural 1: Can 50'nin üzyerindeyse — otomatik yenilenme (saniyede 0.5 hp)
    // Kural 2: Can 50 veya altındaysa — Otomatik yenilenme YOKTUR.
    //   * Eşya kullanıldığında (ApplyItemEffect::Med) regen gersçekleşir.
    //   * SON 15 SANİYEDE HİÇ HASAR ALINMAZSA yenilenme başlar.
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    const float TimeSinceDamage = CurrentTime - LastDamageTakenTime;

    const bool bWellFed =
        Hunger > 75.0f && Thirst > 75.0f && Radiation <= 20.0f;

    if (Health > 50.0f) {
      // --- Can 50 Üzeri: Otomatik regen, tokluk ve radyasyon peşi sıra ---
      if (bWellFed) {
        Health += 0.5f * DeltaTime;
        Health = FMath::Clamp(Health, 0.0f, MaxHealth);
      }
    } else {
      // --- Can 50 Altı: Sadece 15 saniye hasar almamışsa regen açılır ---
      if (bWellFed && TimeSinceDamage >= 15.0f) {
        Health += 0.5f * DeltaTime;
        Health = FMath::Clamp(Health, 0.0f, MaxHealth);
      }
    }

    // Gerçekçi zıplama: Tek zıplama her durumda sabit.
    // Can 25 altında bile JumpMaxCount = 1 — çift zıplama yok.
    JumpMaxCount = 1;

    // Stamina yönetimi ve Koşma mantığı
    if (bIsSprinting && GetVelocity().SizeSquared() > 0 && !bIsExhausted) {
      // 15 ile 0 arasında stamina düşüşünü yavaşlat (saniyede 5 birim)
      // Can 25'in altındaysa yaralı durum: tükenme hızı 20f'e çıkar
      const float BaseDepletionRate = (Health < 25.0f) ? 20.0f : StaminaDepletionRate;
      float CurrentDepletionRate =
          (Stamina < 15.0f) ? 5.0f : BaseDepletionRate;
      Stamina -= DeltaTime * CurrentDepletionRate;
      Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);

      if (Stamina <= 0.0f) {
        Stamina = 0.0f;
        bIsExhausted = true;
        bIsRecovering = true;
        RecoveryDelayTimer =
            1.50f; // Tam bitkinlik durumunda 1.50 saniye bekleme cezası
      }
    } else {
      // Stamina Yenilenmesi
      float TimeSinceLastJump = GetWorld()->GetTimeSeconds() - LastJumpTime;

      if (Stamina < MaxStamina && TimeSinceLastJump > 1.0f) {

        // Eğer bekleme süresindeysek sadece sayacı azalt ve doluma izin verme
        if (bIsRecovering) {
          RecoveryDelayTimer -= DeltaTime;
          if (RecoveryDelayTimer <= 0.0f) {
            bIsRecovering = false; // Süre doldu, artık doluma başlayabiliriz
          }
        } else {
          // Kademeli Artış (Stamina dolarken mevcut değeri kontrol et)
          float CurrentRegenRate = StaminaRecoveryRate; // Normal hız (Örn: 10)

          // Kritik Bölge: Eğer 20'nin altındaysa çok daha yavaş dolar
          if (Stamina < 20.0f) {
            CurrentRegenRate = 5.0f;
          }

          Stamina += DeltaTime * CurrentRegenRate;
          Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);

          // Kilit Mekanizması: 21 birime ulaşana kadar karakter yorgunluktan
          // kurtulamaz
          if (Stamina >= 21.0f) {
            bIsExhausted = false;
            bIsFatigued = false;
          }
        }
      }
    }

    // ==== HAREKET HIZI (SPEED) YÖNETİMİ ====
    if (GetCharacterMovement()) {
      float TargetSpeed = BaseWalkSpeed;

      // Can durumuna göre hedef hızı belirle
      if (bIsConsuming) {
        TargetSpeed = BaseWalkSpeed / 2.0f; // İçerken hız yarıya düşer
      } else if (Health < 25.0f) {
        // Yaralı durumu
        if (bIsSprinting && !bIsExhausted) {
          TargetSpeed = (Stamina > 20.0f) ? InjuredSprintSpeed : InjuredSpeed;
        } else {
          TargetSpeed = InjuredSpeed;
        }
      } else {
        // Normal durum - Kademe Mantığı
        if (bIsExhausted) {
          TargetSpeed =
              300.0f; // 0 Noktası ve bekleme süresi: Mecburi Yürüme kilitli
        } else if (bIsSprinting && Stamina >= 21.0f) {
          TargetSpeed = 600.0f; // 21-100: Koşu
        } else if (bIsSprinting && Stamina > 0.0f && Stamina < 21.0f) {
          TargetSpeed = 400.0f; // 1-20: Yorgun Koşu
        } else {
          TargetSpeed = 300.0f; // Varsayılan yürüme
        }
      }

      // Stamina cezası 1: Stamina 20'nin altına düştüğünde yürüme hızı otomatik
      // olarak %30 yavaşlatılır. Eger exhausted (0 stamina kilitli ceza) ise
      // zaten 300'e sabitlenecektir.
      if (Stamina < 20.0f && !bIsExhausted && TargetSpeed > 0) {
        TargetSpeed *= 0.7f;
      }

      float CurrentSpeed = GetCharacterMovement()->MaxWalkSpeed;
      TargetSpeed *= MovementSpeedMultiplier; // Agirlik kaynakli hiz kancasi

      if (bIsExhausted) {
        // Stamina cezasi 2: 1.55 saniye boyunca karakter hizi %100 ceza 300'e
        // (agirhk carpanindan sonra) sabitlenir. Interp kullanılmaz, anında
        // kilitlenir.
        GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
      } else {
        // FInterpTo ile anlık hızı hedef hıza yumuşak (smooth) şekilde geçir
        GetCharacterMovement()->MaxWalkSpeed =
            FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, 3.0f);
      }
    }
  }

  // ==== DİNAMİK FOV ====
  if (FpsCameraComponent) {
    float TargetFOV = DefaultFOV;

    if (Health < 25.0f) {
      TargetFOV = InjuredFOV; // Can azken tünel görüşü (FOV daralması)
    } else if (bIsSprinting && !bIsFatigued) {
      TargetFOV = SprintFOV;
    } else if (bIsCrouched) {
      TargetFOV = CrouchFOV;
    }

    float CurrentFOV = FpsCameraComponent->FieldOfView;
    if (!FMath::IsNearlyEqual(CurrentFOV, TargetFOV, 0.1f)) {
      FpsCameraComponent->SetFieldOfView(
          FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, FOVInterpSpeed));
    }
  }

  // ==== HASAR GÖRSEL EFEKTLERİ (POST-PROCESS) ====
  if (FpsCameraComponent) {
    float TargetSaturation = 1.0f;
    float TargetFringe = 0.0f;   // SceneFringeIntensity (Blur etkisi yaratır)
    float TargetVignette = 0.4f; // Varsayılan Vignette Intensity

    if (Health < 25.0f) {
      TargetSaturation = 0.0f; // Tamamen siyah/beyaz
      TargetFringe = 4.0f;     // Yüksek şiddetli Color Fringe
      TargetVignette = 1.0f;   // Yoğun Vignette
    } else if (Health < 50.0f) {
      TargetSaturation = 0.5f; // Yarı soluk renk
      TargetFringe = 2.0f;     // Hafif bulanıklık
      TargetVignette = 0.6f;   // Hafif Vignette
    }

    FPostProcessSettings &Settings = FpsCameraComponent->PostProcessSettings;

    // FVector4 için yumuşak geçiş hesaplaması (Interp)
    FVector4 CurrentSaturation = Settings.ColorSaturation;
    if (CurrentSaturation.X == 0.0f && CurrentSaturation.W == 0.0f) {
      // Varsayılan değeri 1,1,1,1 kabul ediyoruz
      CurrentSaturation = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    // Optimizasyon: Sadece gerekli olduğunda efekti uygula
    if (!FMath::IsNearlyEqual(CurrentSaturation.X, TargetSaturation, 0.01f) ||
        !FMath::IsNearlyEqual(Settings.SceneFringeIntensity, TargetFringe, 0.01f) ||
        !FMath::IsNearlyEqual(Settings.VignetteIntensity, TargetVignette, 0.01f)) {

      Settings.bOverride_ColorSaturation = true;
      Settings.bOverride_SceneFringeIntensity = true;
      Settings.bOverride_VignetteIntensity = true;

      // Saturation Geçişi
      float InterpSat = FMath::FInterpTo(CurrentSaturation.X, TargetSaturation, DeltaTime, 1.5f);
      Settings.ColorSaturation = FVector4(InterpSat, InterpSat, InterpSat, 1.0f);

      // Fringe Geçişi
      float CurrentFringe = Settings.SceneFringeIntensity;
      Settings.SceneFringeIntensity = FMath::FInterpTo(CurrentFringe, TargetFringe, DeltaTime, 1.5f);

      // Vignette Geçişi
      float CurrentVignette = Settings.VignetteIntensity;
      Settings.VignetteIntensity = FMath::FInterpTo(CurrentVignette, TargetVignette, DeltaTime, 1.5f);
    }
  }

  // Dinamik Kamera Sarsıntısı (Camera Shake) optimizasyon amacıyla Tick'ten çıkarıldı.
  // Gerçekleştirimi Timer'a taşındı.

  // ==== STAMINA YORGUNLUK SESİ (BREATHING) ====

  // Gelişmiş Ses Kontrolü: 15'in altında koşarken VEYA 1.5 saniyelik tükenme
  // cezası sırasında devam etsin
  if ((Stamina < 15.0f && bIsSprinting && GetVelocity().SizeSquared() > 0 &&
       !bIsExhausted) ||
      bIsRecovering) {
    // Ses bileşeni atanmışsa çalmayı başlat
    if (BreathingAudioComponent) {
      if (BreathingSound && BreathingAudioComponent->Sound != BreathingSound) {
        BreathingAudioComponent->SetSound(BreathingSound);
      }

      // Sadece ve sadece şu an çalmıyorsa başlat (Üst üste binmeyi engeller)
      if (!BreathingAudioComponent->IsPlaying()) {
        BreathingAudioComponent->Play();
      }
    }

    // Geliştirici hata ayıklama mesajı HER ZAMAN çalışsın
    if (GEngine) {
      GEngine->AddOnScreenDebugMessage(5, 2.0f, FColor::Red,
                                       TEXT("!!! KESIK NEFES !!!"));
    }

  } else {
    // Gecikme bittikten sonra (Stamina dolmaya başladığında) veya koşmayı
    // bıraktığında sesi 1 saniyede sönümleyerek durdur (FadeOut)
    if (BreathingAudioComponent && BreathingAudioComponent->IsPlaying()) {
      BreathingAudioComponent->FadeOut(1.0f, 0.0f);
    }
  }
}

// Called to bind functionality to input
void AGercekCharacter::SetupPlayerInputComponent(
    UInputComponent *PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  if (UEnhancedInputComponent *EnhancedInputComponent =
          Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
    // Sprint (Koşma)
    if (SprintAction) {
      EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started,
                                         this, &AGercekCharacter::StartSprint);
      EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed,
                                         this, &AGercekCharacter::StopSprint);
    }

    // Etkileşim (Interact)
    if (InteractAction) {
      EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started,
                                         this, &AGercekCharacter::Interact);
    }

    // Envanteri Aç/Kapat (Tab)
    if (ToggleInventoryAction) {
      EnhancedInputComponent->BindAction(ToggleInventoryAction,
                                         ETriggerEvent::Started, this,
                                         &AGercekCharacter::ToggleInventory);
    }

    // Zıplama eylemi
    if (JumpAction) {
      EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started,
                                         this, &AGercekCharacter::Jump);
      EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed,
                                         this, &AGercekCharacter::StopJumping);
    }

    // Crouch (Eğilme)
    if (CrouchAction) {
      EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started,
                                         this, &AGercekCharacter::StartCrouch);
      EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed,
                                         this, &AGercekCharacter::StopCrouch);
    }

    // Hareket (Move)
    if (MoveAction) {
      EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered,
                                         this, &AGercekCharacter::Move);
    }

    // Kamera Kontrolü (Look)
    if (LookAction) {
      EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered,
                                         this, &AGercekCharacter::Look);
    }
  }

  // Eski tip (Legacy) Input Binding desteği: Envanter açma kapama için
  PlayerInputComponent->BindAction("EnvanterAc", IE_Pressed, this,
                                   &AGercekCharacter::ToggleInventory);

  // Etkileşim (Interact) eylemini bağla (Project Settings -> Input -> Action
  // Mappings)
  PlayerInputComponent->BindAction("Interact", IE_Pressed, this,
                                   &AGercekCharacter::Interact);
}

void AGercekCharacter::StartSprint() {
  if (Stamina > 0.0f && !bIsCrouched && !bIsFatigued) {
    bIsSprinting = true;
    // Hız geçişi Tick içerisinde FInterpTo ile yapılıyor.
  }
}

void AGercekCharacter::StopSprint() {
  bIsSprinting = false;
  // Hız geçişi Tick içerisinde FInterpTo ile yapılıyor.
}

void AGercekCharacter::StartCrouch() {
  if (!bIsSprinting) {
    Crouch();
  }
}

void AGercekCharacter::StopCrouch() { UnCrouch(); }

void AGercekCharacter::Jump() {
  // Gerçekçilik kuralı: Sadece yerde iken zıplanabilir.
  // IsFalling() veya !IsMovingOnGround() ile hava zıplaması tamamen kapatılır.
  if (!GetCharacterMovement() || !GetCharacterMovement()->IsMovingOnGround()) {
    return;
  }

  // Stamina kontrolü (En az 10 stamina gerekli)
  if (Stamina >= 10.0f) {
    // Stamina maliyeti
    Stamina -= 10.0f;
    Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);

    // Gecikme süresi için anlık zamanı kaydet
    LastJumpTime = GetWorld()->GetTimeSeconds();

    // Motorun normal zıplama kodunu çağır
    Super::Jump();
  }
}

// --- YENİ UZMAN (AAA) ETKİLEŞİM SİSTEMİ ---
AActor *AGercekCharacter::PerformInteractionTrace() const {
  if (!FpsCameraComponent)
    return nullptr;

  FVector StartPos = FpsCameraComponent->GetComponentLocation();
  FVector EndPos = StartPos + (FpsCameraComponent->GetForwardVector() * 200.0f);

  FHitResult HitResult;
  FCollisionQueryParams Params;
  Params.AddIgnoredActor(this);

  if (GetWorld()->LineTraceSingleByChannel(HitResult, StartPos, EndPos,
                                           ECC_Visibility, Params)) {
    return HitResult.GetActor();
  }
  return nullptr;
}

FText AGercekCharacter::GetInteractionPrompt() const {
  // Optimizasyon: Her frame (Tick) hesaplamak yerine, timer ile 0.1s de bir atılan
  // LineTrace'in sonucunu (önbellek) döndür.
  return CachedInteractionPrompt;
}

void AGercekCharacter::PerformInteractionTracePeriodic() {
  AActor *HitActor = PerformInteractionTrace();
  CachedInteractableActor = HitActor;

  if (!HitActor) {
    CachedInteractionPrompt = FText::GetEmpty();
    return;
  }

  // Tüccar Kontrolü
  if (AMerchantBase *Merchant = Cast<AMerchantBase>(HitActor)) {
    FString FoundName = Merchant->GetMerchantName().ToString();
    CachedInteractionPrompt = FText::FromString(FString::Printf(TEXT("E - Takas %s"), *FoundName));
    return;
  }

  // Klasik Etkileşim Öğeleri Kontrolü
  if (HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass())) {
    FText ItemName = IInteractable::Execute_GetInteractableName(HitActor);
    if (!ItemName.IsEmpty()) {
      CachedInteractionPrompt = FText::FromString(FString::Printf(TEXT("E - Al %s"), *ItemName.ToString()));
    } else {
      CachedInteractionPrompt = FText::FromString(TEXT("E - Al"));
    }
    return;
  }

  CachedInteractionPrompt = FText::GetEmpty();
}

void AGercekCharacter::PlayCameraShakePeriodic() {
  if (!WalkingCameraShakeClass) return;

  if (APlayerController *PlayerController = Cast<APlayerController>(GetController())) {
    float Speed = GetVelocity().Size();
    bool bIsOnGround = GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround();

    // Sadece yerdeyken ve en az 10 hızında (durmuyor) sarsıntı oynat
    if (bIsOnGround && Speed > 10.0f) {
      float ShakeScale = Speed / BaseWalkSpeed;
      ShakeScale = FMath::Clamp(ShakeScale, 0.1f, 2.0f);
      PlayerController->ClientStartCameraShake(WalkingCameraShakeClass, ShakeScale);
    }
  }
}

void AGercekCharacter::Interact() {
  if (ActiveTradeWidget != nullptr) {
    CloseTradeScreen();
    return;
  }

  AActor *HitActor = PerformInteractionTrace();
  if (!IsValid(HitActor)) {
    return;
  }

  if (AMerchantBase *HitMerchant = Cast<AMerchantBase>(HitActor)) {
    OpenTradeScreen(HitMerchant);
    return;
  }

  if (!HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass())) {
    return;
  }

  if (HasAuthority()) {
    IInteractable::Execute_OnInteract(HitActor, this);
  } else {
    ServerInteract(HitActor);
  }
}

void AGercekCharacter::ServerInteract_Implementation(AActor *TargetActor) {
  if (!IsValid(TargetActor)) {
    return;
  }

  if (!TargetActor->GetClass()->ImplementsInterface(UInteractable::StaticClass())) {
    return;
  }

  if (GetDistanceTo(TargetActor) > 250.0f) {
    return;
  }

  IInteractable::Execute_OnInteract(TargetActor, this);
}

void AGercekCharacter::ToggleInventory() {
  // Her zaman kendi controller'ımızı kullanalım; yoksa world'den al.
  APlayerController *PC = Cast<APlayerController>(GetController());
  if (!PC) {
    PC = GetWorld()->GetFirstPlayerController();
  }
  if (!PC)
    return;

  // Widget ilk kez açılıyorsa oluştur (lazy init — sadece bir kez).
  if (!IsValid(InventoryWidget) && InventoryWidgetClass) {
    InventoryWidget =
        CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);
    if (IsValid(InventoryWidget)) {
      InventoryWidget->AddToViewport(10);
      InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
    }
  }

  if (!IsValid(InventoryWidget))
    return;

  const bool bIsOpen =
      InventoryWidget->GetVisibility() == ESlateVisibility::Visible;

  if (bIsOpen) {
    // ---- KAPAT ------------------------------------------------
    InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);

    // Fareyi gizle, kontrolü tamamen oyuna ver.
    PC->bShowMouseCursor = false;

    FInputModeGameOnly GameMode;
    PC->SetInputMode(GameMode);
  } else {
    // ---- AÇ ---------------------------------------------------
    InventoryWidget->SetVisibility(ESlateVisibility::Visible);

    if (InventoryComponent) {
      InventoryComponent->NativeRefreshUI(InventoryWidget);
    }

    // Fareyi göster.
    PC->bShowMouseCursor = true;

    // Sadece UI girdilerini kabul et (Game Only / UI Only geçişi)
    FInputModeUIOnly UIMode;
    // Widget'a focus ver
    UIMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
    // Tam ekranda farenin pencere dışına çıkmasını engelle.
    UIMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockInFullscreen);
    PC->SetInputMode(UIMode);

    // Debug: mevcut envanter içeriğini logla.
    ShowInventoryDetails();
  }
}

// ==== EŞYA TÜKETİMİ (CONSUMABLES) ====
void AGercekCharacter::ConsumeItem(EItemType Type, float Amount) {
  if (bIsConsuming) {
    return; // Zaten bir şey tüketiyorsak işlemi reddet
  }

  // Animasyon veya başka etkiler için bayrağı açıyoruz
  bIsConsuming = true;

  // Tüketim süresi (Örn. 2 saniye sonra etki etsin)
  float ConsumeDelay = 2.0f;

  FTimerDelegate TimerDel;
  TimerDel.BindUObject(this, &AGercekCharacter::ApplyItemEffect, Type, Amount);

  GetWorld()->GetTimerManager().SetTimer(ConsumeTimerHandle, TimerDel,
                                         ConsumeDelay, false);
}

void AGercekCharacter::ApplyItemEffect(EItemType Type, float Amount) {
  bIsConsuming = false;

  // Denge: Tüm stat artışları (yiyecek, tıbbi kit) sadece Sunucu (Server)
  // üzerinde HasAuthority() kontrolü ile yapılmalıdır.
  if (!HasAuthority()) {
    return;
  }

  switch (Type) {
  case EItemType::Food:
    // Yiyecek: Ac?l??? giderir.
    Hunger += Amount;
    Hunger = FMath::Clamp(Hunger, 0.0f, MaxHunger);
    // Su yerine yiyecek kullan?ld???nda Thirst de k?smen yap?l?r (meyve vb.)
    Thirst += Amount * 0.3f;
    Thirst = FMath::Clamp(Thirst, 0.0f, MaxThirst);
    // Yeterince tok ve susuz de?ilsek stamina buff uygula
    if (Thirst > 80.0f) {
      StaminaRecoveryRate = OriginalStaminaRecoveryRate * 1.5f;
      GetWorld()->GetTimerManager().SetTimer(
          StaminaBuffTimerHandle,
          [this]() { StaminaRecoveryRate = OriginalStaminaRecoveryRate; },
          30.0f, false);
    }
    break;

  case EItemType::Med:
    // Tibbi malzeme: Can? yeniler.
    Health += Amount;
    Health = FMath::Clamp(Health, 0.0f, MaxHealth);
    break;

  case EItemType::Junk:
    // Hurda: Artık radyasyonu azaltmaz, sadece ticari veya üretim amaçlı bir
    // eşya (Trade/Barter).
    break;

  case EItemType::AntiRad:
    // AntiRad (Radyasyon İlacı): Radyasyonu temizler.
    Radiation -= Amount;
    Radiation = FMath::Clamp(Radiation, 0.0f, MaxRadiation);

    // Hardcore Penalty: Ağır kimyasallar vücudu yorar, anında 15 Stamina siler.
    Stamina -= 15.0f;
    Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);

    if (GEngine) {
      GEngine->AddOnScreenDebugMessage(
          -1, 3.0f, FColor::Green,
          TEXT("Radyasyon temizlendi, ancak yorgunluk hissediyorsun!"));
    }
    break;

  default:
    // Silah, Mühimmat, Quest esyalari tüketilemez.
    UE_LOG(LogTemp, Warning,
           TEXT("[ConsumeItem] Bu esya tüketilemez (Type: %%d)."),
           static_cast<int32>(Type));
    break;
  }
}

void AGercekCharacter::ResetStaminaRecoveryBuff() {
  StaminaRecoveryRate = OriginalStaminaRecoveryRate;
}

// --- HASAR KAYDİ (HealthRegen lockout için) ---
float AGercekCharacter::TakeDamage(float DamageAmount,
                                   struct FDamageEvent const &DamageEvent,
                                   class AController *EventInstigator,
                                   AActor *DamageCauser) {
  const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent,
                                               EventInstigator, DamageCauser);

  if (ActualDamage > 0.0f && HasAuthority()) {
    // Son hasar zamanını güncelle (can regen kilidi sıfırlanır)
    LastDamageTakenTime = GetWorld()->GetTimeSeconds();

    Health -= ActualDamage;
    Health = FMath::Clamp(Health, 0.0f, MaxHealth);

    UE_LOG(LogTemp, Log, TEXT("[Damage] %.1f hasar alindi. Kalan can: %.1f"),
           ActualDamage, Health);
  }

  return ActualDamage;
}

float AGercekCharacter::GetWeightRatio() const {
  if (!InventoryComponent)
    return 0.0f;
  // Grid doluluk oranı: Kaplanan hücre sayısı / Toplam hücre sayısı (Cols *
  // Rows)
  const int32 TotalCells =
      InventoryComponent->GetGridColumns() * InventoryComponent->GetGridRows();
  if (TotalCells <= 0)
    return 0.0f;
  return FMath::Clamp(
      static_cast<float>(InventoryComponent->GetOccupiedSlots().Num()) /
          static_cast<float>(TotalCells),
      0.0f, 1.0f);
}

// ==== AĞIRLIK KAYNAKLI HAREKET GÜNCELLEMESİ ====
//
// FOnWeightChanged delegate'i AddItem / RemoveItem sonrasında yayınlanır.
// Bu fonksiyon Tick'e bağli değil — sadece envanter değiştiğinde tetiklenir.
void AGercekCharacter::OnInventoryWeightChanged(float NewWeight,
                                                float MaxWeight) {
  UpdateMovementSpeed(NewWeight, MaxWeight);
}

void AGercekCharacter::UpdateMovementSpeed(float CurrentWeight,
                                           float MaxWeight) {
  // MaxWeight geçersizse bölme hatası önle.
  if (MaxWeight <= 0.0f) {
    MovementSpeedMultiplier = 1.0f;
    return;
  }

  if (CurrentWeight > MaxWeight) {
    // ---- OVERBURDENED: Taşıyan karakter yavaşlar (%50) ----
    // Tick içindeki TargetSpeed *= MovementSpeedMultiplier satırı
    // bu değeri otomatik olarak yansıtır; ek kod gerekmez.
    MovementSpeedMultiplier = 0.5f;

    UE_LOG(
        LogTemp, Warning,
        TEXT("[Movement] Overburdened! %.2f / %.2f kg -- hiz %%50 dusuruldu."),
        CurrentWeight, MaxWeight);
  } else {
    // ---- NORMAL: Tam hız yeniden aktif ----
    MovementSpeedMultiplier = 1.0f;
  }
}

// ==== HAREKET VE KAMERA (ENHANCED INPUT) ====
void AGercekCharacter::Move(const FInputActionValue &Value) {
  // input is a Vector2D
  FVector2D MovementVector = Value.Get<FVector2D>();

  if (Controller != nullptr) {
    // İleri/Geri hareket (W/S)
    AddMovementInput(GetActorForwardVector(), MovementVector.Y);
    // Sağa/Sola hareket (A/D)
    AddMovementInput(GetActorRightVector(), MovementVector.X);
  }
}

void AGercekCharacter::Look(const FInputActionValue &Value) {
  // Guard: Envanter açıksa kamera bakışını engelle.
  // Kullanıcı fare ile UI'da gezinirken kamera dönmemelidir.
  if (IsValid(InventoryWidget) &&
      InventoryWidget->GetVisibility() == ESlateVisibility::Visible) {
    return;
  }

  FVector2D LookAxisVector = Value.Get<FVector2D>();

  if (Controller != nullptr) {
    // Sağa/Sola bakış (Yaw)
    AddControllerYawInput(LookAxisVector.X);
    // Yukarı/Aşağı bakış (Pitch)
    AddControllerPitchInput(LookAxisVector.Y);
  }
}

void AGercekCharacter::ShowInventoryDetails() {
  if (!InventoryComponent) {
    return;
  }

  const TMap<FIntPoint, FName> &Slots = InventoryComponent->GetOccupiedSlots();

  if (Slots.Num() == 0) {
    if (GEngine) {
      GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
                                       TEXT("Envanter Bos"));
    }
    UE_LOG(LogTemp, Warning, TEXT("Envanter Bos"));
    return;
  }

  // Grid  özeti: Benzersiz eşya adlarını ve kapladıkları hücre sayısını göster.
  TMap<FName, int32> ItemCellCounts;
  for (const TPair<FIntPoint, FName> &Pair : Slots) {
    ItemCellCounts.FindOrAdd(Pair.Value)++;
  }

  for (const TPair<FName, int32> &Entry : ItemCellCounts) {
    FString Message = FString::Printf(TEXT("[GRID] %s  |  %d hucre kaplıyor"),
                                      *Entry.Key.ToString(), Entry.Value);

    if (GEngine) {
      GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, Message);
    }
    UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
  }
}

void AGercekCharacter::HandleGridInventoryUpdated() {
  if (!InventoryComponent || !IsValid(InventoryWidget)) {
    return;
  }

  InventoryComponent->NativeRefreshUI(InventoryWidget);
}

// ==== ÇANTADAN KULLANMA VE YERE ATMA (USE & DROP) ====

void AGercekCharacter::UseItemFromInventory(
    const FDataTableRowHandle &ItemRowHandle) {
  if (ItemRowHandle.IsNull() || !InventoryComponent)
    return;

  const FItemDBRow *LegacyRow = ItemRowHandle.GetRow<FItemDBRow>(
      TEXT("AGercekCharacter::UseItemFromInventory.Legacy"));
  const FPostApocItemRow *GridRow = ItemRowHandle.GetRow<FPostApocItemRow>(
      TEXT("AGercekCharacter::UseItemFromInventory.Grid"));

  EItemType ConsumeType = EItemType::Junk;
  float ConsumeAmount = 0.0f;
  bool bCanConsume = false;

  if (LegacyRow) {
    bCanConsume = LegacyRow->ItemType == EItemType::Food ||
                  LegacyRow->ItemType == EItemType::Med ||
                  LegacyRow->ItemType == EItemType::AntiRad;
    ConsumeType = LegacyRow->ItemType;
    ConsumeAmount = static_cast<float>(LegacyRow->ItemValue);
  } else if (GridRow && GridRow->bCanConsume) {
    bCanConsume = true;
    ConsumeAmount = static_cast<float>(GridRow->BaseValue);

    switch (GridRow->Category) {
    case EPostApocItemCategory::Food:
    case EPostApocItemCategory::Drink:
      ConsumeType = EItemType::Food;
      break;
    case EPostApocItemCategory::Medical:
      ConsumeType = EItemType::Med;
      break;
    default:
      bCanConsume = false;
      break;
    }
  }

  if (!bCanConsume) {
    const FString ItemLabel =
        LegacyRow ? LegacyRow->ItemName.ToString()
                  : (GridRow ? GridRow->DisplayName.ToString()
                             : ItemRowHandle.RowName.ToString());
    UE_LOG(
        LogTemp, Warning,
        TEXT(
            "[AGercekCharacter] Tuketilemeyen esya kullanilmaya calisildi: %s"),
        *ItemLabel);
    return;
  }

  if (InventoryComponent->RemoveItemFromGrid(ItemRowHandle.RowName)) {
    ConsumeItem(ConsumeType, ConsumeAmount);
  }
}

void AGercekCharacter::DropItemFromInventory(
    const FDataTableRowHandle &ItemRowHandle) {
  if (ItemRowHandle.IsNull() || !InventoryComponent)
    return;

  const FItemDBRow *LegacyRow = ItemRowHandle.GetRow<FItemDBRow>(
      TEXT("AGercekCharacter::DropItemFromInventory.Legacy"));
  const FPostApocItemRow *GridRow = ItemRowHandle.GetRow<FPostApocItemRow>(
      TEXT("AGercekCharacter::DropItemFromInventory.Grid"));

  const bool bIsQuestItem =
      (LegacyRow && (LegacyRow->ItemType == EItemType::QuestItem ||
                     LegacyRow->ItemType == EItemType::Quest ||
                     LegacyRow->ItemCategory == EItemCategory::Quest)) ||
      (GridRow && GridRow->bQuestItem);

  if (bIsQuestItem) {
    if (GEngine) {
      GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
                                       TEXT("Gorev esyalari yere atilamaz."));
    }
    return;
  }

  if (InventoryComponent->RemoveItemFromGrid(ItemRowHandle.RowName)) {
    if (FpsCameraComponent) {
      FVector DropLocation = FpsCameraComponent->GetComponentLocation() +
                             (FpsCameraComponent->GetForwardVector() * 100.0f);
      SpawnItemInWorld(ItemRowHandle, DropLocation);
    }
  }
}

AActor *
AGercekCharacter::SpawnItemInWorld(const FDataTableRowHandle &ItemRowHandle,
                                   FVector SpawnLocation) {
  if (ItemRowHandle.IsNull() || !GetWorld())
    return nullptr;

  FActorSpawnParameters SpawnParams;
  SpawnParams.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

  // AWorldItemActor oluştur
  AWorldItemActor *SpawnedItem = GetWorld()->SpawnActor<AWorldItemActor>(
      AWorldItemActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator,
      SpawnParams);

  if (SpawnedItem) {
    SpawnedItem->InitializeItemData(ItemRowHandle);
  }

  return SpawnedItem;
}

// ==== TİCARET EKRANI YÖNETİMİ ====

void AGercekCharacter::OpenTradeScreen(AMerchantBase *TargetMerchant) {
  // Hedef tüccar geçersizse işlem yapma
  if (!IsValid(TargetMerchant) || !TradeScreenClass) {
    return;
  }

  // Aktif tüccarı kaydet
  ActiveMerchant = TargetMerchant;

  // Halihazırda açık bir takas ekranı yoksa oluştur
  if (!IsValid(ActiveTradeWidget)) {
    ActiveTradeWidget = CreateWidget<UUserWidget>(GetWorld(), TradeScreenClass);
  }

  if (IsValid(ActiveTradeWidget)) {
    // 1. Ekrandaki TextBlock (Tüccar Adı) güncellemesi
    UTextBlock *MerchantNameText = Cast<UTextBlock>(
        ActiveTradeWidget->GetWidgetFromName(TEXT("Txt_MerchantName")));
    if (MerchantNameText) {
      MerchantNameText->SetText(TargetMerchant->GetMerchantName());
    }

    // 2. Oyuncu ve Tüccar envanter Izgaralarını (Grid) "Enjekte" et ve C++
    // üzerinden yenile
    UUserWidget *TraderGridUI = Cast<UUserWidget>(
        ActiveTradeWidget->GetWidgetFromName(TEXT("TuccarCanta")));
    if (TraderGridUI) {
      UPostApocInventoryComponent *MerchantInv =
          TargetMerchant->GetMerchantInventory();
      if (MerchantInv) {
        // C++ üzerinden Native Refresh (Meryem'in BP loop'larını baypas eder)
        MerchantInv->NativeRefreshUI(TraderGridUI);
      }
    }

    UUserWidget *PlayerGridUI = Cast<UUserWidget>(
        ActiveTradeWidget->GetWidgetFromName(TEXT("OyuncuCanta")));
    if (PlayerGridUI) {
      if (InventoryComponent) {
        // C++ üzerinden Native Refresh
        InventoryComponent->NativeRefreshUI(PlayerGridUI);
      }
    }

    // 3. Buton Bağlantıları (Btn_Ayril, Btn_TakasYap) - Doğrudan C++
    // Fonksiyonlarına Bağlama
    UButton *ExitButton =
        Cast<UButton>(ActiveTradeWidget->GetWidgetFromName(TEXT("Btn_Ayril")));
    if (ExitButton) {
      ExitButton->OnClicked.Clear();
      ExitButton->OnClicked.AddDynamic(this,
                                       &AGercekCharacter::CloseTradeScreen);
    }

    UButton *TradeButton = Cast<UButton>(
        ActiveTradeWidget->GetWidgetFromName(TEXT("Btn_TakasYap")));
    if (TradeButton) {
      TradeButton->OnClicked.Clear();
      TradeButton->OnClicked.AddDynamic(this, &AGercekCharacter::ExecuteTrade);
    }

    // 4. Arayüzü ekrana ekle ve girdi (input) kontrollerini düzenle
    ActiveTradeWidget->AddToViewport(20);

    if (APlayerController *PC = Cast<APlayerController>(GetController())) {
      PC->bShowMouseCursor = true;

      FInputModeUIOnly UIMode;
      UIMode.SetWidgetToFocus(ActiveTradeWidget->TakeWidget());
      UIMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockInFullscreen);
      PC->SetInputMode(UIMode);
    }
  }
}

void AGercekCharacter::ExecuteTrade() {
  if (!IsValid(ActiveTradeWidget) || !ActiveMerchant)
    return;

  // 1. Değer Kontrolü (Barter Value Validation) - Verileri UI'dan Reflection
  // ile çekiyoruz
  float PlayerValue = 0.0f;
  float TraderValue = 0.0f;

  UFunction *GetPlayerValFunc =
      ActiveTradeWidget->FindFunction(TEXT("GetOyuncuTeklifToplami"));
  UFunction *GetMerchantValFunc =
      ActiveTradeWidget->FindFunction(TEXT("GetTuccarIstekToplami"));

  if (GetPlayerValFunc)
    ActiveTradeWidget->ProcessEvent(GetPlayerValFunc, &PlayerValue);
  if (GetMerchantValFunc)
    ActiveTradeWidget->ProcessEvent(GetMerchantValFunc, &TraderValue);

  // KRİTİK: Oyuncunun teklifi tüccarın isteğini karşılamıyorsa kırmızı uyarı
  // ver ve durdur.
  if (PlayerValue < TraderValue || PlayerValue <= 0.0f) {
    if (GEngine) {
      GEngine->AddOnScreenDebugMessage(
          -1, 4.0f, FColor::Red,
          TEXT("Yetersiz Takas Değeri! Daha fazla eşya ekle."));
    }
    return;
  }

  // 2. Takas Edilecek Eşya Listelerinin Toplanması (Listeler UI'daki slotlardan
  // gelir)
  TArray<FDataTableRowHandle> PlayerOfferItems;
  TArray<FDataTableRowHandle> MerchantOfferItems;

  UFunction *GetPlayerListFunc =
      ActiveTradeWidget->FindFunction(TEXT("GetOyuncuTeklifListesi"));
  UFunction *GetMerchantListFunc =
      ActiveTradeWidget->FindFunction(TEXT("GetTuccarTeklifListesi"));

  if (GetPlayerListFunc)
    ActiveTradeWidget->ProcessEvent(GetPlayerListFunc, &PlayerOfferItems);
  if (GetMerchantListFunc)
    ActiveTradeWidget->ProcessEvent(GetMerchantListFunc, &MerchantOfferItems);

  // 3. Sunucu Tarafında Asıl Eşya Yer Değiştirme ve XP İşlemini Gerçekleştir
  // (RPC)
  if (TradeComponent && ActiveMerchant->GetMerchantInventory()) {
    // TradeComponent içindeki C++ mantığı: RemoveItem, TryAddItem ve XP
    // işlemlerini sunucuda yapar.
    TradeComponent->Server_ExecuteTrade(PlayerOfferItems, MerchantOfferItems,
                                   InventoryComponent,
                                   ActiveMerchant->GetMerchantInventory());

    UE_LOG(LogTemp, Display,
           TEXT("[Takas] C++ üzerinden takas emri sunucuya gönderildi."));
  }

  // 4. İşlem bittikten sonra dükkanı kapat (Input modunu ve farenin durumunu
  // sıfırlar)
  CloseTradeScreen();
}

void AGercekCharacter::CloseTradeScreen() {
  if (IsValid(ActiveTradeWidget)) {
    ActiveTradeWidget->RemoveFromParent();
    ActiveTradeWidget = nullptr;
  }

  // Kamerayı ve hareket yeteneklerini oyuncuya geri ver
  if (APlayerController *PC = Cast<APlayerController>(GetController())) {
    PC->bShowMouseCursor = false;

    FInputModeGameOnly GameMode;
    PC->SetInputMode(GameMode);
  }
}



