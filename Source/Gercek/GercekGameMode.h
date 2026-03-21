#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GercekHostSaveGame.h"
#include "GercekGameMode.generated.h"

UCLASS()
class GERCEK_API AGercekGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGercekGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Save System")
	float AutosaveIntervalSeconds = 60.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Save System")
	float RestoreRetryIntervalSeconds = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = "Save System")
	int32 MaxRestoreRetryCount = 12;

private:
	void InitializeHostSaveState();
	void SaveControllerProgress(AController* Controller);
	void SnapshotActivePlayersIntoSave();
	void SnapshotExtensibleWorldState();
	void SaveHostProgressToDisk();
	void QueueRestorePlayerProgress(APlayerController* PlayerController);
	void AttemptRestorePlayerProgress(APlayerController* PlayerController, int32 AttemptNumber);
	void RestorePlayerProgress(APlayerController* PlayerController);
	void RestoreExtensibleWorldState();
	void ResolveDuplicateConnection(APlayerController* NewPlayer);
	APlayerController* FindDuplicatePlayerController(const FString& PersistentPlayerId, const APlayerController* ExcludedController) const;
	bool ValidateLoadedSaveIntegrity() const;
	FString ResolvePersistentPlayerId(AController* Controller) const;
	FString ResolveHostSaveSlotName() const;
	FString BuildIntegrityHash(const UGercekHostSaveGame* SaveGame) const;
	FGercekSavedPlayerRecord* FindSavedPlayerRecord(const FString& PersistentPlayerId);
	const FGercekSavedPlayerRecord* FindSavedPlayerRecord(const FString& PersistentPlayerId) const;
	FGercekSavedMerchantRecord* FindSavedMerchantRecord(const FString& PersistentId);
	const FGercekSavedMerchantRecord* FindSavedMerchantRecord(const FString& PersistentId) const;
	FGercekSavedContainerRecord* FindSavedContainerRecord(const FString& PersistentId);
	const FGercekSavedContainerRecord* FindSavedContainerRecord(const FString& PersistentId) const;
	void HandleAutosaveTimerElapsed();
	void HandleAsyncSaveFinished(const FString& SlotName, const int32 UserIndex, bool bSuccess);

	UPROPERTY()
	TObjectPtr<UGercekHostSaveGame> ActiveHostSave = nullptr;

	UPROPERTY()
	FString ActiveHostSaveSlotName;

	bool bLoadedSaveIntegrityVerified = false;
	bool bSaveInFlight = false;
	bool bPendingFollowupSave = false;

	FTimerHandle AutosaveTimerHandle;
};
