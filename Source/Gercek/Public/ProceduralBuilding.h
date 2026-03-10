#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralBuilding.generated.h"

// Unreal'a bu bileşenleri kullanacağımızı haber veriyoruz (Ön Tanımlama)
class USplineComponent;
class UPCGComponent;

UCLASS()
class GERCEK_API AProceduralBuilding : public AActor {
  GENERATED_BODY()

public:
  // Aktörümüzün yapıcı fonksiyonu (İlk doğduğu an)
  AProceduralBuilding();

  // Binanın temelini çizecek olan Spline (Çizgi) bileşeni
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bina Bilesenleri")
  USplineComponent *BuildingSpline;

  // Duvarları dikecek olan PCG bileşeni
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bina Bilesenleri")
  UPCGComponent *PCGComponent;

  // Editörden anında değiştirebileceğimiz bina genişliği (Santimetre cinsinden)
  UPROPERTY(EditAnywhere, Category = "Bina Ayarlari")
  float BuildingWidth = 1000.0f; // Varsayılan 10 Metre

  // Editörden anında değiştirebileceğimiz bina uzunluğu (Santimetre cinsinden)
  UPROPERTY(EditAnywhere, Category = "Bina Ayarlari")
  float BuildingLength = 1500.0f; // Varsayılan 15 Metre

  // Kaç katlı olacağı
  UPROPERTY(EditAnywhere, Category = "Bina Ayarlari")
  int32 NumberOfFloors = 1; // Varsayılan 1 kat (Sadece zemin)

  // Kat yüksekliği (Santimetre cinsinden)
  UPROPERTY(EditAnywhere, Category = "Bina Ayarlari")
  float FloorHeight = 300.0f; // Varsayılan 3 Metre

  // Üst katların splinelerini tutacağımız dizi
  UPROPERTY()
  TArray<USplineComponent*> FloorSplines;

  // Editörde bir değeri değiştirdiğimizde (Örn: Genişliği artırdığımızda)
  // anında çalışacak olan özel fonksiyon
  virtual void OnConstruction(const FTransform &Transform) override;
};