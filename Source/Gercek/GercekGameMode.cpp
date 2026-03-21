#include "GercekGameMode.h"

#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GercekCharacter.h"
#include "LootContainerBase.h"
#include "MerchantBase.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/SecureHash.h"
#include "MultiplayerSessionSubsystem.h"
#include "PostApocInventoryTypes.h"
#include "WorldItemActor.h"
#include "EngineUtils.h"

AGercekGameMode::AGercekGameMode()
{
	bUseSeamlessTravel = true;
}

void AGercekGameMode::BeginPlay()
{
	Super::BeginPlay();

	InitializeHostSaveState();

	if (HasAuthority() && AutosaveIntervalSeconds > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			AutosaveTimerHandle,
			this,
			&AGercekGameMode::HandleAutosaveTimerElapsed,
			AutosaveIntervalSeconds,
			true);
	}
}

void AGercekGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UE_LOG(LogTemp, Display, TEXT("[GercekGameMode] Yeni bir oyuncu sunucuya katildi."));

	if (!HasAuthority() || !IsValid(NewPlayer))
	{
		return;
	}

	ResolveDuplicateConnection(NewPlayer);
	QueueRestorePlayerProgress(NewPlayer);
}

void AGercekGameMode::Logout(AController* Exiting)
{
	if (HasAuthority() && ActiveHostSave && IsValid(Exiting))
	{
		SaveControllerProgress(Exiting);
		SaveHostProgressToDisk();
	}

	Super::Logout(Exiting);
}

void AGercekGameMode::InitializeHostSaveState()
{
	if (!HasAuthority())
	{
		return;
	}

	ActiveHostSaveSlotName = ResolveHostSaveSlotName();
	if (ActiveHostSaveSlotName.IsEmpty())
	{
		ActiveHostSaveSlotName = TEXT("Gercek_HostWorld");
	}

	const bool bShouldLoadExistingSave = [&]() -> bool
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UMultiplayerSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>())
			{
				const FGercekContinueSessionInfo ContinueInfo = SessionSubsystem->GetContinueSaveInfo();
				return ContinueInfo.bHasSave && ContinueInfo.bWasHost && ContinueInfo.bIsContinueSession;
			}
		}
		return false;
	}();

	if (bShouldLoadExistingSave && UGameplayStatics::DoesSaveGameExist(ActiveHostSaveSlotName, 0))
	{
		ActiveHostSave = Cast<UGercekHostSaveGame>(UGameplayStatics::LoadGameFromSlot(ActiveHostSaveSlotName, 0));
	}

	if (!ActiveHostSave)
	{
		ActiveHostSave = Cast<UGercekHostSaveGame>(UGameplayStatics::CreateSaveGameObject(UGercekHostSaveGame::StaticClass()));
	}

	if (!ActiveHostSave)
	{
		return;
	}

	bLoadedSaveIntegrityVerified = ValidateLoadedSaveIntegrity();
	if (!bLoadedSaveIntegrityVerified)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GercekGameMode] Host save butunluk kontrolunu gecemedi. Yeni save olusturuluyor."));
		ActiveHostSave = Cast<UGercekHostSaveGame>(UGameplayStatics::CreateSaveGameObject(UGercekHostSaveGame::StaticClass()));
		bLoadedSaveIntegrityVerified = true;
		if (!ActiveHostSave)
		{
			return;
		}
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UMultiplayerSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>())
		{
			const FGercekContinueSessionInfo ContinueInfo = SessionSubsystem->GetContinueSaveInfo();
			ActiveHostSave->WorldState.SessionId = ContinueInfo.SessionId;
			ActiveHostSave->WorldState.MapPath = ContinueInfo.MapPath.IsEmpty() ? TEXT("/Game/Istanbul") : ContinueInfo.MapPath;
		}
	}

	ActiveHostSave->WorldState.LastSaveUtc = FDateTime::UtcNow();
	ActiveHostSave->SaveVersion = 4;
	ActiveHostSave->IntegrityHash = BuildIntegrityHash(ActiveHostSave);

	RestoreExtensibleWorldState();
}

void AGercekGameMode::SaveControllerProgress(AController* Controller)
{
	if (!HasAuthority() || !ActiveHostSave || !IsValid(Controller))
	{
		return;
	}

	const FString PersistentPlayerId = ResolvePersistentPlayerId(Controller);
	if (PersistentPlayerId.IsEmpty())
	{
		return;
	}

	FGercekSavedPlayerRecord* ExistingRecord = FindSavedPlayerRecord(PersistentPlayerId);
	if (!ExistingRecord)
	{
		FGercekSavedPlayerRecord NewRecord;
		NewRecord.SteamId = PersistentPlayerId;
		if (Controller->PlayerState)
		{
			NewRecord.PlayerName = Controller->PlayerState->GetPlayerName();
		}
		ActiveHostSave->SavedPlayers.Add(NewRecord);
		ExistingRecord = &ActiveHostSave->SavedPlayers.Last();
	}

	if (Controller->PlayerState)
	{
		ExistingRecord->PlayerName = Controller->PlayerState->GetPlayerName();
	}

	if (const AGercekCharacter* GercekCharacter = Cast<AGercekCharacter>(Controller->GetPawn()))
	{
		GercekCharacter->BuildPersistentPlayerRecord(*ExistingRecord);
	}
	else
	{
		ExistingRecord->LastSeenUtc = FDateTime::UtcNow();
	}
}

void AGercekGameMode::SnapshotActivePlayersIntoSave()
{
	if (!HasAuthority() || !ActiveHostSave)
	{
		return;
	}

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PlayerController = Iterator->Get();
		if (!IsValid(PlayerController))
		{
			continue;
		}

		SaveControllerProgress(PlayerController);
	}
}

void AGercekGameMode::SaveHostProgressToDisk()
{
	if (!HasAuthority() || !ActiveHostSave)
	{
		return;
	}

	if (bSaveInFlight)
	{
		bPendingFollowupSave = true;
		return;
	}

	SnapshotActivePlayersIntoSave();
	SnapshotExtensibleWorldState();
	ActiveHostSave->WorldState.LastSaveUtc = FDateTime::UtcNow();
	ActiveHostSave->SaveVersion = 4;
	ActiveHostSave->IntegrityHash = BuildIntegrityHash(ActiveHostSave);
	bSaveInFlight = true;

	FAsyncSaveGameToSlotDelegate SaveDelegate;
	SaveDelegate.BindUObject(this, &AGercekGameMode::HandleAsyncSaveFinished);
	UGameplayStatics::AsyncSaveGameToSlot(ActiveHostSave, ActiveHostSaveSlotName, 0,
		SaveDelegate);
}

void AGercekGameMode::QueueRestorePlayerProgress(APlayerController* PlayerController)
{
	AttemptRestorePlayerProgress(PlayerController, 0);
}

void AGercekGameMode::AttemptRestorePlayerProgress(APlayerController* PlayerController, int32 AttemptNumber)
{
	if (!HasAuthority() || !IsValid(PlayerController))
	{
		return;
	}

	if (!PlayerController->GetPawn())
	{
		if (AttemptNumber < MaxRestoreRetryCount)
		{
			FTimerHandle RetryTimerHandle;
			FTimerDelegate RetryDelegate;
			RetryDelegate.BindUObject(this, &AGercekGameMode::AttemptRestorePlayerProgress, PlayerController, AttemptNumber + 1);
			GetWorldTimerManager().SetTimer(RetryTimerHandle, RetryDelegate, RestoreRetryIntervalSeconds, false);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[GercekGameMode] Restore zaman asimina ugradi: %s"),
				PlayerController->PlayerState ? *PlayerController->PlayerState->GetPlayerName() : TEXT("Unknown"));
		}
		return;
	}

	RestorePlayerProgress(PlayerController);
}

void AGercekGameMode::RestorePlayerProgress(APlayerController* PlayerController)
{
	if (!HasAuthority() || !ActiveHostSave || !bLoadedSaveIntegrityVerified || !IsValid(PlayerController))
	{
		return;
	}

	AGercekCharacter* GercekCharacter = Cast<AGercekCharacter>(PlayerController->GetPawn());
	if (!GercekCharacter)
	{
		return;
	}

	const FString PersistentPlayerId = ResolvePersistentPlayerId(PlayerController);
	if (PersistentPlayerId.IsEmpty())
	{
		return;
	}

	if (const FGercekSavedPlayerRecord* SavedRecord = FindSavedPlayerRecord(PersistentPlayerId))
	{
		GercekCharacter->ApplyPersistentPlayerRecord(*SavedRecord);
	}
}

void AGercekGameMode::ResolveDuplicateConnection(APlayerController* NewPlayer)
{
	if (!HasAuthority() || !IsValid(NewPlayer))
	{
		return;
	}

	const FString PersistentPlayerId = ResolvePersistentPlayerId(NewPlayer);
	if (PersistentPlayerId.IsEmpty())
	{
		return;
	}

	if (APlayerController* DuplicateController = FindDuplicatePlayerController(PersistentPlayerId, NewPlayer))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GercekGameMode] Ayni Steam ID ile reconnect tespit edildi. Eski baglanti dusuruluyor: %s"), *PersistentPlayerId);
		SaveControllerProgress(DuplicateController);

		if (GameSession)
		{
			GameSession->KickPlayer(DuplicateController, FText::FromString(TEXT("Yeni bir baglanti ile tekrar giris yapildi.")));
		}
	}
}

APlayerController* AGercekGameMode::FindDuplicatePlayerController(const FString& PersistentPlayerId, const APlayerController* ExcludedController) const
{
	if (PersistentPlayerId.IsEmpty())
	{
		return nullptr;
	}

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* ExistingController = Iterator->Get();
		if (!IsValid(ExistingController) || ExistingController == ExcludedController)
		{
			continue;
		}

		if (ResolvePersistentPlayerId(ExistingController) == PersistentPlayerId)
		{
			return ExistingController;
		}
	}

	return nullptr;
}

FString AGercekGameMode::ResolvePersistentPlayerId(AController* Controller) const
{
	if (!IsValid(Controller) || !Controller->PlayerState)
	{
		return FString();
	}

	const FUniqueNetIdRepl& UniqueId = Controller->PlayerState->GetUniqueId();
	if (UniqueId.IsValid())
	{
		return UniqueId->ToString();
	}

	return Controller->PlayerState->GetPlayerName();
}

void AGercekGameMode::SnapshotExtensibleWorldState()
{
	if (!ActiveHostSave)
	{
		return;
	}

	ActiveHostSave->SavedMerchants.Reset();
	ActiveHostSave->SavedContainers.Reset();
	ActiveHostSave->SavedDroppedItems.Reset();
	ActiveHostSave->SavedInteractables.Reset();

	for (TActorIterator<AMerchantBase> It(GetWorld()); It; ++It)
	{
		AMerchantBase* Merchant = *It;
		if (!IsValid(Merchant) || !Merchant->GetMerchantInventory())
		{
			continue;
		}

		FGercekSavedMerchantRecord MerchantRecord;
		MerchantRecord.MerchantPersistentId = Merchant->GetPersistentId();
		Merchant->ExportPersistentInventory(MerchantRecord.InventorySlots,
			MerchantRecord.InventoryDataTablePath);
		ActiveHostSave->SavedMerchants.Add(MerchantRecord);
	}

	for (TActorIterator<ALootContainerBase> It(GetWorld()); It; ++It)
	{
		ALootContainerBase* Container = *It;
		if (!IsValid(Container) || !Container->GetContainerInventory())
		{
			continue;
		}

		FGercekSavedContainerRecord ContainerRecord;
		ContainerRecord.ContainerPersistentId = Container->GetPersistentId();
		Container->ExportPersistentInventory(ContainerRecord.InventorySlots,
			ContainerRecord.InventoryDataTablePath);
		ActiveHostSave->SavedContainers.Add(ContainerRecord);
	}

	for (TActorIterator<AWorldItemActor> It(GetWorld()); It; ++It)
	{
		AWorldItemActor* DroppedItem = *It;
		if (!IsValid(DroppedItem) || !DroppedItem->IsPersistentWorldDrop())
		{
			continue;
		}

		const FDataTableRowHandle ItemHandle = DroppedItem->GetItemData_Implementation();
		if (ItemHandle.IsNull() || !ItemHandle.DataTable)
		{
			continue;
		}

		FGercekSavedDroppedItemRecord DroppedRecord;
		DroppedRecord.ItemDataTablePath = ItemHandle.DataTable->GetPathName();
		DroppedRecord.ItemId = ItemHandle.RowName;
		DroppedRecord.WorldLocation = DroppedItem->GetActorLocation();
		DroppedRecord.WorldRotation = DroppedItem->GetActorRotation();
		DroppedRecord.Condition = DroppedItem->GetItemCondition();
		DroppedRecord.FillState = DroppedItem->GetItemFillState();
		ActiveHostSave->SavedDroppedItems.Add(DroppedRecord);
	}

	ActiveHostSave->SavedMerchants.Sort([](const FGercekSavedMerchantRecord& A,
		const FGercekSavedMerchantRecord& B)
	{
		return A.MerchantPersistentId < B.MerchantPersistentId;
	});

	ActiveHostSave->SavedContainers.Sort([](const FGercekSavedContainerRecord& A,
		const FGercekSavedContainerRecord& B)
	{
		return A.ContainerPersistentId < B.ContainerPersistentId;
	});

	ActiveHostSave->SavedDroppedItems.Sort([](const FGercekSavedDroppedItemRecord& A,
		const FGercekSavedDroppedItemRecord& B)
	{
		if (A.WorldLocation.X != B.WorldLocation.X)
		{
			return A.WorldLocation.X < B.WorldLocation.X;
		}
		if (A.WorldLocation.Y != B.WorldLocation.Y)
		{
			return A.WorldLocation.Y < B.WorldLocation.Y;
		}
		return A.WorldLocation.Z < B.WorldLocation.Z;
	});
}

void AGercekGameMode::RestoreExtensibleWorldState()
{
	if (!ActiveHostSave)
	{
		return;
	}

	for (TActorIterator<AMerchantBase> It(GetWorld()); It; ++It)
	{
		AMerchantBase* Merchant = *It;
		if (!IsValid(Merchant))
		{
			continue;
		}

		if (const FGercekSavedMerchantRecord* SavedRecord =
				FindSavedMerchantRecord(Merchant->GetPersistentId()))
		{
			UDataTable* LoadedDataTable = Merchant->GetMerchantInventory()
				? Merchant->GetMerchantInventory()->ItemDataTable
				: nullptr;
			if (!SavedRecord->InventoryDataTablePath.IsEmpty())
			{
				LoadedDataTable = LoadObject<UDataTable>(nullptr,
					*SavedRecord->InventoryDataTablePath);
			}

			Merchant->ImportPersistentInventory(SavedRecord->InventorySlots,
				LoadedDataTable);
		}
	}

	for (TActorIterator<ALootContainerBase> It(GetWorld()); It; ++It)
	{
		ALootContainerBase* Container = *It;
		if (!IsValid(Container))
		{
			continue;
		}

		if (const FGercekSavedContainerRecord* SavedRecord =
				FindSavedContainerRecord(Container->GetPersistentId()))
		{
			UDataTable* LoadedDataTable = Container->GetContainerInventory()
				? Container->GetContainerInventory()->ItemDataTable
				: nullptr;
			if (!SavedRecord->InventoryDataTablePath.IsEmpty())
			{
				LoadedDataTable = LoadObject<UDataTable>(nullptr,
					*SavedRecord->InventoryDataTablePath);
			}

			Container->ImportPersistentInventory(SavedRecord->InventorySlots,
				LoadedDataTable);
		}
	}

	for (TActorIterator<AWorldItemActor> It(GetWorld()); It; ++It)
	{
		AWorldItemActor* DroppedItem = *It;
		if (IsValid(DroppedItem) && DroppedItem->IsPersistentWorldDrop())
		{
			DroppedItem->Destroy();
		}
	}

	for (const FGercekSavedDroppedItemRecord& DroppedRecord :
		ActiveHostSave->SavedDroppedItems)
	{
		if (DroppedRecord.ItemId.IsNone() || DroppedRecord.ItemDataTablePath.IsEmpty())
		{
			continue;
		}

		UDataTable* LoadedDataTable =
			LoadObject<UDataTable>(nullptr, *DroppedRecord.ItemDataTablePath);
		if (!LoadedDataTable)
		{
			continue;
		}

		FDataTableRowHandle ItemHandle;
		ItemHandle.DataTable = LoadedDataTable;
		ItemHandle.RowName = DroppedRecord.ItemId;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		if (AWorldItemActor* SpawnedDrop = GetWorld()->SpawnActor<AWorldItemActor>(
				AWorldItemActor::StaticClass(), DroppedRecord.WorldLocation,
				DroppedRecord.WorldRotation, SpawnParams))
		{
			SpawnedDrop->InitializeItemData(ItemHandle, DroppedRecord.Condition,
				DroppedRecord.FillState);
			SpawnedDrop->SetPersistentWorldDrop(true);
		}
	}
}

FString AGercekGameMode::ResolveHostSaveSlotName() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const UMultiplayerSessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>())
		{
			const FGercekContinueSessionInfo ContinueInfo = SessionSubsystem->GetContinueSaveInfo();
			if (!ContinueInfo.SaveSlotName.IsEmpty())
			{
				return ContinueInfo.SaveSlotName;
			}
		}
	}

	return TEXT("Gercek_HostWorld");
}

FString AGercekGameMode::BuildIntegrityHash(const UGercekHostSaveGame* SaveGame) const
{
	if (!SaveGame)
	{
		return FString();
	}

	FString SourceString = FString::Printf(
		TEXT("%d|%s|%s|%d|%d|%d|%d|%d"),
		SaveGame->SaveVersion,
		*SaveGame->WorldState.SessionId,
		*SaveGame->WorldState.MapPath,
		SaveGame->SavedPlayers.Num(),
		SaveGame->SavedMerchants.Num(),
		SaveGame->SavedContainers.Num(),
		SaveGame->SavedDroppedItems.Num(),
		SaveGame->SavedInteractables.Num());

	for (const FGercekSavedPlayerRecord& PlayerRecord : SaveGame->SavedPlayers)
	{
		SourceString += FString::Printf(
			TEXT("|%s|%s|%.2f|%.2f|%.2f|%.2f|%.2f|%.2f|%s|%s|%d"),
			*PlayerRecord.SteamId,
			*PlayerRecord.PlayerName,
			PlayerRecord.Health,
			PlayerRecord.Hunger,
			PlayerRecord.Thirst,
			PlayerRecord.Stamina,
			PlayerRecord.Radiation,
			PlayerRecord.TradeXP,
			*PlayerRecord.PlayerLocation.ToString(),
			*PlayerRecord.PlayerRotation.ToString(),
			PlayerRecord.InventorySlots.Num());

		for (const FGercekSavedGridSlot& GridSlot : PlayerRecord.InventorySlots)
		{
			SourceString += FString::Printf(TEXT("|%s|%s|%s|%d|%d|%d"),
				*GridSlot.Location.ToString(),
				*GridSlot.ItemId.ToString(),
				*GridSlot.ItemInstanceId.ToString(EGuidFormats::DigitsWithHyphensInBraces),
				GridSlot.bIsRotated ? 1 : 0,
				GridSlot.Condition,
				static_cast<int32>(GridSlot.FillState));
		}
	}

	for (const FGercekSavedMerchantRecord& MerchantRecord : SaveGame->SavedMerchants)
	{
		SourceString += FString::Printf(TEXT("|%s|%s|%d"),
			*MerchantRecord.MerchantPersistentId,
			*MerchantRecord.InventoryDataTablePath,
			MerchantRecord.InventorySlots.Num());

		for (const FGercekSavedGridSlot& GridSlot : MerchantRecord.InventorySlots)
		{
			SourceString += FString::Printf(TEXT("|%s|%s|%s|%d|%d|%d"),
				*GridSlot.Location.ToString(),
				*GridSlot.ItemId.ToString(),
				*GridSlot.ItemInstanceId.ToString(EGuidFormats::DigitsWithHyphensInBraces),
				GridSlot.bIsRotated ? 1 : 0,
				GridSlot.Condition,
				static_cast<int32>(GridSlot.FillState));
		}
	}

	for (const FGercekSavedContainerRecord& ContainerRecord : SaveGame->SavedContainers)
	{
		SourceString += FString::Printf(TEXT("|%s|%s|%d"),
			*ContainerRecord.ContainerPersistentId,
			*ContainerRecord.InventoryDataTablePath,
			ContainerRecord.InventorySlots.Num());

		for (const FGercekSavedGridSlot& GridSlot : ContainerRecord.InventorySlots)
		{
			SourceString += FString::Printf(TEXT("|%s|%s|%s|%d|%d|%d"),
				*GridSlot.Location.ToString(),
				*GridSlot.ItemId.ToString(),
				*GridSlot.ItemInstanceId.ToString(EGuidFormats::DigitsWithHyphensInBraces),
				GridSlot.bIsRotated ? 1 : 0,
				GridSlot.Condition,
				static_cast<int32>(GridSlot.FillState));
		}
	}

	for (const FGercekSavedDroppedItemRecord& DroppedRecord :
		SaveGame->SavedDroppedItems)
	{
		SourceString += FString::Printf(TEXT("|%s|%s|%s|%s|%d|%d"),
			*DroppedRecord.ItemDataTablePath,
			*DroppedRecord.ItemId.ToString(),
			*DroppedRecord.WorldLocation.ToString(),
			*DroppedRecord.WorldRotation.ToString(),
			DroppedRecord.Condition,
			static_cast<int32>(DroppedRecord.FillState));
	}

	return FMD5::HashAnsiString(*SourceString);
}

bool AGercekGameMode::ValidateLoadedSaveIntegrity() const
{
	if (!ActiveHostSave)
	{
		return false;
	}

	if (ActiveHostSave->IntegrityHash.IsEmpty())
	{
		return ActiveHostSave->SavedPlayers.Num() == 0 &&
			ActiveHostSave->SavedMerchants.Num() == 0 &&
			ActiveHostSave->SavedContainers.Num() == 0 &&
			ActiveHostSave->SavedDroppedItems.Num() == 0 &&
			ActiveHostSave->SavedInteractables.Num() == 0;
	}

	if (ActiveHostSave->SaveVersion < 4)
	{
		return true;
	}

	return ActiveHostSave->IntegrityHash == BuildIntegrityHash(ActiveHostSave);
}

FGercekSavedPlayerRecord* AGercekGameMode::FindSavedPlayerRecord(const FString& PersistentPlayerId)
{
	if (!ActiveHostSave)
	{
		return nullptr;
	}

	for (FGercekSavedPlayerRecord& PlayerRecord : ActiveHostSave->SavedPlayers)
	{
		if (PlayerRecord.SteamId == PersistentPlayerId)
		{
			return &PlayerRecord;
		}
	}

	return nullptr;
}

const FGercekSavedPlayerRecord* AGercekGameMode::FindSavedPlayerRecord(const FString& PersistentPlayerId) const
{
	if (!ActiveHostSave)
	{
		return nullptr;
	}

	for (const FGercekSavedPlayerRecord& PlayerRecord : ActiveHostSave->SavedPlayers)
	{
		if (PlayerRecord.SteamId == PersistentPlayerId)
		{
			return &PlayerRecord;
		}
	}

	return nullptr;
}

FGercekSavedMerchantRecord* AGercekGameMode::FindSavedMerchantRecord(
	const FString& PersistentId)
{
	if (!ActiveHostSave)
	{
		return nullptr;
	}

	for (FGercekSavedMerchantRecord& MerchantRecord : ActiveHostSave->SavedMerchants)
	{
		if (MerchantRecord.MerchantPersistentId == PersistentId)
		{
			return &MerchantRecord;
		}
	}

	return nullptr;
}

const FGercekSavedMerchantRecord* AGercekGameMode::FindSavedMerchantRecord(
	const FString& PersistentId) const
{
	if (!ActiveHostSave)
	{
		return nullptr;
	}

	for (const FGercekSavedMerchantRecord& MerchantRecord : ActiveHostSave->SavedMerchants)
	{
		if (MerchantRecord.MerchantPersistentId == PersistentId)
		{
			return &MerchantRecord;
		}
	}

	return nullptr;
}

FGercekSavedContainerRecord* AGercekGameMode::FindSavedContainerRecord(
	const FString& PersistentId)
{
	if (!ActiveHostSave)
	{
		return nullptr;
	}

	for (FGercekSavedContainerRecord& ContainerRecord : ActiveHostSave->SavedContainers)
	{
		if (ContainerRecord.ContainerPersistentId == PersistentId)
		{
			return &ContainerRecord;
		}
	}

	return nullptr;
}

const FGercekSavedContainerRecord* AGercekGameMode::FindSavedContainerRecord(
	const FString& PersistentId) const
{
	if (!ActiveHostSave)
	{
		return nullptr;
	}

	for (const FGercekSavedContainerRecord& ContainerRecord :
		ActiveHostSave->SavedContainers)
	{
		if (ContainerRecord.ContainerPersistentId == PersistentId)
		{
			return &ContainerRecord;
		}
	}

	return nullptr;
}

void AGercekGameMode::HandleAutosaveTimerElapsed()
{
	SaveHostProgressToDisk();
}

void AGercekGameMode::HandleAsyncSaveFinished(const FString& SlotName,
	const int32 UserIndex, bool bSuccess)
{
	bSaveInFlight = false;

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[GercekGameMode] Async save basarisiz oldu. Slot: %s UserIndex: %d"),
			*SlotName, UserIndex);
	}

	if (bPendingFollowupSave)
	{
		bPendingFollowupSave = false;
		SaveHostProgressToDisk();
	}
}
