#include "MerchantBase.h"
#include "PostApocInventoryTypes.h"

// Varsayılan değerlerin ayarlandığı kurucu (Constructor)
AMerchantBase::AMerchantBase()
{
    // Tüccarın Tick fonksiyonuna ihtiyacı yoksa performansı artırmak için false yapıyoruz
    PrimaryActorTick.bCanEverTick = false;

    // Tüccarın kendi envanter bileşenini oluştur
    MerchantInventory = CreateDefaultSubobject<UPostApocInventoryComponent>(TEXT("MerchantInventory"));
    if (MerchantInventory)
    {
        // Varsayılan Grid Boyutları
        MerchantInventory->GridColumns = 10;
        MerchantInventory->GridRows = 10;
    }
}

// Oyun başlangıcında tetiklenir
void AMerchantBase::BeginPlay()
{
    Super::BeginPlay();

    if (MerchantInventory)
    {
        // Envanter ızgara boyutlarını Blueprint ayarlarından çekerek yapılandır
        MerchantInventory->GridColumns = GridColumns;
        MerchantInventory->GridRows = GridRows;

        // "InitialStock" dizisindeki her bir eşyayı sırayla tüccar envanterine (ızgaraya) yerleştir
        for (const FDataTableRowHandle& ItemHandle : InitialStock)
        {
            // Null kontrolü (Zero-Pointer Policy)
            if (!ItemHandle.IsNull())
            {
                // Eşya Handle'ı ile eşyayı ızgaraya sığdırmayı dener
                if (MerchantInventory->TryAddItem(ItemHandle))
                {
                    UE_LOG(LogTemp, Display, TEXT("[Dükkan] %s başarıyla rafa dizildi."), *ItemHandle.RowName.ToString());
                }
                else
                {
                    // Yer yoksa veya çakışma varsa hata logu
                    UE_LOG(LogTemp, Error, TEXT("[Dükkan] HATA: %s dükkan rafına SIĞMADI!"), *ItemHandle.RowName.ToString());
                }
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Dükkan] Kritik Hata: MerchantInventory bileşeni bulunamadı!"));
    }
}
