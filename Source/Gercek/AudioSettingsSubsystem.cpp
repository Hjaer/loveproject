#include "AudioSettingsSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"

// Motor ilk ayağa kalktığında (Oyun logosu çıktıktan 1 saniye sonra) tetiklenir:
void UAudioSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 1. DİSKTEN DOSYA ARAMASI YAPIYORUZ
	FString SName = TEXT("AudioSettingsSlot");
	
	// Bilgisayarın AppData / Saved klasöründe böyle bir dosya var mı? (Daha önce kaydedilmiş mi?)
	if (UGameplayStatics::DoesSaveGameExist(SName, 0))
	{
		// Dosyayı bulduysak onu alıp bizim "LoadedSaveGame" değişkenimize okutuyoruz
		LoadedSaveGame = Cast<UAudioSaveGame>(UGameplayStatics::LoadGameFromSlot(SName, 0));
	}
	else
	{
		// Adam oyunu ilk kez oynamış, ortada dosya yok! Hemen ona tertemiz %100 oranlı sıfır bir save dosyası üretiyoruz
		LoadedSaveGame = Cast<UAudioSaveGame>(UGameplayStatics::CreateSaveGameObject(UAudioSaveGame::StaticClass()));
		LoadedSaveGame->SaveSlotName = SName;
		UGameplayStatics::SaveGameToSlot(LoadedSaveGame, SName, 0); // Kredi kartını cüzdana boş bir şekilde yerleştirdik
	}

	// 2. YÜKLENEN AYARLARI GERÇEK MOTOR SİSTEMİNE ZERK ETME
	// Eğer yükleme başarılıysa, Editör Ayarlarına Meryem'in yerleştirdiği ses dosyalarını çekiyoruz
	const UGercekAudioSettings* Settings = GetDefault<UGercekAudioSettings>();
	if (Settings && LoadedSaveGame)
	{
		// Cüzdandaki (SaveGame'deki) oranları (Volume) asıl işi yapan motora iletiyoruz
		ApplyVolumeToEngine(Settings->MasterSoundClass, LoadedSaveGame->MasterVolume);
		ApplyVolumeToEngine(Settings->MusicSoundClass, LoadedSaveGame->MusicVolume);
		ApplyVolumeToEngine(Settings->SFXSoundClass, LoadedSaveGame->SFXVolume);
		ApplyVolumeToEngine(Settings->UISoundClass, LoadedSaveGame->UIVolume);
	}
}

// -------------------------------------------------------------
// SET (KAYDEDİCİ) FONKSİYONLARI: UI'dan gelen seviyeyi (0 ile 1 arası) alır
// -------------------------------------------------------------
void UAudioSettingsSubsystem::SetMasterVolume(float NewVolume)
{
	if (!LoadedSaveGame) return;
	
	// FMath::Clamp: Yanlışlıkla eksi (-) veya 1'den büyük volüm girilirse sistemi çökertmemek için 0 ile 1 arasına mandallar (Kilitler)
	LoadedSaveGame->MasterVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
	
	// Değitimizi yaptık, şimdi motordaki o Class'i kısıyoruz
	const UGercekAudioSettings* Settings = GetDefault<UGercekAudioSettings>();
	ApplyVolumeToEngine(Settings->MasterSoundClass, LoadedSaveGame->MasterVolume);

	// Değişimi anında diske fırınlıyoruz (Otomatik save)
	SaveAudioSettings();
}

void UAudioSettingsSubsystem::SetMusicVolume(float NewVolume)
{
	if (!LoadedSaveGame) return;
	LoadedSaveGame->MusicVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
	
	const UGercekAudioSettings* Settings = GetDefault<UGercekAudioSettings>();
	ApplyVolumeToEngine(Settings->MusicSoundClass, LoadedSaveGame->MusicVolume);
	
	SaveAudioSettings();
}

void UAudioSettingsSubsystem::SetSFXVolume(float NewVolume)
{
	if (!LoadedSaveGame) return;
	LoadedSaveGame->SFXVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
	
	const UGercekAudioSettings* Settings = GetDefault<UGercekAudioSettings>();
	ApplyVolumeToEngine(Settings->SFXSoundClass, LoadedSaveGame->SFXVolume);
	
	SaveAudioSettings();
}

void UAudioSettingsSubsystem::SetUIVolume(float NewVolume)
{
	if (!LoadedSaveGame) return;
	LoadedSaveGame->UIVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
	
	const UGercekAudioSettings* Settings = GetDefault<UGercekAudioSettings>();
	ApplyVolumeToEngine(Settings->UISoundClass, LoadedSaveGame->UIVolume);
	
	SaveAudioSettings();
}

// -------------------------------------------------------------
// GET (OKUYUCU) FONKSİYONLARI: Widget'lar açıldığında oradan okur
// -------------------------------------------------------------
float UAudioSettingsSubsystem::GetMasterVolume() const
{
	return LoadedSaveGame ? LoadedSaveGame->MasterVolume : 1.0f;
}

float UAudioSettingsSubsystem::GetMusicVolume() const
{
	return LoadedSaveGame ? LoadedSaveGame->MusicVolume : 1.0f;
}

float UAudioSettingsSubsystem::GetSFXVolume() const
{
	return LoadedSaveGame ? LoadedSaveGame->SFXVolume : 1.0f;
}

float UAudioSettingsSubsystem::GetUIVolume() const
{
	return LoadedSaveGame ? LoadedSaveGame->UIVolume : 1.0f;
}

// Tüm değerleri anında diske basan ek güvenlik fonksiyonu
void UAudioSettingsSubsystem::SaveAudioSettings()
{
	if (LoadedSaveGame)
	{
		UGameplayStatics::SaveGameToSlot(LoadedSaveGame, LoadedSaveGame->SaveSlotName, 0);
	}
}

/*
 * İŞİN MUTFAK (ARKA PLAN) KISMI: 
 * Asla Blueprint'e (UI'a) sızmaz, sadece C++ kendi kendine çalıştırır.
 */
void UAudioSettingsSubsystem::ApplyVolumeToEngine(FSoftObjectPath SoundClassPath, float InVolume)
{
	const UGercekAudioSettings* Settings = GetDefault<UGercekAudioSettings>();
	if (!Settings) return;

	// Meryem'in Project Settings'den seçtiği "Ana Ses Mikser" yollarını (Soft Path) bul ve GERÇEK bir objeye (.uasset) dönüştür.
	// TryLoad() komutu, oyunu dondurmadan dosyayı RAM'e usulca yükler.
	USoundMix* RealMix = Cast<USoundMix>(Settings->MainSoundMix.TryLoad());
	if (RealMix)
	{
		// 1) O Mikseri motora ilet (Aktif et ki çalışsın - Push it)
		UGameplayStatics::PushSoundMixModifier(this, RealMix);
		
		// 2) Kısılacak ya da açılacak hedef kanal dosyasını (Music, MFX vb.) yükle
		USoundClass* RealClass = Cast<USoundClass>(SoundClassPath.TryLoad());
		if (RealClass)
		{
			// Sihirli Unreal Motor Komutu: Seçili mikserin (RealMix) içinden, şurada belirttiğim kanalın (RealClass) Volume (Sesi) kısmını benim slider'dan yolladığım (InVolume) değerle ez (Override)! FadeIn 0 veriyoruz ki slideri kaydırırken ses anında değişsin.
			UGameplayStatics::SetSoundMixClassOverride(this, RealMix, RealClass, InVolume, 1.0f, 0.0f, true);
		}
	}
}
