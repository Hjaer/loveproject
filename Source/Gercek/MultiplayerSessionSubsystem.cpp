// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

UMultiplayerSessionSubsystem::UMultiplayerSessionSubsystem()
{
}

void UMultiplayerSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Oyundaki aktif Online Subsystem'i bul (Eğer Steam açıksa Steam'i, kapalıysa Null/Normal ağı bulur)
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		// Steam bulduktan sonra, Session (Oturum) arayüzünü alıp Pointer'ımıza (adres defterimize) kaydediyoruz.
		SessionInterface = Subsystem->GetSessionInterface();
	}
}

void UMultiplayerSessionSubsystem::CreateServer(int32 MaxPlayers, bool bIsLAN)
{
	if (!SessionInterface.IsValid()) return;

	// Eğer bilgisayarda asılı kalmış eski bir GameSession (Oturum) varsa onu yok ediyoruz ki temiz bir başlangıç yapalım.
	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}

	// Steam'den "Oda kuruldu!" cevabı geldiğinde bizim 'OnCreateSessionComplete' fonksiyonumuzun çalışması için dinleyiciyi bağlıyoruz.
	SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionSubsystem::OnCreateSessionComplete));

	// Oturum Ayarlarını (FOnlineSessionSettings) hazırlıyoruz
	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->bIsLANMatch = bIsLAN;               // LAN mı Steam mi? Seçiyoruz.
	SessionSettings->NumPublicConnections = MaxPlayers;    // Odaya kaç kişi girebilir? (2)
	SessionSettings->bAllowJoinInProgress = true;        // Oyun başlasa bile biri sonradan katılabilir mi?
	SessionSettings->bAllowJoinViaPresence = true;       // Steam listesinden arkadaşın üstüne tıklayıp "Oyuna Katıl" denebilir mi?
	SessionSettings->bShouldAdvertise = true;            // Odamız diğer oyuncular tarafından bulunabilsin mi? (Arama listesinde çıksın mı?)
	SessionSettings->bUsesPresence = true;               // Steam profilinde "Şu an oynuyor" bilgisi gözüksün mü?
	SessionSettings->bUseLobbiesIfAvailable = true;      // Steam Lobby sistemini kullan.

	// Bu bizim odamızın gizli şifresi! Sadece "GercekCoop" şifresine sahip olan odaları arayacağız ki başka oyunlarla karışmayalım.
	SessionSettings->Set(FName("MATCH_TYPE"), FString("GercekCoop"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// Ayarları hazırladık, şimdi Steam'e "Odayı Kur!" emrini veriyoruz.
	SessionInterface->CreateSession(0, NAME_GameSession, *SessionSettings);
}

void UMultiplayerSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	// İşlem bittiği için dinleyiciyi (Delegate) temizliyoruz ki hafıza dolmasın.
	SessionInterface->ClearOnCreateSessionCompleteDelegates(this);

	if (bWasSuccessful)
	{
		// Oda başarıyla kurulduysa, ana yürüme haritasına oyuncuyu ServerTravel ile yolluyoruz.
		// "listen" eki, haritayı bir "Sunucu (Host)" statüsünde açmasını sağlar. Diğerleri bu kapıdan içeri girecek.
		UWorld* World = GetWorld();
		if (World)
		{
			// Input Modunu ve Fare imlecini oyuna geri dondur!
			APlayerController* PlayerController = World->GetFirstPlayerController();
			if (PlayerController)
			{
				FInputModeGameOnly InputModeData;
				PlayerController->SetInputMode(InputModeData);
				PlayerController->bShowMouseCursor = false;
			}

			World->ServerTravel("/Game/FirstPerson/Lvl_FirstPerson?listen");
		}
	}

	// UI'a (Widget/Blueprint) haberi veriyoruz. ("Oda başarıyla kuruldu veya kurulamadı!" şeklinde)
	OnCreateSessionCompleteEvent.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionSubsystem::FindServer(int32 MaxSearchResults, bool bIsLAN)
{
	if (!SessionInterface.IsValid()) return;

	// Steam'den "Arama bitti!" cevabı geldiğinde çalışacak fonksiyonu dinlemeye alıyoruz.
	SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMultiplayerSessionSubsystem::OnFindSessionsComplete));

	// Arama Ayarları (FOnlineSessionSearch)
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults; // Genelde yüksek tutulur (örn: 10000) çünkü Steam dünya çapındaki tüm 480 (Spacewar) test odalarını arar.
	LastSessionSearch->bIsLanQuery = bIsLAN;
	
	// Odayı kurarken "GercekCoop" şifresi vermiştik. Arama yaparken Steam'e diyoruz ki: "Sadece içinde GercekCoop şifresi olan odaları bana getir!"
	LastSessionSearch->QuerySettings.Set(FName("PRESENCESEARCH"), true, EOnlineComparisonOp::Equals);

	// Kendi oyuncumuzu (LocalPlayer) temsil ederek aramayı başlat.
	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	
	// Note: Engine expects a UniqueNetIdRef, but passing the raw dereferenced pointer can cause C2665 error if implicit conversion fails
	SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef());
}

void UMultiplayerSessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	// İşlem bitti, temizlik.
	SessionInterface->ClearOnFindSessionsCompleteDelegates(this);

	// Blueprint'in anlayabileceği oda sonuçları listesi
	TArray<FBlueprintSessionResult> ResultsForBP;

	if (bWasSuccessful && LastSessionSearch.IsValid())
	{
		// Steam'in bulduğu tüm odaları tek tek dönüyoruz (Döngü)
		for (auto& Result : LastSessionSearch->SearchResults)
		{
			// Odanın şifresini kontrol ediyoruz
			FString MatchType;
			Result.Session.SessionSettings.Get(FName("MATCH_TYPE"), MatchType);

			// Eğer bulduğumuz odanın şifresi bizim "GercekCoop" ismine eşitse, onu Blueprint listesine ekle!
			if (MatchType == FString("GercekCoop"))
			{
				FBlueprintSessionResult BPResult;
				BPResult.OnlineResult = Result;
				ResultsForBP.Add(BPResult);
			}
		}
	}

	// Arama bittiğini Blueprint'e haber ver. İstediğimiz gibi bir oda buldu mu? Ve işte bulduğu odaların (sadece bizim oyunumuza ait olanların) listesi!
	OnFindSessionsCompleteEvent.Broadcast(bWasSuccessful, ResultsForBP);
}

void UMultiplayerSessionSubsystem::JoinServer(const FBlueprintSessionResult& SessionResult)
{
	if (!SessionInterface.IsValid()) return;

	// "Katılma işlemi bitti" cevabını dinlemeye başla
	SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionSubsystem::OnJoinSessionComplete));

	// Blueprint'ten gelen odaya (Seçili Odaya) katıl!
	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult.OnlineResult);
}

void UMultiplayerSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	// İşlem bitti, temizlik.
	SessionInterface->ClearOnJoinSessionCompleteDelegates(this);

	bool bWasSuccessful = (Result == EOnJoinSessionCompleteResult::Success);

	if (bWasSuccessful)
	{
		// Eğer katılma işlemi başarılıysa Steam bize Host'un (Sunucunun) gizli IP adresini (veya bağlantı kodunu) verir.
		FString Address;
		if (SessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
		{
			// Bağlantı kodunu aldıktan sonra, ClientTravel (İstemci Yolculuğu) ile oyuncumuzu ana sunucuya ışınlıyoruz!
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}

	// Katılma sonucunu Blueprint'e haber ver.
	OnJoinSessionCompleteEvent.Broadcast(bWasSuccessful);
}
