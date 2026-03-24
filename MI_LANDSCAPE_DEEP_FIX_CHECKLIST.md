# MI_Landscape Deep Fix Checklist

`Fill Layer` denendiginde sorun cozulmediyse, problem artik sadece `Layer Info` eksigi olarak gorulmemelidir. Bu noktada en guclu adaylar:

1. `RVT` kurulumu eksik
2. Material yanlis landscape actor veya proxy uzerinde
3. `MI_Landscape` icindeki static switch / parameter durumu bu harita icin yanlis
4. Paketin ornek haritasindaki destekleyici actor/volume kurulumu sizin haritada yok

Kontrol edilecek ana assetler:

- [MI_Landscape.uasset](C:\GercekProje\Gercek\Content\PostABundle\Materials\Nature\Landscape\MI_Landscape.uasset)
- [M_Landscape.uasset](C:\GercekProje\Gercek\Content\PostABundle\Materials\MasterMaterials\M_Landscape.uasset)
- [Grass_LayerInfo.uasset](C:\GercekProje\Gercek\Content\PostABundle\Materials\Nature\Landscape\Grass_LayerInfo.uasset)
- [Rock_LayerInfo.uasset](C:\GercekProje\Gercek\Content\PostABundle\Materials\Nature\Landscape\Rock_LayerInfo.uasset)
- [Sand_LayerInfo.uasset](C:\GercekProje\Gercek\Content\PostABundle\Materials\Nature\Landscape\Sand_LayerInfo.uasset)
- [SmoothSand_LayerInfo.uasset](C:\GercekProje\Gercek\Content\PostABundle\Materials\Nature\Landscape\SmoothSand_LayerInfo.uasset)
- [Road_LayerInfo.uasset](C:\GercekProje\Gercek\Content\PostABundle\Materials\Nature\Landscape\Road_LayerInfo.uasset)
- [RVT_Landscape.uasset](C:\GercekProje\Gercek\Content\PostABundle\Textures\Effects\RVT_Landscape.uasset)
- [RVT_Landscape_Height.uasset](C:\GercekProje\Gercek\Content\PostABundle\Textures\Effects\RVT_Landscape_Height.uasset)
- [RVT_Landscape_SVT.uasset](C:\GercekProje\Gercek\Content\PostABundle\Textures\Effects\RVT_Landscape_SVT.uasset)

## Asama 1: Dogru Landscape Actor Uzerinde Misiniz?

1. World Outliner'da gercek landscape actor'u secin.
2. World Partition kullaniyorsaniz landscape streaming proxy'leri de kontrol edin.
3. `Landscape Material` alaninda [MI_Landscape.uasset](C:\GercekProje\Gercek\Content\PostABundle\Materials\Nature\Landscape\MI_Landscape.uasset) gercekten atanmis olmali.
4. Sadece root landscape actor'a degil, gerekiyorsa proxy'lere de ayni material uygulanmali.

Beklenen sonuc:

- layer listesi material ile uyumlu gorunmeli
- proxy'lerde farkli veya bos material kalmamali

## Asama 2: Layer Info Dogrulamasi

1. `Landscape Mode > Paint` acin.
2. Su layer'larin her birinin ilgili PostABundle `LayerInfo` asset'ine baglandigini dogrulayin:
   - Grass
   - Rock
   - Sand
   - SmoothSand
   - Road
3. Daha once olusturulmus custom `LayerInfo` dosyalari varsa, gecici olarak PostABundle'in kendi `LayerInfo` dosyalariyla test edin.
4. Sonra tek bir temel layer'da tekrar `Fill Layer` yapin.

Not:

`Fill Layer` cozum olmadiysa bile bu adim atlanmamalidir; cunku sonraki RVT testinin temiz olmasi icin temel layer baglantisi garanti olmalidir.

## Asama 3: RVT Kurulumu

Bu paket materyal zincirinde `RuntimeVirtualTexture` izi tasiyor. `M_Landscape` icinde `RVT_Landscape` geciyor. Bu nedenle siyah landscape'in bir sonraki ana adayi RVT eksigidir.

Kontrol listesi:

1. Haritada bir veya daha fazla `Runtime Virtual Texture Volume` var mi?
2. Bu volume landscape'in tamamini kapsiyor mu?
3. Volume icinde kullanilan RVT asset'leri su paket asset'leriyle uyumlu mu:
   - [RVT_Landscape.uasset](C:\GercekProje\Gercek\Content\PostABundle\Textures\Effects\RVT_Landscape.uasset)
   - [RVT_Landscape_Height.uasset](C:\GercekProje\Gercek\Content\PostABundle\Textures\Effects\RVT_Landscape_Height.uasset)
   - [RVT_Landscape_SVT.uasset](C:\GercekProje\Gercek\Content\PostABundle\Textures\Effects\RVT_Landscape_SVT.uasset)
4. Landscape actor'un `Virtual Texture` / `Draw in Virtual Textures` alanlari bos mu dolu mu kontrol edin.
5. RVT volume yerlesik degilse, paketin ornek map'ine bakarak ayni kurulumu kendi map'inize kopyalayin.

Beklenen sonuc:

- landscape materyali RVT'ye yazabiliyor olmali
- RVT volume sahayi kapsiyor olmali

## Asama 4: Material Instance Parameter Kontrolu

1. [MI_Landscape.uasset](C:\GercekProje\Gercek\Content\PostABundle\Materials\Nature\Landscape\MI_Landscape.uasset) acin.
2. Tum parametre gruplarini tek tek inceleyin.
3. Ozellikle `Static Switch`, `Virtual Texture`, `Layer`, `Normal`, `Height`, `Blend` gruplarinda override olan degerlere bakin.
4. Ornek map'teki ayni instance veya ayni parent setup ile karsilastirin.
5. `Apply` ve `Save` alin.

Amaç:

- instance bu projede yanlis bir switch durumunda mi anlamak
- parent material dogru ama instance override bozuk mu izole etmek

## Asama 5: Parent Material ile Izolasyon Testi

Bu test siyahligin kaynagini cok hizli ayirir.

1. Gecici bir landscape uzerinde veya mevcut landscape'i kopyalayarak test edin.
2. [M_Landscape.uasset](C:\GercekProje\Gercek\Content\PostABundle\Materials\MasterMaterials\M_Landscape.uasset) dogrudan ataninca ne olduguna bakin.
3. Ayrica [MI_Landscape.uasset](C:\GercekProje\Gercek\Content\PostABundle\Materials\Nature\Landscape\MI_Landscape.uasset) ile karsilastirin.

Yorum:

- parent da siyahsa sorun instance degil, setup kaynaklidir
- parent calisiyor instance siyahsa sorun instance override kaynaklidir

## Asama 6: ExampleWorld Karsilastirmasi

Paketin kendi referans haritalari:

- [ExampleWorld_01.umap](C:\GercekProje\Gercek\Content\PostABundle\Maps\ExampleWorld_01.umap)
- [Showcase.umap](C:\GercekProje\Gercek\Content\PostABundle\Maps\Showcase.umap)

Bu map'ler production temel olarak kullanilmamali, ama landscape setup karsilastirmasi icin cok degerli.

Bakilacaklar:

1. Landscape actor detaylari
2. RVT volume varligi
3. Landscape material ve proxy ayarlari
4. Layer info baglantilari
5. Material instance override'lari

## Asama 7: En Guvenli Kurtarma Senaryosu

Eger mevcut haritada her sey dogru gorunuyor ama hala siyahsa, en guvenli yeniden kurulum akisi su:

1. Yeni bos test map olustur.
2. Yeni kucuk bir landscape create et.
3. Sadece [MI_Landscape.uasset](C:\GercekProje\Gercek\Content\PostABundle\Materials\Nature\Landscape\MI_Landscape.uasset) ata.
4. Paketin kendi `LayerInfo` asset'lerini bagla.
5. Gerekli RVT volume'leri ekle.
6. Tek bir layer fill yap.

Sonuc yorumu:

- test map'te calisiyorsa sorun ana map setup'indadir
- test map'te de siyahsa sorun paket/material/RVT zincirindedir

## Hazirlik Notu

Su anki belirtiye gore, bir sonraki en guclu odak `RVT + proxy/material assignment + instance parameter state` ucgeni olmalidir. `Fill Layer` denemesi cozum vermedigi icin artik sadece paint paneline odaklanmak dogru olmaz.
