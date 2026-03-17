// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// --- Temel Unreal başlıkları ---
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h"

// --- Proje başlıkları ---
// FPostApocItemRow (ItemSize, ItemIcon, ItemID vb.) burada tanımlı.
#include "PostApocItemTypes.h"

// --- GENERATED HEADER: MUTLAKA SON SATIR OLMALI ---
#include "PostApocGridItem.generated.h"

// İleri bildirimler — include maliyetini düşürür.
class USizeBox;
class UImage;

/**
 * UPostApocGridItem
 *
 * Tetris ızgarasında tek bir eşyayı temsil eden UserWidget.
 *
 * Görevleri:
 *   • DataTable'dan eşya boyutunu okuyarak ItemSizeBox'ı yeniden boyutlandırır.
 *   • Eşyanın ikonunu ItemIcon Image'ına uygular.
 *   • bIsRotated ise genişlik ↔ yükseklik değerlerini değiştirir.
 *
 * Kullanım:
 *   Blueprint veya C++ tarafında ItemVerisi doldurulduktan sonra
 *   NativePreConstruct (editor önizleme) ve BeginPlay (çalışma zamanı)
 *   sırasında RefreshVisuals() otomatik olarak tetiklenir.
 */
UCLASS()
class GERCEK_API UPostApocGridItem : public UUserWidget
{
    GENERATED_BODY()

public:
    // ================================================================
    //  VERI — Blueprint'ten veya C++'tan doldurulur.
    // ================================================================

    /**
     * ItemVerisi
     * Eşyanın DataTable satır referansı.
     * ExposeOnSpawn=true => Widget oluşturulurken "Create Widget" node'unda
     * doğrudan pin olarak görünür.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
              meta = (ExposeOnSpawn = "true"),
              Category = "PostApoc Grid Item")
    FDataTableRowHandle ItemVerisi;

    /**
     * bIsRotated
     * true ise eşya 90° döndürülmüş kabul edilir:
     * Genişlik ↔ Yükseklik değerleri yer değiştirir.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
              meta = (ExposeOnSpawn = "true"),
              Category = "PostApoc Grid Item")
    bool bIsRotated = false;

protected:
    // ================================================================
    //  UI BAĞLANTILARI — UMG Designer'da aynı adla eşleşmeli!
    // ================================================================

    /**
     * ItemSizeBox
     * Eşyanın piksel boyutunu belirleyen SizeBox.
     * WidthOverride ve HeightOverride bu kutucuk üzerinden ayarlanır.
     */
    UPROPERTY(BlueprintReadOnly,
              meta = (BindWidget),
              Category = "PostApoc Grid Item | Widgets")
    TObjectPtr<USizeBox> ItemSizeBox;

    /**
     * ItemIcon
     * Eşyanın ikonunu gösteren Image widget'ı.
     */
    UPROPERTY(BlueprintReadOnly,
              meta = (BindWidget),
              Category = "PostApoc Grid Item | Widgets")
    TObjectPtr<UImage> ItemIcon;

    // ================================================================
    //  OVERRIDE'LAR
    // ================================================================

    /**
     * NativePreConstruct
     * Editor önizlemesinde ve çalışma zamanında widget hazır olduğunda
     * RefreshVisuals()'ı tetikler.
     */
    virtual void NativePreConstruct() override;

private:
    // ================================================================
    //  YARDIMCI FONKSİYON
    // ================================================================

    /**
     * RefreshVisuals
     * DataTable'dan okunan değerlere göre ItemSizeBox ve ItemIcon'u günceller.
     * Güvenlik kontrolleri: null DataTable, null row, null texture.
     */
    void RefreshVisuals();
};
