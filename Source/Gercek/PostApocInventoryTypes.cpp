// Fill out your copyright notice in the Description page of Project Settings.

#include "PostApocInventoryTypes.h"
#include "Engine/DataTable.h"

// ============================================================
//  UPostApocInventoryComponent — Implementasyon
// ============================================================

UPostApocInventoryComponent::UPostApocInventoryComponent()
{
    // Tick kapalı: ızgara durumu sadece mutasyonlarda değişir,
    // her frame güncellenmesi gereksizdir.
    PrimaryComponentTick.bCanEverTick = false;
}

void UPostApocInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    // Başlangıçta ızgara boş; OccupiedSlots zaten TMap ile sıfırlanmış.
}

// ------------------------------------------------------------
//  CalculateBarterValue
//  Formül: InBaseValue * (InCondition * ConditionWeight)
// ------------------------------------------------------------
float UPostApocInventoryComponent::CalculateBarterValue(
    float InBaseValue, float InCondition, float ConditionWeight) const
{
    // Kondisyonu ve ağırlığı 0-1 aralığında sıkıştır; negatif değerleri engelle.
    const float SafeCondition = FMath::Clamp(InCondition, 0.0f, 1.0f);
    const float SafeWeight    = FMath::Max(ConditionWeight, 0.0f);

    return InBaseValue * (SafeCondition * SafeWeight);
}

// ------------------------------------------------------------
//  CheckSpace
//  TopLeftIndex'ten başlayarak ItemSize (döndürme hesaba katılarak)
//  kadar alanın müsait olup olmadığını kontrol eder.
// ------------------------------------------------------------
bool UPostApocInventoryComponent::CheckSpace(
    FIntPoint TopLeftIndex, FIntPoint ItemSize, bool bIsRotated) const
{
    // Döndürme aktifse X (genişlik) ve Y (yükseklik) yer değiştirir.
    const int32 EffectiveWidth  = bIsRotated ? ItemSize.Y : ItemSize.X;
    const int32 EffectiveHeight = bIsRotated ? ItemSize.X : ItemSize.Y;

    // Her eşyanın boyutu en az 1x1 olmalı.
    if (EffectiveWidth <= 0 || EffectiveHeight <= 0)
    {
        return false;
    }

    // Tüm hücreleri sola-üstten sağa-alta tara.
    for (int32 Row = 0; Row < EffectiveHeight; ++Row)
    {
        for (int32 Col = 0; Col < EffectiveWidth; ++Col)
        {
            const int32 CheckX = TopLeftIndex.X + Col;
            const int32 CheckY = TopLeftIndex.Y + Row;

            // --- Sınır Kontrolü ---
            // Hücre ızgara sınırlarının dışındaysa yer yok.
            if (CheckX < 0 || CheckX >= GridColumns ||
                CheckY < 0 || CheckY >= GridRows)
            {
                return false;
            }

            // --- Çakışma Kontrolü ---
            // Bu koordinatta zaten bir eşya varsa yer yok.
            const FIntPoint CellToCheck(CheckX, CheckY);
            if (OccupiedSlots.Contains(CellToCheck))
            {
                return false;
            }
        }
    }

    // Tüm hücreler boş ve sınır içi: yer var.
    return true;
}

// ------------------------------------------------------------
//  TryAddItem
//  DataTable satır referansından eşya verisini okur, ızgarada
//  uygun bir boş alan arar ve bulunursa tüm hücreleri işaretler.
// ------------------------------------------------------------
bool UPostApocInventoryComponent::TryAddItem(FDataTableRowHandle ItemRowHandle)
{
    // Boş referans kontrolü.
    if (ItemRowHandle.IsNull())
    {
        return false;
    }

    // DataTable'dan satır verisini çek (FPostApocItemRow — doğru struct).
    const FPostApocItemRow* ItemData = ItemRowHandle.GetRow<FPostApocItemRow>(TEXT("TryAddItem"));
    if (!ItemData)
    {
        UE_LOG(LogTemp, Warning,
               TEXT("[Grid Inventory] TryAddItem: DataTable satiri bulunamadi — '%s'"),
               *ItemRowHandle.RowName.ToString());
        return false;
    }

    const FIntPoint ItemSize   = ItemData->ItemSize;
    const bool      bCanRotate = ItemData->bCanBeRotated;

    // Önce normal yönde boş alan ara.
    FIntPoint FoundLocation;
    bool bFoundSpace          = FindEmptySpace(ItemSize, /*bCheckRotated=*/false, FoundLocation);
    bool bIsRotatedWhenPlaced = false;

    // Normal yönde yer yoksa ve eşya döndürülebiliyorsa, döndürülmüş dene.
    if (!bFoundSpace && bCanRotate)
    {
        bFoundSpace           = FindEmptySpace(ItemSize, /*bCheckRotated=*/true, FoundLocation);
        bIsRotatedWhenPlaced  = bFoundSpace;
    }

    if (bFoundSpace)
    {
        // Döndürme durumuna göre gerçek genişlik/yüksekliği belirle.
        const int32 EffectiveWidth  = bIsRotatedWhenPlaced ? ItemSize.Y : ItemSize.X;
        const int32 EffectiveHeight = bIsRotatedWhenPlaced ? ItemSize.X : ItemSize.Y;

        // Eşyanın kapladığı tüm hücreleri dolu olarak işaretle.
        // Anahtar: hücre koordinatı  |  Değer: satır adı (benzersiz kimlik)
        for (int32 Row = 0; Row < EffectiveHeight; ++Row)
        {
            for (int32 Col = 0; Col < EffectiveWidth; ++Col)
            {
                const FIntPoint Cell(FoundLocation.X + Col, FoundLocation.Y + Row);
                OccupiedSlots.Add(Cell, ItemRowHandle.RowName);
            }
        }

        UE_LOG(LogTemp, Log,
               TEXT("[Grid Inventory] '%s' eklendi — Konum: (%d, %d)  Dondurulmus: %s"),
               *ItemData->DisplayName.ToString(),
               FoundLocation.X, FoundLocation.Y,
               bIsRotatedWhenPlaced ? TEXT("Evet") : TEXT("Hayir"));
        return true;
    }

    // Boş alan bulunamadı.
    UE_LOG(LogTemp, Warning,
           TEXT("[Grid Inventory] Yer yok! '%s' cantaya sigmadi."),
           *ItemData->DisplayName.ToString());
    return false;
}

// ------------------------------------------------------------
//  FindEmptySpace
//  Izgara üzerinde sol-üstten sağ-alta tarayarak ItemSize
//  boyutuna (veya döndürülmüş boyutuna) sığan ilk boş hücreyi
//  bulur ve OutFoundLocation'a yazar.
// ------------------------------------------------------------
bool UPostApocInventoryComponent::FindEmptySpace(
    FIntPoint ItemSize, bool bCheckRotated, FIntPoint& OutFoundLocation) const
{
    for (int32 Y = 0; Y < GridRows; ++Y)
    {
        for (int32 X = 0; X < GridColumns; ++X)
        {
            const FIntPoint CurrentCell(X, Y); // Düzeltme: (X,X) değil (X,Y)

            if (CheckSpace(CurrentCell, ItemSize, bCheckRotated))
            {
                OutFoundLocation = CurrentCell;
                return true;
            }
        }
    }

    // Izgara tamamen dolu ya da bu boyut için yer yok.
    return false;
}

// ------------------------------------------------------------
//  RemoveItemFromGrid
//  OccupiedSlots TMap'ini tarayarak verilen RowName'e ait tüm
//  hücre kayıtlarını bir geçişte kaldırır.
//  Eşyanın tüm hücreleri işaretlendiği için (TryAddItem'da eklendi)
//  sadece Value eşleşmesi yeterlidir — Key'e göre arama yapılmaz.
// ------------------------------------------------------------
bool UPostApocInventoryComponent::RemoveItemFromGrid(FName ItemRowName)
{
    if (ItemRowName.IsNone())
    {
        return false;
    }

    // Silinecek hücreleri topla (TMap iterasyonu sırasında mutasyon yapılmaz).
    TArray<FIntPoint> CellsToRemove;
    for (const TPair<FIntPoint, FName>& Pair : OccupiedSlots)
    {
        if (Pair.Value == ItemRowName)
        {
            CellsToRemove.Add(Pair.Key);
        }
    }

    if (CellsToRemove.Num() == 0)
    {
        UE_LOG(LogTemp, Warning,
               TEXT("[Grid Inventory] RemoveItemFromGrid: '%s' izgararada bulunamadi."),
               *ItemRowName.ToString());
        return false;
    }

    for (const FIntPoint& Cell : CellsToRemove)
    {
        OccupiedSlots.Remove(Cell);
    }

    UE_LOG(LogTemp, Log,
           TEXT("[Grid Inventory] '%s' izgaradan kaldirildi (%d hucre serbest bırakıldı)."),
           *ItemRowName.ToString(), CellsToRemove.Num());
    return true;
}

// ------------------------------------------------------------
//  GetItemCountInGrid
//  Belirtilen RowName'e ait kaç hücrenin dolu olduğunu döndürür.
//  Uyarı: Bu, hücre sayısıdır — adet sayısı değil.
//  1x1 eşya = 1 hücre, 2x3 eşya = 6 hücre döndürür.
// ------------------------------------------------------------
int32 UPostApocInventoryComponent::GetItemCountInGrid(FName ItemRowName) const
{
    if (ItemRowName.IsNone())
    {
        return 0;
    }

    int32 Count = 0;
    for (const TPair<FIntPoint, FName>& Pair : OccupiedSlots)
    {
        if (Pair.Value == ItemRowName)
        {
            ++Count;
        }
    }
    return Count;
}

// ------------------------------------------------------------
//  ExecuteTrade — Takas İşlemini Gerçekleştiren C++ Fonksiyonu
//  Blueprint'ten çağrılır.
// ------------------------------------------------------------
void UPostApocInventoryComponent::ExecuteTrade(
    TArray<FDataTableRowHandle> PlayerOfferItems,
    TArray<FDataTableRowHandle> TraderOfferItems)
{
    // 1. OYUNCUNUN VERDİKLERİNİ SİL (Çantadan çıkar)
    // Not: İleride RemoveItem fonksiyonu yazıldığında buradaki yorum satırları açılacak.
    /*
    for (const FDataTableRowHandle& ItemToRemove : PlayerOfferItems)
    {
        // RemoveItem(ItemToRemove);
    }
    */

    // 2. TÜCCARDAN ALINANLARI ÇANTAYA YERLEŞTİR
    for (const FDataTableRowHandle& ItemToAdd : TraderOfferItems)
    {
        // TryAddItem, eşyayı Tetris çantasına yerleştirmeye çalışır
        bool bSuccess = TryAddItem(ItemToAdd);

        if (bSuccess)
        {
            UE_LOG(LogTemp, Display, TEXT("[Takas] Esya basariyla Tetris cantaya eklendi."));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[Takas] Cantada yer yok! Esya sigmadi."));
            // İleride buraya sığmayan eşyayı yere atma (SpawnActor) kodu eklenecek.
        }
    }

    // 3. UI'I GÜNCELLE
    // Takas bittikten sonra Blueprint tarafında arayüzü yenilemek için ileride buraya bir Delegate eklenecek.
}
