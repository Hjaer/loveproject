#include "MerchantBase.h"

#include "Components/CapsuleComponent.h"
#include "GercekCharacter.h"
#include "PostApocInventoryTypes.h"
#include "PostApocItemTypes.h"
#include "WorldInventoryComponent.h"

AMerchantBase::AMerchantBase()
{
	PrimaryActorTick.bCanEverTick = false;

	MerchantInventory = CreateDefaultSubobject<UWorldInventoryComponent>(TEXT("MerchantInventory"));
	if (MerchantInventory)
	{
		MerchantInventory->GridColumns = 10;
		MerchantInventory->GridRows = 10;
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	}
}

void AMerchantBase::BeginPlay()
{
	Super::BeginPlay();

	if (!MerchantInventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Merchant] MerchantInventory component is missing."));
		return;
	}

	MerchantInventory->GridColumns = GridColumns;
	MerchantInventory->GridRows = GridRows;

	if (MerchantInventory->GetItemInstances().Num() > 0)
	{
		return;
	}

	for (const FDataTableRowHandle& ItemHandle : InitialStock)
	{
		if (!ItemHandle.IsNull())
		{
			EConsumableFillState FillState = EConsumableFillState::NotApplicable;
			if (const FPostApocItemRow* ItemData =
					ItemHandle.GetRow<FPostApocItemRow>(TEXT("MerchantBase::BeginPlay")))
			{
				if (ItemData->Category == EPostApocItemCategory::Food ||
					ItemData->Category == EPostApocItemCategory::Drink)
				{
					FillState = EConsumableFillState::Full;
				}
			}

			FGuid AddedInstanceId;
			MerchantInventory->TryAddItem(ItemHandle, AddedInstanceId, 100, FillState);
		}
	}
}

void AMerchantBase::OnInteract_Implementation(AGercekCharacter* Player)
{
	if (!IsValid(Player))
	{
		return;
	}

	Player->ClientOpenTradeScreen(this);
}

FText AMerchantBase::GetInteractionPrompt_Implementation(AGercekCharacter* Player)
{
	const FText DisplayName = GetInteractableName_Implementation();
	if (DisplayName.IsEmpty())
	{
		return FText::FromString(TEXT("E - Takas"));
	}

	return FText::FromString(FString::Printf(TEXT("E - Takas %s"), *DisplayName.ToString()));
}

FText AMerchantBase::GetInteractableName_Implementation()
{
	return MerchantName;
}

FDataTableRowHandle AMerchantBase::GetItemData_Implementation()
{
	return FDataTableRowHandle();
}

FString AMerchantBase::GetPersistentId() const
{
	return PersistentId.IsEmpty() ? GetPathName() : PersistentId;
}

void AMerchantBase::ExportPersistentInventory(TArray<FGercekSavedGridSlot>& OutSlots,
	FString& OutDataTablePath) const
{
	OutSlots.Reset();
	OutDataTablePath.Reset();

	if (MerchantInventory)
	{
		TArray<FGridSlotData> RuntimeSlots;
		MerchantInventory->ExportSaveData(RuntimeSlots, OutDataTablePath);
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

void AMerchantBase::ImportPersistentInventory(
	const TArray<FGercekSavedGridSlot>& InSlots,
	UDataTable* InDataTable)
{
	if (MerchantInventory)
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

		MerchantInventory->ImportSaveData(RuntimeSlots, InDataTable);
	}
}
