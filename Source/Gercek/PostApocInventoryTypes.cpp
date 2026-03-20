// Fill out your copyright notice in the Description page of Project Settings.

#include "PostApocInventoryTypes.h"
#include "Engine/DataTable.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "PostApocGridItem.h"
#include "GercekCharacter.h"
#include "Net/UnrealNetwork.h"

// ============================================================
//  UPostApocInventoryComponent — Implementasyon
// ============================================================

UPostApocInventoryComponent::UPostApocInventoryComponent()
{
    // Tick kapalı: ızgara durumu sadece mutasyonlarda değişir,
    // her frame güncellenmesi gereksizdir.
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UPostApocInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    // Başlangıçta ızgara boş; OccupiedSlotsArray sıfırdan başlar.
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
            bool bIsOccupied = false;
            for (const FGridSlotData& SlotData : OccupiedSlotsArray)
            {
                if (SlotData.Location == CellToCheck)
                {
                    bIsOccupied = true;
                    break;
                }
            }
            if (bIsOccupied)
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

    // UI refresh ve sonraki drop/trade islemleri icin aktif tabloyu sabitle.
    if (!ItemDataTable && ItemRowHandle.DataTable)
    {
        ItemDataTable = const_cast<UDataTable*>(ItemRowHandle.DataTable.Get());
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
                FGridSlotData NewSlot;
                NewSlot.Location = FIntPoint(FoundLocation.X + Col, FoundLocation.Y + Row);
                NewSlot.ItemID = ItemRowHandle.RowName;
                OccupiedSlotsArray.Add(NewSlot);
            }
        }

        UE_LOG(LogTemp, Log,
               TEXT("[Grid Inventory] '%s' eklendi — Konum: (%d, %d)  Dondurulmus: %s"),
               *ItemData->DisplayName.ToString(),
               FoundLocation.X, FoundLocation.Y,
               bIsRotatedWhenPlaced ? TEXT("Evet") : TEXT("Hayir"));
        BroadcastGridChanged();
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

    bool bRemovedAny = false;
    for (int32 i = OccupiedSlotsArray.Num() - 1; i >= 0; --i)
    {
        if (OccupiedSlotsArray[i].ItemID == ItemRowName)
        {
            OccupiedSlotsArray.RemoveAt(i);
            bRemovedAny = true;
        }
    }

    if (!bRemovedAny)
    {
        UE_LOG(LogTemp, Warning,
               TEXT("[Grid Inventory] RemoveItemFromGrid: '%s' izgarada bulunamadi."),
               *ItemRowName.ToString());
        return false;
    }

    UE_LOG(LogTemp, Log,
           TEXT("[Grid Inventory] '%s' izgaradan kaldirildi."),
           *ItemRowName.ToString());
    BroadcastGridChanged();
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
    for (const FGridSlotData& SlotData : OccupiedSlotsArray)
    {
        if (SlotData.ItemID == ItemRowName)
        {
            ++Count;
        }
    }
    return Count;
}

float UPostApocInventoryComponent::GetInventoryValue() const
{
    float TotalValue = 0.0f;
    if (!ItemDataTable) {
        return TotalValue;
    }

    TMap<FName, int32> OccupiedCellCounts;
    for (const FGridSlotData& SlotData : OccupiedSlotsArray)
    {
        OccupiedCellCounts.FindOrAdd(SlotData.ItemID)++;
    }

    for (const TPair<FName, int32>& Entry : OccupiedCellCounts)
    {
        const FPostApocItemRow* ItemData =
            ItemDataTable->FindRow<FPostApocItemRow>(Entry.Key, TEXT("GetInventoryValue"));
        if (!ItemData) {
            continue;
        }

        const int32 ItemArea = FMath::Max(1, ItemData->ItemSize.X * ItemData->ItemSize.Y);
        const int32 ItemCount = FMath::Max(1, Entry.Value / ItemArea);
        TotalValue += ItemData->BaseValue * ItemCount;
    }

    return TotalValue;
}

void UPostApocInventoryComponent::NativeRefreshUI(UUserWidget* GridWidget)
{
    // Güvenlik Kontrolleri
    if (!IsValid(GridWidget) || !GridItemWidgetClass || !ItemDataTable)
    {
        return;
    }

    // Canvas Panel'i bul ( Designer tarafında IsVariable=True ve adı "GridCanvas" olmalı)
    UCanvasPanel* GridCanvas = Cast<UCanvasPanel>(GridWidget->GetWidgetFromName(TEXT("GridCanvas")));
    if (!GridCanvas)
    {
        return;
    }

    // Eski widget'ları temizle
    GridCanvas->ClearChildren();

    // İşlenmiş hücreleri takip et (Eşya boyutuna göre atlamak için)
    TSet<FIntPoint> ProcessedCells;

    for (int32 Y = 0; Y < GridRows; ++Y)
    {
        for (int32 X = 0; X < GridColumns; ++X)
        {
            FIntPoint CurrentSlot(X, Y);

            // Eğer bu hücre daha önce bir eşyanın parçası olarak işlendiyse atla
            if (ProcessedCells.Contains(CurrentSlot))
            {
                continue;
            }

            // Hücrede bir eşya var mı?
            FName FoundItemName = NAME_None;
            for (const FGridSlotData& SlotData : OccupiedSlotsArray)
            {
                if (SlotData.Location == CurrentSlot)
                {
                    FoundItemName = SlotData.ItemID;
                    break;
                }
            }

            if (FoundItemName != NAME_None)
            {
                FName RowName = FoundItemName;

                // Data Table'dan eşya verisini çek (Boyut bilgisi için)
                const FPostApocItemRow* ItemData = ItemDataTable->FindRow<FPostApocItemRow>(RowName, TEXT("NativeRefreshGrid"));
                if (!ItemData) continue;

                // Grid Item Widget'ını yarat
                UPostApocGridItem* NewItemWidget = CreateWidget<UPostApocGridItem>(GridWidget, GridItemWidgetClass);
                if (NewItemWidget)
                {
                    // Veriyi enjekte et
                    NewItemWidget->ItemVerisi.DataTable = ItemDataTable;
                    NewItemWidget->ItemVerisi.RowName = RowName;

                    // Arayüzü C++ üzerinden zorla yenile (Grid ikonlarının görünmesi için şart)
                    NewItemWidget->RefreshVisuals();

                    // Canvas üzerine yerleştir
                    UCanvasPanelSlot* CanvasSlot = GridCanvas->AddChildToCanvas(NewItemWidget);
                    if (CanvasSlot)
                    {
                        CanvasSlot->SetAutoSize(true);
                        // Piksel koordinatına çevir (X * 50, Y * 50)
                        CanvasSlot->SetPosition(FVector2D(X * TileSize, Y * TileSize));
                    }

                    // Eşyanın kapladığı tüm hücreleri "Processed" olarak işaretle
                    for (int32 i = 0; i < ItemData->ItemSize.X; ++i)
                    {
                        for (int32 j = 0; j < ItemData->ItemSize.Y; ++j)
                        {
                            ProcessedCells.Add(FIntPoint(X + i, Y + j));
                        }
                    }
                }
            }
        }
    }
}

bool UPostApocInventoryComponent::HandleItemDrop(FName ItemRowName, FIntPoint NewLocation)
{
    // Eşyayı mevcut yerinden kaldır
    if (!RemoveItemFromGrid(ItemRowName))
    {
        return false;
    }

    // Yeni yere eklemeyi dene
    FDataTableRowHandle ItemHandle;
    ItemHandle.DataTable = ItemDataTable;
    ItemHandle.RowName = ItemRowName;

    // Not: TryAddItem içinde FindEmptySpace kullanılır, bu yüzden NewLocation'ı doğrudan kullanan
    // daha spesifik bir AddItem fonksiyonuna ihtiyaç olabilir. 
    // Ancak şimdilik TryAddItem ile en yakın boşluğa yerleştirme mantığını koruyoruz.
    // (Gelişmiş koordinatlı drop mantığı için CheckSpace + Manual Add kullanılabilir)
    
    if (CheckSpace(NewLocation, FIntPoint(1,1), false)) // Basit check, TryAddItem zaten detaylı bakar
    {
        // Gerçek koordinatlı ekleme mantığı:
        const FPostApocItemRow* ItemData = ItemDataTable->FindRow<FPostApocItemRow>(ItemRowName, TEXT("HandleItemDrop"));
        if (ItemData && CheckSpace(NewLocation, ItemData->ItemSize, false))
        {
             for (int32 Row = 0; Row < ItemData->ItemSize.Y; ++Row)
             {
                 for (int32 Col = 0; Col < ItemData->ItemSize.X; ++Col)
                 {
                     FGridSlotData NewSlot;
                     NewSlot.Location = FIntPoint(NewLocation.X + Col, NewLocation.Y + Row);
                     NewSlot.ItemID = ItemRowName;
                     OccupiedSlotsArray.Add(NewSlot);
                 }
             }
             BroadcastGridChanged();
             return true;
        }
    }
    
    // Başarısız olursa eski yerine geri dönmeyi veya en uygun yere atmayı deneyebiliriz
    return TryAddItem(ItemHandle);
}

void UPostApocInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UPostApocInventoryComponent, OccupiedSlotsArray);
}

void UPostApocInventoryComponent::OnRep_GridUpdated()
{
    // Arayüzü yenilemek için delegate'i tetikliyoruz. Karakter ya da arayüz bu eventi dinleyebilir
    OnGridUpdated.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("[Grid Inventory] OnRep_GridUpdated tetiklendi, UI yenilenmeli."));
}

TMap<FIntPoint, FName> UPostApocInventoryComponent::GetOccupiedSlots() const
{
    TMap<FIntPoint, FName> Map;
    for (const FGridSlotData& SlotData : OccupiedSlotsArray)
    {
        Map.Add(SlotData.Location, SlotData.ItemID);
    }
    return Map;
}

void UPostApocInventoryComponent::BroadcastGridChanged()
{
    OnGridUpdated.Broadcast();
}
