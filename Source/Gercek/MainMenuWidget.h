#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UWidgetSwitcher;
class UComboBoxString;
class UCheckBox;
class USlider;
class UProgressBar;
class UImage;
class UMultiplayerSessionSubsystem;

// Meryem'in güncel Blueprint hiyerarşisine birebir uyumlu (Görsele Göre) Optimizasyonlu Sınıf
UCLASS()
class GERCEK_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual bool Initialize() override;

	// Widget ekrana ilk geldiği anda, fareyi ve girişi sadece arayüze kitlemek için kullanacağız.
	virtual void NativeConstruct() override;

	// Ekranda boşluğa (veya resme) tıklandığında tıklamayı arkaya sızdırmadan YUTMAK için kullanılır.
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/* ========================================================
	 * 1. ARKA PLAN VE TELEVİZYONLAR (Switcher)
	 * ======================================================== */
	UPROPERTY(meta = (BindWidget))
	UImage* IIMG_Background;

	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* WS_MenuPages;

	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* WS_AyarDetaylari;

	/* ========================================================
	 * 2. ANA VE SUNUCU BUTONLARI
	 * ======================================================== */
	UPROPERTY(meta = (BindWidget))
	UButton* Button_Play;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Settings;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_Quit;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_CreateServer;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_JoinServer;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_BackFromPlay;

	/* ========================================================
	 * 3. AYARLAR SOL YAN PANEL KATEGORİ BUTONLARI
	 * ======================================================== */
	UPROPERTY(meta = (BindWidget))
	UButton* BTN_GERCEK_GORUNTU;

	UPROPERTY(meta = (BindWidget))
	UButton* BTN_GERCEK_KONTROL;

	UPROPERTY(meta = (BindWidget))
	UButton* BTN_GERCEK_SES;

	// Geri Butonu (Ayarlardan Ana Menüye Dönüş)
	UPROPERTY(meta = (BindWidget))
	UButton* Button_BackFromSettings;

	/* ========================================================
	 * 4. GÖRÜNTÜ AYARLARI KONTROLLERİ (Video Settings)
	 * ======================================================== */
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* CB_Quality;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* CB_WindowMode;

	UPROPERTY(meta = (BindWidget))
	UCheckBox* Chk_VSync;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_ApplyVideo;

	/* ========================================================
	 * 5. SES AYARLARI (Audio Settings)
	 * ======================================================== */
	
	// --- Genel Ses ---
	UPROPERTY(meta = (BindWidget))
	USlider* Slider_MasterVolume;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PB_MasterVolume;

	// --- Efekt Sesi (SFX) ---
	UPROPERTY(meta = (BindWidget))
	USlider* Slider_SFXVolume;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PB_SFXVolume;

	// --- Müzik Sesi (Music) ---
	UPROPERTY(meta = (BindWidget))
	USlider* Slider_MusicVolume;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PB_MusicVolume;

private:
	/* ========================================================
	 * FONKSİYON TETİKLEYİCİLERİ (OnClicked & OnValueChanged)
	 * ======================================================== */

	UFUNCTION() void OnPlayClicked();
	UFUNCTION() void OnSettingsClicked();
	UFUNCTION() void OnQuitClicked();

	UFUNCTION() void OnCreateServerClicked();
	UFUNCTION() void OnJoinServerClicked();
	UFUNCTION() void OnBackFromPlayClicked();

	UFUNCTION() void OnBTN_GERCEK_GORUNTU_Clicked();
	UFUNCTION() void OnBTN_GERCEK_KONTROL_Clicked();
	UFUNCTION() void OnBTN_GERCEK_SES_Clicked();

	UFUNCTION() void OnBackFromSettingsClicked();
	UFUNCTION() void OnApplyVideoClicked();

	// --- Ses Slider Değişim (OnValueChanged) Tetikleyicileri ---
	UFUNCTION() void OnMasterVolumeChanged(float Value);
	UFUNCTION() void OnSFXVolumeChanged(float Value);
	UFUNCTION() void OnMusicVolumeChanged(float Value);

	/* ========================================================
	 * ARKA PLAN SİSTEMLERİ
	 * ======================================================== */
	UPROPERTY()
	UMultiplayerSessionSubsystem* MultiplayerSubsystem;
};
