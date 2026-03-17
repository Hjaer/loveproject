// Fill out your copyright notice in the Description page of Project Settings.

#include "PostApocGridItem.h"

// --- UMG bileşen başlıkları ---
#include "Components/SizeBox.h"
#include "Components/Image.h"

// Izgara hücresi piksel boyutu (her hücre = 50x50 px).
// Bu değeri merkezi bir sabit olarak tanımlamak, ileride değiştirmeyi kolaylaştırır.
static constexpr float GRID_CELL_SIZE = 50.0f;

// ============================================================
//  NativePreConstruct
//  UMG Designer'da her değişiklikte ve çalışma zamanı başlangıcında
//  çağrılır. Widget bileşenlerinin hazır olmasını garantilemek için
//  buradan RefreshVisuals() tetikliyoruz.
// ============================================================
void UPostApocGridItem::NativePreConstruct()
{
    // Üst sınıfın NativePreConstruct'ını çağırmak zorunludur.
    Super::NativePreConstruct();

    // Görsel güncelleme: eşya verisi ve widget bileşenleri hazırsa uygula.
    RefreshVisuals();
}

// ============================================================
//  RefreshVisuals
//  DataTable'dan okunan ItemSize ve ItemIcon değerlerine göre
//  ItemSizeBox boyutunu ve ItemIcon dokusunu günceller.
// ============================================================
void UPostApocGridItem::RefreshVisuals()
{
    // ── Güvenlik Kontrolü 1: Widget bileşenleri hazır mı? ──────────────────
    // BindWidget ile bağlanan bileşenler teorik olarak null olamaz,
    // ancak Designer önizlemesinde veya yanlış isimlendirmede null gelebilir.
    if (!ItemSizeBox || !ItemIcon)
    {
        UE_LOG(LogTemp, Warning,
               TEXT("[PostApocGridItem] RefreshVisuals: ItemSizeBox veya ItemIcon null! "
                    "UMG Designer'da widget isimlerini kontrol et."));
        return;
    }

    // ── Güvenlik Kontrolü 2: DataTable referansı dolu mu? ─────────────────
    if (ItemVerisi.IsNull())
    {
        // Henüz veri atanmamış — sessizce çık (editor önizlemesinde normaldir).
        return;
    }

    // ── DataTable'dan Satır Verisini Çek ──────────────────────────────────
    const FPostApocItemRow* ItemData =
        ItemVerisi.GetRow<FPostApocItemRow>(TEXT("PostApocGridItem::RefreshVisuals"));

    // ── Güvenlik Kontrolü 3: Satır gerçekten var mı? ──────────────────────
    if (!ItemData)
    {
        UE_LOG(LogTemp, Warning,
               TEXT("[PostApocGridItem] RefreshVisuals: '%s' satırı DataTable'da bulunamadı."),
               *ItemVerisi.RowName.ToString());
        return;
    }

    // ── Boyut Hesaplama ───────────────────────────────────────────────────
    // ItemSize: X = Genişlik (sütun sayısı), Y = Yükseklik (satır sayısı).
    // Her hücre GRID_CELL_SIZE (50px) büyüklüğünde, bu yüzden çarparız.
    float CalculatedWidth  = static_cast<float>(ItemData->ItemSize.X) * GRID_CELL_SIZE;
    float CalculatedHeight = static_cast<float>(ItemData->ItemSize.Y) * GRID_CELL_SIZE;

    // ── Döndürme Kontrolü ─────────────────────────────────────────────────
    // bIsRotated = true ise eşya 90° döndürülmüş demektir:
    // Genişlik ↔ Yükseklik değerleri yer değiştirir.
    if (bIsRotated)
    {
        Swap(CalculatedWidth, CalculatedHeight);
    }

    // ── SizeBox Boyutlandırma ─────────────────────────────────────────────
    // SetWidthOverride / SetHeightOverride: SizeBox'ın sabit boyutunu belirler.
    ItemSizeBox->SetWidthOverride(CalculatedWidth);
    ItemSizeBox->SetHeightOverride(CalculatedHeight);

    // ── İkon Güncelleme ───────────────────────────────────────────────────
    // ItemIcon, FPostApocItemRow'da TSoftObjectPtr<UTexture2D> olarak tanımlı.
    // Runtime'da kullanmak için önce belleğe yüklememiz (LoadSynchronous) gerekir.
    // Not: Eğer birçok eşya aynı anda yükleniyorsa asenkron yükleme tercih edilebilir.
    if (ItemData->ItemIcon.IsNull())
    {
        UE_LOG(LogTemp, Warning,
               TEXT("[PostApocGridItem] RefreshVisuals: '%s' için ikon atanmamış."),
               *ItemVerisi.RowName.ToString());
        // İkon yoksa görüntüyü boş bırakıyoruz — crash riski yok.
        return;
    }

    // Soft pointer'ı senkron olarak belleğe yükle.
    UTexture2D* LoadedIcon = ItemData->ItemIcon.LoadSynchronous();

    if (LoadedIcon)
    {
        // Image widget'ına dokuyu uygula.
        ItemIcon->SetBrushFromTexture(LoadedIcon, /*bMatchSize=*/true);

        UE_LOG(LogTemp, Log,
               TEXT("[PostApocGridItem] '%s' başarıyla güncellendi — "
                    "Boyut: %.0fpx x %.0fpx | Döndürülmüş: %s"),
               *ItemVerisi.RowName.ToString(),
               CalculatedWidth,
               CalculatedHeight,
               bIsRotated ? TEXT("Evet") : TEXT("Hayır"));
    }
    else
    {
        UE_LOG(LogTemp, Warning,
               TEXT("[PostApocGridItem] RefreshVisuals: '%s' ikonu yüklenemedi."),
               *ItemVerisi.RowName.ToString());
    }
}
