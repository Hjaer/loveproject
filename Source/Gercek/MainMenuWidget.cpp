#include "MainMenuWidget.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/Slider.h"
#include "Components/WidgetSwitcher.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MultiplayerSessionSubsystem.h"

bool UMainMenuWidget::Initialize() {
  // C++ hiyerarşiyi devralır, herhangi bir isim uyuşmazlığında hayaleti önler.
  if (!Super::Initialize()) {
    return false;
  }

  /* ========================================================
   * BAĞLAMA (BINDING) İŞLEMLERİ
   * ======================================================== */

  // --------- Kanal 0 (Ana Menü / Oyna vb.) ---------
  if (Button_Play)
    Button_Play->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPlayClicked);
  if (Button_Settings)
    Button_Settings->OnClicked.AddDynamic(this,
                                          &UMainMenuWidget::OnSettingsClicked);
  if (Button_Quit)
    Button_Quit->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);

  // --------- Kanal 1 (Sunucu Ekranı) ---------
  if (Button_CreateServer)
    Button_CreateServer->OnClicked.AddDynamic(
        this, &UMainMenuWidget::OnCreateServerClicked);
  if (Button_JoinServer)
    Button_JoinServer->OnClicked.AddDynamic(
        this, &UMainMenuWidget::OnJoinServerClicked);
  if (Button_BackFromPlay)
    Button_BackFromPlay->OnClicked.AddDynamic(
        this, &UMainMenuWidget::OnBackFromPlayClicked);

  // --------- Kanal 2 (Ayarlar Yan Panel) ---------
  if (BTN_GERCEK_GORUNTU)
    BTN_GERCEK_GORUNTU->OnClicked.AddDynamic(
        this, &UMainMenuWidget::OnBTN_GERCEK_GORUNTU_Clicked);
  if (BTN_GERCEK_KONTROL)
    BTN_GERCEK_KONTROL->OnClicked.AddDynamic(
        this, &UMainMenuWidget::OnBTN_GERCEK_KONTROL_Clicked);
  if (BTN_GERCEK_SES)
    BTN_GERCEK_SES->OnClicked.AddDynamic(
        this, &UMainMenuWidget::OnBTN_GERCEK_SES_Clicked);

  // Ayarlar Geri Butonu
  if (Button_BackFromSettings)
    Button_BackFromSettings->OnClicked.AddDynamic(
        this, &UMainMenuWidget::OnBackFromSettingsClicked);

  // --------- Görüntü Ayarları Uygula Butonu ---------
  if (Button_ApplyVideo)
    Button_ApplyVideo->OnClicked.AddDynamic(
        this, &UMainMenuWidget::OnApplyVideoClicked);

  // --------- Ses Ayarları (Slider ve Progress Bar) ---------
  if (Slider_MasterVolume) {
    Slider_MasterVolume->OnValueChanged.AddDynamic(
        this, &UMainMenuWidget::OnMasterVolumeChanged);
    if (PB_MasterVolume)
      PB_MasterVolume->SetPercent(Slider_MasterVolume->GetValue());
  }
  if (Slider_SFXVolume) {
    Slider_SFXVolume->OnValueChanged.AddDynamic(
        this, &UMainMenuWidget::OnSFXVolumeChanged);
    if (PB_SFXVolume)
      PB_SFXVolume->SetPercent(Slider_SFXVolume->GetValue());
  }
  if (Slider_MusicVolume) {
    Slider_MusicVolume->OnValueChanged.AddDynamic(
        this, &UMainMenuWidget::OnMusicVolumeChanged);
    if (PB_MusicVolume)
      PB_MusicVolume->SetPercent(Slider_MusicVolume->GetValue());
  }

  /* ========================================================
   * MOTOR AYAĞA KALKARKEN STEAM SİSTEMİNİ BUL
   * ======================================================== */
  UGameInstance *GameInstance = GetGameInstance();
  if (GameInstance) {
    MultiplayerSubsystem =
        GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
  }

  return true;
}

/* ========================================================
 * GİRİŞ (INPUT) VE KONTROL ODAK KORUMASI
 * ======================================================== */

void UMainMenuWidget::NativeConstruct() {
  Super::NativeConstruct();

  // KÖK ÇÖZÜM 1: Widget'ın kendisini Focus (Odak) alabilir hale getiriyoruz.
  // Blueprint ayarlarında bozulmuş olsa bile C++ bunu ezip zorunlu tutacak.
  SetIsFocusable(true);

  // KÖK ÇÖZÜM 2: Ana widget'i, Root'u (Canvas Panel) ve Resmi "Visible" (Sert
  // Çarpışma) olarak kilitliyoruz. Böylece "boşluklar" dahil hiçbir alan şeffaf
  // davranıp tıklamayı oyuna geçirmez.
  SetVisibility(ESlateVisibility::Visible);

  if (UWidget *RootW = GetRootWidget()) {
    RootW->SetVisibility(ESlateVisibility::Visible);
  }

  if (IIMG_Background) {
    IIMG_Background->SetVisibility(ESlateVisibility::Visible);
  }

  // 1- Widget açılır açılmaz Player Controller'ı bul
  if (APlayerController *PC = UGameplayStatics::GetPlayerController(this, 0)) {
    // 2- Giriş modunu tamamen 'Sadece Arayüz' (UI Only) olarak ayarla
    FInputModeUIOnly InputModeData;

    // Odağı bu widget'ın kendisine kitle
    InputModeData.SetWidgetToFocus(TakeWidget());

    // Fareyi pencereye kilitleme konusunda esnek ol
    InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

    // 3- Cihaza, hazırladığımız bu kuralları empoze et (Uygula)
    PC->SetInputMode(InputModeData);

    // 4- Farenin sürekli ekranda görünmesini sağla
    PC->SetShowMouseCursor(true);
  }
}

FReply
UMainMenuWidget::NativeOnMouseButtonDown(const FGeometry &InGeometry,
                                         const FPointerEvent &InMouseEvent) {
  // Tıklamayı yut VE en önemlisi odak noktasını (focus) kendinde tut!
  // Yoksa Focus motora düşer ve farenin görünürlüğü sıfırlanır.
  return FReply::Handled().SetUserFocus(TakeWidget(), EFocusCause::Mouse);
}

FReply UMainMenuWidget::NativeOnMouseButtonDoubleClick(
    const FGeometry &InGeometry, const FPointerEvent &InMouseEvent) {
  // Çift tıklamaların da yutulmasını ve Focus'un bizde kalmasını garantiye
  // alıyoruz.
  return FReply::Handled().SetUserFocus(TakeWidget(), EFocusCause::Mouse);
}

/* ========================================================
 * ANA SAYFA VE SUNUCU GEÇİŞLERİ (WS_MenuPages)
 * ======================================================== */

void UMainMenuWidget::OnPlayClicked() {
  // Oynaya basınca kanal 1: Sunucu Seçimi
  if (WS_MenuPages)
    WS_MenuPages->SetActiveWidgetIndex(1);
}

void UMainMenuWidget::OnSettingsClicked() {
  // Ayarlara basınca kanal 2: Ayarlar Paneli
  if (WS_MenuPages)
    WS_MenuPages->SetActiveWidgetIndex(2);
  // Ayarların ilk açılan detay sayfası (0: Görüntü) olsun.
  if (WS_AyarDetaylari)
    WS_AyarDetaylari->SetActiveWidgetIndex(0);
}

void UMainMenuWidget::OnQuitClicked() {
  // Sızıntısız kapatma
  if (APlayerController *PC = UGameplayStatics::GetPlayerController(this, 0)) {
    UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, true);
  }
}

// ------ Sunucu Butonları ------
void UMainMenuWidget::OnCreateServerClicked() {
  if (GEngine)
    GEngine->AddOnScreenDebugMessage(
        -1, 5.f, FColor::Yellow,
        TEXT("Sunucu Olusturuluyor... Steam Subsystem Devrede!"));
  if (MultiplayerSubsystem)
    MultiplayerSubsystem->CreateServer();
}

void UMainMenuWidget::OnJoinServerClicked() {
  if (GEngine)
    GEngine->AddOnScreenDebugMessage(
        -1, 5.f, FColor::Yellow,
        TEXT("Sunucu Araniyor... Steam Subsystem Devrede!"));
  if (MultiplayerSubsystem)
    MultiplayerSubsystem->FindServer();
}

void UMainMenuWidget::OnBackFromPlayClicked() {
  // Sunucudan geriye basılınca Ana Menü (0. Kanal)
  if (WS_MenuPages)
    WS_MenuPages->SetActiveWidgetIndex(0);
}

/* ========================================================
 * YAN PANEL SEKMELERİ (WS_AyarDetaylari)
 * ======================================================== */

void UMainMenuWidget::OnBTN_GERCEK_GORUNTU_Clicked() {
  if (WS_AyarDetaylari)
    WS_AyarDetaylari->SetActiveWidgetIndex(0);
}

void UMainMenuWidget::OnBTN_GERCEK_KONTROL_Clicked() {
  if (WS_AyarDetaylari)
    WS_AyarDetaylari->SetActiveWidgetIndex(1);
}

void UMainMenuWidget::OnBTN_GERCEK_SES_Clicked() {
  if (WS_AyarDetaylari)
    WS_AyarDetaylari->SetActiveWidgetIndex(2);
}

void UMainMenuWidget::OnBackFromSettingsClicked() {
  // Ayarlardan geriye dönerken Kanal 0 (Ana Ekran) atla.
  if (WS_MenuPages)
    WS_MenuPages->SetActiveWidgetIndex(0);
}

/* ========================================================
 * AYARLAR UYGULAMA MERKEZİ
 * ======================================================== */

void UMainMenuWidget::OnApplyVideoClicked() {
  if (GEngine) {
    UGameUserSettings *UserSettings = GEngine->GetGameUserSettings();
    if (UserSettings) {
      // Kalite - Doku - Skala
      if (CB_Quality) {
        int32 QualityIndex = CB_Quality->GetSelectedIndex();
        UserSettings->SetOverallScalabilityLevel(QualityIndex);
      }

      // Tam Ekran / Pencere
      if (CB_WindowMode) {
        int32 WindowModeIndex = CB_WindowMode->GetSelectedIndex();
        if (WindowModeIndex == 0) // Tam Ekran
        {
          UserSettings->SetFullscreenMode(EWindowMode::Fullscreen);
        } else if (WindowModeIndex == 1) // Pencereli
        {
          UserSettings->SetFullscreenMode(EWindowMode::Windowed);
        }
      }

      // VSync (Ekran Dalgalanması Tıklaması)
      if (Chk_VSync) {
        UserSettings->SetVSyncEnabled(Chk_VSync->IsChecked());
      }

      // Motor ayarlarına zorla yazdırır (Hiçbir popup ekranı/15 saniye uyarısı
      // sormaz!)
      UserSettings->ApplySettings(false);
    }
  }
}

/* ========================================================
 * SES AYARLARI MERKEZİ (Slider & Progress Bar Eşzamanlaması)
 * ======================================================== */

void UMainMenuWidget::OnMasterVolumeChanged(float Value) {
  // "float Value" parametresi, oyuncu fare ile slider'ı her kaydırdığında (0.0
  // ile 1.0 arası) bize gelen eksen değeridir. Progress Bar null mu (yani
  // silinmiş mi, oyunda yok mu) diye güvenlik kalkanı (Crash yeme engeli)
  // kuruyoruz.
  if (PB_MasterVolume) {
    // İlerleme çubuğunun doluluk oranını (Percent) Slider'dan gelen değere
    // birebir kopyalayıp eşitliyoruz!
    PB_MasterVolume->SetPercent(Value);
  }
}

void UMainMenuWidget::OnSFXVolumeChanged(float Value) {
  if (PB_SFXVolume) {
    // Efekt Slider'ı çekildikçe Efekt Progress bar'ına "Şu anda Slider X
    // değerinde, sen de kendini o oranda doldur" mesajını veriyoruz.
    PB_SFXVolume->SetPercent(Value);
  }
}

void UMainMenuWidget::OnMusicVolumeChanged(float Value) {
  if (PB_MusicVolume) {
    // Müzik Slider'ı hareket ettirildikçe ilgili eşi benzeri ProgressBar da o
    // oranda kendini günceller.
    PB_MusicVolume->SetPercent(Value);
  }
}
