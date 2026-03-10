#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UWidgetSwitcher;
class UMultiplayerSessionSubsystem;

// Meryem'in yeni tasarimina birebir uymak amaciyla guncellenmis, Switcher mantikli Ana Menu Sinifi
UCLASS()
class GERCEK_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual bool Initialize() override;

	/* ========================================================
	 * UI BILESENLERI -> Isimleri Blueprint ile HARFI HARFINE AYNIDIR
	 * ======================================================== */

	// Ana Switcher (Televizyon)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UWidgetSwitcher* WS_MenuPages;

	// --- Kanal 0 (Ana Menü) Butonları ---
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* Button_Play;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* Button_Settings;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* Button_Quit;

	// --- Kanal 1 (Sunucu) Butonları ---
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* Button_CreateServer;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* Button_JoinServer;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* Button_BackFromPlay;

	// --- Kanal 2 (Ayarlar) İçeriği ---
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UWidgetSwitcher* WS_SettingsTabs; // Ayarlar icindeki sekmeler (Audio, Video vb.)

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* Btn_Tab_Audio;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* Btn_Tab_Video;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* Btn_Tab_Controls;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UButton* Button_BackFromSettings;

private:
	/* ========================================================
	 * BUTON TIKLANMA (OnClicked) OLAYLARI 
	 * ======================================================== */
	
	// Ana Menu Buton Baglantilari
	UFUNCTION()
	void OnPlayClicked();

	UFUNCTION()
	void OnSettingsClicked();

	UFUNCTION()
	void OnQuitClicked();

	// Sunucu Ekrani Buton Baglantilari
	UFUNCTION()
	void OnCreateServerClicked();

	UFUNCTION()
	void OnJoinServerClicked();

	UFUNCTION()
	void OnBackFromPlayClicked();

	// Ayarlar Ekrani Sekme (Tab) Gecisleri
	UFUNCTION()
	void OnTabAudioClicked();

	UFUNCTION()
	void OnTabVideoClicked();

	UFUNCTION()
	void OnTabControlsClicked();

	UFUNCTION()
	void OnBackFromSettingsClicked();

	/* ========================================================
	 * ARKA PLAN SISTEMLERI
	 * ======================================================== */

	// Steam / Odalar arasi gecis icin Multiplayer sistemimiz
	UPROPERTY()
	UMultiplayerSessionSubsystem* MultiplayerSubsystem;
};
