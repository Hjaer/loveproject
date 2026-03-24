# PostABundle RVT Simplified Setup

Bu dokumanin amaci, `PostABundle` icindeki RVT zincirini projeye gore sadeleştirmektir. Hedef:

- siyah landscape riskini azaltmak
- gereksiz demo bagimliliklarini tasimamak
- sadece `Landscape + ground blend` icin gereken minimum setup'i kullanmak

## Karar

Bu proje icin onerilen sade kurulum:

1. Tek `Landscape Material Instance`
2. Tek ana `RVT Volume`
3. Yalnizca gerekli RVT asset'leri
4. RVT'yi tum dunyanin merkez mekanigi degil, sadece zemin blend yardimcisi olarak kullanmak

## Paket Icindeki RVT Asset'leri

- [RVT_Landscape.uasset](C:\GercekProje\Gercek\Content\PostABundle\Textures\Effects\RVT_Landscape.uasset)
- [RVT_Landscape_Height.uasset](C:\GercekProje\Gercek\Content\PostABundle\Textures\Effects\RVT_Landscape_Height.uasset)
- [RVT_Landscape_SVT.uasset](C:\GercekProje\Gercek\Content\PostABundle\Textures\Effects\RVT_Landscape_SVT.uasset)

## Sadelestirme Karari

Bu proje icin ilk asamada:

- `RVT_Landscape` kullan
- `RVT_Landscape_Height` kullan
- `RVT_Landscape_SVT` kullanma

Gerekce:

- renk/albedo/materyal blend icin ana RVT yeterli
- yukseklik bazli blend gerekiyorsa `Height` yararlidir
- `SVT` ek karmasiklik getirir, ilk stabil kurulum icin gereksizdir

## Onerilen Minimum Mimari

### 1. Landscape Material

Kullan:

- [MI_Landscape.uasset](C:\GercekProje\Gercek\Content\PostABundle\Materials\Nature\Landscape\MI_Landscape.uasset)

Kontrol et:

- landscape actor'da bu material instance atanmis olmali
- streaming proxy varsa onlarda da uyumlu olmali

### 2. RVT Volume

Haritada yalnizca `1` ana `Runtime Virtual Texture Volume` kullan.

Kurallar:

- playable area'yi kaplasin
- gereksizce tum dev haritayi kaplamasin
- once merkez gameplay alani kapsansin

### 3. RVT Asset Baglantisi

Volume veya ilgili alanlarda:

- ana renk/zemin icin [RVT_Landscape.uasset](C:\GercekProje\Gercek\Content\PostABundle\Textures\Effects\RVT_Landscape.uasset)
- gerekiyorsa yukseklik icin [RVT_Landscape_Height.uasset](C:\GercekProje\Gercek\Content\PostABundle\Textures\Effects\RVT_Landscape_Height.uasset)

`RVT_Landscape_SVT` ilk stabil asamada disarida kalmali.

### 4. Mesh Blend Kapsami

RVT blend'i ilk asamada sadece su tip mesh'lerde kullan:

- yol kenari gecisleri
- kaya tabani
- buyuk bina tabani
- hero prop zemin oturumu

Tum mesh library'ye RVT baglamak gerekmez.

## Siyah Landscape Icin Pratik Yaklasim

`Fill Layer` ise yaramadiysa, asagidaki sade test akisini uygula:

1. Mevcut landscape'te [MI_Landscape.uasset](C:\GercekProje\Gercek\Content\PostABundle\Materials\Nature\Landscape\MI_Landscape.uasset) kullan.
2. PostABundle `LayerInfo` asset'lerini bagla.
3. Tek bir `RVT Volume` ekle.
4. Sadece `RVT_Landscape` ve gerekiyorsa `RVT_Landscape_Height` kullan.
5. `SVT` zincirini devreye alma.
6. Tek bir temel layer fill yap.

Eger hala siyahsa:

1. `MI_Landscape` icindeki RVT ile ilgili static switch/override'lari kontrol et.
2. Gecici bir test material instance kopyasi olustur.
3. RVT tabanli ozellikleri kapatip tekrar dene.

Bu testin amaci:

- sorun RVT baglantisinda mi
- yoksa layer/material instance kurulumunda mi
ayirmaktir.

## Ornek Kurulum Akisi

1. `Istanbul` map icinde test icin kucuk bir gameplay bolgesi sec.
2. O bolgeyi kapsayan tek bir `RVT Volume` yerlestir.
3. Landscape ve kritik blend mesh'leri sadece bu bolgede dogrula.
4. Sistem stabil olunca kapsami genislet.

Bu yaklasim tum haritayi bir anda baglamaktan daha guvenlidir.

## Bu Projede Neleri Bilerek Kullanmiyoruz

Ilk stabil asamada sunlari tasimiyoruz:

- tum ExampleWorld setup'ini
- gereksiz demo actor zincirlerini
- `RVT_Landscape_SVT`
- tum mesh library icin yaygin RVT baglantisi

## En Guvenli Editor Sirasi

1. [MI_Landscape.uasset](C:\GercekProje\Gercek\Content\PostABundle\Materials\Nature\Landscape\MI_Landscape.uasset) ac
2. Material instance parameter gruplarini kontrol et
3. Landscape actor ve proxy material atamalarini dogrula
4. Tek `RVT Volume` kur
5. `RVT_Landscape` ve gerekiyorsa `RVT_Landscape_Height` bagla
6. PostABundle layer info'lariyla test et
7. Siyah kalirsa RVT devre disi test material instance'i ile izolasyon yap

## Nihai Oneri

Bu proje icin en dogru hedef:

- `RVT`yi gorsel destekleyici katman olarak kullanmak
- ana oynanis ve cevre omurgasini RVT'ye bagimli hale getirmemek

Yani RVT:

- gerekli oldugu yerde
- sinirli kapsamada
- kolay test edilebilir sekilde
kullanilmali.
