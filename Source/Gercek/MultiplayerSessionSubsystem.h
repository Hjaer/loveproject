// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "FindSessionsCallbackProxy.h" // FBlueprintSessionResult için gerekli
#include "MultiplayerSessionSubsystem.generated.h"

// --- DELEGATES (Temsilciler - Numaratörlerimiz) ---
// Oda Kurma ve Odaya Katılma sadece "Başarılı mı (true/false)?" bilgisini döndürür.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, bool, bWasSuccessful);

// Oda Bulma işlemi hem "Başarılı mı?" bilgisini hem de "Bulunan Odaların Listesini (SearchResults)" döndürür.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, bool, bWasSuccessful, const TArray<FBlueprintSessionResult>&, SearchResults);

/**
 * Subsystem (Alt Sistem) Nedir?:
 * UGameInstanceSubsystem, oyun açıldığı saniye otomatik olarak doğan ve oyun tamamen kapanana kadar asla ölmeyen (yok olmayan) bir yapıdır.
 * AAA oyunlardaki "Çöpçatanlık" (Matchmaking) servisleri gibi gizli bir hizmetçidir.
 * Level değişse bile ölmez, bu yüzden ağ (network) ve oturum (session) işlemleri için en güvenilir yerdir.
 */
UCLASS(BlueprintType)
class GERCEK_API UMultiplayerSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiplayerSessionSubsystem();

	// Oyunun en başında Subsystem doğarken çalışan hazırlık fonksiyonu
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ODAYI KURAN (Host) İÇİN FONKSİYON
	// MaxPlayers=2 yaptık, LAN maçını kapattık (bIsLAN = false), yani doğrudan Steam'i hedefler.
	UFUNCTION(BlueprintCallable, Category = "Steam Co-Op")
	void CreateServer(int32 MaxPlayers = 2, bool bIsLAN = false);

	// ODAYI ARAYAN (Client) İÇİN FONKSİYON
	// MaxSearchResults: Steam'de aynı anda kaç açık oda arayacağımız.
	UFUNCTION(BlueprintCallable, Category = "Steam Co-Op")
	void FindServer(int32 MaxSearchResults = 10000, bool bIsLAN = false);

	// BULUNAN ODAYA KATILMAK İÇİN FONKSİYON
	// FindServer işleminin "SearchResults" listesinden seçilen bir odayı buraya takacağız.
	UFUNCTION(BlueprintCallable, Category = "Steam Co-Op")
	void JoinServer(const FBlueprintSessionResult& SessionResult);

	// Blueprint'te "Bind" (Bağlama) yapıp titreşmesini bekleyeceğimiz Event'ler
	UPROPERTY(BlueprintAssignable, Category = "Steam Co-Op | Events")
	FMultiplayerOnCreateSessionComplete OnCreateSessionCompleteEvent;

	UPROPERTY(BlueprintAssignable, Category = "Steam Co-Op | Events")
	FMultiplayerOnFindSessionsComplete OnFindSessionsCompleteEvent;

	UPROPERTY(BlueprintAssignable, Category = "Steam Co-Op | Events")
	FMultiplayerOnJoinSessionComplete OnJoinSessionCompleteEvent;

protected:
	// Steam arka planda işi bitirdiğinde C++ tarafında bizi uyaran iç fonksiyonlar
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

private:
	// Pointer (İşaretçi) Nedir?: Pointer (* veya Ptr), evin kendisi değil, evin adresinin yazılı olduğu bir kağıt parçasıdır.
	// Koca bir Online sistemi hafızada sürekli oradan oraya taşımak yerine, sadece "Adresi şurada" diyerek (Pointer ile) oyunun çok hızlı çalışmasını sağlarız.
	
	// Steam Oturum sistemini yöneten ana adres (Pointer)
	IOnlineSessionPtr SessionInterface;
	
	// Yaptığımız aramanın (Find) detaylarını aklında tutan adres (Pointer)
	TSharedPtr<class FOnlineSessionSearch> LastSessionSearch;
};
