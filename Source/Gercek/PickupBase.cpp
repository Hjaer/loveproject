#include "PickupBase.h"

#include "Components/StaticMeshComponent.h"
#include "GercekCharacter.h"
#include "ItemTypes.h"
#include "PostApocInventoryTypes.h"
#include "PostApocItemTypes.h"

APickupBase::APickupBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	RootComponent = PickupMesh;

	PickupMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PickupMesh->SetCollisionResponseToAllChannels(ECR_Block);
	PickupMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	PickupMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	PickupMesh->SetGenerateOverlapEvents(true);
	PickupMesh->SetSimulatePhysics(false);
}

void APickupBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyVisualFromData();
}

void APickupBase::InitializeItemData(const FDataTableRowHandle& InHandle,
	int32 InCondition, EConsumableFillState InFillState)
{
	ItemRowHandle = InHandle;
	ItemCondition = FMath::Clamp(InCondition, 10, 100);
	ItemFillState = InFillState;
	ApplyVisualFromData();
}

void APickupBase::Interact(AGercekCharacter* Player)
{
	if (!IsValid(Player) || ItemRowHandle.IsNull())
	{
		return;
	}

	UPostApocInventoryComponent* Inventory =
		Player->FindComponentByClass<UPostApocInventoryComponent>();
	if (!Inventory)
	{
		return;
	}

	FGuid AddedInstanceId;
	if (Inventory->TryAddItem(ItemRowHandle, AddedInstanceId, ItemCondition,
		ResolveFillStateForPickup()))
	{
		Destroy();
	}
}

void APickupBase::OnInteract_Implementation(AGercekCharacter* Player)
{
	Interact(Player);
}

FText APickupBase::GetInteractionPrompt_Implementation(AGercekCharacter* Player)
{
	const FText ItemName = GetInteractableName_Implementation();
	if (ItemName.IsEmpty())
	{
		return FText::GetEmpty();
	}

	return FText::FromString(FString::Printf(TEXT("E - Al %s"), *ItemName.ToString()));
}

FText APickupBase::GetInteractableName_Implementation()
{
	const FItemDBRow* Row = ResolveItemRow();
	if (!Row || Row->ItemName.IsEmpty())
	{
		return FText::GetEmpty();
	}

	return Row->ItemName;
}

FDataTableRowHandle APickupBase::GetItemData_Implementation()
{
	return ItemRowHandle;
}

const FItemDBRow* APickupBase::ResolveItemRow() const
{
	if (ItemRowHandle.IsNull())
	{
		return nullptr;
	}

	return ItemRowHandle.GetRow<FItemDBRow>(TEXT("PickupBase::ResolveItemRow"));
}

void APickupBase::ApplyVisualFromData()
{
	const FItemDBRow* Row = ResolveItemRow();
	if (!Row || !PickupMesh)
	{
		return;
	}

	if (!Row->PickupMesh.IsNull())
	{
		if (UStaticMesh* LoadedMesh = Row->PickupMesh.LoadSynchronous())
		{
			PickupMesh->SetStaticMesh(LoadedMesh);
		}
	}

	const bool bIsHeavy = Row->ItemWeight > HeavyWeightThreshold;
	PickupMesh->SetSimulatePhysics(!bIsHeavy);
}

EConsumableFillState APickupBase::ResolveFillStateForPickup() const
{
	if (ItemFillState != EConsumableFillState::NotApplicable)
	{
		return ItemFillState;
	}

	const FPostApocItemRow* GridRow =
		ItemRowHandle.GetRow<FPostApocItemRow>(TEXT("PickupBase::ResolveFillStateForPickup"));
	if (!GridRow ||
		(GridRow->Category != EPostApocItemCategory::Food &&
		 GridRow->Category != EPostApocItemCategory::Drink))
	{
		return EConsumableFillState::NotApplicable;
	}

	switch (ConsumableFillRule)
	{
	case EConsumableLootFillRule::AlwaysFull:
		return EConsumableFillState::Full;
	case EConsumableLootFillRule::AlwaysHalfFull:
	case EConsumableLootFillRule::StandardWorldLoot:
		return EConsumableFillState::HalfFull;
	case EConsumableLootFillRule::SpecialZoneLoot:
		return FMath::FRand() <= 0.15f ? EConsumableFillState::Full
									   : EConsumableFillState::HalfFull;
	default:
		return EConsumableFillState::Full;
	}
}
