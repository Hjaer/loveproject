#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "MerchantBase.generated.h"

// Ileriye dönük tanımlamalar (Forward declarations)
class UPostApocInventoryComponent;

UCLASS()
class GERCEK_API AMerchantBase : public ACharacter
{
    GENERATED_BODY()

public:
    AMerchantBase();

protected:
    virtual void BeginPlay() override;

public:
    // Tüccarın görüntülenen mağaza adı
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant Info")
    FText MerchantName;

    // Tüccara ait ızgara tabanlı özel envanter bileşeni
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Merchant Inventory")
    UPostApocInventoryComponent* MerchantInventory;

    // Tüccarın sahip olacağı ızgara (grid) sütun sayısı
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant Inventory")
    int32 GridColumns = 10;

    // Tüccarın sahip olacağı ızgara (grid) satır sayısı
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant Inventory")
    int32 GridRows = 10;

    // Başlangıçta tüccarın envanter raflarına yerleştirilecek eşya listesi
    // Zero-Pointer policy: FDataTableRowHandle kullanımı
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Merchant Inventory")
    TArray<FDataTableRowHandle> InitialStock;

    // -- Blueprint Getter Fonksiyonları (Encapsulation) --

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Merchant Info")
    FText GetMerchantName() const { return MerchantName; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Merchant Inventory")
    UPostApocInventoryComponent* GetMerchantInventory() const { return MerchantInventory; }
};
