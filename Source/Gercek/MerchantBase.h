#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "GercekHostSaveGame.h"
#include "Interactable.h"
#include "PostApocInventoryTypes.h"
#include "MerchantBase.generated.h"

class UWorldInventoryComponent;

UCLASS()
class GERCEK_API AMerchantBase : public ACharacter, public IInteractable
{
	GENERATED_BODY()

public:
	AMerchantBase();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant Info")
	FText MerchantName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Merchant Inventory")
	UWorldInventoryComponent* MerchantInventory = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant Inventory")
	int32 GridColumns = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant Inventory")
	int32 GridRows = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant Inventory")
	TArray<FDataTableRowHandle> InitialStock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Persistence")
	FString PersistentId;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Merchant Info")
	FText GetMerchantName() const { return MerchantName; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Merchant Inventory")
	UWorldInventoryComponent* GetMerchantInventory() const { return MerchantInventory; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Persistence")
	FString GetPersistentId() const;

	UFUNCTION(BlueprintCallable, Category = "Persistence")
	void ExportPersistentInventory(TArray<FGercekSavedGridSlot>& OutSlots,
		FString& OutDataTablePath) const;

	UFUNCTION(BlueprintCallable, Category = "Persistence")
	void ImportPersistentInventory(const TArray<FGercekSavedGridSlot>& InSlots,
		UDataTable* InDataTable);

	virtual void OnInteract_Implementation(class AGercekCharacter* Player) override;
	virtual FText GetInteractionPrompt_Implementation(class AGercekCharacter* Player) override;
	virtual FText GetInteractableName_Implementation() override;
	virtual FDataTableRowHandle GetItemData_Implementation() override;
};
