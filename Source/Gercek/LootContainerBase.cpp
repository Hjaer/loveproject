#include "LootContainerBase.h"

#include "Components/StaticMeshComponent.h"
#include "GercekCharacter.h"
#include "Net/UnrealNetwork.h"
#include "PlayerInventoryComponent.h"
#include "PostApocInventoryTypes.h"
#include "PostApocItemTypes.h"
#include "WorldInventoryComponent.h"

ALootContainerBase::ALootContainerBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	ContainerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ContainerMesh"));
	RootComponent = ContainerMesh;

	ContainerMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ContainerMesh->SetCollisionResponseToAllChannels(ECR_Block);
	ContainerMesh->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	ContainerMesh->SetGenerateOverlapEvents(false);
	ContainerMesh->SetSimulatePhysics(false);

	ContainerInventory = CreateDefaultSubobject<UWorldInventoryComponent>(TEXT("ContainerInventory"));
	if (ContainerInventory)
	{
		ContainerInventory->GridColumns = GridColumns;
		ContainerInventory->GridRows = GridRows;
	}
}

void ALootContainerBase::BeginPlay()
{
	Super::BeginPlay();

	if (!ContainerInventory)
	{
		return;
	}

	ContainerInventory->GridColumns = GridColumns;
	ContainerInventory->GridRows = GridRows;

	if (ContainerInventory->GetItemInstances().Num() > 0)
	{
		return;
	}

	for (const FDataTableRowHandle& ItemHandle : InitialLoot)
	{
		if (!ItemHandle.IsNull())
		{
			FGuid AddedInstanceId;
			ContainerInventory->TryAddItem(ItemHandle, AddedInstanceId, 100,
				ResolveFillStateForItem(ItemHandle));
		}
	}
}

void ALootContainerBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALootContainerBase, bIsInUse);
	DOREPLIFETIME(ALootContainerBase, CurrentLootingPlayerName);
}

bool ALootContainerBase::IsAvailableForPlayer(const AGercekCharacter* Player) const
{
	return CanPlayerAccess(Player);
}

bool ALootContainerBase::TransferItemToPlayer(AGercekCharacter* Player, FGuid ItemInstanceId)
{
	if (!HasAuthority() || !IsValid(Player) || !ContainerInventory || !ItemInstanceId.IsValid())
	{
		return false;
	}

	if (!CanPlayerAccess(Player))
	{
		return false;
	}

	UPlayerInventoryComponent* PlayerInventory = Player->GetPlayerInventoryComponent();
	if (!PlayerInventory)
	{
		return false;
	}

	FDataTableRowHandle ItemHandle;
	FGridItemInstanceView ItemInstanceView;
	if (!ContainerInventory->GetItemInstanceView(ItemInstanceId, ItemInstanceView))
	{
		return false;
	}

	ItemHandle = ItemInstanceView.ItemHandle;

	if (!ContainerInventory->RemoveItemByInstanceId(ItemInstanceId))
	{
		return false;
	}

	FGuid AddedInstanceId = ItemInstanceId;
	if (!PlayerInventory->TryAddItem(ItemHandle, AddedInstanceId,
		ItemInstanceView.Condition, ItemInstanceView.FillState))
	{
		ContainerInventory->TryAddItem(ItemHandle, AddedInstanceId,
			ItemInstanceView.Condition, ItemInstanceView.FillState);
		return false;
	}

	return true;
}

bool ALootContainerBase::TransferItemFromPlayer(AGercekCharacter* Player, FGuid ItemInstanceId)
{
	if (!HasAuthority() || !IsValid(Player) || !ContainerInventory || !ItemInstanceId.IsValid())
	{
		return false;
	}

	if (!CanPlayerAccess(Player))
	{
		return false;
	}

	UPlayerInventoryComponent* PlayerInventory = Player->GetPlayerInventoryComponent();
	if (!PlayerInventory)
	{
		return false;
	}

	FDataTableRowHandle ItemHandle;
	FGridItemInstanceView ItemInstanceView;
	if (!PlayerInventory->GetItemInstanceView(ItemInstanceId, ItemInstanceView))
	{
		return false;
	}

	ItemHandle = ItemInstanceView.ItemHandle;

	if (!PlayerInventory->RemoveItemByInstanceId(ItemInstanceId))
	{
		return false;
	}

	FGuid AddedInstanceId = ItemInstanceId;
	if (!ContainerInventory->TryAddItem(ItemHandle, AddedInstanceId,
		ItemInstanceView.Condition, ItemInstanceView.FillState))
	{
		PlayerInventory->TryAddItem(ItemHandle, AddedInstanceId,
			ItemInstanceView.Condition, ItemInstanceView.FillState);
		return false;
	}

	return true;
}

void ALootContainerBase::ReleaseContainer(AGercekCharacter* Player)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!CurrentLootingPlayer.IsValid() || CurrentLootingPlayer.Get() == Player)
	{
		CurrentLootingPlayer = nullptr;
		RefreshReplicatedUsageState();
	}
}

void ALootContainerBase::OnInteract_Implementation(AGercekCharacter* Player)
{
	if (!HasAuthority() || !IsValid(Player) || !CanPlayerAccess(Player))
	{
		return;
	}

	SetCurrentLootingPlayer(Player);
	Player->ClientOpenLootContainer(this);
}

FText ALootContainerBase::GetInteractionPrompt_Implementation(AGercekCharacter* Player)
{
	if (!IsAvailableForPlayer(Player))
	{
		return FText::FromString(TEXT("Mesgul"));
	}

	const FText DisplayName = GetInteractableName_Implementation();
	if (DisplayName.IsEmpty())
	{
		return FText::FromString(TEXT("E - Ara"));
	}

	return FText::FromString(FString::Printf(TEXT("E - Ara %s"), *DisplayName.ToString()));
}

FText ALootContainerBase::GetInteractableName_Implementation()
{
	return ContainerName;
}

FDataTableRowHandle ALootContainerBase::GetItemData_Implementation()
{
	return FDataTableRowHandle();
}

void ALootContainerBase::SetCurrentLootingPlayer(AGercekCharacter* Player)
{
	CurrentLootingPlayer = Player;
	RefreshReplicatedUsageState();
}

bool ALootContainerBase::CanPlayerAccess(const AGercekCharacter* Player) const
{
	if (!IsValid(Player))
	{
		return false;
	}

	if (GetDistanceTo(Player) > AccessDistance)
	{
		return false;
	}

	if (bAllowSharedAccess)
	{
		return true;
	}

	return !CurrentLootingPlayer.IsValid() || CurrentLootingPlayer.Get() == Player;
}

void ALootContainerBase::RefreshReplicatedUsageState()
{
	bIsInUse = CurrentLootingPlayer.IsValid();
	CurrentLootingPlayerName = bIsInUse ? CurrentLootingPlayer->GetName() : FString();
}

FString ALootContainerBase::GetPersistentId() const
{
	return PersistentId.IsEmpty() ? GetPathName() : PersistentId;
}

void ALootContainerBase::ExportPersistentInventory(
	TArray<FGercekSavedGridSlot>& OutSlots,
	FString& OutDataTablePath) const
{
	OutSlots.Reset();
	OutDataTablePath.Reset();

	if (ContainerInventory)
	{
		TArray<FGridSlotData> RuntimeSlots;
		ContainerInventory->ExportSaveData(RuntimeSlots, OutDataTablePath);
		OutSlots.Reserve(RuntimeSlots.Num());

		for (const FGridSlotData& RuntimeSlot : RuntimeSlots)
		{
			FGercekSavedGridSlot SavedSlot;
			SavedSlot.Location = RuntimeSlot.Location;
			SavedSlot.ItemInstanceId = RuntimeSlot.ItemInstanceId;
			SavedSlot.ItemId = RuntimeSlot.ItemRowName;
			SavedSlot.bIsRotated = RuntimeSlot.bIsRotated;
			SavedSlot.Condition = RuntimeSlot.Condition;
			SavedSlot.FillState = RuntimeSlot.FillState;
			OutSlots.Add(SavedSlot);
		}
	}
}

void ALootContainerBase::ImportPersistentInventory(
	const TArray<FGercekSavedGridSlot>& InSlots, UDataTable* InDataTable)
{
	if (ContainerInventory)
	{
		TArray<FGridSlotData> RuntimeSlots;
		RuntimeSlots.Reserve(InSlots.Num());

		for (const FGercekSavedGridSlot& SavedSlot : InSlots)
		{
			FGridSlotData RuntimeSlot;
			RuntimeSlot.Location = SavedSlot.Location;
			RuntimeSlot.ItemInstanceId = SavedSlot.ItemInstanceId;
			RuntimeSlot.ItemRowName = SavedSlot.ItemId;
			RuntimeSlot.bIsRotated = SavedSlot.bIsRotated;
			RuntimeSlot.Condition = SavedSlot.Condition;
			RuntimeSlot.FillState = SavedSlot.FillState;
			RuntimeSlots.Add(RuntimeSlot);
		}

		ContainerInventory->ImportSaveData(RuntimeSlots, InDataTable);
	}
}

EConsumableFillState ALootContainerBase::ResolveFillStateForItem(
	const FDataTableRowHandle& ItemHandle) const
{
	if (ItemHandle.IsNull())
	{
		return EConsumableFillState::NotApplicable;
	}

	const FPostApocItemRow* ItemData =
		ItemHandle.GetRow<FPostApocItemRow>(TEXT("LootContainerBase::ResolveFillStateForItem"));
	if (!ItemData ||
		(ItemData->Category != EPostApocItemCategory::Food &&
		 ItemData->Category != EPostApocItemCategory::Drink))
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
