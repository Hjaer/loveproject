#pragma once

#include "CoreMinimal.h"
#include "FindSessionsCallbackProxy.h"
#include "GameFramework/SaveGame.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MultiplayerSessionSubsystem.generated.h"

UENUM(BlueprintType)
enum class EGercekServerVisibility : uint8
{
	Public UMETA(DisplayName = "Public"),
	FriendsOnly UMETA(DisplayName = "FriendsOnly"),
	Private UMETA(DisplayName = "Private")
};

USTRUCT(BlueprintType)
struct FGercekSessionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	int32 MaxPlayers = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	bool bIsLAN = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	EGercekServerVisibility Visibility = EGercekServerVisibility::Public;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString Password;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString SaveSlotName = TEXT("Gercek_HostWorld");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString SessionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	FString MapPath = TEXT("/Game/Istanbul");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	bool bIsContinueSession = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	bool bTravelOnCreate = true;
};

USTRUCT(BlueprintType)
struct FGercekSessionBrowserResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Session")
	FBlueprintSessionResult SessionResult;

	UPROPERTY(BlueprintReadOnly, Category = "Session")
	FString HostName;

	UPROPERTY(BlueprintReadOnly, Category = "Session")
	FString SessionId;

	UPROPERTY(BlueprintReadOnly, Category = "Session")
	FString SaveSlotName;

	UPROPERTY(BlueprintReadOnly, Category = "Session")
	int32 CurrentPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Session")
	int32 MaxPlayers = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Session")
	int32 PingInMs = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Session")
	bool bIsPasswordProtected = false;

	UPROPERTY(BlueprintReadOnly, Category = "Session")
	bool bIsContinueSession = false;

	UPROPERTY(BlueprintReadOnly, Category = "Session")
	EGercekServerVisibility Visibility = EGercekServerVisibility::Public;
};

USTRUCT(BlueprintType)
struct FGercekSteamFriendInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Steam")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Steam")
	FString UniqueNetId;

	UPROPERTY(BlueprintReadOnly, Category = "Steam")
	FString StatusText;

	UPROPERTY(BlueprintReadOnly, Category = "Steam")
	bool bIsOnline = false;

	UPROPERTY(BlueprintReadOnly, Category = "Steam")
	bool bIsPlayingThisGame = false;

	UPROPERTY(BlueprintReadOnly, Category = "Steam")
	bool bIsJoinable = false;
};

USTRUCT(BlueprintType)
struct FGercekContinueSessionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Continue")
	bool bHasSave = false;

	UPROPERTY(BlueprintReadOnly, Category = "Continue")
	bool bWasHost = false;

	UPROPERTY(BlueprintReadOnly, Category = "Continue")
	bool bIsContinueSession = false;

	UPROPERTY(BlueprintReadOnly, Category = "Continue")
	bool bIsPasswordProtected = false;

	UPROPERTY(BlueprintReadOnly, Category = "Continue")
	EGercekServerVisibility Visibility = EGercekServerVisibility::Public;

	UPROPERTY(BlueprintReadOnly, Category = "Continue")
	FString SaveSlotName = TEXT("Gercek_HostWorld");

	UPROPERTY(BlueprintReadOnly, Category = "Continue")
	FString SessionId;

	UPROPERTY(BlueprintReadOnly, Category = "Continue")
	FString MapPath = TEXT("/Game/Istanbul");

	UPROPERTY(BlueprintReadOnly, Category = "Continue")
	FString HostDisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Continue")
	FString HostNetId;

	UPROPERTY(BlueprintReadOnly, Category = "Continue")
	FString Password;

	UPROPERTY(BlueprintReadOnly, Category = "Continue")
	FDateTime LastSuccessfulSaveUtc;
};

UCLASS()
class GERCEK_API UGercekCoopSessionSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FGercekContinueSessionInfo SavedSession;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, bool, bWasSuccessful, const TArray<FBlueprintSessionResult>&, SearchResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnServerBrowserResultsUpdated, const TArray<FGercekSessionBrowserResult>&, Results);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnFriendsListUpdated, const TArray<FGercekSteamFriendInfo>&, Friends);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnContinueSaveAvailabilityChanged, bool, bHasAvailableSave, const FGercekContinueSessionInfo&, ContinueInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnOperationMessage, const FString&, Message);

UCLASS(Blueprintable, BlueprintType)
class GERCEK_API UMultiplayerSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiplayerSessionSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Steam Co-Op")
	void CreateServer(int32 MaxPlayers = 4, bool bIsLAN = false);

	UFUNCTION(BlueprintCallable, Category = "Steam Co-Op")
	void CreateServerWithConfig(const FGercekSessionConfig& SessionConfig);

	UFUNCTION(BlueprintCallable, Category = "Steam Co-Op")
	void ContinueLastSession();

	UFUNCTION(BlueprintCallable, Category = "Steam Co-Op")
	void FindServer(int32 MaxSearchResults = 10000, bool bIsLAN = false);

	UFUNCTION(BlueprintCallable, Category = "Steam Co-Op")
	void FindServers(bool bOnlyFriendsSessions, int32 MaxSearchResults = 10000, bool bIsLAN = false);

	UFUNCTION(BlueprintCallable, Category = "Steam Co-Op")
	void JoinServer(const FBlueprintSessionResult& SessionResult);

	UFUNCTION(BlueprintCallable, Category = "Steam Co-Op")
	void JoinServerWithPassword(const FBlueprintSessionResult& SessionResult, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "Steam Co-Op")
	void RequestSteamFriends();

	UFUNCTION(BlueprintCallable, Category = "Steam Co-Op")
	bool ShowInviteFriendsUI();

	UFUNCTION(BlueprintPure, Category = "Steam Co-Op")
	bool HasContinueSave() const;

	UFUNCTION(BlueprintPure, Category = "Steam Co-Op")
	FGercekContinueSessionInfo GetContinueSaveInfo() const;

	UFUNCTION(BlueprintCallable, Category = "Steam Co-Op")
	void UpdateHostContinueSaveAfterSuccessfulSave(const FString& SaveSlotName,
		const FString& SessionId, const FString& MapPath,
		const FDateTime& LastSaveUtc);

	UPROPERTY(BlueprintAssignable, Category = "Steam Co-Op | Events")
	FMultiplayerOnCreateSessionComplete OnCreateSessionCompleteEvent;

	UPROPERTY(BlueprintAssignable, Category = "Steam Co-Op | Events")
	FMultiplayerOnFindSessionsComplete OnFindSessionsCompleteEvent;

	UPROPERTY(BlueprintAssignable, Category = "Steam Co-Op | Events")
	FMultiplayerOnJoinSessionComplete OnJoinSessionCompleteEvent;

	UPROPERTY(BlueprintAssignable, Category = "Steam Co-Op | Events")
	FMultiplayerOnServerBrowserResultsUpdated OnServerBrowserResultsUpdatedEvent;

	UPROPERTY(BlueprintAssignable, Category = "Steam Co-Op | Events")
	FMultiplayerOnFriendsListUpdated OnFriendsListUpdatedEvent;

	UPROPERTY(BlueprintAssignable, Category = "Steam Co-Op | Events")
	FMultiplayerOnContinueSaveAvailabilityChanged OnContinueSaveAvailabilityChangedEvent;

	UPROPERTY(BlueprintAssignable, Category = "Steam Co-Op | Events")
	FMultiplayerOnOperationMessage OnOperationMessageEvent;

protected:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnReadFriendsListComplete(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

private:
	void BroadcastOperationMessage(const FString& Message) const;
	void BroadcastContinueState() const;
	void CacheContinueSession(const FGercekContinueSessionInfo& ContinueInfo);
	void LoadContinueSessionCache();
	FGercekSessionConfig BuildConfigFromContinueSave() const;
	bool IsContinueInfoUsable(const FGercekContinueSessionInfo& ContinueInfo) const;
	FGercekSessionBrowserResult BuildBrowserResult(const FOnlineSessionSearchResult& SearchResult) const;
	EGercekServerVisibility ResolveVisibility(const FOnlineSessionSettings& SessionSettings) const;
	bool ValidatePasswordForSession(const FOnlineSessionSearchResult& SearchResult, const FString& Password) const;
	FString HashPassword(const FString& Password) const;
	FString GetLocalNetIdString() const;
	FString GetLocalPlayerNickname() const;
	void ResetPendingJoinState();

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<class FOnlineSessionSearch> LastSessionSearch;

	bool bCreateSessionAfterDestroy = false;
	bool bPendingFindFriendsOnly = false;
	bool bAutoJoinContinueSession = false;
	int32 PendingMaxPlayers = 4;
	bool bPendingIsLAN = false;
	FString PendingContinueSessionId;
	FGercekSessionConfig PendingSessionConfig;
	FGercekSessionBrowserResult PendingJoinBrowserResult;
	FString PendingJoinPassword;
	FGercekContinueSessionInfo CachedContinueInfo;
};
