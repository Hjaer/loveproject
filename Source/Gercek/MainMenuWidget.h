#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SlateEnums.h"
#include "MultiplayerSessionSubsystem.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UComboBoxString;
class UEditableTextBox;
class UImage;
class UMultiplayerSessionSubsystem;
class UTextBlock;
class UWidget;
class UWidgetSwitcher;

UCLASS()
class GERCEK_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void RefreshJoinServerLists();

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void JoinListedServer(const FGercekSessionBrowserResult& ServerResult, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "Main Menu")
	void OpenInvitePanel();

	UFUNCTION(BlueprintPure, Category = "Main Menu")
	bool HasContinueSave() const;

	UFUNCTION(BlueprintPure, Category = "Main Menu")
	FGercekContinueSessionInfo GetContinueSaveInfo() const;

protected:
	virtual bool Initialize() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Main Menu")
	void BP_OnServerBrowserResultsUpdated(const TArray<FGercekSessionBrowserResult>& Results);

	UFUNCTION(BlueprintImplementableEvent, Category = "Main Menu")
	void BP_OnSteamFriendsUpdated(const TArray<FGercekSteamFriendInfo>& Friends);

	UFUNCTION(BlueprintImplementableEvent, Category = "Main Menu")
	void BP_OnContinueAvailabilityUpdated(bool bHasAvailableSave, const FGercekContinueSessionInfo& ContinueInfo);

	UFUNCTION(BlueprintImplementableEvent, Category = "Main Menu")
	void BP_OnMenuStatusMessage(const FString& Message);

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UImage> IIMG_Background = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> WS_MenuPages = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> WS_MainMenu = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher_MainMenu = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_NewGame = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Continue = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Exit = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_JoinServer = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_CreateServer = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_BackToMain = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Create = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_InviteFriends = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_BackToNewGame = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_BackFromJoin = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_BackFromInvite = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UComboBoxString> ServerType = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UEditableTextBox> InputPassword = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TXT_ServerTypeValidation = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
	int32 MainPageIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
	int32 NewGamePageIndex = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
	int32 CreateServerPageIndex = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
	int32 JoinServerPageIndex = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigation")
	int32 InviteFriendsPageIndex = 4;

private:
	UFUNCTION()
	void OnNewGameClicked();

	UFUNCTION()
	void OnContinueClicked();

	UFUNCTION()
	void OnExitClicked();

	UFUNCTION()
	void OnJoinServerClicked();

	UFUNCTION()
	void OnCreateServerClicked();

	UFUNCTION()
	void OnBackToMainClicked();

	UFUNCTION()
	void OnCreateClicked();

	UFUNCTION()
	void OnServerTypeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnInviteFriendsClicked();

	UFUNCTION()
	void OnBackToNewGameClicked();

	UFUNCTION()
	void OnBackFromJoinClicked();

	UFUNCTION()
	void OnBackFromInviteClicked();

	UFUNCTION()
	void HandleCreateSessionComplete(bool bWasSuccessful);

	UFUNCTION()
	void HandleJoinSessionComplete(bool bWasSuccessful);

	UFUNCTION()
	void HandleServerBrowserResultsUpdated(const TArray<FGercekSessionBrowserResult>& Results);

	UFUNCTION()
	void HandleSteamFriendsUpdated(const TArray<FGercekSteamFriendInfo>& Friends);

	UFUNCTION()
	void HandleContinueAvailabilityUpdated(bool bHasAvailableSave, const FGercekContinueSessionInfo& ContinueInfo);

	UFUNCTION()
	void HandleOperationMessage(const FString& Message);

	UWidgetSwitcher* ResolveMenuSwitcher() const;
	void SetActivePage(int32 PageIndex) const;
	void SetServerTypeValidationMessage(const FText& Message) const;
	bool HasValidSelectedServerType() const;
	EGercekServerVisibility ResolveSelectedServerVisibility() const;
	void EnsureDefaultServerTypeOptions();

	UPROPERTY()
	TObjectPtr<UMultiplayerSessionSubsystem> MultiplayerSubsystem = nullptr;
};
