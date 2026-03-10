#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFramework/SaveGame.h"
#include "Engine/DeveloperSettings.h"
#include "AudioSettingsSubsystem.generated.h"

// İleriye dönük sınıf bildirimleri (C++ derleyici hızı için)
class USoundMix;
class USoundClass;

/*
 * =========================================================================
 * 1) SES KAYIT SİSTEMİ (.sav dosyası) - (Meryem ve Hazar için)
 * Bu sınıf, oyunu açıp kapattığımızda seslerin sıfırlanmaması için 
 * diskte tuttuğumuz kaydın ("SaveGame") ta kendisidir. Cüzdan görevi görür.
 * =========================================================================
 */
UCLASS()
class GERCEK_API UAudioSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Bilgisayarda kaydedileceği sabit isim (Değişmez, bulması kolay olur)
	UPROPERTY(VisibleAnywhere, Category = "Ses Ayarları Kaydi")
	FString SaveSlotName = TEXT("AudioSettingsSlot");

	// Ses kaydırıcılarının (Slider) varsayılan pozisyonları: 1.0 (%100 Ses)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Ses Ayarları")
	float MasterVolume = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Ses Ayarları")
	float MusicVolume = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Ses Ayarları")
	float SFXVolume = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Ses Ayarları")
	float UIVolume = 1.0f;
};

/*
 * =========================================================================
 * 2) AAA ÖZEL EDİTÖR AYARLARI MENÜSÜ -> (DİKKAT MERYEM! BURASI SENİN İÇİN)
 * Subsystem'ler Editörde sürüklenip bırakılmazlar, hayalet olarak arkada yaşarlar. 
 * Bu yüzden sesleri (SoundMix ve SoundClass'ları) oyuna tanıtabilmen için sana 
 * Unreal Engine'in en üst Editör Menüsünün içine özel bir kategori açtık!
 * 
 * NASIL ATANACAK: 
 * Üstten "Edit -> Project Settings" yolunu aç.
 * Sol kısımdan aşağı inip "Game" başlığı altındaki "Gercek Audio Settings"e tıkla! 
 * İşte atamaları yapacağın yer orasıdır.
 * =========================================================================
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Gercek Audio Settings"))
class GERCEK_API UGercekAudioSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Tüm sesleri tek bir ana panodan kısmaya yarayan karıştırıcı (Genelde ana SoundMix)
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Ses Dosyalari", meta=(AllowedClasses="/Script/Engine.SoundMix"))
	FSoftObjectPath MainSoundMix;

	// Ana Ses (Master) kanalı
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Ses Dosyalari", meta=(AllowedClasses="/Script/Engine.SoundClass"))
	FSoftObjectPath MasterSoundClass;

	// Müzik ses kanalı
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Ses Dosyalari", meta=(AllowedClasses="/Script/Engine.SoundClass"))
	FSoftObjectPath MusicSoundClass;

	// Efekt ses (Silah, ayak sesi vb.) kanalı
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Ses Dosyalari", meta=(AllowedClasses="/Script/Engine.SoundClass"))
	FSoftObjectPath SFXSoundClass;

	// Arayüz tıklama, menü vb. ses kanalı
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Ses Dosyalari", meta=(AllowedClasses="/Script/Engine.SoundClass"))
	FSoftObjectPath UISoundClass;
};

/*
 * =========================================================================
 * 3) ANA SES SUBSYSTEM'İ (Makine Dairesi)
 * C++ üzerinden sese emir veren dev yöneticimizdir.
 * Meryem; sen Slider'ların "On Value Changed" kutucuğundan kablo çekip 
 * bu alt kısımdaki SetMasterVolume vb. düğümleri (node'ları) çağıracaksın!
 * Geri kalanı bizde.
 * =========================================================================
 */
UCLASS()
class GERCEK_API UAudioSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Sistem oyuna ilk girildiği an kendi kendine tetiklenen başlangıç motoru
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ---------------------------------------------------------
	// KISIM A: UI'dan Veri Alıp Oran Değiştirenler
	// ---------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category="Ses Sistemi")
	void SetMasterVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category="Ses Sistemi")
	void SetMusicVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category="Ses Sistemi")
	void SetSFXVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category="Ses Sistemi")
	void SetUIVolume(float NewVolume);

	// ---------------------------------------------------------
	// KISIM B: UI Açılırken "Slider" Çubuğu Nerede Dursun? Diyen Get'ler
	// Bunları Blueprint'te Set (Ayarlama) yaparken "Value" pinine bağlayın.
	// ---------------------------------------------------------

	UFUNCTION(BlueprintPure, Category="Ses Sistemi")
	float GetMasterVolume() const;

	UFUNCTION(BlueprintPure, Category="Ses Sistemi")
	float GetMusicVolume() const;

	UFUNCTION(BlueprintPure, Category="Ses Sistemi")
	float GetSFXVolume() const;

	UFUNCTION(BlueprintPure, Category="Ses Sistemi")
	float GetUIVolume() const;

	// ---------------------------------------------------------
	// KISIM C: Elle Kaydetme 
	// (Menüden "Ayarları Kaydet ve Geri Dön" tuşu yaparsanız buna bağlayın).
	// Normalde Set ettikçe otomatik kayıt eder ama ek emniyet olsun.
	// ---------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category="Ses Sistemi")
	void SaveAudioSettings();

private:
	// Arka planda dönen sihir: "Bu ses Class'ının oranını bu değere zorla!" emrini aktaran işçimiz
	void ApplyVolumeToEngine(FSoftObjectPath SoundClassPath, float InVolume);

	// Ses kaydını belleğimizde (RAM'de) canlı tuttuğumuz değişken
	UPROPERTY()
	UAudioSaveGame* LoadedSaveGame;
};
