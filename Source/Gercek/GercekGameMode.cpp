#include "GercekGameMode.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

AGercekGameMode::AGercekGameMode()
{
	// 4 Kişilik Co-Op Steam bağlantılarında yükleme ekranında kopmaları/düşmeleri (timeout)
	// engellemek için kesintisiz geçiş (Seamless Travel) aktifleştirildi.
	bUseSeamlessTravel = true;
}

void AGercekGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// İlerleyen aşamada oyuncuların doğacağı PlayerStart mekanikleri
	// (Örn: Dolu olmayan PlayerStart'ı seçmek) burada özelleştirilebilir.
	UE_LOG(LogTemp, Display, TEXT("[GercekGameMode] Yeni bir oyuncu sunucuya katildi."));
}
