#include "ProceduralBuilding.h"
#include "Components/SplineComponent.h"
#include "PCGComponent.h" // PCG eklentisinin başlık dosyasını dahil ediyoruz

// Bu kısım aktör sahneye ilk koyulduğunda 1 kere çalışır
AProceduralBuilding::AProceduralBuilding() {
  // Binanın her saniye güncellenmesine gerek yok, performansı koruruz.
  PrimaryActorTick.bCanEverTick = false;

  // 1. Spline (Çizgi) bileşenini oluştur ve ana merkez (Root) yap
  BuildingSpline =
      CreateDefaultSubobject<USplineComponent>(TEXT("BuildingSpline"));
  RootComponent = BuildingSpline;
  BuildingSpline->SetClosedLoop(
      true); // Odanın 4. duvarını (sonunu) otomatik kapatır

  // 2. PCG bileşenini oluştur (PCG mantıksal bir bileşen olduğu için
  // SetupAttachment KULLANILMAZ)
  PCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("PCGComponent"));
}

// Bu kısım editörde aktörü hareket ettirdiğinde veya ayarlarla oynadığında
// çalışır
void AProceduralBuilding::OnConstruction(const FTransform &Transform) {
  Super::OnConstruction(Transform);

  // --- YENİ AKILLI SİSTEM: Girilen değeri otomatik olarak duvar genişliğine
  // (300'e) yuvarlar --- (Örneğin sen 1000 yazsan bile kod onu otomatik 900'e
  // çeker, böylece duvarlar asla çatlamaz)
  float GercekDuvarGenisligi = 300.0f;
  float AksamayanGenislik =
      FMath::GridSnap(BuildingWidth, GercekDuvarGenisligi);
  float AksamayanUzunluk =
      FMath::GridSnap(BuildingLength, GercekDuvarGenisligi);

  // Sıfır olmasını engellemek için binaya en az 1 duvar boyutu (300) veriyoruz
  AksamayanGenislik = FMath::Max(GercekDuvarGenisligi, AksamayanGenislik);
  AksamayanUzunluk = FMath::Max(GercekDuvarGenisligi, AksamayanUzunluk);

  if (BuildingSpline) {
    // Önce eski çizgi noktalarını tamamen temizle (Yoksa üst üste binerler)
    BuildingSpline->ClearSplinePoints();

    // Genişlik ve Uzunluk ayarlarına göre 4 yeni köşe noktası ekle (Akıllı
    // hesaplanmış değerleri kullanıyoruz)
    BuildingSpline->AddSplinePoint(FVector(0, 0, 0),
                                   ESplineCoordinateSpace::Local);
    BuildingSpline->AddSplinePoint(FVector(AksamayanGenislik, 0, 0),
                                   ESplineCoordinateSpace::Local);
    BuildingSpline->AddSplinePoint(
        FVector(AksamayanGenislik, AksamayanUzunluk, 0),
        ESplineCoordinateSpace::Local);
    BuildingSpline->AddSplinePoint(FVector(0, AksamayanUzunluk, 0),
                                   ESplineCoordinateSpace::Local);

    // Çizgilerin kavisli (yuvarlak) olmasını engelle, köşeleri tam 90 derece
    // dik yap
    for (int32 i = 0; i < BuildingSpline->GetNumberOfSplinePoints(); i++) {
      BuildingSpline->SetSplinePointType(i, ESplinePointType::Linear);
    }
  }
}