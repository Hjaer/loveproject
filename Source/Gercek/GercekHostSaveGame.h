#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PostApocItemTypes.h"
#include "GercekHostSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FGercekSavedGridSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FIntPoint Location = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FGuid ItemInstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	bool bIsRotated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 Condition = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	EConsumableFillState FillState = EConsumableFillState::NotApplicable;
};

USTRUCT(BlueprintType)
struct FGercekSavedPlayerRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString SteamId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FVector PlayerLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FRotator PlayerRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float Hunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float Thirst = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float Stamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float Radiation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	float TradeXP = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	ETradeKnowledge CurrentKnowledge = ETradeKnowledge::Novice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TArray<FGercekSavedGridSlot> InventorySlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString InventoryDataTablePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FDateTime LastSeenUtc;
};

USTRUCT(BlueprintType)
struct FGercekSavedWorldState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString SessionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString MapPath = TEXT("/Game/Istanbul");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FDateTime LastSaveUtc;
};

USTRUCT(BlueprintType)
struct FGercekSavedDroppedItemRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString ItemDataTablePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FRotator WorldRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 Condition = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	EConsumableFillState FillState = EConsumableFillState::NotApplicable;
};

USTRUCT(BlueprintType)
struct FGercekSavedMerchantRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString MerchantPersistentId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TArray<FGercekSavedGridSlot> InventorySlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString InventoryDataTablePath;
};

USTRUCT(BlueprintType)
struct FGercekSavedContainerRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString ContainerPersistentId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TArray<FGercekSavedGridSlot> InventorySlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString InventoryDataTablePath;
};

USTRUCT(BlueprintType)
struct FGercekSavedInteractableState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString PersistentId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	bool bWasConsumed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	bool bIsOpened = false;
};

UCLASS()
class GERCEK_API UGercekHostSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	int32 SaveVersion = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FString IntegrityHash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FGercekSavedWorldState WorldState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TArray<FGercekSavedPlayerRecord> SavedPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TArray<FGercekSavedMerchantRecord> SavedMerchants;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TArray<FGercekSavedContainerRecord> SavedContainers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TArray<FGercekSavedDroppedItemRecord> SavedDroppedItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	TArray<FGercekSavedInteractableState> SavedInteractables;
};
