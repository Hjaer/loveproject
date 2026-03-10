#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerSessionSubsystem.h"

bool UMainMenuWidget::Initialize()
{
	// Super::Initialize bilesenleri canlandirir. Eger BindWidget kisminda hatali isim varsa burada patlar.
	if (!Super::Initialize())
	{
		return false;
	}

	/* ========================================================
	 * KABLO BAGLAMA ASAMASI (AddDynamic)
	 * Blueprint'teki tikanabilen o yesil "On Clicked" pinlerini 
	 * asagidaki C++ fonksiyonlarina direkt bagliyoruz.
	 * ======================================================== */

	// 1- Ana Menü Butonlari
	if (Button_Play)
	{
		Button_Play->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPlayClicked);
	}
	if (Button_Settings)
	{
		Button_Settings->OnClicked.AddDynamic(this, &UMainMenuWidget::OnSettingsClicked);
	}
	if (Button_Quit)
	{
		Button_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	}

	// 2- Sunucu Menüsü Butonlari
	if (Button_CreateServer)
	{
		Button_CreateServer->OnClicked.AddDynamic(this, &UMainMenuWidget::OnCreateServerClicked);
	}
	if (Button_JoinServer)
	{
		Button_JoinServer->OnClicked.AddDynamic(this, &UMainMenuWidget::OnJoinServerClicked);
	}
	if (Button_BackFromPlay)
	{
		Button_BackFromPlay->OnClicked.AddDynamic(this, &UMainMenuWidget::OnBackFromPlayClicked);
	}

	// 3- Ayarlar Menüsü Sekmeleri ve Geri Butonu
	if (Btn_Tab_Audio)
	{
		Btn_Tab_Audio->OnClicked.AddDynamic(this, &UMainMenuWidget::OnTabAudioClicked);
	}
	if (Btn_Tab_Video)
	{
		Btn_Tab_Video->OnClicked.AddDynamic(this, &UMainMenuWidget::OnTabVideoClicked);
	}
	if (Btn_Tab_Controls)
	{
		Btn_Tab_Controls->OnClicked.AddDynamic(this, &UMainMenuWidget::OnTabControlsClicked);
	}
	if (Button_BackFromSettings)
	{
		Button_BackFromSettings->OnClicked.AddDynamic(this, &UMainMenuWidget::OnBackFromSettingsClicked);
	}

	/* ========================================================
	 * MOTOR AYAĞA KALKARKEN STEAM SİSTEMİNİ BUL
	 * ======================================================== */
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
	}

	return true;
}

// -------------------------------------------------------------
// KANAL 0 (ANA MENÜ) TETIKLEYICILERI
// -------------------------------------------------------------

void UMainMenuWidget::OnPlayClicked()
{
	// Oynaya basinca "Ana Switcher" televizyonunun kanalini 1'e ayarlayip Sunucu Secimi sayfasina gidelim
	if (WS_MenuPages)
	{
		WS_MenuPages->SetActiveWidgetIndex(1);
	}
}

void UMainMenuWidget::OnSettingsClicked()
{
	// Ayarlara basinca "Ana Switcher" televizyonunun kanalini 2'ye ayarlayalim
	if (WS_MenuPages)
	{
		WS_MenuPages->SetActiveWidgetIndex(2);
	}

	// Ayarlara ilk tiklandiginda, icerideki kücük televizyonun kanalini 0 (Audio) yaparak baslatalim
	if (WS_SettingsTabs)
	{
		WS_SettingsTabs->SetActiveWidgetIndex(0);
	}
}

void UMainMenuWidget::OnQuitClicked()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Oyun Kapaniyor..."));
	
	// Oyundan cikis yap
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, true);
	}
}

// -------------------------------------------------------------
// KANAL 1 (SUNUCU EKRANI) TETIKLEYICILERI
// -------------------------------------------------------------

void UMainMenuWidget::OnCreateServerClicked()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Sunucu Olusturuluyor... Steam Subsystem Devrede!"));
	
	if (MultiplayerSubsystem)
	{
		MultiplayerSubsystem->CreateServer();
	}
}

void UMainMenuWidget::OnJoinServerClicked()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Sunucu Araniyor... Steam Subsystem Devrede!"));

	if (MultiplayerSubsystem)
	{
		MultiplayerSubsystem->FindServer();
	}
}

void UMainMenuWidget::OnBackFromPlayClicked()
{
	// Sunucu ekranindan geri gelince, Ana Menu televizyonunu tekrar 0. kanala (Ana Menu) dondur.
	if (WS_MenuPages)
	{
		WS_MenuPages->SetActiveWidgetIndex(0);
	}
}

// -------------------------------------------------------------
// KANAL 2 (AYARLAR EKRANI) SEKME TETIKLEYICILERI
// -------------------------------------------------------------

void UMainMenuWidget::OnTabAudioClicked()
{
	// Sesler (Audio) sekmesine tiklaninca Ayarlar icindeki kucuk televizyonun (WS_SettingsTabs) kanalini 0 yap.
	if (WS_SettingsTabs)
	{
		WS_SettingsTabs->SetActiveWidgetIndex(0);
	}
}

void UMainMenuWidget::OnTabVideoClicked()
{
	// Goruntu (Video) sekmesine tiklaninca Ayarlar icindeki kucuk televizyonun (WS_SettingsTabs) kanalini 1 yap.
	if (WS_SettingsTabs)
	{
		WS_SettingsTabs->SetActiveWidgetIndex(1);
	}
}

void UMainMenuWidget::OnTabControlsClicked()
{
	// Kontroller (Controls) sekmesine tiklaninca Ayarlar icindeki kucuk televizyonun (WS_SettingsTabs) kanalini 2 yap.
	if (WS_SettingsTabs)
	{
		WS_SettingsTabs->SetActiveWidgetIndex(2);
	}
}

void UMainMenuWidget::OnBackFromSettingsClicked()
{
	// Ayarlarda gezinirken isin bitip Geri butonuna bastiginda yine Ana Televizyonu (WS_MenuPages) 0'a, yani menuye dondurur.
	if (WS_MenuPages)
	{
		WS_MenuPages->SetActiveWidgetIndex(0);
	}
}
