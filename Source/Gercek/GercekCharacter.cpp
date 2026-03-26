// Fill out your copyright notice in the Description page of Project Settings.

#include "GercekCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "CollisionQueryParams.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/AudioComponent.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/InputComponent.h"
#include "Components/PanelWidget.h"
#include "Components/PostProcessComponent.h"
#include "Components/TextBlock.h"
#include "DrawDebugHelpers.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/DataTable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GercekHostSaveGame.h"
#include "Interactable.h"
#include "Kismet/KismetTextLibrary.h"
#include "MerchantBase.h"
#include "Net/UnrealNetwork.h"
#include "PlayerInventoryComponent.h"
#include "PostApocInventoryGridWidget.h"
#include "PostApocInventoryFallbackWidget.h"
#include "PostApocInventoryTypes.h"
#include "PostApocGridItem.h"
#include "PostApocHUDWidget.h"
#include "PostApocItemTypes.h"
#include "PostApocTradeOfferWidgets.h"
#include "TimerManager.h"
#include "TradeComponent.h"
#include "LootContainerBase.h"
#include "WorldInventoryComponent.h"
#include "WorldItemActor.h"

namespace GercekInteraction {
constexpr ECollisionChannel TraceChannel = ECC_GameTraceChannel2;
}

namespace {

UPostApocInventoryGridWidget* ResolveInventoryGridWidget(UUserWidget* RootWidget) {
  if (!IsValid(RootWidget)) {
    return nullptr;
  }

  if (UPostApocInventoryGridWidget* DirectGrid =
          Cast<UPostApocInventoryGridWidget>(RootWidget)) {
    return DirectGrid;
  }

  static const FName CandidateNames[] = {
      TEXT("PlayerInventoryGrid"),
      TEXT("InventoryGridUI"),
      TEXT("PlayerGridUI"),
      TEXT("OyuncuCanta")};

  for (const FName& CandidateName : CandidateNames) {
    if (UWidget* FoundWidget = RootWidget->GetWidgetFromName(CandidateName)) {
      if (UPostApocInventoryGridWidget* GridWidget =
              Cast<UPostApocInventoryGridWidget>(FoundWidget)) {
        return GridWidget;
      }
    }
  }

  return nullptr;
}

UUserWidget* ResolveInventoryRefreshTarget(UUserWidget* RootWidget) {
  if (UPostApocInventoryGridWidget* GridWidget =
          ResolveInventoryGridWidget(RootWidget)) {
    return GridWidget;
  }

  return RootWidget;
}

TSubclassOf<UUserWidget> ResolvePreferredInventoryWidgetClass(
    TSubclassOf<UUserWidget> AssignedClass) {
  if (UClass* CanonicalInventoryWidgetClass =
          LoadClass<UUserWidget>(nullptr,
                                 TEXT("/Game/Gercek/Inventory/WBP_InventoryScreen.WBP_InventoryScreen_C"))) {
    return CanonicalInventoryWidgetClass;
  }

  if (AssignedClass &&
      !AssignedClass->IsChildOf(UPostApocGridItem::StaticClass())) {
    return AssignedClass;
  }

  return UPostApocInventoryFallbackWidget::StaticClass();
}

void EnsureInventoryBackground(UUserWidget* RootWidget,
                               UPostApocInventoryComponent* InventoryComponent) {
  if (!IsValid(RootWidget)) {
    return;
  }

  if (!RootWidget->IsA(UPostApocInventoryFallbackWidget::StaticClass())) {
    return;
  }

  RootWidget->SetRenderOpacity(1.0f);
  RootWidget->SetVisibility(ESlateVisibility::Visible);

  if (!RootWidget->WidgetTree) {
    return;
  }

  if (UWidget* TreeRoot = RootWidget->WidgetTree->RootWidget) {
    TreeRoot->SetVisibility(ESlateVisibility::Visible);
    TreeRoot->SetRenderOpacity(1.0f);
  }

  UPanelWidget* TargetPanel = nullptr;
  UCanvasPanel* CanvasPanel = nullptr;

  CanvasPanel = Cast<UCanvasPanel>(RootWidget->GetWidgetFromName(TEXT("GridCanvas")));
  if (!CanvasPanel) {
    if (UPostApocInventoryGridWidget* GridWidget =
            ResolveInventoryGridWidget(RootWidget)) {
      CanvasPanel = Cast<UCanvasPanel>(
          GridWidget->GetWidgetFromName(TEXT("GridCanvas")));
    }
  }

  if (CanvasPanel) {
    TargetPanel = CanvasPanel;
  } else {
    TargetPanel = Cast<UPanelWidget>(RootWidget->WidgetTree->RootWidget);
    CanvasPanel = Cast<UCanvasPanel>(TargetPanel);
  }

  if (!TargetPanel) {
    CanvasPanel = RootWidget->WidgetTree->ConstructWidget<UCanvasPanel>(
        UCanvasPanel::StaticClass(), TEXT("InventoryRuntimeRoot"));
    if (!CanvasPanel) {
      return;
    }
    RootWidget->WidgetTree->RootWidget = CanvasPanel;
    TargetPanel = CanvasPanel;
  }

  for (int32 ChildIndex = 0; ChildIndex < TargetPanel->GetChildrenCount();
       ++ChildIndex) {
    if (UWidget* ExistingChild = TargetPanel->GetChildAt(ChildIndex)) {
      if (ExistingChild->GetFName() == TEXT("InventoryBackground")) {
        return;
      }
    }
  }

  UBorder* Background = RootWidget->WidgetTree->ConstructWidget<UBorder>(
      UBorder::StaticClass(), TEXT("InventoryBackground"));
  if (!Background) {
    return;
  }

  Background->SetBrushColor(FLinearColor(0.02f, 0.02f, 0.02f, 0.85f));
  Background->SetVisibility(ESlateVisibility::Visible);

  UTextBlock* TitleText = RootWidget->WidgetTree->ConstructWidget<UTextBlock>(
      UTextBlock::StaticClass(), TEXT("InventoryTitle"));
  if (TitleText) {
    TitleText->SetText(FText::FromString(TEXT("ENVANTER")));
    TitleText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    Background->SetContent(TitleText);
  }

  if (CanvasPanel) {
    UCanvasPanelSlot* CanvasSlot = CanvasPanel->AddChildToCanvas(Background);
    if (!CanvasSlot) {
      return;
    }

    const float TileSize = InventoryComponent ? InventoryComponent->TileSize : 50.0f;
    const int32 Columns = InventoryComponent ? InventoryComponent->GridColumns : 10;
    const int32 Rows = InventoryComponent ? InventoryComponent->GridRows : 10;
    const FVector2D PanelSize(
        FMath::Max(350.0f, Columns * TileSize + 24.0f),
        FMath::Max(350.0f, Rows * TileSize + 24.0f));

    CanvasSlot->SetAutoSize(false);
    CanvasSlot->SetPosition(FVector2D(40.0f, 40.0f));
    CanvasSlot->SetSize(PanelSize);
    CanvasSlot->SetZOrder(-100);
    return;
  }

  TargetPanel->AddChild(Background);
}

} // namespace


// Sets default values
AGercekCharacter::AGercekCharacter() {
  // Set this character to call Tick() every frame.  You can turn this off to
  // improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;

  // Kamera oluÅŸturma
  FpsCameraComponent =
      CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
  FpsCameraComponent->SetupAttachment(RootComponent);
  FpsCameraComponent->bUsePawnControlRotation = true;

  // FPS Karakter Rotasyon Kilitleri (Kritik)
  bUseControllerRotationPitch = false;
  bUseControllerRotationYaw = true;
  bUseControllerRotationRoll = false;

  // Ses BileÅŸeni OluÅŸturma
  BreathingAudioComponent =
      CreateDefaultSubobject<UAudioComponent>(TEXT("BreathingAudioComponent"));
  BreathingAudioComponent->SetupAttachment(RootComponent);
  BreathingAudioComponent->bAutoActivate = false; // BaÅŸlangÄ±Ã§ta Ã§almasÄ±n

  // Grid (Tetris) TabanlÄ± Envanter BileÅŸeni BaÅŸlatma
  InventoryComponent = CreateDefaultSubobject<UPlayerInventoryComponent>(
      TEXT("InventoryComponent"));
  if (InventoryComponent) {
    InventoryComponent->GridColumns = 10;
    InventoryComponent->GridRows = 10;
    InventoryComponent->TileSize = 50.0f;

    if (!InventoryComponent->GridItemWidgetClass) {
      static ConstructorHelpers::FClassFinder<UUserWidget> GridItemWidgetBP(
          TEXT("/Game/Gercek/Inventory/WBP_GridItem"));
      if (GridItemWidgetBP.Succeeded()) {
        InventoryComponent->GridItemWidgetClass = GridItemWidgetBP.Class;
      }
    }

    if (!InventoryComponent->ItemDataTable) {
      static ConstructorHelpers::FObjectFinder<UDataTable> ItemDataTableAsset(
          TEXT("/Game/Gercek/Datas/PostApocItems.PostApocItems"));
      if (ItemDataTableAsset.Succeeded()) {
        InventoryComponent->ItemDataTable = ItemDataTableAsset.Object;
      }
    }
  }

  if (!DefaultMappingContext) {
    static ConstructorHelpers::FObjectFinder<UInputMappingContext>
        DefaultMappingContextAsset(TEXT("/Game/Gercek/IMC_Default.IMC_Default"));
    if (DefaultMappingContextAsset.Succeeded()) {
      DefaultMappingContext = DefaultMappingContextAsset.Object;
    }
  }

  if (!ToggleInventoryAction) {
    static ConstructorHelpers::FObjectFinder<UInputAction> InventoryActionAsset(
        TEXT("/Game/Gercek/IA_Inventory.IA_Inventory"));
    if (InventoryActionAsset.Succeeded()) {
      ToggleInventoryAction = InventoryActionAsset.Object;
    }
  }

  if (!InventoryWidgetClass) {
      static ConstructorHelpers::FClassFinder<UUserWidget> InventoryWidgetBP(
          TEXT("/Game/Gercek/Inventory/WBP_InventoryScreen"));
      if (InventoryWidgetBP.Succeeded()) {
        InventoryWidgetClass = InventoryWidgetBP.Class;
      }
  }

  if (!LootContainerWidgetClass) {
    static ConstructorHelpers::FClassFinder<UUserWidget> LootContainerWidgetBP(
        TEXT("/Game/Gercek/Inventory/WBP_LootContainerScreen"));
    if (LootContainerWidgetBP.Succeeded()) {
      LootContainerWidgetClass = LootContainerWidgetBP.Class;
    }
  }

  if (!TradeScreenClass) {
    static ConstructorHelpers::FClassFinder<UUserWidget> TradeScreenWidgetBP(
        TEXT("/Game/Gercek/Inventory/WBP_TradeScreen_YENI"));
    if (TradeScreenWidgetBP.Succeeded()) {
      TradeScreenClass = TradeScreenWidgetBP.Class;
    }
  }

  TradeComponent =
      CreateDefaultSubobject<UTradeComponent>(TEXT("TradeComponent"));

  DefaultFOV = 90.0f;
  SprintFOV = 110.0f;
  CrouchFOV = 80.0f;
  InjuredFOV = 70.0f; // Can %25 altÄ±ndayken daralacak FOV
  FOVInterpSpeed = 10.0f;

  Health = 100.0f;
  Radiation = 0.0f;
  Hunger = 100.0f;
  Thirst = 100.0f;
  MaxStamina = 100.0f;
  Stamina = MaxStamina;

  StaminaDepletionRate = 12.5f;
  StaminaRecoveryRate = 10.0f;
  MovingStaminaRecoveryMultiplier = 1.0f;
  IdleStaminaRecoveryMultiplier = 1.35f;
  CrouchStaminaRecoveryMultiplier = 1.25f;
  HungerDecreaseRate = 0.05f;
  ThirstDecreaseRate = 0.07f;
  WalkHungerConsumptionMultiplier = 1.0f;
  SprintHungerConsumptionMultiplier = 1.15f;
  CrouchHungerConsumptionMultiplier = 0.7f;
  WalkThirstConsumptionMultiplier = 1.0f;
  SprintThirstConsumptionMultiplier = 1.75f;
  CrouchThirstConsumptionMultiplier = 0.65f;

  bIsSprinting = false;
  bIsFatigued = false;
  bIsExhausted = false;
  bIsConsuming = false;
  bIsRecovering = false;

  RecoveryDelayTimer = 0.0f;

  OriginalStaminaRecoveryRate = 10.0f;

  LastJumpTime =
      -10.0f; // BaÅŸlangÄ±Ã§ta zÄ±plama beklemesini sÄ±fÄ±rlamak iÃ§in eksi bir deÄŸer

  // EÄŸilme (Crouch) Ã¶zelliÄŸini aktif et
  if (GetCharacterMovement()) {
    GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
    GetCharacterMovement()->MaxWalkSpeed = 300.0f; // Normal yÃ¼rÃ¼me hÄ±zÄ±
    GetCharacterMovement()->bOrientRotationToMovement =
        false; // FPS iÃ§in kapalÄ± olmalÄ±

    // BaÅŸlangÄ±Ã§ hÄ±z limitleri
    BaseWalkSpeed = 300.0f;
    SprintSpeed = 600.0f;
    InjuredSpeed = 150.0f;       // Can azken walk hÄ±zÄ±
    InjuredSprintSpeed = 300.0f; // Can azken sprint hÄ±zÄ±
  }

  // --- CO-OP ALTYAPISI (EKLEME) ---
  bReplicates = true;
  SetReplicateMovement(true);

  // Ticaret / XP BaÅŸlangÄ±Ã§
  TradeXP = 0.0f;
  CurrentKnowledge = ETradeKnowledge::Novice;

  // Son hasar alma zamanÄ±nÄ± baÅŸlangÄ±Ã§ta Ã§ok kÃ¼Ã§Ã¼k bir deÄŸere ayarla (hemen
  // regen baÅŸlasÄ±n)
  LastDamageTakenTime = -999.0f;
}

bool AGercekCharacter::ShouldBroadcastSurvivalValue(
    float CurrentValue, float LastBroadcastValue, bool bForce) const {
  return bForce || LastBroadcastValue < 0.0f ||
         FMath::Abs(CurrentValue - LastBroadcastValue) >=
             SurvivalBroadcastThreshold;
}

void AGercekCharacter::EmitSurvivalStatChangeEvents(bool bForce) {
  if (!IsLocallyControlled()) {
    return;
  }

  if (ShouldBroadcastSurvivalValue(Health, LastBroadcastHealth, bForce)) {
    OnHealthChanged.Broadcast(Health, MaxHealth);
    LastBroadcastHealth = Health;
  }

  const float EffectiveMaxStamina = GetEffectiveMaxStamina();
  if (ShouldBroadcastSurvivalValue(Stamina, LastBroadcastStamina, bForce)) {
    OnStaminaChanged.Broadcast(Stamina, EffectiveMaxStamina);
    LastBroadcastStamina = Stamina;
  }

  if (ShouldBroadcastSurvivalValue(Hunger, LastBroadcastHunger, bForce)) {
    OnHungerChanged.Broadcast(Hunger, MaxHunger);
    LastBroadcastHunger = Hunger;
  }

  if (ShouldBroadcastSurvivalValue(Thirst, LastBroadcastThirst, bForce)) {
    OnThirstChanged.Broadcast(Thirst, MaxThirst);
    LastBroadcastThirst = Thirst;
  }
}

void AGercekCharacter::BroadcastCurrentSurvivalStats(bool bForce) {
  EmitSurvivalStatChangeEvents(bForce);
}

void AGercekCharacter::EnsureLocalHUDCreated() {
  if (!IsLocallyControlled() || !PlayerHUDClass) {
    return;
  }

  APlayerController *PlayerController = Cast<APlayerController>(Controller);
  if (!PlayerController) {
    return;
  }

  if (!PlayerHUDWidget) {
    PlayerHUDWidget =
        CreateWidget<UPostApocHUDWidget>(PlayerController, PlayerHUDClass);
  }

  if (PlayerHUDWidget && !PlayerHUDWidget->IsInViewport()) {
    PlayerHUDWidget->AddToViewport();
  }

  if (PlayerHUDWidget) {
    PlayerHUDWidget->RefreshCharacterBinding();
  }
}

void AGercekCharacter::SyncPlayerInventoryToOwner() {
  if (!HasAuthority() || !InventoryComponent || IsLocallyControlled()) {
    return;
  }

  TArray<FGridSlotData> SyncedSlots;
  FString DataTablePath;
  InventoryComponent->ExportSaveData(SyncedSlots, DataTablePath);
  ClientSyncPlayerInventory(SyncedSlots, DataTablePath);
}

// --- REPLICATION KAYIT FONKSÄ°YONU (EKLEME) ---
void AGercekCharacter::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty> &OutLifetimeProps) const {
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(AGercekCharacter, Health);
  DOREPLIFETIME_CONDITION(AGercekCharacter, Radiation, COND_OwnerOnly);
  DOREPLIFETIME_CONDITION(AGercekCharacter, Stamina, COND_OwnerOnly);
  DOREPLIFETIME_CONDITION(AGercekCharacter, Hunger, COND_OwnerOnly);
  DOREPLIFETIME_CONDITION(AGercekCharacter, Thirst, COND_OwnerOnly);

  // Ticaret XP ve Bilgi Seviyesi
  DOREPLIFETIME_CONDITION(AGercekCharacter, TradeXP, COND_OwnerOnly);
  DOREPLIFETIME_CONDITION(AGercekCharacter, CurrentKnowledge,
                          COND_OwnerOnly);
}

// ==== TÄ°CARET VE BÄ°LGÄ° SEVÄ°YESÄ° (TRADE KNOWLEDGE) ====

void AGercekCharacter::RefreshTradeKnowledgeTierFromXP() {
  if (TradeXP >= 5000.0f) {
    CurrentKnowledge = ETradeKnowledge::Expert;
  } else if (TradeXP >= 2000.0f) {
    CurrentKnowledge = ETradeKnowledge::Apprentice;
  } else {
    CurrentKnowledge = ETradeKnowledge::Novice;
  }
}

void AGercekCharacter::AddTradeXP(float Amount) {
  // Sadece yetkili sunucuda Ã§alÄ±ÅŸÄ±r
  if (!HasAuthority())
    return;

  TradeXP += Amount;
  RefreshTradeKnowledgeTierFromXP();
}

void AGercekCharacter::BuildPersistentPlayerRecord(
    FGercekSavedPlayerRecord &OutRecord) const {
  OutRecord.PlayerLocation = GetActorLocation();
  OutRecord.PlayerRotation = GetActorRotation();
  OutRecord.Health = Health;
  OutRecord.Hunger = Hunger;
  OutRecord.Thirst = Thirst;
  OutRecord.Stamina = Stamina;
  OutRecord.Radiation = Radiation;
  OutRecord.TradeXP = TradeXP;
  OutRecord.CurrentKnowledge = CurrentKnowledge;
  OutRecord.LastSeenUtc = FDateTime::UtcNow();

  OutRecord.InventorySlots.Reset();
  OutRecord.InventoryDataTablePath.Reset();

  if (InventoryComponent) {
    TArray<FGridSlotData> SavedSlots;
    InventoryComponent->ExportSaveData(SavedSlots, OutRecord.InventoryDataTablePath);
    OutRecord.InventorySlots.Reserve(SavedSlots.Num());

    for (const FGridSlotData &SavedSlot : SavedSlots) {
      FGercekSavedGridSlot GridSlot;
      GridSlot.Location = SavedSlot.Location;
      GridSlot.ItemInstanceId = SavedSlot.ItemInstanceId;
      GridSlot.ItemId = SavedSlot.ItemRowName;
      GridSlot.bIsRotated = SavedSlot.bIsRotated;
      GridSlot.Condition = SavedSlot.Condition;
      GridSlot.FillState = SavedSlot.FillState;
      OutRecord.InventorySlots.Add(GridSlot);
    }
  }
}

void AGercekCharacter::ApplyPersistentPlayerRecord(
    const FGercekSavedPlayerRecord &InRecord) {
  if (!HasAuthority()) {
    return;
  }

  SetActorLocationAndRotation(InRecord.PlayerLocation, InRecord.PlayerRotation);

  Health = FMath::Clamp(InRecord.Health, 0.0f, MaxHealth);
  Hunger = FMath::Clamp(InRecord.Hunger, 0.0f, MaxHunger);
  Thirst = FMath::Clamp(InRecord.Thirst, 0.0f, MaxThirst);
  Stamina = FMath::Clamp(InRecord.Stamina, 0.0f, MaxStamina);
  Radiation = FMath::Clamp(InRecord.Radiation, 0.0f, MaxRadiation);
  TradeXP = FMath::Max(0.0f, InRecord.TradeXP);
  CurrentKnowledge = InRecord.CurrentKnowledge;
  RefreshTradeKnowledgeTierFromXP();
  Stamina = FMath::Clamp(Stamina, 0.0f, GetEffectiveMaxStamina());

  if (InventoryComponent) {
    TArray<FGridSlotData> SavedSlots;
    SavedSlots.Reserve(InRecord.InventorySlots.Num());

    for (const FGercekSavedGridSlot &SavedSlot : InRecord.InventorySlots) {
      FGridSlotData GridSlot;
      GridSlot.Location = SavedSlot.Location;
      GridSlot.ItemInstanceId = SavedSlot.ItemInstanceId;
      GridSlot.ItemRowName = SavedSlot.ItemId;
      GridSlot.bIsRotated = SavedSlot.bIsRotated;
      GridSlot.Condition = SavedSlot.Condition;
      GridSlot.FillState = SavedSlot.FillState;
      SavedSlots.Add(GridSlot);
    }

    UDataTable *LoadedDataTable = InventoryComponent->ItemDataTable;
    if (!InRecord.InventoryDataTablePath.IsEmpty()) {
      LoadedDataTable = LoadObject<UDataTable>(nullptr, *InRecord.InventoryDataTablePath);
    }

    InventoryComponent->ImportSaveData(SavedSlots, LoadedDataTable);
  }

  EmitSurvivalStatChangeEvents(true);
}

// Meryem ve Hazar iÃ§in not:
// Bu fonksiyon "const" olarak iÅŸaretlenmiÅŸtir, Ã§Ã¼nkÃ¼ karakterin state (durum)
// bilgisini deÄŸiÅŸtirmez; sadece girilen BaseValue argÃ¼manÄ±na ve oyuncunun
// sahip olduÄŸu Trade Knowledge XP seviyesine baÄŸlÄ± olarak okuma (read-only)
// yapar.
FText AGercekCharacter::GetKnowledgeAdjustedValue(float BaseValue) const {
  switch (CurrentKnowledge) {
  case ETradeKnowledge::Novice:
    // Acemi: DeÄŸeri hiÃ§ bilemez, soru iÅŸareti dÃ¶neriz.
    return FText::FromString(TEXT("???"));

  case ETradeKnowledge::Apprentice: {
    // Ã‡Ä±rak: %33 hata/yanÄ±lma payÄ± ile tahmin verebilir.
    int32 MinVal = FMath::RoundToInt(BaseValue * 0.67f);
    int32 MaxVal = FMath::RoundToInt(BaseValue * 1.33f);

    // Tahmin aralÄ±ÄŸÄ± (Min - Max) Ã¶rneÄŸin: "~ 75 - 125" formatÄ±.
    return FText::FromString(
        FString::Printf(TEXT("~ %d - %d"), MinVal, MaxVal));
  }

  case ETradeKnowledge::Expert: {
    // Uzman: Tam deÄŸeri noktasÄ± virgÃ¼lÃ¼ne hatasÄ±z bilir.
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

  // Oyun baÅŸÄ±nda nefes sesinin Ã§almamasÄ±nÄ± garantiye alÄ±yoruz
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

    // Sadece yerel oyuncuda etkileÅŸim ve kamera timer'larÄ± Ã§alÄ±ÅŸsÄ±n.
    GetWorld()->GetTimerManager().SetTimer(
        InteractionTimerHandle, this,
        &AGercekCharacter::PerformInteractionTracePeriodic, InteractionTraceInterval, true);
    GetWorld()->GetTimerManager().SetTimer(
        CameraShakeTimerHandle, this, &AGercekCharacter::PlayCameraShakePeriodic,
        0.35f, true);

    EnsureLocalHUDCreated();

    EmitSurvivalStatChangeEvents(true);
  }
}

void AGercekCharacter::PawnClientRestart() {
  Super::PawnClientRestart();

  if (APlayerController *PlayerController =
          Cast<APlayerController>(GetController())) {

    // Oyun/UI odak sorunlarÄ±nÄ± engellemek iÃ§in fare ve girdi modunu zorla
    // GameOnly yap.
    FInputModeGameOnly GameMode;
    PlayerController->SetInputMode(GameMode);
    PlayerController->bShowMouseCursor = false;

    if (UEnhancedInputLocalPlayerSubsystem *Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
                PlayerController->GetLocalPlayer())) {
      // SÄ±kÄ±ÅŸmÄ±ÅŸ ve eski tuÅŸ atamalarÄ±nÄ± (stuck states) temizle
      Subsystem->ClearAllMappings();

      if (DefaultMappingContext) {
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
        UE_LOG(LogTemp, Display,
               TEXT("[EnhancedInput] DefaultMappingContext forcibly injected "
                    "and UI Input cleared via C++."));
      }
    }
  }

  EnsureLocalHUDCreated();
  EmitSurvivalStatChangeEvents(true);
}

// Called every frame
void AGercekCharacter::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // Sunucu (Server) yetkili stat gÃ¼ncellemeleri
  if (HasAuthority()) {
    // Zamanla aÃ§lÄ±k ve susuzluk azalmasÄ±
    const float CurrentHungerDecreaseRate =
        HungerDecreaseRate * GetCurrentHungerConsumptionMultiplier();
    const float CurrentThirstDecreaseRate =
        ThirstDecreaseRate * GetCurrentThirstConsumptionMultiplier();

    if (Hunger > 0.0f) {
      Hunger -= DeltaTime * CurrentHungerDecreaseRate;
      Hunger = FMath::Clamp(Hunger, 0.0f, 100.0f);
    } else {
      Health -= HungerHealthDecayRate * DeltaTime;
      Health = FMath::Clamp(Health, 0.0f, MaxHealth);
    }

    if (Thirst > 0.0f) {
      Thirst -= DeltaTime * CurrentThirstDecreaseRate;
      Thirst = FMath::Clamp(Thirst, 0.0f, 100.0f);
    }

    if (Radiation >= MaxRadiation) {
      Health -= RadiationHealthDecayRate * DeltaTime;
      Health = FMath::Clamp(Health, 0.0f, MaxHealth);
    }

    // ==== SAÄLIK YENÄ°LENMESÄ° (KADEMELÄ° DÄ°ZAYN) ====
    // Kural 1: Can 50'nin Ã¼zyerindeyse â€” otomatik yenilenme (saniyede 0.5 hp)
    // Kural 2: Can 50 veya altÄ±ndaysa â€” Otomatik yenilenme YOKTUR.
    //   * EÅŸya kullanÄ±ldÄ±ÄŸÄ±nda (ApplyItemEffect::Med) regen gersÃ§ekleÅŸir.
    //   * SON 15 SANÄ°YEDE HÄ°Ã‡ HASAR ALINMAZSA yenilenme baÅŸlar.
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    const float TimeSinceDamage = CurrentTime - LastDamageTakenTime;

    const bool bWellFed =
        Hunger > 75.0f && Thirst > 75.0f && Radiation < MaxRadiation;

    if (Health > 50.0f) {
      // --- Can 50 Ãœzeri: Otomatik regen, tokluk ve radyasyon peÅŸi sÄ±ra ---
      if (bWellFed) {
        Health += 0.5f * DeltaTime;
        Health = FMath::Clamp(Health, 0.0f, MaxHealth);
      }
    } else {
      // --- Can 50 AltÄ±: Sadece 15 saniye hasar almamÄ±ÅŸsa regen aÃ§Ä±lÄ±r ---
      if (bWellFed && TimeSinceDamage >= 15.0f) {
        Health += 0.5f * DeltaTime;
        Health = FMath::Clamp(Health, 0.0f, MaxHealth);
      }
    }

    // GerÃ§ekÃ§i zÄ±plama: Tek zÄ±plama her durumda sabit.
    // Can 25 altÄ±nda bile JumpMaxCount = 1 â€” Ã§ift zÄ±plama yok.
    JumpMaxCount = 1;

    // Stamina yÃ¶netimi ve KoÅŸma mantÄ±ÄŸÄ±
    if (bIsSprinting && GetVelocity().SizeSquared() > 0 && !bIsExhausted) {
      // 15 ile 0 arasÄ±nda stamina dÃ¼ÅŸÃ¼ÅŸÃ¼nÃ¼ yavaÅŸlat (saniyede 5 birim)
      // Can 25'in altÄ±ndaysa yaralÄ± durum: tÃ¼kenme hÄ±zÄ± 20f'e Ã§Ä±kar
      const float BaseDepletionRate = (Health < 25.0f) ? 20.0f : StaminaDepletionRate;
      float CurrentDepletionRate =
          (Stamina < 15.0f) ? 5.0f : BaseDepletionRate;
      Stamina -= DeltaTime * CurrentDepletionRate;
      Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);

      if (Stamina <= 0.0f) {
        Stamina = 0.0f;
        bIsSprinting = false;
        bIsExhausted = true;
        bIsRecovering = true;
        RecoveryDelayTimer =
            1.50f; // Tam bitkinlik durumunda 1.50 saniye bekleme cezasÄ±
      }
    } else {
      // Stamina Yenilenmesi
      float TimeSinceLastJump = GetWorld()->GetTimeSeconds() - LastJumpTime;

      const float EffectiveMaxStamina = GetEffectiveMaxStamina();
      if (Stamina < EffectiveMaxStamina && TimeSinceLastJump > 1.0f) {

        // EÄŸer bekleme sÃ¼resindeysek sadece sayacÄ± azalt ve doluma izin verme
        if (bIsRecovering) {
          RecoveryDelayTimer -= DeltaTime;
          if (RecoveryDelayTimer <= 0.0f) {
            bIsRecovering = false; // SÃ¼re doldu, artÄ±k doluma baÅŸlayabiliriz
          }
        } else {
          // Kademeli ArtÄ±ÅŸ (Stamina dolarken mevcut deÄŸeri kontrol et)
          float CurrentRegenRate = StaminaRecoveryRate; // Normal hÄ±z (Ã–rn: 10)

          // Kritik BÃ¶lge: EÄŸer 20'nin altÄ±ndaysa Ã§ok daha yavaÅŸ dolar
          if (Stamina < 20.0f) {
            CurrentRegenRate = 5.0f;
          }

          CurrentRegenRate *= GetCurrentStaminaRecoveryMultiplier();

          Stamina += DeltaTime * CurrentRegenRate;
          Stamina = FMath::Clamp(Stamina, 0.0f, EffectiveMaxStamina);

          // Kilit MekanizmasÄ±: 21 birime ulaÅŸana kadar karakter yorgunluktan
          // kurtulamaz
          if (Stamina >= 21.0f) {
            bIsExhausted = false;
            bIsFatigued = false;
          }
        }
      }
    }

    Stamina = FMath::Clamp(Stamina, 0.0f, GetEffectiveMaxStamina());

    // ==== HAREKET HIZI (SPEED) YÃ–NETÄ°MÄ° ====
    if (GetCharacterMovement()) {
      float TargetSpeed = BaseWalkSpeed;

      // Can durumuna gÃ¶re hedef hÄ±zÄ± belirle
      if (bIsConsuming) {
        TargetSpeed = BaseWalkSpeed / 2.0f; // Ä°Ã§erken hÄ±z yarÄ±ya dÃ¼ÅŸer
      } else if (Health < 25.0f) {
        // YaralÄ± durumu
        if (bIsSprinting && !bIsExhausted) {
          TargetSpeed = (Stamina > 20.0f) ? InjuredSprintSpeed : InjuredSpeed;
        } else {
          TargetSpeed = InjuredSpeed;
        }
      } else {
        // Normal durum - Kademe MantÄ±ÄŸÄ±
        if (bIsExhausted) {
          TargetSpeed =
              300.0f; // 0 NoktasÄ± ve bekleme sÃ¼resi: Mecburi YÃ¼rÃ¼me kilitli
        } else if (bIsSprinting && Stamina >= 21.0f) {
          TargetSpeed = 600.0f; // 21-100: KoÅŸu
        } else if (bIsSprinting && Stamina > 0.0f && Stamina < 21.0f) {
          TargetSpeed = 400.0f; // 1-20: Yorgun KoÅŸu
        } else {
          TargetSpeed = 300.0f; // VarsayÄ±lan yÃ¼rÃ¼me
        }
      }

      // Stamina cezasÄ± 1: Stamina 20'nin altÄ±na dÃ¼ÅŸtÃ¼ÄŸÃ¼nde yÃ¼rÃ¼me hÄ±zÄ± otomatik
      // olarak %30 yavaÅŸlatÄ±lÄ±r. Eger exhausted (0 stamina kilitli ceza) ise
      // zaten 300'e sabitlenecektir.
      if (Stamina < 20.0f && !bIsExhausted && TargetSpeed > 0) {
        TargetSpeed *= 0.7f;
      }

      float CurrentSpeed = GetCharacterMovement()->MaxWalkSpeed;
      TargetSpeed *= MovementSpeedMultiplier; // Agirlik kaynakli hiz kancasi

      if (bIsExhausted) {
        // Stamina cezasi 2: 1.55 saniye boyunca karakter hizi %100 ceza 300'e
        // (agirhk carpanindan sonra) sabitlenir. Interp kullanÄ±lmaz, anÄ±nda
        // kilitlenir.
        GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;
      } else {
        // FInterpTo ile anlÄ±k hÄ±zÄ± hedef hÄ±za yumuÅŸak (smooth) ÅŸekilde geÃ§ir
        GetCharacterMovement()->MaxWalkSpeed =
            FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, 3.0f);
      }
    }
  }

  const bool bInventoryVisible =
      IsValid(InventoryWidget) &&
      InventoryWidget->GetVisibility() == ESlateVisibility::Visible;
  if (IsLocallyControlled() && Hunger <= 0.0f && Controller != nullptr &&
      !bInventoryVisible && !IsValid(ActiveTradeWidget) &&
      !IsValid(ActiveLootContainerWidget)) {
    HungerAimSwayTime += DeltaTime * StarvationAimSwaySpeed;
    AddControllerYawInput(FMath::Sin(HungerAimSwayTime) *
                          StarvationAimSwayStrength);
    AddControllerPitchInput(FMath::Cos(HungerAimSwayTime * 0.7f) *
                            StarvationAimSwayStrength * 0.6f);
  }

  // ==== DÄ°NAMÄ°K FOV ====
  if (FpsCameraComponent) {
    float TargetFOV = DefaultFOV;

    if (Health < 25.0f) {
      TargetFOV = InjuredFOV; // Can azken tÃ¼nel gÃ¶rÃ¼ÅŸÃ¼ (FOV daralmasÄ±)
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

  // ==== HASAR GÃ–RSEL EFEKTLERÄ° (POST-PROCESS) ====
  if (FpsCameraComponent) {
    float TargetSaturation = 1.0f;
    float TargetFringe = 0.0f;   // SceneFringeIntensity (Blur etkisi yaratÄ±r)
    float TargetVignette = 0.4f; // VarsayÄ±lan Vignette Intensity

    if (Health < 25.0f) {
      TargetSaturation = 0.0f; // Tamamen siyah/beyaz
      TargetFringe = 4.0f;     // YÃ¼ksek ÅŸiddetli Color Fringe
      TargetVignette = 1.0f;   // YoÄŸun Vignette
    } else if (Health < 50.0f) {
      TargetSaturation = 0.5f; // YarÄ± soluk renk
      TargetFringe = 2.0f;     // Hafif bulanÄ±klÄ±k
      TargetVignette = 0.6f;   // Hafif Vignette
    }

    FPostProcessSettings &Settings = FpsCameraComponent->PostProcessSettings;

    // FVector4 iÃ§in yumuÅŸak geÃ§iÅŸ hesaplamasÄ± (Interp)
    FVector4 CurrentSaturation = Settings.ColorSaturation;
    if (CurrentSaturation.X == 0.0f && CurrentSaturation.W == 0.0f) {
      // VarsayÄ±lan deÄŸeri 1,1,1,1 kabul ediyoruz
      CurrentSaturation = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    // Optimizasyon: Sadece gerekli olduÄŸunda efekti uygula
    if (!FMath::IsNearlyEqual(CurrentSaturation.X, TargetSaturation, 0.01f) ||
        !FMath::IsNearlyEqual(Settings.SceneFringeIntensity, TargetFringe, 0.01f) ||
        !FMath::IsNearlyEqual(Settings.VignetteIntensity, TargetVignette, 0.01f)) {

      Settings.bOverride_ColorSaturation = true;
      Settings.bOverride_SceneFringeIntensity = true;
      Settings.bOverride_VignetteIntensity = true;

      // Saturation GeÃ§iÅŸi
      float InterpSat = FMath::FInterpTo(CurrentSaturation.X, TargetSaturation, DeltaTime, 1.5f);
      Settings.ColorSaturation = FVector4(InterpSat, InterpSat, InterpSat, 1.0f);

      // Fringe GeÃ§iÅŸi
      float CurrentFringe = Settings.SceneFringeIntensity;
      Settings.SceneFringeIntensity = FMath::FInterpTo(CurrentFringe, TargetFringe, DeltaTime, 1.5f);

      // Vignette GeÃ§iÅŸi
      float CurrentVignette = Settings.VignetteIntensity;
      Settings.VignetteIntensity = FMath::FInterpTo(CurrentVignette, TargetVignette, DeltaTime, 1.5f);
    }
  }

  // Dinamik Kamera SarsÄ±ntÄ±sÄ± (Camera Shake) optimizasyon amacÄ±yla Tick'ten Ã§Ä±karÄ±ldÄ±.
  // GerÃ§ekleÅŸtirimi Timer'a taÅŸÄ±ndÄ±.

  // ==== STAMINA YORGUNLUK SESÄ° (BREATHING) ====

  // GeliÅŸmiÅŸ Ses KontrolÃ¼: 15'in altÄ±nda koÅŸarken VEYA 1.5 saniyelik tÃ¼kenme
  // cezasÄ± sÄ±rasÄ±nda devam etsin
  if ((Stamina < 15.0f && bIsSprinting && GetVelocity().SizeSquared() > 0 &&
       !bIsExhausted) ||
      bIsRecovering) {
    // Ses bileÅŸeni atanmÄ±ÅŸsa Ã§almayÄ± baÅŸlat
    if (BreathingAudioComponent) {
      if (BreathingSound && BreathingAudioComponent->Sound != BreathingSound) {
        BreathingAudioComponent->SetSound(BreathingSound);
      }

      // Sadece ve sadece ÅŸu an Ã§almÄ±yorsa baÅŸlat (Ãœst Ã¼ste binmeyi engeller)
      if (!BreathingAudioComponent->IsPlaying()) {
        BreathingAudioComponent->Play();
      }
    }

  } else {
    // Gecikme bittikten sonra (Stamina dolmaya baÅŸladÄ±ÄŸÄ±nda) veya koÅŸmayÄ±
    // bÄ±raktÄ±ÄŸÄ±nda sesi 1 saniyede sÃ¶nÃ¼mleyerek durdur (FadeOut)
    if (BreathingAudioComponent && BreathingAudioComponent->IsPlaying()) {
      BreathingAudioComponent->FadeOut(1.0f, 0.0f);
    }
  }

  EmitSurvivalStatChangeEvents(false);
}

// Called to bind functionality to input
void AGercekCharacter::SetupPlayerInputComponent(
    UInputComponent *PlayerInputComponent) {
  Super::SetupPlayerInputComponent(PlayerInputComponent);

  bool bInventoryBoundWithEnhancedInput = false;

  if (UEnhancedInputComponent *EnhancedInputComponent =
          Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
    // Sprint (KoÅŸma)
    if (SprintAction) {
      EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started,
                                         this, &AGercekCharacter::StartSprint);
      EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed,
                                         this, &AGercekCharacter::StopSprint);
    }

    // EtkileÅŸim (Interact)
    if (InteractAction) {
      EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started,
                                         this, &AGercekCharacter::Interact);
    }

    // Envanteri AÃ§/Kapat (Tab)
    if (ToggleInventoryAction) {
      EnhancedInputComponent->BindAction(ToggleInventoryAction,
                                         ETriggerEvent::Started, this,
                                         &AGercekCharacter::ToggleInventory);
      bInventoryBoundWithEnhancedInput = true;
    }

    // ZÄ±plama eylemi
    if (JumpAction) {
      EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started,
                                         this, &AGercekCharacter::Jump);
      EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed,
                                         this, &AGercekCharacter::StopJumping);
    }

    // Crouch (EÄŸilme)
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

    // Kamera KontrolÃ¼ (Look)
    if (LookAction) {
      EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered,
                                         this, &AGercekCharacter::Look);
    }
  }

  // Legacy input only as fallback; otherwise Tab fires twice and re-closes UI.
  if (!bInventoryBoundWithEnhancedInput) {
    PlayerInputComponent->BindAction("EnvanterAc", IE_Pressed, this,
                                     &AGercekCharacter::ToggleInventory);
  }

  // EtkileÅŸim (Interact) eylemini baÄŸla (Project Settings -> Input -> Action
  // Mappings)
  PlayerInputComponent->BindAction("Interact", IE_Pressed, this,
                                   &AGercekCharacter::Interact);
}

void AGercekCharacter::StartSprint() {
  if (Stamina > 0.0f && !bIsCrouched && !bIsFatigued) {
    bIsSprinting = true;
    if (!HasAuthority()) {
      ServerSetSprinting(true);
    }
    // HÄ±z geÃ§iÅŸi Tick iÃ§erisinde FInterpTo ile yapÄ±lÄ±yor.
  }
}

void AGercekCharacter::StopSprint() {
  bIsSprinting = false;
  if (!HasAuthority()) {
    ServerSetSprinting(false);
  }
  // HÄ±z geÃ§iÅŸi Tick iÃ§erisinde FInterpTo ile yapÄ±lÄ±yor.
}

void AGercekCharacter::StartCrouch() {
  if (bIsSprinting) {
    StopSprint();
  }

  Crouch();
}

void AGercekCharacter::StopCrouch() {
  UnCrouch();
}

void AGercekCharacter::Jump() {
  // GerÃ§ekÃ§ilik kuralÄ±: Sadece yerde iken zÄ±planabilir.
  // IsFalling() veya !IsMovingOnGround() ile hava zÄ±plamasÄ± tamamen kapatÄ±lÄ±r.
  if (!GetCharacterMovement() || !GetCharacterMovement()->IsMovingOnGround()) {
    return;
  }

  // Stamina kontrolÃ¼ (En az 10 stamina gerekli)
  if (Stamina >= 10.0f) {
    // Stamina maliyeti
    Stamina -= 10.0f;
    Stamina = FMath::Clamp(Stamina, 0.0f, GetEffectiveMaxStamina());

    // Gecikme sÃ¼resi iÃ§in anlÄ±k zamanÄ± kaydet
    LastJumpTime = GetWorld()->GetTimeSeconds();

    // Motorun normal zÄ±plama kodunu Ã§aÄŸÄ±r
    Super::Jump();
  }
}

// --- YENÄ° UZMAN (AAA) ETKÄ°LEÅÄ°M SÄ°STEMÄ° ---
AActor *AGercekCharacter::PerformInteractionTrace() const {
  if (!FpsCameraComponent)
    return nullptr;

  FVector StartPos = FpsCameraComponent->GetComponentLocation();
  FVector EndPos =
      StartPos + (FpsCameraComponent->GetForwardVector() * InteractionTraceDistance);

  FHitResult HitResult;
  FCollisionQueryParams Params;
  Params.AddIgnoredActor(this);

  if (GetWorld()->LineTraceSingleByChannel(HitResult, StartPos, EndPos,
                                           GercekInteraction::TraceChannel,
                                           Params)) {
    AActor *HitActor = HitResult.GetActor();
    if (IsValid(HitActor) &&
        HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass())) {
      return HitActor;
    }
  }
  return nullptr;
}

FText AGercekCharacter::GetInteractionPrompt() const {
  // Optimizasyon: Her frame (Tick) hesaplamak yerine, timer ile 0.1s de bir atÄ±lan
  // LineTrace'in sonucunu (Ã¶nbellek) dÃ¶ndÃ¼r.
  return CachedInteractionPrompt;
}

void AGercekCharacter::PerformInteractionTracePeriodic() {
  AActor *HitActor = PerformInteractionTrace();
  CachedInteractableActor = HitActor;

  if (!HitActor) {
    CachedInteractionPrompt = FText::GetEmpty();
    return;
  }

  CachedInteractionPrompt =
      IInteractable::Execute_GetInteractionPrompt(HitActor, this);
  if (CachedInteractionPrompt.IsEmpty()) {
    CachedInteractionPrompt = FText::GetEmpty();
  }
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
  if (ActiveLootContainerWidget != nullptr) {
    CloseLootContainer();
    return;
  }
  if (ActiveTradeWidget != nullptr) {
    CloseTradeScreen();
    return;
  }

  AActor *HitActor =
      IsValid(CachedInteractableActor) ? CachedInteractableActor : PerformInteractionTrace();
  if (!IsValid(HitActor) ||
      !HitActor->GetClass()->ImplementsInterface(UInteractable::StaticClass())) {
    return;
  }

  if (HasAuthority()) {
    IInteractable::Execute_OnInteract(HitActor, this);
  } else {
    ServerInteract(HitActor);
  }
}

void AGercekCharacter::ServerInteract_Implementation(AActor *TargetActor) {
  AActor *VerifiedActor = PerformInteractionTrace();
  if (!IsValid(VerifiedActor) ||
      !VerifiedActor->GetClass()->ImplementsInterface(UInteractable::StaticClass())) {
    return;
  }

  if (IsValid(TargetActor) && VerifiedActor != TargetActor) {
    return;
  }

  if (GetDistanceTo(VerifiedActor) > InteractionTraceDistance) {
    return;
  }

  IInteractable::Execute_OnInteract(VerifiedActor, this);
}

void AGercekCharacter::ClientOpenTradeScreen_Implementation(
    AMerchantBase *TargetMerchant) {
  if (IsValid(TargetMerchant)) {
    OpenTradeScreen(TargetMerchant);
  }
}

void AGercekCharacter::ClientOpenLootContainer_Implementation(
    ALootContainerBase *TargetContainer) {
  if (IsValid(TargetContainer)) {
    OpenLootContainer(TargetContainer);
  }
}

void AGercekCharacter::ClientSyncPlayerInventory_Implementation(
    const TArray<FGridSlotData> &SyncedSlots, const FString &DataTablePath) {
  if (!InventoryComponent) {
    return;
  }

  UDataTable *LoadedDataTable = InventoryComponent->ItemDataTable;
  if (!DataTablePath.IsEmpty()) {
    LoadedDataTable = LoadObject<UDataTable>(nullptr, *DataTablePath);
  }

  InventoryComponent->ImportSaveData(SyncedSlots, LoadedDataTable);
}

void AGercekCharacter::ToggleInventory() {
  APlayerController *PC = Cast<APlayerController>(GetController());
  if (!PC || !PC->IsLocalController()) {
    UE_LOG(LogTemp, Warning,
           TEXT("[Inventory] ToggleInventory aborted: invalid or non-local controller."));
    return;
  }

  UE_LOG(LogTemp, Display, TEXT("[Inventory] ToggleInventory called on %s"),
         *GetNameSafe(this));

  const TSubclassOf<UUserWidget> PreferredInventoryWidgetClass =
      ResolvePreferredInventoryWidgetClass(InventoryWidgetClass);

  UE_LOG(LogTemp, Display,
         TEXT("[Inventory] AssignedClass=%s PreferredClass=%s"),
         *GetNameSafe(InventoryWidgetClass),
         *GetNameSafe(PreferredInventoryWidgetClass));

  if (!PreferredInventoryWidgetClass) {
    UE_LOG(LogTemp, Error,
           TEXT("[Inventory] No valid inventory widget class resolved."));
    if (GEngine) {
      GEngine->AddOnScreenDebugMessage(
          -1, 4.0f, FColor::Red,
          TEXT("Inventory error: widget class bulunamadi."));
    }
    return;
  }

  // Widget ilk kez aÃ§Ä±lÄ±yorsa oluÅŸtur (lazy init â€” sadece bir kez).
  if (!IsValid(InventoryWidget) && PreferredInventoryWidgetClass) {
    InventoryWidget =
        CreateWidget<UUserWidget>(GetWorld(), PreferredInventoryWidgetClass);
    if (IsValid(InventoryWidget)) {
      UE_LOG(LogTemp, Display, TEXT("[Inventory] Inventory widget created: %s"),
             *GetNameSafe(InventoryWidget));
      InventoryWidget->AddToViewport(10);
      if (InventoryWidget->IsA(UPostApocInventoryFallbackWidget::StaticClass())) {
        InventoryWidget->SetPositionInViewport(FVector2D(60.0f, 60.0f), false);
        InventoryWidget->SetDesiredSizeInViewport(FVector2D(620.0f, 620.0f));
      }
      InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
    } else {
      UE_LOG(LogTemp, Error, TEXT("[Inventory] Failed to create inventory widget."));
      if (GEngine) {
        GEngine->AddOnScreenDebugMessage(
            -1, 4.0f, FColor::Red,
            TEXT("Inventory error: widget create basarisiz."));
      }
    }
  }

  if (!IsValid(InventoryWidget)) {
    UE_LOG(LogTemp, Error, TEXT("[Inventory] InventoryWidget invalid after creation."));
    return;
  }

  const bool bIsOpen =
      InventoryWidget->GetVisibility() == ESlateVisibility::Visible;

  if (bIsOpen) {
    // ---- KAPAT ------------------------------------------------
    InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
    UE_LOG(LogTemp, Display, TEXT("[Inventory] Inventory closed."));

    // Fareyi gizle, kontrolÃ¼ tamamen oyuna ver.
    PC->bShowMouseCursor = false;

    FInputModeGameOnly GameMode;
    PC->SetInputMode(GameMode);
  } else {
    // ---- AÃ‡ ---------------------------------------------------
    InventoryWidget->SetVisibility(ESlateVisibility::Visible);
    if (InventoryWidget->IsA(UPostApocInventoryFallbackWidget::StaticClass())) {
      InventoryWidget->SetPositionInViewport(FVector2D(60.0f, 60.0f), false);
      InventoryWidget->SetDesiredSizeInViewport(FVector2D(620.0f, 620.0f));
    }
    UE_LOG(LogTemp, Display, TEXT("[Inventory] Inventory opened."));

    if (UPostApocInventoryGridWidget *InventoryGridWidget =
            ResolveInventoryGridWidget(InventoryWidget)) {
      InventoryGridWidget->InitializeGridContext(
          InventoryComponent, this, nullptr,
          EPostApocInventoryGridRole::PlayerInventory);
      InventoryGridWidget->SetUseDefaultItemActions(true);
      UE_LOG(LogTemp, Display, TEXT("[Inventory] Grid context initialized."));
    } else {
      UE_LOG(LogTemp, Warning,
             TEXT("[Inventory] No inventory grid widget found inside root widget."));
    }

    EnsureInventoryBackground(InventoryWidget, InventoryComponent);

    if (InventoryComponent) {
      InventoryComponent->NativeRefreshUI(
          ResolveInventoryRefreshTarget(InventoryWidget));
      UE_LOG(LogTemp, Display, TEXT("[Inventory] NativeRefreshUI called."));
    } else {
      UE_LOG(LogTemp, Error, TEXT("[Inventory] InventoryComponent missing."));
    }

    // Fareyi gÃ¶ster.
    PC->bShowMouseCursor = true;

    // TAB ile tekrar kapatabilmek iÃ§in oyun girdisini tamamen kesmeyelim.
    FInputModeGameAndUI UIMode;
    UIMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockInFullscreen);
    UIMode.SetHideCursorDuringCapture(false);
    PC->SetInputMode(UIMode);

    // Debug: mevcut envanter iÃ§eriÄŸini logla.
    ShowInventoryDetails();
  }
}

// ==== EÅYA TÃœKETÄ°MÄ° (CONSUMABLES) ====
void AGercekCharacter::ConsumeItem(EItemType Type, float Amount) {
  if (bIsConsuming) {
    return; // Zaten bir ÅŸey tÃ¼ketiyorsak iÅŸlemi reddet
  }

  // Animasyon veya baÅŸka etkiler iÃ§in bayraÄŸÄ± aÃ§Ä±yoruz
  bIsConsuming = true;

  // TÃ¼ketim sÃ¼resi (Ã–rn. 2 saniye sonra etki etsin)
  float ConsumeDelay = 2.0f;

  FTimerDelegate TimerDel;
  TimerDel.BindUObject(this, &AGercekCharacter::ApplyItemEffect, Type, Amount);

  GetWorld()->GetTimerManager().SetTimer(ConsumeTimerHandle, TimerDel,
                                         ConsumeDelay, false);
}

float AGercekCharacter::ResolveConsumableAmount(
    const FPostApocItemRow& ItemRow, const EConsumableFillState FillState) const {
  const float BaseAmount =
      ItemRow.ConsumeAmount > 0.0f ? ItemRow.ConsumeAmount
                                   : static_cast<float>(ItemRow.BaseValue);
  if (ItemRow.Category == EPostApocItemCategory::Food ||
      ItemRow.Category == EPostApocItemCategory::Drink) {
    return FillState == EConsumableFillState::HalfFull ? BaseAmount * 0.5f
                                                       : BaseAmount;
  }

  return BaseAmount;
}

float AGercekCharacter::GetEffectiveMaxStamina() const {
  return Thirst <= 0.0f ? MaxStamina * 0.5f : MaxStamina;
}

float AGercekCharacter::GetCurrentStaminaRecoveryMultiplier() const {
  float RecoveryMultiplier = MovingStaminaRecoveryMultiplier;
  const bool bIsStationary = GetVelocity().SizeSquared() <= 1.0f;

  if (bIsStationary) {
    RecoveryMultiplier *= IdleStaminaRecoveryMultiplier;
  }

  if (bIsCrouched) {
    RecoveryMultiplier *= CrouchStaminaRecoveryMultiplier;
  }

  return RecoveryMultiplier;
}

float AGercekCharacter::GetCurrentHungerConsumptionMultiplier() const {
  const bool bIsActivelySprinting =
      bIsSprinting && GetVelocity().SizeSquared() > 0.0f && !bIsExhausted;

  if (bIsCrouched) {
    return CrouchHungerConsumptionMultiplier;
  }

  if (bIsActivelySprinting) {
    return SprintHungerConsumptionMultiplier;
  }

  return WalkHungerConsumptionMultiplier;
}

float AGercekCharacter::GetCurrentThirstConsumptionMultiplier() const {
  const bool bIsActivelySprinting =
      bIsSprinting && GetVelocity().SizeSquared() > 0.0f && !bIsExhausted;

  if (bIsCrouched) {
    return CrouchThirstConsumptionMultiplier;
  }

  if (bIsActivelySprinting) {
    return SprintThirstConsumptionMultiplier;
  }

  return WalkThirstConsumptionMultiplier;
}

void AGercekCharacter::ServerSetSprinting_Implementation(bool bNewSprinting) {
  bIsSprinting = bNewSprinting && !bIsCrouched && !bIsFatigued && Stamina > 0.0f;
}

void AGercekCharacter::ApplyItemEffect(EItemType Type, float Amount) {
  bIsConsuming = false;

  // Denge: TÃ¼m stat artÄ±ÅŸlarÄ± (yiyecek, tÄ±bbi kit) sadece Sunucu (Server)
  // Ã¼zerinde HasAuthority() kontrolÃ¼ ile yapÄ±lmalÄ±dÄ±r.
  if (!HasAuthority()) {
    return;
  }

  switch (Type) {
  case EItemType::Food:
    Hunger += Amount;
    Hunger = FMath::Clamp(Hunger, 0.0f, MaxHunger);
    Stamina = FMath::Clamp(Stamina, 0.0f, GetEffectiveMaxStamina());
    break;

  case EItemType::Drink:
    Thirst += Amount;
    Thirst = FMath::Clamp(Thirst, 0.0f, MaxThirst);
    if (Thirst > 80.0f) {
      StaminaRecoveryRate = OriginalStaminaRecoveryRate * 1.25f;
      GetWorld()->GetTimerManager().SetTimer(
          StaminaBuffTimerHandle, this,
          &AGercekCharacter::ResetStaminaRecoveryBuff, 20.0f, false);
    }

    Stamina = FMath::Clamp(Stamina, 0.0f, GetEffectiveMaxStamina());
    break;

  case EItemType::Med:
    // Tibbi malzeme: Can? yeniler.
    Health += Amount;
    Health = FMath::Clamp(Health, 0.0f, MaxHealth);
    break;

  case EItemType::Junk:
    // Hurda: ArtÄ±k radyasyonu azaltmaz, sadece ticari veya Ã¼retim amaÃ§lÄ± bir
    // eÅŸya (Trade/Barter).
    break;

  case EItemType::AntiRad:
    Radiation -= Amount;
    Radiation = FMath::Clamp(Radiation, 0.0f, MaxRadiation);

    // Hardcore Penalty: AÄŸÄ±r kimyasallar vÃ¼cudu yorar, anÄ±nda 15 Stamina siler.
    Stamina -= 15.0f;
    Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);

    if (GEngine) {
      GEngine->AddOnScreenDebugMessage(
          -1, 3.0f, FColor::Green,
          TEXT("Radyasyon temizlendi, ancak yorgunluk hissediyorsun!"));
    }
    break;

  default:
    // Silah, MÃ¼himmat, Quest esyalari tÃ¼ketilemez.
    UE_LOG(LogTemp, Warning,
           TEXT("[ConsumeItem] Bu esya tÃ¼ketilemez (Type: %%d)."),
           static_cast<int32>(Type));
    break;
  }

}

void AGercekCharacter::ResetStaminaRecoveryBuff() {
  StaminaRecoveryRate = OriginalStaminaRecoveryRate;
}

// --- HASAR KAYDÄ° (HealthRegen lockout iÃ§in) ---
float AGercekCharacter::TakeDamage(float DamageAmount,
                                   struct FDamageEvent const &DamageEvent,
                                   class AController *EventInstigator,
                                   AActor *DamageCauser) {
  const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent,
                                               EventInstigator, DamageCauser);

  if (ActualDamage > 0.0f && HasAuthority()) {
    // Son hasar zamanÄ±nÄ± gÃ¼ncelle (can regen kilidi sÄ±fÄ±rlanÄ±r)
    LastDamageTakenTime = GetWorld()->GetTimeSeconds();

    Health -= ActualDamage;
    Health = FMath::Clamp(Health, 0.0f, MaxHealth);

    UE_LOG(LogTemp, Log, TEXT("[Damage] %.1f hasar alindi. Kalan can: %.1f"),
           ActualDamage, Health);
  }

  EmitSurvivalStatChangeEvents(false);

  return ActualDamage;
}

float AGercekCharacter::GetWeightRatio() const {
  if (!InventoryComponent)
    return 0.0f;
  // Grid doluluk oranÄ±: Kaplanan hÃ¼cre sayÄ±sÄ± / Toplam hÃ¼cre sayÄ±sÄ± (Cols *
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

// ==== AÄIRLIK KAYNAKLI HAREKET GÃœNCELLEMESÄ° ====
//
// FOnWeightChanged delegate'i AddItem / RemoveItem sonrasÄ±nda yayÄ±nlanÄ±r.
// Bu fonksiyon Tick'e baÄŸli deÄŸil â€” sadece envanter deÄŸiÅŸtiÄŸinde tetiklenir.
void AGercekCharacter::OnInventoryWeightChanged(float NewWeight,
                                                float MaxWeight) {
  UpdateMovementSpeed(NewWeight, MaxWeight);
}

void AGercekCharacter::UpdateMovementSpeed(float CurrentWeight,
                                           float MaxWeight) {
  // MaxWeight geÃ§ersizse bÃ¶lme hatasÄ± Ã¶nle.
  if (MaxWeight <= 0.0f) {
    MovementSpeedMultiplier = 1.0f;
    return;
  }

  if (CurrentWeight > MaxWeight) {
    // ---- OVERBURDENED: TaÅŸÄ±yan karakter yavaÅŸlar (%50) ----
    // Tick iÃ§indeki TargetSpeed *= MovementSpeedMultiplier satÄ±rÄ±
    // bu deÄŸeri otomatik olarak yansÄ±tÄ±r; ek kod gerekmez.
    MovementSpeedMultiplier = 0.5f;

    UE_LOG(
        LogTemp, Warning,
        TEXT("[Movement] Overburdened! %.2f / %.2f kg -- hiz %%50 dusuruldu."),
        CurrentWeight, MaxWeight);
  } else {
    // ---- NORMAL: Tam hÄ±z yeniden aktif ----
    MovementSpeedMultiplier = 1.0f;
  }
}

// ==== HAREKET VE KAMERA (ENHANCED INPUT) ====
void AGercekCharacter::Move(const FInputActionValue &Value) {
  // input is a Vector2D
  FVector2D MovementVector = Value.Get<FVector2D>();

  if (Controller != nullptr) {
    // Ä°leri/Geri hareket (W/S)
    AddMovementInput(GetActorForwardVector(), MovementVector.Y);
    // SaÄŸa/Sola hareket (A/D)
    AddMovementInput(GetActorRightVector(), MovementVector.X);
  }
}

void AGercekCharacter::Look(const FInputActionValue &Value) {
  // Guard: Envanter aÃ§Ä±ksa kamera bakÄ±ÅŸÄ±nÄ± engelle.
  // KullanÄ±cÄ± fare ile UI'da gezinirken kamera dÃ¶nmemelidir.
  if (IsValid(InventoryWidget) &&
      InventoryWidget->GetVisibility() == ESlateVisibility::Visible) {
    return;
  }

  FVector2D LookAxisVector = Value.Get<FVector2D>();

  if (Controller != nullptr) {
    // SaÄŸa/Sola bakÄ±ÅŸ (Yaw)
    AddControllerYawInput(LookAxisVector.X);
    // YukarÄ±/AÅŸaÄŸÄ± bakÄ±ÅŸ (Pitch)
    AddControllerPitchInput(LookAxisVector.Y);
  }
}

void AGercekCharacter::ShowInventoryDetails() {
#if UE_BUILD_SHIPPING || UE_BUILD_TEST
  return;
#else
  if (!InventoryComponent) {
    return;
  }

  const TArray<FGridItemInstanceView> ItemInstances =
      InventoryComponent->GetItemInstances();

  if (ItemInstances.Num() == 0) {
    UE_LOG(LogTemp, Verbose, TEXT("Envanter Bos"));
    return;
  }

  TMap<FName, int32> ItemCounts;
  for (const FGridItemInstanceView &ItemInstance : ItemInstances) {
    ItemCounts.FindOrAdd(ItemInstance.ItemHandle.RowName)++;
  }

  for (const TPair<FName, int32> &Entry : ItemCounts) {
    FString Message = FString::Printf(TEXT("[GRID] %s  |  %d adet"),
                                      *Entry.Key.ToString(), Entry.Value);
    UE_LOG(LogTemp, Verbose, TEXT("%s"), *Message);
  }
#endif
}

void AGercekCharacter::HandleGridInventoryUpdated() {
  SyncPlayerInventoryToOwner();

  if (InventoryComponent && IsValid(InventoryWidget)) {
    InventoryComponent->NativeRefreshUI(
        ResolveInventoryRefreshTarget(InventoryWidget));
  }

  if (IsValid(ActiveLootContainerWidget)) {
    RefreshLootContainerUI();
  }

  if (IsValid(ActiveTradeWidget)) {
    RefreshTradeUI();
  }
}

void AGercekCharacter::OpenLootContainer(ALootContainerBase *TargetContainer) {
  if (!IsValid(TargetContainer) || !LootContainerWidgetClass) {
    return;
  }

  ActiveLootContainer = TargetContainer;

  if (!IsValid(ActiveLootContainerWidget)) {
    ActiveLootContainerWidget =
        CreateWidget<UUserWidget>(GetWorld(), LootContainerWidgetClass);
  }

  if (!IsValid(ActiveLootContainerWidget)) {
    return;
  }

  if (UTextBlock *ContainerNameText = Cast<UTextBlock>(
          ActiveLootContainerWidget->GetWidgetFromName(TEXT("Txt_ContainerName")))) {
    ContainerNameText->SetText(TargetContainer->GetInteractableName_Implementation());
  }

  if (UButton *CloseButton = Cast<UButton>(
          ActiveLootContainerWidget->GetWidgetFromName(TEXT("Btn_CloseContainer")))) {
    CloseButton->OnClicked.Clear();
    CloseButton->OnClicked.AddDynamic(this, &AGercekCharacter::CloseLootContainer);
  }

  if (TargetContainer->GetContainerInventory()) {
    TargetContainer->GetContainerInventory()->OnGridUpdated.RemoveDynamic(
        this, &AGercekCharacter::HandleActiveLootContainerUpdated);
    TargetContainer->GetContainerInventory()->OnGridUpdated.AddDynamic(
        this, &AGercekCharacter::HandleActiveLootContainerUpdated);
  }

  RefreshLootContainerUI();
  ActiveLootContainerWidget->AddToViewport(20);

  if (APlayerController *PC = Cast<APlayerController>(GetController())) {
    PC->bShowMouseCursor = true;

    FInputModeUIOnly UIMode;
    UIMode.SetWidgetToFocus(ActiveLootContainerWidget->TakeWidget());
    UIMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockInFullscreen);
    PC->SetInputMode(UIMode);
  }
}

void AGercekCharacter::CloseLootContainer() {
  if (IsValid(ActiveLootContainer) && ActiveLootContainer->GetContainerInventory()) {
    ActiveLootContainer->GetContainerInventory()->OnGridUpdated.RemoveDynamic(
        this, &AGercekCharacter::HandleActiveLootContainerUpdated);
  }

  if (IsValid(ActiveLootContainerWidget)) {
    ActiveLootContainerWidget->RemoveFromParent();
    ActiveLootContainerWidget = nullptr;
  }

  if (IsValid(ActiveLootContainer)) {
    if (HasAuthority()) {
      ActiveLootContainer->ReleaseContainer(this);
    } else {
      ServerCloseLootContainer(ActiveLootContainer);
    }
  }

  ActiveLootContainer = nullptr;

  if (APlayerController *PC = Cast<APlayerController>(GetController())) {
    PC->bShowMouseCursor = false;
    FInputModeGameOnly GameMode;
    PC->SetInputMode(GameMode);
  }
}

void AGercekCharacter::RefreshLootContainerUI() {
  if (!IsValid(ActiveLootContainerWidget) || !IsValid(ActiveLootContainer)) {
    return;
  }

  if (UUserWidget *ContainerGridUI = Cast<UUserWidget>(
          ActiveLootContainerWidget->GetWidgetFromName(TEXT("ContainerGridUI")))) {
    if (UPostApocInventoryComponent *ContainerInventory =
            ActiveLootContainer->GetContainerInventory()) {
      if (UPostApocInventoryGridWidget *ContainerGridWidget =
              Cast<UPostApocInventoryGridWidget>(ContainerGridUI)) {
        ContainerGridWidget->InitializeGridContext(
            ContainerInventory, this, ActiveLootContainer,
            EPostApocInventoryGridRole::LootContainer);
        ContainerGridWidget->SetUseDefaultItemActions(true);
      }
      ContainerInventory->NativeRefreshUI(ContainerGridUI);
    }
  }

  if (UUserWidget *PlayerGridUI = Cast<UUserWidget>(
          ActiveLootContainerWidget->GetWidgetFromName(TEXT("PlayerGridUI")))) {
    if (InventoryComponent) {
      if (UPostApocInventoryGridWidget *PlayerGridWidget =
              Cast<UPostApocInventoryGridWidget>(PlayerGridUI)) {
        PlayerGridWidget->InitializeGridContext(
            InventoryComponent, this, ActiveLootContainer,
            EPostApocInventoryGridRole::PlayerInventory);
        PlayerGridWidget->SetUseDefaultItemActions(true);
      }
      InventoryComponent->NativeRefreshUI(PlayerGridUI);
    }
  }
}

void AGercekCharacter::HandleActiveLootContainerUpdated() {
  RefreshLootContainerUI();
}

void AGercekCharacter::RequestTakeItemFromLootContainer(FGuid ItemInstanceId) {
  if (!IsValid(ActiveLootContainer) || !ItemInstanceId.IsValid()) {
    return;
  }

  if (HasAuthority()) {
    ActiveLootContainer->TransferItemToPlayer(this, ItemInstanceId);
    RefreshLootContainerUI();
  } else {
    ServerTakeItemFromLootContainer(ActiveLootContainer, ItemInstanceId);
  }
}

void AGercekCharacter::RequestStoreItemInLootContainer(FGuid ItemInstanceId) {
  if (!IsValid(ActiveLootContainer) || !ItemInstanceId.IsValid()) {
    return;
  }

  if (HasAuthority()) {
    ActiveLootContainer->TransferItemFromPlayer(this, ItemInstanceId);
    RefreshLootContainerUI();
  } else {
    ServerStoreItemInLootContainer(ActiveLootContainer, ItemInstanceId);
  }
}

void AGercekCharacter::ServerTakeItemFromLootContainer_Implementation(
    ALootContainerBase *TargetContainer, FGuid ItemInstanceId) {
  if (IsValid(TargetContainer) && ItemInstanceId.IsValid()) {
    TargetContainer->TransferItemToPlayer(this, ItemInstanceId);
  }
}

void AGercekCharacter::ServerStoreItemInLootContainer_Implementation(
    ALootContainerBase *TargetContainer, FGuid ItemInstanceId) {
  if (IsValid(TargetContainer) && ItemInstanceId.IsValid()) {
    TargetContainer->TransferItemFromPlayer(this, ItemInstanceId);
  }
}

void AGercekCharacter::ServerCloseLootContainer_Implementation(
    ALootContainerBase *TargetContainer) {
  if (IsValid(TargetContainer)) {
    TargetContainer->ReleaseContainer(this);
  }
}


// ==== Ã‡ANTADAN KULLANMA VE YERE ATMA (USE & DROP) ====

void AGercekCharacter::UseItemFromInventory(
    const FDataTableRowHandle &ItemRowHandle) {
  if (ItemRowHandle.IsNull() || !InventoryComponent) {
    return;
  }

  FGuid ItemInstanceId;
  if (InventoryComponent->FindFirstItemInstanceByRowName(ItemRowHandle.RowName,
                                                         ItemInstanceId)) {
    UseItemInstanceFromInventory(ItemInstanceId);
  }
}

void AGercekCharacter::UseItemInstanceFromInventory(FGuid ItemInstanceId) {
  if (!ItemInstanceId.IsValid() || !InventoryComponent) {
    return;
  }

  if (!HasAuthority()) {
    ServerUseItemInstanceFromInventory(ItemInstanceId);
    return;
  }

  FGridItemInstanceView ItemInstanceView;
  if (!InventoryComponent->GetItemInstanceView(ItemInstanceId, ItemInstanceView)) {
    return;
  }

  FDataTableRowHandle ItemRowHandle;
  if (!InventoryComponent->GetItemHandleForInstance(ItemInstanceId,
                                                    ItemRowHandle) ||
      ItemRowHandle.IsNull()) {
    return;
  }

  const FPostApocItemRow *GridRow = ItemRowHandle.GetRow<FPostApocItemRow>(
      TEXT("AGercekCharacter::UseItemFromInventory.Grid"));

  EItemType ConsumeType = EItemType::Junk;
  float ConsumeAmount = 0.0f;
  bool bCanConsume = false;

  if (GridRow && GridRow->bCanConsume) {
    switch (GridRow->ConsumeEffectType) {
    case EPostApocConsumableEffectType::Food:
      ConsumeType = EItemType::Food;
      bCanConsume = true;
      break;
    case EPostApocConsumableEffectType::Drink:
      ConsumeType = EItemType::Drink;
      bCanConsume = true;
      break;
    case EPostApocConsumableEffectType::Heal:
      ConsumeType = EItemType::Med;
      bCanConsume = true;
      break;
    case EPostApocConsumableEffectType::AntiRad:
      ConsumeType = EItemType::AntiRad;
      bCanConsume = true;
      break;
    default:
      bCanConsume = false;
      break;
    }

    if (bCanConsume) {
      ConsumeAmount =
          ResolveConsumableAmount(*GridRow, ItemInstanceView.FillState);
    }
  }

  if (!bCanConsume) {
    const FString ItemLabel =
        GridRow ? GridRow->DisplayName.ToString() : ItemRowHandle.RowName.ToString();
    UE_LOG(
        LogTemp, Warning,
        TEXT(
            "[AGercekCharacter] Tuketilemeyen esya kullanilmaya calisildi: %s"),
        *ItemLabel);
    return;
  }

  if (InventoryComponent->RemoveItemByInstanceId(ItemInstanceId)) {
    ConsumeItem(ConsumeType, ConsumeAmount);
  }
}

void AGercekCharacter::ServerUseItemInstanceFromInventory_Implementation(
    FGuid ItemInstanceId) {
  UseItemInstanceFromInventory(ItemInstanceId);
}

void AGercekCharacter::DropItemFromInventory(
    const FDataTableRowHandle &ItemRowHandle) {
  if (ItemRowHandle.IsNull() || !InventoryComponent) {
    return;
  }

  FGuid ItemInstanceId;
  if (InventoryComponent->FindFirstItemInstanceByRowName(ItemRowHandle.RowName,
                                                         ItemInstanceId)) {
    DropItemInstanceFromInventory(ItemInstanceId);
  }
}

void AGercekCharacter::DropItemInstanceFromInventory(FGuid ItemInstanceId) {
  if (!ItemInstanceId.IsValid() || !InventoryComponent) {
    return;
  }

  if (!HasAuthority()) {
    ServerDropItemInstanceFromInventory(ItemInstanceId);
    return;
  }

  FGridItemInstanceView ItemInstanceView;
  if (!InventoryComponent->GetItemInstanceView(ItemInstanceId,
                                               ItemInstanceView) ||
      ItemInstanceView.ItemHandle.IsNull()) {
    return;
  }

  const FDataTableRowHandle& ItemRowHandle = ItemInstanceView.ItemHandle;

  const FPostApocItemRow *GridRow = ItemRowHandle.GetRow<FPostApocItemRow>(
      TEXT("AGercekCharacter::DropItemFromInventory.Grid"));

  const bool bIsQuestItem = GridRow && GridRow->bQuestItem;

  if (bIsQuestItem) {
    if (GEngine) {
      GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
                                       TEXT("Gorev esyalari yere atilamaz."));
    }
    return;
  }

  if (InventoryComponent->RemoveItemByInstanceId(ItemInstanceId)) {
    if (FpsCameraComponent) {
      FVector DropLocation = FpsCameraComponent->GetComponentLocation() +
                             (FpsCameraComponent->GetForwardVector() * 100.0f);
      SpawnItemInWorld(ItemRowHandle, DropLocation, ItemInstanceView.Condition,
                       ItemInstanceView.FillState);
    }
  }
}

void AGercekCharacter::ServerDropItemInstanceFromInventory_Implementation(
    FGuid ItemInstanceId) {
  DropItemInstanceFromInventory(ItemInstanceId);
}

FText AGercekCharacter::GetKnowledgeAdjustedTradeValueText(
    const int32 ActualValue) const {
  return GetKnowledgeAdjustedValue(static_cast<float>(ActualValue));
}

int32 AGercekCharacter::GetKnowledgePurchaseValue(const int32 ActualValue) const {
  switch (CurrentKnowledge) {
  case ETradeKnowledge::Novice:
    return ActualValue;
  case ETradeKnowledge::Apprentice:
    return FMath::RoundToInt(static_cast<float>(ActualValue) * 0.95f);
  case ETradeKnowledge::Expert:
    return FMath::RoundToInt(static_cast<float>(ActualValue) * 0.90f);
  default:
    return ActualValue;
  }
}

AActor *
AGercekCharacter::SpawnItemInWorld(const FDataTableRowHandle &ItemRowHandle,
                                   FVector SpawnLocation,
                                   int32 ItemCondition,
                                   const EConsumableFillState FillState) {
  if (ItemRowHandle.IsNull() || !GetWorld())
    return nullptr;

  FActorSpawnParameters SpawnParams;
  SpawnParams.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

  // AWorldItemActor oluÅŸtur
  AWorldItemActor *SpawnedItem = GetWorld()->SpawnActor<AWorldItemActor>(
      AWorldItemActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator,
      SpawnParams);

  if (SpawnedItem) {
    SpawnedItem->InitializeItemData(ItemRowHandle, ItemCondition, FillState);
    SpawnedItem->SetPersistentWorldDrop(true);
  }

  return SpawnedItem;
}

TArray<FGuid>* AGercekCharacter::GetTradeOfferArray(
    const EPostApocInventoryGridRole SourceRole) {
  switch (SourceRole) {
  case EPostApocInventoryGridRole::PlayerInventory:
    return &PlayerTradeOfferItems;
  case EPostApocInventoryGridRole::MerchantInventory:
    return &MerchantTradeOfferItems;
  default:
    return nullptr;
  }
}

const TArray<FGuid>* AGercekCharacter::GetTradeOfferArray(
    const EPostApocInventoryGridRole SourceRole) const {
  switch (SourceRole) {
  case EPostApocInventoryGridRole::PlayerInventory:
    return &PlayerTradeOfferItems;
  case EPostApocInventoryGridRole::MerchantInventory:
    return &MerchantTradeOfferItems;
  default:
    return nullptr;
  }
}

bool AGercekCharacter::BuildTradeOfferEntryData(
    UPostApocInventoryComponent* SourceInventory, const FGuid ItemInstanceId,
    FPostApocTradeOfferEntryData& OutEntryData,
    const bool bApplyKnowledgePurchaseDiscount) const {
  if (!SourceInventory || !ItemInstanceId.IsValid()) {
    return false;
  }

  FGridItemInstanceView ItemInstance;
  if (!SourceInventory->GetItemInstanceView(ItemInstanceId, ItemInstance)) {
    return false;
  }

  const FPostApocItemRow* ItemData =
      ItemInstance.ItemHandle.GetRow<FPostApocItemRow>(
          TEXT("BuildTradeOfferEntryData"));
  if (!ItemData) {
    return false;
  }

  OutEntryData.ItemInstanceId = ItemInstanceId;
  OutEntryData.ItemName = ItemData->DisplayName;
  OutEntryData.Condition = ItemInstance.Condition;
  OutEntryData.FillState = ItemInstance.FillState;
  if (!SourceInventory->GetItemDisplayStateForInstance(ItemInstanceId,
                                                       OutEntryData.DisplayStateText)) {
    OutEntryData.DisplayStateText = FText::GetEmpty();
  }
  OutEntryData.EffectiveValue =
      SourceInventory->GetItemValueForInstance(ItemInstanceId);
  if (bApplyKnowledgePurchaseDiscount) {
    OutEntryData.EffectiveValue =
        GetKnowledgePurchaseValue(OutEntryData.EffectiveValue);
  }
  OutEntryData.DisplayValueText =
      GetKnowledgeAdjustedTradeValueText(OutEntryData.EffectiveValue);
  return true;
}

void AGercekCharacter::SanitizeTradeOfferSelections() {
  auto SanitizeArray = [](UPostApocInventoryComponent* Inventory,
                          TArray<FGuid>& OfferItems) {
    if (!Inventory) {
      OfferItems.Reset();
      return;
    }

    TSet<FGuid> UniqueItems;
    for (int32 Index = OfferItems.Num() - 1; Index >= 0; --Index) {
      FGridItemInstanceView ItemInstance;
      if (!Inventory->GetItemInstanceView(OfferItems[Index], ItemInstance) ||
          UniqueItems.Contains(OfferItems[Index])) {
        OfferItems.RemoveAt(Index);
        continue;
      }

      UniqueItems.Add(OfferItems[Index]);
    }
  };

  SanitizeArray(InventoryComponent, PlayerTradeOfferItems);
  SanitizeArray(ActiveMerchant ? ActiveMerchant->GetMerchantInventory() : nullptr,
                MerchantTradeOfferItems);
}

void AGercekCharacter::EnsureTradeOfferPanels() {
  if (!IsValid(ActiveTradeWidget)) {
    return;
  }

  if (!IsValid(PlayerTradeOfferPanel)) {
    PlayerTradeOfferPanel =
        CreateWidget<UPostApocTradeOfferPanelWidget>(
            GetWorld(), UPostApocTradeOfferPanelWidget::StaticClass());
  }
  if (!IsValid(MerchantTradeOfferPanel)) {
    MerchantTradeOfferPanel =
        CreateWidget<UPostApocTradeOfferPanelWidget>(
            GetWorld(), UPostApocTradeOfferPanelWidget::StaticClass());
  }
  if (!IsValid(PlayerTradeOfferPanel) || !IsValid(MerchantTradeOfferPanel)) {
    return;
  }

  PlayerTradeOfferPanel->Configure(this,
                                   EPostApocInventoryGridRole::PlayerInventory,
                                   FText::FromString(TEXT("Oyuncu Teklifi")));
  MerchantTradeOfferPanel->Configure(
      this, EPostApocInventoryGridRole::MerchantInventory,
      FText::FromString(TEXT("Tuccar Istegi")));

  UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(
      ActiveTradeWidget->GetWidgetFromName(TEXT("CanvasPanel_0")));
  if (!RootCanvas) {
    RootCanvas = Cast<UCanvasPanel>(ActiveTradeWidget->GetRootWidget());
  }
  if (!RootCanvas) {
    return;
  }

  auto AttachPanelIfNeeded = [RootCanvas](UUserWidget* TradeWidget,
                                          const TCHAR* AnchorTextName,
                                          UUserWidget* PanelWidget,
                                          const FVector2D FallbackPosition) {
    if (PanelWidget->GetParent()) {
      return;
    }

    FVector2D PanelPosition = FallbackPosition;
    if (UTextBlock* AnchorText =
            Cast<UTextBlock>(TradeWidget->GetWidgetFromName(AnchorTextName))) {
      if (UCanvasPanelSlot* AnchorSlot =
              Cast<UCanvasPanelSlot>(AnchorText->Slot)) {
        PanelPosition =
            AnchorSlot->GetPosition() + FVector2D(0.0f, 40.0f);
      }
    }

    if (UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(PanelWidget)) {
      PanelSlot->SetAutoSize(false);
      PanelSlot->SetSize(FVector2D(260.0f, 220.0f));
      PanelSlot->SetPosition(PanelPosition);
    }
  };

  AttachPanelIfNeeded(ActiveTradeWidget, TEXT("Txt_OyuncuTeklif"),
                      PlayerTradeOfferPanel, FVector2D(40.0f, 420.0f));
  AttachPanelIfNeeded(ActiveTradeWidget, TEXT("Txt_TuccarIstek"),
                      MerchantTradeOfferPanel, FVector2D(860.0f, 420.0f));
}

void AGercekCharacter::RefreshTradeOfferPanels() {
  if (!IsValid(ActiveTradeWidget) || !ActiveMerchant) {
    return;
  }

  EnsureTradeOfferPanels();
  SanitizeTradeOfferSelections();

  auto RefreshPanel = [this](UPostApocInventoryComponent* SourceInventory,
                             const TArray<FGuid>& OfferItems,
                             UPostApocTradeOfferPanelWidget* OfferPanel,
                             UTextBlock* SummaryText,
                             const bool bApplyKnowledgePurchaseDiscount) {
    if (!SourceInventory || !OfferPanel) {
      return;
    }

    TArray<FPostApocTradeOfferEntryData> EntryData;
    EntryData.Reserve(OfferItems.Num());
    for (const FGuid& ItemInstanceId : OfferItems) {
      FPostApocTradeOfferEntryData OfferEntry;
      if (BuildTradeOfferEntryData(SourceInventory, ItemInstanceId, OfferEntry,
                                   bApplyKnowledgePurchaseDiscount)) {
        EntryData.Add(OfferEntry);
      }
    }

    int32 ActualTotalValue = 0;
    for (const FPostApocTradeOfferEntryData& OfferEntry : EntryData) {
      ActualTotalValue += OfferEntry.EffectiveValue;
    }
    const FText DisplayTotalValue =
        GetKnowledgeAdjustedTradeValueText(ActualTotalValue);

    if (SummaryText) {
      SummaryText->SetText(FText::FromString(FString::Printf(
          TEXT("%s"), *DisplayTotalValue.ToString())));
    }
    OfferPanel->RefreshEntries(
        EntryData,
        FText::FromString(
            FString::Printf(TEXT("Toplam: %s"), *DisplayTotalValue.ToString())));
  };

  UTextBlock* PlayerSummaryText = Cast<UTextBlock>(
      ActiveTradeWidget->GetWidgetFromName(TEXT("Txt_OyuncuTeklif")));
  UTextBlock* MerchantSummaryText = Cast<UTextBlock>(
      ActiveTradeWidget->GetWidgetFromName(TEXT("Txt_TuccarIstek")));

  RefreshPanel(InventoryComponent, PlayerTradeOfferItems, PlayerTradeOfferPanel,
               PlayerSummaryText, false);
  RefreshPanel(ActiveMerchant->GetMerchantInventory(), MerchantTradeOfferItems,
               MerchantTradeOfferPanel, MerchantSummaryText, true);
}

void AGercekCharacter::RefreshTradeUI() {
  if (!IsValid(ActiveTradeWidget) || !ActiveMerchant) {
    return;
  }

  if (UUserWidget* TraderGridUI = Cast<UUserWidget>(
          ActiveTradeWidget->GetWidgetFromName(TEXT("TuccarCanta")))) {
    if (UPostApocInventoryComponent* MerchantInv =
            ActiveMerchant->GetMerchantInventory()) {
      if (UPostApocInventoryGridWidget* TraderGridWidget =
              Cast<UPostApocInventoryGridWidget>(TraderGridUI)) {
        TraderGridWidget->InitializeGridContext(
            MerchantInv, this, nullptr,
            EPostApocInventoryGridRole::MerchantInventory);
        TraderGridWidget->SetUseDefaultItemActions(false);
      }
      MerchantInv->NativeRefreshUI(TraderGridUI);
    }
  }

  if (UUserWidget* PlayerGridUI = Cast<UUserWidget>(
          ActiveTradeWidget->GetWidgetFromName(TEXT("OyuncuCanta")))) {
    if (InventoryComponent) {
      if (UPostApocInventoryGridWidget* PlayerGridWidget =
              Cast<UPostApocInventoryGridWidget>(PlayerGridUI)) {
        PlayerGridWidget->InitializeGridContext(
            InventoryComponent, this, nullptr,
            EPostApocInventoryGridRole::PlayerInventory);
        PlayerGridWidget->SetUseDefaultItemActions(false);
      }
      InventoryComponent->NativeRefreshUI(PlayerGridUI);
    }
  }

  RefreshTradeOfferPanels();
}

void AGercekCharacter::ToggleTradeOfferItem(
    const EPostApocInventoryGridRole SourceRole, const FGuid ItemInstanceId) {
  TArray<FGuid>* OfferItems = GetTradeOfferArray(SourceRole);
  if (!OfferItems || !ItemInstanceId.IsValid()) {
    return;
  }

  if (OfferItems->Contains(ItemInstanceId)) {
    OfferItems->Remove(ItemInstanceId);
  } else {
    OfferItems->Add(ItemInstanceId);
  }

  RefreshTradeOfferPanels();
}

void AGercekCharacter::HandleTradeInventoryUpdated() { RefreshTradeUI(); }

void AGercekCharacter::ClientHandleTradeResult_Implementation(
    const bool bSuccess, const FString& Message) {
  if (GEngine && !Message.IsEmpty()) {
    GEngine->AddOnScreenDebugMessage(
        -1, 4.0f, bSuccess ? FColor::Green : FColor::Red, Message);
  }

  if (bSuccess) {
    PlayerTradeOfferItems.Reset();
    MerchantTradeOfferItems.Reset();
    CloseTradeScreen();
    return;
  }

  RefreshTradeUI();
}

// ==== TÄ°CARET EKRANI YÃ–NETÄ°MÄ° ====

void AGercekCharacter::OpenTradeScreen(AMerchantBase *TargetMerchant) {
  // Hedef tÃ¼ccar geÃ§ersizse iÅŸlem yapma
  if (!IsValid(TargetMerchant) || !TradeScreenClass) {
    return;
  }

  // Aktif tÃ¼ccarÄ± kaydet
  ActiveMerchant = TargetMerchant;
  PlayerTradeOfferItems.Reset();
  MerchantTradeOfferItems.Reset();

  // HalihazÄ±rda aÃ§Ä±k bir takas ekranÄ± yoksa oluÅŸtur
  if (!IsValid(ActiveTradeWidget)) {
    ActiveTradeWidget = CreateWidget<UUserWidget>(GetWorld(), TradeScreenClass);
  }

  if (IsValid(ActiveTradeWidget)) {
    // 1. Ekrandaki TextBlock (TÃ¼ccar AdÄ±) gÃ¼ncellemesi
    UTextBlock *MerchantNameText = Cast<UTextBlock>(
        ActiveTradeWidget->GetWidgetFromName(TEXT("Txt_MerchantName")));
    if (MerchantNameText) {
      MerchantNameText->SetText(TargetMerchant->GetMerchantName());
    }

    // 2. Oyuncu ve TÃ¼ccar envanter IzgaralarÄ±nÄ± (Grid) "Enjekte" et ve C++
    // Ã¼zerinden yenile
    if (UPostApocInventoryComponent *MerchantInv =
            TargetMerchant->GetMerchantInventory()) {
      MerchantInv->OnGridUpdated.RemoveDynamic(this,
                                               &AGercekCharacter::HandleTradeInventoryUpdated);
      MerchantInv->OnGridUpdated.AddDynamic(this,
                                            &AGercekCharacter::HandleTradeInventoryUpdated);
    }

    // 3. Buton BaÄŸlantÄ±larÄ± (Btn_Ayril, Btn_TakasYap) - DoÄŸrudan C++
    // FonksiyonlarÄ±na BaÄŸlama
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

    // 4. ArayÃ¼zÃ¼ ekrana ekle ve girdi (input) kontrollerini dÃ¼zenle
    ActiveTradeWidget->AddToViewport(20);

    if (APlayerController *PC = Cast<APlayerController>(GetController())) {
      PC->bShowMouseCursor = true;

      FInputModeUIOnly UIMode;
      UIMode.SetWidgetToFocus(ActiveTradeWidget->TakeWidget());
      UIMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockInFullscreen);
      PC->SetInputMode(UIMode);
    }

    RefreshTradeUI();
  }
}

void AGercekCharacter::ExecuteTrade() {
  if (!IsValid(ActiveTradeWidget) || !ActiveMerchant)
    return;

  SanitizeTradeOfferSelections();
  if (PlayerTradeOfferItems.Num() == 0 || MerchantTradeOfferItems.Num() == 0) {
    if (GEngine) {
      GEngine->AddOnScreenDebugMessage(
          -1, 4.0f, FColor::Red,
          TEXT("Takas icin iki tarafa da teklif eklenmeli."));
    }
    return;
  }

  if (TradeComponent && ActiveMerchant->GetMerchantInventory()) {
    TradeComponent->Server_ExecuteTrade(PlayerTradeOfferItems,
                                        MerchantTradeOfferItems,
                                        InventoryComponent,
                                        ActiveMerchant->GetMerchantInventory());
    UE_LOG(LogTemp, Display,
           TEXT("[Takas] Instance-id tabanli takas emri sunucuya gonderildi."));
  }
}

void AGercekCharacter::CloseTradeScreen() {
  if (IsValid(ActiveMerchant) && ActiveMerchant->GetMerchantInventory()) {
    ActiveMerchant->GetMerchantInventory()->OnGridUpdated.RemoveDynamic(
        this, &AGercekCharacter::HandleTradeInventoryUpdated);
  }

  if (IsValid(PlayerTradeOfferPanel)) {
    PlayerTradeOfferPanel->RemoveFromParent();
    PlayerTradeOfferPanel = nullptr;
  }

  if (IsValid(MerchantTradeOfferPanel)) {
    MerchantTradeOfferPanel->RemoveFromParent();
    MerchantTradeOfferPanel = nullptr;
  }

  PlayerTradeOfferItems.Reset();
  MerchantTradeOfferItems.Reset();

  if (IsValid(ActiveTradeWidget)) {
    ActiveTradeWidget->RemoveFromParent();
    ActiveTradeWidget = nullptr;
  }

  ActiveMerchant = nullptr;

  // KamerayÄ± ve hareket yeteneklerini oyuncuya geri ver
  if (APlayerController *PC = Cast<APlayerController>(GetController())) {
    PC->bShowMouseCursor = false;

    FInputModeGameOnly GameMode;
    PC->SetInputMode(GameMode);
  }
}










