#include "MainMenuWidget.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

bool UMainMenuWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		MultiplayerSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
	}

	if (Btn_NewGame)
	{
		Btn_NewGame->OnClicked.AddDynamic(this, &UMainMenuWidget::OnNewGameClicked);
	}
	if (Btn_Continue)
	{
		Btn_Continue->OnClicked.AddDynamic(this, &UMainMenuWidget::OnContinueClicked);
	}
	if (Btn_Exit)
	{
		Btn_Exit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnExitClicked);
	}
	if (Btn_JoinServer)
	{
		Btn_JoinServer->OnClicked.AddDynamic(this, &UMainMenuWidget::OnJoinServerClicked);
	}
	if (Btn_CreateServer)
	{
		Btn_CreateServer->OnClicked.AddDynamic(this, &UMainMenuWidget::OnCreateServerClicked);
	}
	if (Btn_BackToMain)
	{
		Btn_BackToMain->OnClicked.AddDynamic(this, &UMainMenuWidget::OnBackToMainClicked);
	}
	if (Btn_Create)
	{
		Btn_Create->OnClicked.AddDynamic(this, &UMainMenuWidget::OnCreateClicked);
	}
	if (Btn_InviteFriends)
	{
		Btn_InviteFriends->OnClicked.AddDynamic(this, &UMainMenuWidget::OnInviteFriendsClicked);
	}
	if (Btn_BackToNewGame)
	{
		Btn_BackToNewGame->OnClicked.AddDynamic(this, &UMainMenuWidget::OnBackToNewGameClicked);
	}
	if (Btn_BackFromJoin)
	{
		Btn_BackFromJoin->OnClicked.AddDynamic(this, &UMainMenuWidget::OnBackFromJoinClicked);
	}
	if (Btn_BackFromInvite)
	{
		Btn_BackFromInvite->OnClicked.AddDynamic(this, &UMainMenuWidget::OnBackFromInviteClicked);
	}

	if (MultiplayerSubsystem)
	{
		MultiplayerSubsystem->OnCreateSessionCompleteEvent.AddDynamic(this, &UMainMenuWidget::HandleCreateSessionComplete);
		MultiplayerSubsystem->OnJoinSessionCompleteEvent.AddDynamic(this, &UMainMenuWidget::HandleJoinSessionComplete);
		MultiplayerSubsystem->OnServerBrowserResultsUpdatedEvent.AddDynamic(this, &UMainMenuWidget::HandleServerBrowserResultsUpdated);
		MultiplayerSubsystem->OnFriendsListUpdatedEvent.AddDynamic(this, &UMainMenuWidget::HandleSteamFriendsUpdated);
		MultiplayerSubsystem->OnContinueSaveAvailabilityChangedEvent.AddDynamic(this, &UMainMenuWidget::HandleContinueAvailabilityUpdated);
		MultiplayerSubsystem->OnOperationMessageEvent.AddDynamic(this, &UMainMenuWidget::HandleOperationMessage);
	}

	EnsureDefaultServerTypeOptions();

	return true;
}

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	SetVisibility(ESlateVisibility::Visible);

	if (UWidget* RootWidget = GetRootWidget())
	{
		RootWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if (IIMG_Background)
	{
		IIMG_Background->SetVisibility(ESlateVisibility::Visible);
	}

	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		FInputModeUIOnly InputModeData;
		InputModeData.SetWidgetToFocus(TakeWidget());
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputModeData);
		PlayerController->SetShowMouseCursor(true);
	}

	SetActivePage(MainPageIndex);

	if (MultiplayerSubsystem)
	{
		HandleContinueAvailabilityUpdated(MultiplayerSubsystem->HasContinueSave(), MultiplayerSubsystem->GetContinueSaveInfo());
	}
}

void UMainMenuWidget::NativeDestruct()
{
	if (MultiplayerSubsystem)
	{
		MultiplayerSubsystem->OnCreateSessionCompleteEvent.RemoveDynamic(this, &UMainMenuWidget::HandleCreateSessionComplete);
		MultiplayerSubsystem->OnJoinSessionCompleteEvent.RemoveDynamic(this, &UMainMenuWidget::HandleJoinSessionComplete);
		MultiplayerSubsystem->OnServerBrowserResultsUpdatedEvent.RemoveDynamic(this, &UMainMenuWidget::HandleServerBrowserResultsUpdated);
		MultiplayerSubsystem->OnFriendsListUpdatedEvent.RemoveDynamic(this, &UMainMenuWidget::HandleSteamFriendsUpdated);
		MultiplayerSubsystem->OnContinueSaveAvailabilityChangedEvent.RemoveDynamic(this, &UMainMenuWidget::HandleContinueAvailabilityUpdated);
		MultiplayerSubsystem->OnOperationMessageEvent.RemoveDynamic(this, &UMainMenuWidget::HandleOperationMessage);
	}

	Super::NativeDestruct();
}

FReply UMainMenuWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled().SetUserFocus(TakeWidget(), EFocusCause::Mouse);
}

FReply UMainMenuWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled().SetUserFocus(TakeWidget(), EFocusCause::Mouse);
}

void UMainMenuWidget::RefreshJoinServerLists()
{
	if (!MultiplayerSubsystem)
	{
		HandleOperationMessage(TEXT("Steam session sistemi bulunamadi."));
		return;
	}

	MultiplayerSubsystem->FindServers(false);
	MultiplayerSubsystem->RequestSteamFriends();
}

void UMainMenuWidget::JoinListedServer(const FGercekSessionBrowserResult& ServerResult, const FString& Password)
{
	if (!MultiplayerSubsystem)
	{
		HandleOperationMessage(TEXT("Steam session sistemi bulunamadi."));
		return;
	}

	MultiplayerSubsystem->JoinServerWithPassword(ServerResult.SessionResult, Password);
}

void UMainMenuWidget::OpenInvitePanel()
{
	SetActivePage(InviteFriendsPageIndex);

	if (!MultiplayerSubsystem)
	{
		HandleOperationMessage(TEXT("Steam session sistemi bulunamadi."));
		return;
	}

	MultiplayerSubsystem->RequestSteamFriends();

	if (!MultiplayerSubsystem->ShowInviteFriendsUI())
	{
		HandleOperationMessage(TEXT("Davet penceresi acilamadi. Aktif bir Steam oturumu olmasi gerekiyor olabilir."));
	}
}

bool UMainMenuWidget::HasContinueSave() const
{
	return MultiplayerSubsystem && MultiplayerSubsystem->HasContinueSave();
}

FGercekContinueSessionInfo UMainMenuWidget::GetContinueSaveInfo() const
{
	return MultiplayerSubsystem ? MultiplayerSubsystem->GetContinueSaveInfo() : FGercekContinueSessionInfo();
}

void UMainMenuWidget::OnNewGameClicked()
{
	SetActivePage(NewGamePageIndex);
}

void UMainMenuWidget::OnContinueClicked()
{
	if (!MultiplayerSubsystem)
	{
		HandleOperationMessage(TEXT("Continue icin gerekli session sistemi bulunamadi."));
		return;
	}

	MultiplayerSubsystem->ContinueLastSession();
}

void UMainMenuWidget::OnExitClicked()
{
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		UKismetSystemLibrary::QuitGame(this, PlayerController, EQuitPreference::Quit, true);
	}
}

void UMainMenuWidget::OnJoinServerClicked()
{
	SetActivePage(JoinServerPageIndex);
	RefreshJoinServerLists();
}

void UMainMenuWidget::OnCreateServerClicked()
{
	SetActivePage(CreateServerPageIndex);
}

void UMainMenuWidget::OnBackToMainClicked()
{
	SetActivePage(MainPageIndex);
}

void UMainMenuWidget::OnCreateClicked()
{
	if (!MultiplayerSubsystem)
	{
		HandleOperationMessage(TEXT("Sunucu olusturma sistemi bulunamadi."));
		return;
	}

	FGercekSessionConfig SessionConfig;
	SessionConfig.MaxPlayers = 4;
	SessionConfig.bIsLAN = false;
	SessionConfig.Visibility = ResolveSelectedServerVisibility();
	SessionConfig.MapPath = TEXT("/Game/Istanbul");
	SessionConfig.bTravelOnCreate = true;

	if (InputPassword)
	{
		SessionConfig.Password = InputPassword->GetText().ToString().TrimStartAndEnd();
	}

	MultiplayerSubsystem->CreateServerWithConfig(SessionConfig);
}

void UMainMenuWidget::OnInviteFriendsClicked()
{
	OpenInvitePanel();
}

void UMainMenuWidget::OnBackToNewGameClicked()
{
	SetActivePage(NewGamePageIndex);
}

void UMainMenuWidget::OnBackFromJoinClicked()
{
	SetActivePage(NewGamePageIndex);
}

void UMainMenuWidget::OnBackFromInviteClicked()
{
	SetActivePage(CreateServerPageIndex);
}

void UMainMenuWidget::HandleCreateSessionComplete(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		HandleOperationMessage(TEXT("Sunucu olusturulamadi."));
	}
}

void UMainMenuWidget::HandleJoinSessionComplete(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		HandleOperationMessage(TEXT("Sunucuya katilma islemi basarisiz oldu."));
	}
}

void UMainMenuWidget::HandleServerBrowserResultsUpdated(const TArray<FGercekSessionBrowserResult>& Results)
{
	BP_OnServerBrowserResultsUpdated(Results);
}

void UMainMenuWidget::HandleSteamFriendsUpdated(const TArray<FGercekSteamFriendInfo>& Friends)
{
	BP_OnSteamFriendsUpdated(Friends);
}

void UMainMenuWidget::HandleContinueAvailabilityUpdated(bool bHasAvailableSave, const FGercekContinueSessionInfo& ContinueInfo)
{
	BP_OnContinueAvailabilityUpdated(bHasAvailableSave, ContinueInfo);
}

void UMainMenuWidget::HandleOperationMessage(const FString& Message)
{
	BP_OnMenuStatusMessage(Message);
}

UWidgetSwitcher* UMainMenuWidget::ResolveMenuSwitcher() const
{
	if (WS_MenuPages)
	{
		return WS_MenuPages;
	}

	if (WS_MainMenu)
	{
		return WS_MainMenu;
	}

	return WidgetSwitcher_MainMenu;
}

void UMainMenuWidget::SetActivePage(int32 PageIndex) const
{
	if (UWidgetSwitcher* MenuSwitcher = ResolveMenuSwitcher())
	{
		MenuSwitcher->SetActiveWidgetIndex(PageIndex);
	}
}

EGercekServerVisibility UMainMenuWidget::ResolveSelectedServerVisibility() const
{
	if (!ServerType)
	{
		return EGercekServerVisibility::Public;
	}

	const FString SelectedValue = ServerType->GetSelectedOption().TrimStartAndEnd();

	if (SelectedValue.Equals(TEXT("FriendsOnly"), ESearchCase::IgnoreCase) ||
		SelectedValue.Equals(TEXT("Friends Only"), ESearchCase::IgnoreCase) ||
		SelectedValue.Equals(TEXT("Arkadaslar"), ESearchCase::IgnoreCase) ||
		SelectedValue.Equals(TEXT("Sadece Arkadaslar"), ESearchCase::IgnoreCase))
	{
		return EGercekServerVisibility::FriendsOnly;
	}

	if (SelectedValue.Equals(TEXT("Private"), ESearchCase::IgnoreCase) ||
		SelectedValue.Equals(TEXT("Ozel"), ESearchCase::IgnoreCase))
	{
		return EGercekServerVisibility::Private;
	}

	return EGercekServerVisibility::Public;
}

void UMainMenuWidget::EnsureDefaultServerTypeOptions()
{
	if (!ServerType || ServerType->GetOptionCount() > 0)
	{
		return;
	}

	ServerType->AddOption(TEXT("Public"));
	ServerType->AddOption(TEXT("FriendsOnly"));
	ServerType->AddOption(TEXT("Private"));
	ServerType->SetSelectedIndex(0);
}
