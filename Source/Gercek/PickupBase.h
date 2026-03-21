#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "PostApocItemTypes.h"
#include "PickupBase.generated.h"

class AGercekCharacter;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class GERCEK_API APickupBase : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	APickupBase();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	TObjectPtr<UStaticMeshComponent> PickupMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (RowType = "ItemDBRow"))
	FDataTableRowHandle ItemRowHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	float HeavyWeightThreshold = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup",
		meta = (ClampMin = "10", ClampMax = "100"))
	int32 ItemCondition = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	EConsumableLootFillRule ConsumableFillRule =
		EConsumableLootFillRule::StandardWorldLoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	EConsumableFillState ItemFillState = EConsumableFillState::NotApplicable;

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	virtual void InitializeItemData(const FDataTableRowHandle& InHandle,
		int32 InCondition = 100,
		EConsumableFillState InFillState = EConsumableFillState::NotApplicable);

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	virtual void Interact(AGercekCharacter* Player);

	virtual void OnInteract_Implementation(AGercekCharacter* Player) override;
	virtual FText GetInteractionPrompt_Implementation(AGercekCharacter* Player) override;
	virtual FText GetInteractableName_Implementation() override;
	virtual FDataTableRowHandle GetItemData_Implementation() override;

protected:
	const struct FItemDBRow* ResolveItemRow() const;
	void ApplyVisualFromData();
	EConsumableFillState ResolveFillStateForPickup() const;
};
