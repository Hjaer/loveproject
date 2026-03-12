// Fill out your copyright notice in the Description page of Project Settings.

#include "GercekCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "CollisionQueryParams.h"
#include "Components/AudioComponent.h"
#include "Components/InputComponent.h"
#include "Components/PostProcessComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interactable.h"
#include "InventoryComponent.h"
#include "InventoryWidget.h"
#include "TimerManager.h"
#include "WorldItemActor.h"
// Replication için gerekli include (EKLEME)
#include "Net/UnrealNetwork.h"

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

  // Ses Bileşeni Oluşturma
  BreathingAudioComponent =
      CreateDefaultSubobject<UAudioComponent>(TEXT("BreathingAudioComponent"));
  BreathingAudioComponent->SetupAttachment(RootComponent);
  BreathingAudioComponent->bAutoActivate = false; // Başlangıçta çalmasın

  // Normal varsayılanlar...
  CurrentInteractItemName = TEXT("");
  CurrentInteractable = nullptr;

  // Envanter Bileşeni Başlatma
  InventoryComponent =
      CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));

  // Widget Referansı
  static ConstructorHelpers::FClassFinder<UUserWidget>
      InteractWidgetClassFinder(TEXT("/Game/Gercek/UI/WBP_Interact"));
  if (InteractWidgetClassFinder.Succeeded()) {
    InteractWidgetClass = InteractWidgetClassFinder.Class;
  }
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

    // Başlangıç hız limitleri
    BaseWalkSpeed = 300.0f;
    SprintSpeed = 600.0f;
    InjuredSpeed = 150.0f;       // Can azken walk hızı
    InjuredSprintSpeed = 300.0f; // Can azken sprint hızı
  }

  // --- CO-OP ALTYAPISI (EKLEME) ---
  bReplicates = true;
  SetReplicateMovement(true);
}

// --- REPLICATION KAYIT FONKSİYONU (EKLEME) ---
void AGercekCharacter::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(AGercekCharacter, Health);
  DOREPLIFETIME(AGercekCharacter, Stamina);
  DOREPLIFETIME(AGercekCharacter, Hunger);
  DOREPLIFETIME(AGercekCharacter, Thirst);
}

// Called when the game starts or when spawned
void AGercekCharacter::BeginPlay() {
  Super::BeginPlay();

  if (InventoryComponent) {
    InventoryComponent->OnWeightChanged.AddDynamic(
        this, &AGercekCharacter::OnInventoryWeightChanged);
  }

  if (!BreathingAudioComponent) {
    BreathingAudioComponent = FindComponentByClass<UAudioComponent>();
  }

  // Oyun başında nefes sesinin çalmamasını garantiye alıyoruz
  if (BreathingAudioComponent) {
    BreathingAudioComponent->Stop();
  }

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

  // HUD Oluşturma ve Ekrana Ekleme
  if (PlayerHUDClass) {
    UUserWidget *HUDWidget =
        CreateWidget<UUserWidget>(GetWorld(), PlayerHUDClass);
    if (HUDWidget) {
      HUDWidget->AddToViewport();
    }
  }

  // Etkileşim Widget'ını Başlat (Görünmez Olarak)
  if (InteractWidgetClass) {
    InteractWidget = CreateWidget<UUserWidget>(GetWorld(), InteractWidgetClass);
    if (InteractWidget) {
      InteractWidget->AddToViewport(999);
      InteractWidget->SetVisibility(ESlateVisibility::Hidden);
    }
  }
}

// Called every frame
void AGercekCharacter::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // Bakışla Algılama Sistemi (Her Karede Işın Yollayarak HUD Kontrolü)
  CheckForInteractables();

  // Her zaman ekranda çalışıp çalışmadığını gösteren mesaj
  if (GEngine) {
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

    // Tokluk İyileşmesi (Hem Açlık hem Susuzluk %75 üzerindeyse ve Radyasyon
    // düşükse canı artır)
    if (Hunger > 75.0f && Thirst > 75.0f && Radiation <= 20.0f) {
      Health += 0.5f * DeltaTime;
      Health = FMath::Clamp(Health, 0.0f, MaxHealth);
    }

    // Zıplama kısıtlaması (Can 25'ten azsa çift zıplama kapatılır)
    if (Health < 25.0f) {
      JumpMaxCount = 1;
    } else {
      JumpMaxCount = 2; // Veya projedeki varsayılan değer neyse onu kullanın
    }

    // Stamina yönetimi ve Koşma mantığı
    if (bIsSprinting && GetVelocity().SizeSquared() > 0 && !bIsExhausted) {
      // 15 ile 0 arasında stamina düşüşünü yavaşlat (saniyede 5 birim)
      float CurrentDepletionRate =
          (Stamina < 15.0f) ? 5.0f : StaminaDepletionRate;
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
    FpsCameraComponent->SetFieldOfView(
        FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, FOVInterpSpeed));
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

    // Post Process Ayarlarına Müdahale
    FPostProcessSettings &Settings = FpsCameraComponent->PostProcessSettings;
    Settings.bOverride_ColorSaturation = true;
    Settings.bOverride_SceneFringeIntensity = true;

    // FVector4 için yumuşak geçiş hesaplaması (Interp)
    FVector4 CurrentSaturation = Settings.ColorSaturation;
    if (CurrentSaturation.X == 0.0f && CurrentSaturation.W == 0.0f) {
      // Varsayılan değeri 1,1,1,1 kabul ediyoruz
      CurrentSaturation = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    // Saturation Geçişi: X, Y, Z renk kanallarını hedefe doğru yumuşat (W
    // kanalı opaklık/alpha)
    float InterpSat = FMath::FInterpTo(CurrentSaturation.X, TargetSaturation,
                                       DeltaTime, 1.5f);
    Settings.ColorSaturation = FVector4(InterpSat, InterpSat, InterpSat, 1.0f);

    // Fringe (Bulanıklık/Renk Kayması) Geçişi
    float CurrentFringe = Settings.SceneFringeIntensity;
    Settings.SceneFringeIntensity =
        FMath::FInterpTo(CurrentFringe, TargetFringe, DeltaTime, 1.5f);

    // Vignette Geçişi
    Settings.bOverride_VignetteIntensity = true;
    float CurrentVignette = Settings.VignetteIntensity;
    Settings.VignetteIntensity =
        FMath::FInterpTo(CurrentVignette, TargetVignette, DeltaTime, 1.5f);
  }

  // ==== DİNAMİK KAMERA SARSINTISI (CAMERA SHAKE) ====

  if (WalkingCameraShakeClass) {
    if (APlayerController *PlayerController =
            Cast<APlayerController>(GetController())) {
      float Speed = GetVelocity().Size();
      bool bIsOnGround =
          GetCharacterMovement() && GetCharacterMovement()->IsMovingOnGround();

      // Sadece yerdeyken ve karakter hareket ediyorsa sarsıntı oynatılır
      if (bIsOnGround && Speed > 0.0f) {

        // Hıza dayalı sarsıntı şiddeti, sarsıntıyı dinamik kılar
        // Varsayılan yürüme hızımız 300, Koşma hızımız 600 olduğuna göre:
        float ShakeScale = Speed / BaseWalkSpeed;

        // İsteğe bağlı ince ayar: Scale'i Clamp'leyerek çok absürt değerleri
        // önle
        ShakeScale = FMath::Clamp(ShakeScale, 0.1f, 2.0f);

        PlayerController->ClientStartCameraShake(WalkingCameraShakeClass,
                                                 ShakeScale);
      }
    }
  }

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

void AGercekCharacter::Interact() {
  // Guard: Nothing to interact with.
  if (!CurrentInteractable) {
    return;
  }

  // Guard: IsValid covers the case where the actor was just destroyed
  // (e.g. by a competing interaction or another system).
  if (!IsValid(CurrentInteractable)) {
    CurrentInteractable = nullptr;
    CurrentInteractItemName = TEXT("");
    return;
  }

  // Only proceed if the actor truly implements the interface.
  if (!CurrentInteractable->GetClass()->ImplementsInterface(
          UInteractable::StaticClass())) {
    return;
  }

  // --- SINGLE RESPONSIBILITY ---
  // WorldItemActor::OnInteract_Implementation owns:
  //   - FindComponentByClass<UInventoryComponent>()
  //   - AddItem()
  //   - Destroy()
  // We do NOT duplicate those calls here. Doing so causes a dangling
  // pointer crash when Destroy() is called on an already-destroyed actor.
  IInteractable::Execute_OnInteract(CurrentInteractable, this);

  // After the interact, the actor may or may not have been destroyed.
  // We clear our reference regardless — it's either gone or no longer targeted.
  CurrentInteractable = nullptr;
  CurrentInteractItemName = TEXT("");

  // Tüm listener'ları (WBP_Interact dahil) boş text ile haberdar et.
  // Sadece widget'ı gizlemek yetmez; delegate aracılığıyla text de
  // sıfırlanmalı, aksi hâlde listener'daki text label eski değeri göstermeye
  // devam edebilir.
  OnInteractTargetChanged.Broadcast(FText::GetEmpty());

  // Hide the interaction prompt widget.
  if (InteractWidget) {
    InteractWidget->SetVisibility(ESlateVisibility::Hidden);
  }

  // If the inventory widget is open, trigger a UI refresh (type-safe).
  if (IsValid(InventoryWidget)) {
    if (UInventoryWidget *TypedInventoryWidget =
            Cast<UInventoryWidget>(InventoryWidget)) {
      TypedInventoryWidget->RefreshInventory();
    } else {
      UFunction *RefreshFunc =
          InventoryWidget->FindFunction(TEXT("RefreshInventory"));
      if (RefreshFunc) {
        InventoryWidget->ProcessEvent(RefreshFunc, nullptr);
      }
    }
  }
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
      if (UInventoryWidget *TypedInventoryWidget =
              Cast<UInventoryWidget>(InventoryWidget)) {
        TypedInventoryWidget->SetInventoryComponent(InventoryComponent);
      }
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

    // Fareyi göster.
    PC->bShowMouseCursor = true;

    // Oyun InputAction'ları hâlâ çalışsın (Tab/Esc ile tekrar kapanabilsin),
    // ama mouse bakış girdisi Look() içindeki guard tarafından kesilir.
    FInputModeGameAndUI UIMode;
    // Widget'a focus ver — klavye eventi de UI'ya ulaşsın.
    UIMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
    // Tam ekranda farenin pencere dışına çıkmasını engelle.
    UIMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockInFullscreen);
    PC->SetInputMode(UIMode);

    // Debug: mevcut envanter içeriğini logla.
    ShowInventoryDetails();
  }
}

void AGercekCharacter::CheckForInteractables() {
  if (!FpsCameraComponent)
    return;

  FVector StartPosition = FpsCameraComponent->GetComponentLocation();
  FVector ForwardVector = FpsCameraComponent->GetForwardVector();
  FVector EndPosition = StartPosition + (ForwardVector * 500.0f);

  FHitResult HitResult;
  FCollisionQueryParams CollisionParams;
  CollisionParams.AddIgnoredActor(this);

  // ECC_Visibility — WorldItemActor'ın BlockAllDynamic profili bu kanalı
  // block eder; özel kanal (ECC_GameTraceChannel2) değil.
  GetWorld()->LineTraceSingleByChannel(HitResult, StartPosition, EndPosition,
                                       ECC_Visibility, CollisionParams);

  if (HitResult.bBlockingHit && IsValid(HitResult.GetActor())) {
    AActor *HitActor = HitResult.GetActor();

    if (HitActor->GetClass()->ImplementsInterface(
            UInteractable::StaticClass())) {
      // Only update when the target actually changes — avoid per-frame
      // Broadcast spam.
      if (HitActor != CurrentInteractable) {
        CurrentInteractable = HitActor;

        // Pull the display name from the interface.
        FText InteractName =
            IInteractable::Execute_GetInteractableName(CurrentInteractable);

        if (InteractName.IsEmpty()) {
          // Eğer özel bir isim dönmediyse sadece "E-Al" yaz
          CurrentInteractItemName = TEXT("E-Al");
        } else {
          // İsim döndüyse "E-Al [İsim]" formatında yaz (Kütüphaneden "Paslı
          // Çelik Zincir" gelirse "E-Al Paslı Çelik Zincir" olur)
          CurrentInteractItemName =
              FString::Printf(TEXT("E-Al %s"), *InteractName.ToString());
        }

        // Broadcast to all listeners (WBP_Interact, HUD, etc.)
        // Post-Apocalyptic Note: The world speaks through events, not polling.
        OnInteractTargetChanged.Broadcast(
            FText::FromString(CurrentInteractItemName));

        // Show the interact widget.
        if (InteractWidget) {
          InteractWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
      }
      return; // Target acquired — exit early.
    }
  }

  // Reached here: nothing interactable in range.
  // If we HAD a target before, clear it and notify listeners.
  if (CurrentInteractable != nullptr) {
    CurrentInteractable = nullptr;
    CurrentInteractItemName = TEXT("");

    // Broadcast empty FText — listener should hide the prompt text.
    OnInteractTargetChanged.Broadcast(FText::GetEmpty());

    if (InteractWidget) {
      InteractWidget->SetVisibility(ESlateVisibility::Hidden);
    }
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
    // Hurda: Radyasyonu azalt?r (AntiRad yerine junk kategorisinde
    // tutulabilir, ya da ilerleyen versiyonda ayr? bir AntiRad eklenebilir).
    Radiation -= Amount;
    Radiation = FMath::Clamp(Radiation, 0.0f, MaxRadiation);
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

float AGercekCharacter::GetWeightRatio() const {
  if (!InventoryComponent)
    return 0.0f;
  return InventoryComponent->GetCapacityRatio();
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

FText AGercekCharacter::GetInteractLabelText() const {
  if (CurrentInteractItemName.IsEmpty()) {
    return FText::GetEmpty();
  }
  return FText::FromString(CurrentInteractItemName);
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

  TArray<FInventorySlot> Items = InventoryComponent->GetInventoryForUI();

  if (Items.Num() == 0) {
    if (GEngine) {
      GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
                                       TEXT("Envanter Bos"));
    }
    UE_LOG(LogTemp, Warning, TEXT("Envanter Bos"));
    return;
  }

  for (const FInventorySlot &Slot : Items) {
    if (!Slot.IsValid())
      continue;

    const FItemDBRow *Row = Slot.GetRow();
    if (!Row)
      continue;

    FString Message = FString::Printf(TEXT("Esya: %s x%d | Agirlik: %.2f kg"),
                                      *Row->ItemName.ToString(), Slot.Quantity,
                                      Row->ItemWeight * Slot.Quantity);

    if (GEngine) {
      GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, Message);
    }
    UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
  }
}

// ==== ÇANTADAN KULLANMA VE YERE ATMA (USE & DROP) ====

void AGercekCharacter::UseItemFromInventory(const FDataTableRowHandle &ItemRowHandle) {
  if (ItemRowHandle.IsNull() || !InventoryComponent) return;

  const FItemDBRow *Row = ItemRowHandle.GetRow<FItemDBRow>(TEXT("AGercekCharacter::UseItemFromInventory"));
  if (!Row) return;

  if (Row->ItemType == EItemType::Food || Row->ItemType == EItemType::Med) {
    // Çantadan 1 adet silmeyi dener
    if (InventoryComponent->RemoveItem(ItemRowHandle, 1)) {
      // Amount olarak ItemValue kullan (İyileştirme miktarı)
      ConsumeItem(Row->ItemType, Row->ItemValue);
    }
  } else {
    UE_LOG(LogTemp, Warning, TEXT("[AGercekCharacter] Tüketilemeyen esya kullanilmaya calisildi: %s"), *Row->ItemName.ToString());
  }
}

void AGercekCharacter::DropItemFromInventory(const FDataTableRowHandle &ItemRowHandle) {
  if (ItemRowHandle.IsNull() || !InventoryComponent) return;

  const FItemDBRow *Row = ItemRowHandle.GetRow<FItemDBRow>(TEXT("AGercekCharacter::DropItemFromInventory"));
  if (!Row) return;

  if (Row->ItemType == EItemType::QuestItem || Row->ItemType == EItemType::Quest) {
    if (GEngine) {
      GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Görev eşyaları yere atılamaz."));
    }
    return;
  }

  // Çantadan 1 adet silmeyi dener
  if (InventoryComponent->RemoveItem(ItemRowHandle, 1)) {
    if (FpsCameraComponent) {
      // Karakterin baktığı yöne doğru 100 birim ileriye Drop lokasyonu hesapla
      FVector DropLocation = FpsCameraComponent->GetComponentLocation() + (FpsCameraComponent->GetForwardVector() * 100.0f);
      SpawnItemInWorld(ItemRowHandle, DropLocation);
    }
  }
}

AActor* AGercekCharacter::SpawnItemInWorld(const FDataTableRowHandle &ItemRowHandle, FVector SpawnLocation) {
  if (ItemRowHandle.IsNull() || !GetWorld()) return nullptr;

  FActorSpawnParameters SpawnParams;
  SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

  // AWorldItemActor oluştur 
  AWorldItemActor* SpawnedItem = GetWorld()->SpawnActor<AWorldItemActor>(AWorldItemActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);

  if (SpawnedItem) {
    SpawnedItem->InitializeItemData(ItemRowHandle);
  }

  return SpawnedItem;
}
