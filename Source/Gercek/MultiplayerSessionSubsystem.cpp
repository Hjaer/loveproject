#include "MultiplayerSessionSubsystem.h"

#include "Interfaces/OnlineExternalUIInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlinePresenceInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Guid.h"
#include "Misc/SecureHash.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

namespace GercekSessionKeys
{
	static const FName MatchType(TEXT("MATCH_TYPE"));
	static const FName PresenceSearch(TEXT("PRESENCESEARCH"));
	static const FName ServerVisibility(TEXT("SERVER_VISIBILITY"));
	static const FName PasswordProtected(TEXT("PASSWORD_PROTECTED"));
	static const FName PasswordHash(TEXT("PASSWORD_HASH"));
	static const FName SessionId(TEXT("SESSION_ID"));
	static const FName SaveSlot(TEXT("SAVE_SLOT"));
	static const FName HostName(TEXT("HOST_NAME"));
	static const FName HostNetId(TEXT("HOST_NET_ID"));
	static const FName ContinueSession(TEXT("CONTINUE_SESSION"));
	static const FName MapPath(TEXT("MAP_PATH"));
}

namespace GercekSaveSlots
{
	static const TCHAR* ContinueMetadata = TEXT("Gercek_CoopSession");
}

UMultiplayerSessionSubsystem::UMultiplayerSessionSubsystem()
{
}

void UMultiplayerSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();
	}

	LoadContinueSessionCache();
	BroadcastContinueState();
}

void UMultiplayerSessionSubsystem::CreateServer(int32 MaxPlayers, bool bIsLAN)
{
	FGercekSessionConfig SessionConfig;
	SessionConfig.MaxPlayers = MaxPlayers;
	SessionConfig.bIsLAN = bIsLAN;
	CreateServerWithConfig(SessionConfig);
}

void UMultiplayerSessionSubsystem::CreateServerWithConfig(const FGercekSessionConfig& SessionConfig)
{
	if (!SessionInterface.IsValid())
	{
		BroadcastOperationMessage(TEXT("Steam session arayuzu bulunamadi."));
		OnCreateSessionCompleteEvent.Broadcast(false);
		return;
	}

	PendingSessionConfig = SessionConfig;
	if (PendingSessionConfig.MaxPlayers < 1)
	{
		PendingSessionConfig.MaxPlayers = 1;
	}
	if (PendingSessionConfig.MaxPlayers > 4)
	{
		PendingSessionConfig.MaxPlayers = 4;
	}
	if (PendingSessionConfig.MapPath.IsEmpty())
	{
		PendingSessionConfig.MapPath = TEXT("/Game/Istanbul");
	}
	if (PendingSessionConfig.SaveSlotName.IsEmpty())
	{
		PendingSessionConfig.SaveSlotName = TEXT("Gercek_HostWorld");
	}
	if (PendingSessionConfig.SessionId.IsEmpty())
	{
		PendingSessionConfig.SessionId = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);
	}

	if (const FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession))
	{
		bCreateSessionAfterDestroy = true;
		PendingMaxPlayers = PendingSessionConfig.MaxPlayers;
		bPendingIsLAN = PendingSessionConfig.bIsLAN;
		SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
			FOnDestroySessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionSubsystem::OnDestroySessionComplete));
		SessionInterface->DestroySession(ExistingSession->SessionName);
		return;
	}

	SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionSubsystem::OnCreateSessionComplete));

	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShared<FOnlineSessionSettings>();
	SessionSettings->bIsLANMatch = PendingSessionConfig.bIsLAN;
	SessionSettings->NumPublicConnections = PendingSessionConfig.Visibility == EGercekServerVisibility::Private ? 0 : PendingSessionConfig.MaxPlayers;
	SessionSettings->NumPrivateConnections = PendingSessionConfig.Visibility == EGercekServerVisibility::Private ? PendingSessionConfig.MaxPlayers : 0;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowInvites = true;
	SessionSettings->bAllowJoinViaPresence = PendingSessionConfig.Visibility != EGercekServerVisibility::Private;
	SessionSettings->bAllowJoinViaPresenceFriendsOnly = PendingSessionConfig.Visibility == EGercekServerVisibility::FriendsOnly;
	SessionSettings->bShouldAdvertise = PendingSessionConfig.Visibility != EGercekServerVisibility::Private;
	SessionSettings->bUsesPresence = true;
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->bUseLobbiesVoiceChatIfAvailable = true;

	const FString PasswordHash = HashPassword(PendingSessionConfig.Password);
	const FString HostName = GetLocalPlayerNickname();
	const FString LocalNetId = GetLocalNetIdString();

	SessionSettings->Set(GercekSessionKeys::MatchType, FString(TEXT("GercekCoop")), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(GercekSessionKeys::ServerVisibility, static_cast<int32>(PendingSessionConfig.Visibility), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(GercekSessionKeys::PasswordProtected, !PendingSessionConfig.Password.IsEmpty(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(GercekSessionKeys::PasswordHash, PasswordHash, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(GercekSessionKeys::SessionId, PendingSessionConfig.SessionId, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(GercekSessionKeys::SaveSlot, PendingSessionConfig.SaveSlotName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(GercekSessionKeys::HostName, HostName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(GercekSessionKeys::HostNetId, LocalNetId, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(GercekSessionKeys::ContinueSession, PendingSessionConfig.bIsContinueSession, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	SessionSettings->Set(GercekSessionKeys::MapPath, PendingSessionConfig.MapPath, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	SessionInterface->CreateSession(0, NAME_GameSession, *SessionSettings);
}

void UMultiplayerSessionSubsystem::ContinueLastSession()
{
	if (!CachedContinueInfo.bHasSave)
	{
		BroadcastOperationMessage(TEXT("Devam edilecek kayitli bir co-op oturumu bulunamadi."));
		return;
	}

	const FString LocalNetId = GetLocalNetIdString();
	if (CachedContinueInfo.bWasHost || (!CachedContinueInfo.HostNetId.IsEmpty() && CachedContinueInfo.HostNetId == LocalNetId))
	{
		CreateServerWithConfig(BuildConfigFromContinueSave());
		return;
	}

	bAutoJoinContinueSession = true;
	PendingContinueSessionId = CachedContinueInfo.SessionId;
	PendingJoinPassword = CachedContinueInfo.Password;
	FindServers(false);
}

void UMultiplayerSessionSubsystem::FindServer(int32 MaxSearchResults, bool bIsLAN)
{
	FindServers(false, MaxSearchResults, bIsLAN);
}

void UMultiplayerSessionSubsystem::FindServers(bool bOnlyFriendsSessions, int32 MaxSearchResults, bool bIsLAN)
{
	if (!SessionInterface.IsValid())
	{
		BroadcastOperationMessage(TEXT("Steam session arayuzu bulunamadi."));
		TArray<FBlueprintSessionResult> EmptyResults;
		OnFindSessionsCompleteEvent.Broadcast(false, EmptyResults);
		OnServerBrowserResultsUpdatedEvent.Broadcast(TArray<FGercekSessionBrowserResult>());
		return;
	}

	SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMultiplayerSessionSubsystem::OnFindSessionsComplete));

	LastSessionSearch = MakeShared<FOnlineSessionSearch>();
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = bIsLAN;
	LastSessionSearch->PingBucketSize = 50;
	LastSessionSearch->QuerySettings.Set(GercekSessionKeys::PresenceSearch, true, EOnlineComparisonOp::Equals);

	bPendingFindFriendsOnly = bOnlyFriendsSessions;

	if (!GetWorld())
	{
		BroadcastOperationMessage(TEXT("Dunya baglami bulunamadi."));
		TArray<FBlueprintSessionResult> EmptyResults;
		OnFindSessionsCompleteEvent.Broadcast(false, EmptyResults);
		OnServerBrowserResultsUpdatedEvent.Broadcast(TArray<FGercekSessionBrowserResult>());
		return;
	}

	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!LocalPlayer || !LocalPlayer->GetPreferredUniqueNetId().IsValid())
	{
		BroadcastOperationMessage(TEXT("Steam kimligi alinmadan sunucu aramasi yapilamadi."));
		TArray<FBlueprintSessionResult> EmptyResults;
		OnFindSessionsCompleteEvent.Broadcast(false, EmptyResults);
		OnServerBrowserResultsUpdatedEvent.Broadcast(TArray<FGercekSessionBrowserResult>());
		return;
	}

	SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef());
}

void UMultiplayerSessionSubsystem::JoinServer(const FBlueprintSessionResult& SessionResult)
{
	JoinServerWithPassword(SessionResult, FString());
}

void UMultiplayerSessionSubsystem::JoinServerWithPassword(const FBlueprintSessionResult& SessionResult, const FString& Password)
{
	if (!SessionInterface.IsValid())
	{
		BroadcastOperationMessage(TEXT("Steam session arayuzu bulunamadi."));
		OnJoinSessionCompleteEvent.Broadcast(false);
		return;
	}

	if (!ValidatePasswordForSession(SessionResult.OnlineResult, Password))
	{
		BroadcastOperationMessage(TEXT("Sunucu sifresi hatali veya eksik."));
		OnJoinSessionCompleteEvent.Broadcast(false);
		return;
	}

	PendingJoinBrowserResult = BuildBrowserResult(SessionResult.OnlineResult);
	PendingJoinPassword = Password;

	SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionSubsystem::OnJoinSessionComplete));

	if (!GetWorld())
	{
		BroadcastOperationMessage(TEXT("Dunya baglami bulunamadi."));
		OnJoinSessionCompleteEvent.Broadcast(false);
		ResetPendingJoinState();
		return;
	}

	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!LocalPlayer || !LocalPlayer->GetPreferredUniqueNetId().IsValid())
	{
		BroadcastOperationMessage(TEXT("Steam kimligi alinmadan katilma yapilamadi."));
		OnJoinSessionCompleteEvent.Broadcast(false);
		ResetPendingJoinState();
		return;
	}

	SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult.OnlineResult);
}

void UMultiplayerSessionSubsystem::RequestSteamFriends()
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		if (IOnlineFriendsPtr FriendsInterface = Subsystem->GetFriendsInterface())
		{
			const FString FriendsListName = TEXT("default");
			if (FriendsInterface->ReadFriendsList(0, FriendsListName,
				FOnReadFriendsListComplete::CreateUObject(this, &UMultiplayerSessionSubsystem::OnReadFriendsListComplete)))
			{
				return;
			}
		}
	}

	BroadcastOperationMessage(TEXT("Steam arkadas listesi yuklenemedi."));
	OnFriendsListUpdatedEvent.Broadcast(TArray<FGercekSteamFriendInfo>());
}

bool UMultiplayerSessionSubsystem::ShowInviteFriendsUI()
{
	if (!SessionInterface.IsValid() || !SessionInterface->GetNamedSession(NAME_GameSession))
	{
		return false;
	}

	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		if (IOnlineExternalUIPtr ExternalUI = Subsystem->GetExternalUIInterface())
		{
			return ExternalUI->ShowInviteUI(0, NAME_GameSession);
		}
	}

	return false;
}

bool UMultiplayerSessionSubsystem::HasContinueSave() const
{
	return CachedContinueInfo.bHasSave;
}

FGercekContinueSessionInfo UMultiplayerSessionSubsystem::GetContinueSaveInfo() const
{
	return CachedContinueInfo;
}

void UMultiplayerSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegates(this);
	}

	if (bWasSuccessful)
	{
		FGercekContinueSessionInfo ContinueInfo;
		ContinueInfo.bHasSave = true;
		ContinueInfo.bWasHost = true;
		ContinueInfo.bIsContinueSession = PendingSessionConfig.bIsContinueSession;
		ContinueInfo.bIsPasswordProtected = !PendingSessionConfig.Password.IsEmpty();
		ContinueInfo.Visibility = PendingSessionConfig.Visibility;
		ContinueInfo.SaveSlotName = PendingSessionConfig.SaveSlotName;
		ContinueInfo.SessionId = PendingSessionConfig.SessionId;
		ContinueInfo.MapPath = PendingSessionConfig.MapPath;
		ContinueInfo.HostDisplayName = GetLocalPlayerNickname();
		ContinueInfo.HostNetId = GetLocalNetIdString();
		ContinueInfo.Password = PendingSessionConfig.Password;
		CacheContinueSession(ContinueInfo);

		if (PendingSessionConfig.bTravelOnCreate)
		{
			if (UWorld* World = GetWorld())
			{
				if (APlayerController* PlayerController = World->GetFirstPlayerController())
				{
					FInputModeGameOnly InputModeData;
					PlayerController->SetInputMode(InputModeData);
					PlayerController->bShowMouseCursor = false;
				}

				World->ServerTravel(FString::Printf(TEXT("%s?listen"), *PendingSessionConfig.MapPath));
			}
		}
	}
	else
	{
		BroadcastOperationMessage(TEXT("Steam sunucusu olusturulamadi."));
	}

	OnCreateSessionCompleteEvent.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegates(this);
	}

	if (bWasSuccessful && bCreateSessionAfterDestroy)
	{
		bCreateSessionAfterDestroy = false;
		CreateServerWithConfig(PendingSessionConfig);
		return;
	}

	if (!bWasSuccessful)
	{
		BroadcastOperationMessage(TEXT("Eski Steam oturumu kapatilamadi."));
	}

	bCreateSessionAfterDestroy = false;
}

void UMultiplayerSessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegates(this);
	}

	TArray<FBlueprintSessionResult> ResultsForBP;
	TArray<FGercekSessionBrowserResult> BrowserResults;

	if (bWasSuccessful && LastSessionSearch.IsValid())
	{
		for (const FOnlineSessionSearchResult& SearchResult : LastSessionSearch->SearchResults)
		{
			FString MatchType;
			SearchResult.Session.SessionSettings.Get(GercekSessionKeys::MatchType, MatchType);
			if (MatchType != TEXT("GercekCoop"))
			{
				continue;
			}

			const FGercekSessionBrowserResult BrowserResult = BuildBrowserResult(SearchResult);
			if (bPendingFindFriendsOnly && BrowserResult.Visibility != EGercekServerVisibility::FriendsOnly)
			{
				continue;
			}

			FBlueprintSessionResult BlueprintResult;
			BlueprintResult.OnlineResult = SearchResult;
			ResultsForBP.Add(BlueprintResult);
			BrowserResults.Add(BrowserResult);
		}
	}

	OnFindSessionsCompleteEvent.Broadcast(bWasSuccessful, ResultsForBP);
	OnServerBrowserResultsUpdatedEvent.Broadcast(BrowserResults);

	if (bAutoJoinContinueSession)
	{
		bAutoJoinContinueSession = false;

		for (const FGercekSessionBrowserResult& BrowserResult : BrowserResults)
		{
			if (BrowserResult.SessionId == PendingContinueSessionId)
			{
				JoinServerWithPassword(BrowserResult.SessionResult, PendingJoinPassword);
				return;
			}
		}

		BroadcastOperationMessage(TEXT("Kayitli oturum su anda aktif degil veya bulunamadi."));
		PendingContinueSessionId.Reset();
		PendingJoinPassword.Reset();
	}
}

void UMultiplayerSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegates(this);
	}

	const bool bWasSuccessful = Result == EOnJoinSessionCompleteResult::Success;
	if (bWasSuccessful)
	{
		FGercekContinueSessionInfo ContinueInfo;
		ContinueInfo.bHasSave = true;
		ContinueInfo.bWasHost = false;
		ContinueInfo.bIsContinueSession = PendingJoinBrowserResult.bIsContinueSession;
		ContinueInfo.bIsPasswordProtected = PendingJoinBrowserResult.bIsPasswordProtected;
		ContinueInfo.Visibility = PendingJoinBrowserResult.Visibility;
		ContinueInfo.SaveSlotName = PendingJoinBrowserResult.SaveSlotName;
		ContinueInfo.SessionId = PendingJoinBrowserResult.SessionId;
		ContinueInfo.MapPath = TEXT("/Game/Istanbul");
		ContinueInfo.HostDisplayName = PendingJoinBrowserResult.HostName;
		ContinueInfo.Password = PendingJoinPassword;
		CacheContinueSession(ContinueInfo);

		FString Address;
		if (SessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
		{
			if (APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController())
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
	else
	{
		BroadcastOperationMessage(TEXT("Sunucuya katilma basarisiz oldu."));
	}

	OnJoinSessionCompleteEvent.Broadcast(bWasSuccessful);
	ResetPendingJoinState();
}

void UMultiplayerSessionSubsystem::OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	TArray<FGercekSteamFriendInfo> FriendInfos;

	if (bWasSuccessful)
	{
		if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
		{
			if (IOnlineFriendsPtr FriendsInterface = Subsystem->GetFriendsInterface())
			{
				TArray<TSharedRef<FOnlineFriend>> Friends;
				if (FriendsInterface->GetFriendsList(LocalUserNum, ListName, Friends))
				{
					for (const TSharedRef<FOnlineFriend>& Friend : Friends)
					{
						FGercekSteamFriendInfo Info;
						Info.DisplayName = Friend->GetDisplayName();
						Info.UniqueNetId = Friend->GetUserId()->ToString();
						Info.StatusText = Friend->GetPresence().Status.StatusStr;
						Info.bIsOnline = Friend->GetPresence().bIsOnline;
						Info.bIsPlayingThisGame = Friend->GetPresence().bIsPlayingThisGame;
						Info.bIsJoinable = Friend->GetPresence().bIsJoinable;
						FriendInfos.Add(Info);
					}
				}
			}
		}
	}
	else
	{
		BroadcastOperationMessage(ErrorStr.IsEmpty() ? TEXT("Steam arkadas listesi okunamadi.") : ErrorStr);
	}

	OnFriendsListUpdatedEvent.Broadcast(FriendInfos);
}

void UMultiplayerSessionSubsystem::BroadcastOperationMessage(const FString& Message) const
{
	const_cast<UMultiplayerSessionSubsystem*>(this)->OnOperationMessageEvent.Broadcast(Message);
}

void UMultiplayerSessionSubsystem::BroadcastContinueState() const
{
	const_cast<UMultiplayerSessionSubsystem*>(this)->OnContinueSaveAvailabilityChangedEvent.Broadcast(CachedContinueInfo.bHasSave, CachedContinueInfo);
}

void UMultiplayerSessionSubsystem::CacheContinueSession(const FGercekContinueSessionInfo& ContinueInfo)
{
	CachedContinueInfo = ContinueInfo;

	UGercekCoopSessionSaveGame* SaveGame = Cast<UGercekCoopSessionSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UGercekCoopSessionSaveGame::StaticClass()));
	if (SaveGame)
	{
		SaveGame->SavedSession = CachedContinueInfo;
		UGameplayStatics::SaveGameToSlot(SaveGame, GercekSaveSlots::ContinueMetadata, 0);
	}

	BroadcastContinueState();
}

void UMultiplayerSessionSubsystem::LoadContinueSessionCache()
{
	CachedContinueInfo = FGercekContinueSessionInfo();
	CachedContinueInfo.SaveSlotName = TEXT("Gercek_HostWorld");

	if (UGameplayStatics::DoesSaveGameExist(GercekSaveSlots::ContinueMetadata, 0))
	{
		if (UGercekCoopSessionSaveGame* SaveGame = Cast<UGercekCoopSessionSaveGame>(
			UGameplayStatics::LoadGameFromSlot(GercekSaveSlots::ContinueMetadata, 0)))
		{
			CachedContinueInfo = SaveGame->SavedSession;
		}
	}
}

FGercekSessionConfig UMultiplayerSessionSubsystem::BuildConfigFromContinueSave() const
{
	FGercekSessionConfig SessionConfig;
	SessionConfig.MaxPlayers = 4;
	SessionConfig.bIsLAN = false;
	SessionConfig.Visibility = CachedContinueInfo.Visibility;
	SessionConfig.Password = CachedContinueInfo.Password;
	SessionConfig.SaveSlotName = CachedContinueInfo.SaveSlotName;
	SessionConfig.SessionId = CachedContinueInfo.SessionId;
	SessionConfig.MapPath = CachedContinueInfo.MapPath.IsEmpty() ? TEXT("/Game/Istanbul") : CachedContinueInfo.MapPath;
	SessionConfig.bIsContinueSession = true;
	SessionConfig.bTravelOnCreate = true;
	return SessionConfig;
}

FGercekSessionBrowserResult UMultiplayerSessionSubsystem::BuildBrowserResult(const FOnlineSessionSearchResult& SearchResult) const
{
	FGercekSessionBrowserResult BrowserResult;
	BrowserResult.SessionResult.OnlineResult = SearchResult;
	BrowserResult.HostName = SearchResult.Session.OwningUserName;
	BrowserResult.CurrentPlayers = SearchResult.Session.SessionSettings.NumPublicConnections + SearchResult.Session.SessionSettings.NumPrivateConnections -
		SearchResult.Session.NumOpenPublicConnections - SearchResult.Session.NumOpenPrivateConnections;
	BrowserResult.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections + SearchResult.Session.SessionSettings.NumPrivateConnections;
	BrowserResult.PingInMs = SearchResult.PingInMs;
	BrowserResult.Visibility = ResolveVisibility(SearchResult.Session.SessionSettings);

	SearchResult.Session.SessionSettings.Get(GercekSessionKeys::HostName, BrowserResult.HostName);
	SearchResult.Session.SessionSettings.Get(GercekSessionKeys::SessionId, BrowserResult.SessionId);
	SearchResult.Session.SessionSettings.Get(GercekSessionKeys::SaveSlot, BrowserResult.SaveSlotName);
	SearchResult.Session.SessionSettings.Get(GercekSessionKeys::ContinueSession, BrowserResult.bIsContinueSession);
	SearchResult.Session.SessionSettings.Get(GercekSessionKeys::PasswordProtected, BrowserResult.bIsPasswordProtected);

	return BrowserResult;
}

EGercekServerVisibility UMultiplayerSessionSubsystem::ResolveVisibility(const FOnlineSessionSettings& SessionSettings) const
{
	int32 VisibilityValue = static_cast<int32>(EGercekServerVisibility::Public);
	SessionSettings.Get(GercekSessionKeys::ServerVisibility, VisibilityValue);
	return static_cast<EGercekServerVisibility>(VisibilityValue);
}

bool UMultiplayerSessionSubsystem::ValidatePasswordForSession(const FOnlineSessionSearchResult& SearchResult, const FString& Password) const
{
	bool bPasswordProtected = false;
	SearchResult.Session.SessionSettings.Get(GercekSessionKeys::PasswordProtected, bPasswordProtected);
	if (!bPasswordProtected)
	{
		return true;
	}

	FString StoredPasswordHash;
	SearchResult.Session.SessionSettings.Get(GercekSessionKeys::PasswordHash, StoredPasswordHash);
	return StoredPasswordHash == HashPassword(Password);
}

FString UMultiplayerSessionSubsystem::HashPassword(const FString& Password) const
{
	return Password.IsEmpty() ? FString() : FMD5::HashAnsiString(*Password);
}

FString UMultiplayerSessionSubsystem::GetLocalNetIdString() const
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		if (IOnlineIdentityPtr IdentityInterface = Subsystem->GetIdentityInterface())
		{
			if (FUniqueNetIdPtr NetId = IdentityInterface->GetUniquePlayerId(0))
			{
				return NetId->ToString();
			}
		}
	}

	return FString();
}

FString UMultiplayerSessionSubsystem::GetLocalPlayerNickname() const
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		if (IOnlineIdentityPtr IdentityInterface = Subsystem->GetIdentityInterface())
		{
			const FString Nickname = IdentityInterface->GetPlayerNickname(0);
			if (!Nickname.IsEmpty())
			{
				return Nickname;
			}
		}
	}

	return TEXT("Host");
}

void UMultiplayerSessionSubsystem::ResetPendingJoinState()
{
	PendingJoinBrowserResult = FGercekSessionBrowserResult();
	PendingJoinPassword.Reset();
	PendingContinueSessionId.Reset();
}
