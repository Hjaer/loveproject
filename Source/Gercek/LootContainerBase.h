#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "GercekHostSaveGame.h"
#include "Interactable.h"
#include "PostApocInventoryTypes.h"
#include "PostApocItemTypes.h"
#include "LootContainerBase.generated.h"

class AGercekCharacter;
class UStaticMeshComponent;
class UWorldInventoryComponent;

UCLASS(Blueprintable)
class GERCEK_API ALootContainerBase : public AActor, public IInteractable
{
	GENERATED_BODY()

public:
	ALootContainerBase();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container")
	TObjectPtr<UStaticMeshComponent> ContainerMesh = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Container")
	TObjectPtr<UWorldInventoryComponent> ContainerInventory = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	FText ContainerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	TArray<FDataTableRowHandle> InitialLoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	EConsumableLootFillRule ConsumableFillRule =
		EConsumableLootFillRule::StandardWorldLoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	int32 GridColumns = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	int32 GridRows = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	float AccessDistance = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	bool bAllowSharedAccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
	FString PersistentId;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Container")
	UWorldInventoryComponent* GetContainerInventory() const { return ContainerInventory; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Persistence")
	FString GetPersistentId() const;

	UFUNCTION(BlueprintCallable, Category = "Persistence")
	void ExportPersistentInventory(TArray<FGercekSavedGridSlot>& OutSlots,
		FString& OutDataTablePath) const;

	UFUNCTION(BlueprintCallable, Category = "Persistence")
	void ImportPersistentInventory(const TArray<FGercekSavedGridSlot>& InSlots,
		UDataTable* InDataTable);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Container")
	bool IsAvailableForPlayer(const AGercekCharacter* Player) const;

	UFUNCTION(BlueprintCallable, Category = "Container")
	bool TransferItemToPlayer(AGercekCharacter* Player, FGuid ItemInstanceId);

	UFUNCTION(BlueprintCallable, Category = "Container")
	bool TransferItemFromPlayer(AGercekCharacter* Player, FGuid ItemInstanceId);

	UFUNCTION(BlueprintCallable, Category = "Container")
	void ReleaseContainer(AGercekCharacter* Player);

	virtual void OnInteract_Implementation(AGercekCharacter* Player) override;
	virtual FText GetInteractionPrompt_Implementation(AGercekCharacter* Player) override;
	virtual FText GetInteractableName_Implementation() override;
	virtual FDataTableRowHandle GetItemData_Implementation() override;

protected:
	void SetCurrentLootingPlayer(AGercekCharacter* Player);
	bool CanPlayerAccess(const AGercekCharacter* Player) const;
	void RefreshReplicatedUsageState();
	EConsumableFillState ResolveFillStateForItem(
		const FDataTableRowHandle& ItemHandle) const;

	UPROPERTY(Replicated)
	bool bIsInUse = false;

	UPROPERTY(Replicated)
	FString CurrentLootingPlayerName;

	TWeakObjectPtr<AGercekCharacter> CurrentLootingPlayer;
};
