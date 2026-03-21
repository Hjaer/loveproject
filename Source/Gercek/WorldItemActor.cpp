// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldItemActor.h"
#include "Components/StaticMeshComponent.h"
#include "GercekCharacter.h"
#include "PostApocInventoryTypes.h" // Grid tabanlı yeni sistem

// Sets default values
AWorldItemActor::AWorldItemActor() {
  PrimaryActorTick.bCanEverTick = false;
  bReplicates = true;
  SetReplicateMovement(true);

  MeshComponent =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
  RootComponent = MeshComponent;

  // Collision: ECC_Visibility kanalını açıkça block et —
  // CheckForInteractables içindeki LineTraceSingleByChannel(ECC_Visibility)
  // bu ayar sayesinde nesneyi bulabilir.
  MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
  MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
  MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
  MeshComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
  MeshComponent->SetGenerateOverlapEvents(true);
  MeshComponent->SetSimulatePhysics(true);
}

void AWorldItemActor::BeginPlay() { Super::BeginPlay(); }

// ---------------------------------------------------------------------------
// Helper: resolve the DataTable row for this actor.
// ---------------------------------------------------------------------------
static const FPostApocItemRow *ResolveRow(const FDataTableRowHandle &Handle) {
  if (Handle.IsNull())
    return nullptr;
  return Handle.GetRow<FPostApocItemRow>(TEXT("WorldItemActor::ResolveRow"));
}

// ---------------------------------------------------------------------------
// IInteractable — Player pressed E on this actor.
// ---------------------------------------------------------------------------
void AWorldItemActor::OnInteract_Implementation(AGercekCharacter *Player) {
  // Guard: row not configured.
  const FPostApocItemRow *Row = ResolveRow(ItemRowHandle);
  if (!Row) {
    UE_LOG(LogTemp, Warning,
           TEXT("[WorldItem] '%s' has no valid ItemRowHandle. Cannot pick up."),
           *GetName());
    return;
  }

  // Guard: no player.
  if (!Player) {
    UE_LOG(LogTemp, Error,
           TEXT("[WorldItem] OnInteract called with null Player!"));
    return;
  }

  // Fetch the player's inventory.
  UPostApocInventoryComponent *Inventory =
      Player->FindComponentByClass<UPostApocInventoryComponent>();
  if (!Inventory) {
    UE_LOG(LogTemp, Warning,
           TEXT("[WorldItem] Player '%s' has no PostApocInventoryComponent."),
           *Player->GetName());
    return;
  }

  // Grid tabanlı ekleme: yer varsa sahneyi temizle, yoksa uyar.
  FGuid AddedInstanceId;
  if (Inventory->TryAddItem(ItemRowHandle, AddedInstanceId, ItemCondition,
                            ItemFillState)) {
    Destroy();
  } else {
    UE_LOG(LogTemp, Warning,
           TEXT("[WorldItem] Could not pick up '%s'. Grid is full or item is "
                "invalid."),
           *Row->DisplayName.ToString());
  }
}

// ---------------------------------------------------------------------------
// IInteractable — Return the display name for the interaction prompt.
// Geçerliyse "[E] ItemName", geçersizse boş FText döner.
// ---------------------------------------------------------------------------

FText AWorldItemActor::GetInteractionPrompt_Implementation(
    AGercekCharacter *Player) {
  const FText ItemName = GetInteractableName_Implementation();
  if (ItemName.IsEmpty()) {
    return FText::GetEmpty();
  }

  return FText::FromString(
      FString::Printf(TEXT("E - Al %s"), *ItemName.ToString()));
}
FText AWorldItemActor::GetInteractableName_Implementation() {
  const FPostApocItemRow *Row = ResolveRow(ItemRowHandle);
  if (!Row || Row->DisplayName.IsEmpty()) {
    return FText::GetEmpty();
  }
  // Sadece ham eşya adını döndür.
  // GercekCharacter::CheckForInteractables zaten "[E] Al - " ön ekini ekler;
  // burada tekrar eklersek ekranda çift prefix görünür.
  return Row->DisplayName;
}

// ---------------------------------------------------------------------------
// IInteractable — Return handle only (Zero-Pointer). PostApocItems tablosu
// WorldItemActor'da editörde atanır; handle aynen döndürülür.
// ---------------------------------------------------------------------------
FDataTableRowHandle AWorldItemActor::GetItemData_Implementation() {
  return ItemRowHandle;
}

// ---------------------------------------------------------------------------
// Initialize Item Data when spawned via Drop (Çantadan atılma durumunda)
// ---------------------------------------------------------------------------
void AWorldItemActor::InitializeItemData(const FDataTableRowHandle& InHandle,
                                         const int32 InCondition,
                                         const EConsumableFillState InFillState) {
  ItemRowHandle = InHandle;
  ItemCondition = FMath::Clamp(InCondition, 10, 100);
  ItemFillState = InFillState;

  const FPostApocItemRow *Row = ResolveRow(ItemRowHandle);
  if (!Row) return;

  if (!Row->PickupMesh.IsNull() && MeshComponent) {
    UStaticMesh *LoadedMesh = Row->PickupMesh.LoadSynchronous();
    if (LoadedMesh) {
      MeshComponent->SetStaticMesh(LoadedMesh);
      
      // Eşya ağırlığına göre fizik kuralları
      const bool bIsHeavy = (Row->Weight > 5.0f);
      MeshComponent->SetSimulatePhysics(!bIsHeavy);
    }
  }
}
