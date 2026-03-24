# PostABundle Istanbul District Plan

Bu planin amaci, `PostABundle` icerigini Istanbul haritasina rastgele serpmek yerine district tabanli, okunabilir ve performansli bir acik dunya dili kurmaktir.

## Ana Ilke

Harita tek bir "guzel sahne" gibi degil, birbirinden farkli 4 ana district gibi dusunulmelidir:

1. `Suburb / Residential`
2. `Commercial / City Spine`
3. `Industrial / Logistics`
4. `Nature Overgrowth / Collapse`

Bu ayrim paket icindeki mevcut siniflandirmayla da uyumludur:

- [City_DataLayer.uasset](C:\GercekProje\Gercek\Content\PostABundle\Maps\City_DataLayer.uasset)
- [Industrial_DataLayer1.uasset](C:\GercekProje\Gercek\Content\PostABundle\Maps\Industrial_DataLayer1.uasset)
- [Nature_DataLayer.uasset](C:\GercekProje\Gercek\Content\PostABundle\Maps\Nature_DataLayer.uasset)
- [Suburb_DataLayer.uasset](C:\GercekProje\Gercek\Content\PostABundle\Maps\Suburb_DataLayer.uasset)

## District 1: Suburb / Residential

Bu bolge oyuncuya "yasanmis hayat kalintisi" hissi vermelidir. Ana tema:

- dusuk katli konut
- dar sokaklar
- bahce ve duvar kalintilari
- ic mekana girebilen loot odakli alanlar
- terk edilmis aile hayati izleri

Kullanilacak paket omurgasi:

- [BP_Building_LH1.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Buildings\BP_Building_LH1.uasset)
- [BP_ConcreteHousing.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Buildings\BP_ConcreteHousing.uasset)
- [BP_SidingNarrow.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Buildings\BP_SidingNarrow.uasset)
- [BP_SidingWide.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Buildings\BP_SidingWide.uasset)
- [BP_Refrigerator.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Environment\Furniture\BP_Refrigerator.uasset)
- [BP_KitchenSink.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Environment\Furniture\BP_KitchenSink.uasset)
- [BP_WallIvy.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Environment\Scatters\Exterior\BP_WallIvy.uasset)

Oynanis amaci:

- erken oyun yiyecek ve su loot'u
- ilk sandik / container bolgeleri
- dusuk riskli kesif

## District 2: Commercial / City Spine

Bu bolge "yikik ama hala ayakta duran sehir omurgasi" gibi hissettirmelidir. Ana tema:

- genis cadde
- dukkanlar
- orta yogunlukta bina silueti
- uzak gorus hatlari
- trader veya hub cepleri

Kullanilacak paket omurgasi:

- [BP_Building_Shop.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Buildings\BP_Building_Shop.uasset)
- [BP_Building_Shop_Pentice.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Buildings\BP_Building_Shop_Pentice.uasset)
- [BP_Building_Diner.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Buildings\BP_Building_Diner.uasset)
- [BP_Building_AP1.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Buildings\BP_Building_AP1.uasset)
- [BP_Building_CityBase.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Buildings\BP_Building_CityBase.uasset)
- [BP_Building_Sidewalk.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Buildings\BP_Building_Sidewalk.uasset)
- [BP_ParkedCars.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Environment\Splines\Exterior\BP_ParkedCars.uasset)
- [BP_WirePoles.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Environment\Splines\Exterior\BP_WirePoles.uasset)

Oynanis amaci:

- trader bolgesi
- barter / sosyal co-op omurgasi
- daha gorunur ama daha riskli loot

## District 3: Industrial / Logistics

Bu bolge orta-gec oyun tehdit ve odul bolgesi olmalidir. Ana tema:

- depolar
- tanklar
- metal yapilar
- boru ve kolon yogunlugu
- acik ama tehlikeli traversal

Kullanilacak paket omurgasi:

- [BP_Industrial.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Buildings\BP_Industrial.uasset)
- [BP_ConcreteStruct.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Buildings\BP_ConcreteStruct.uasset)
- [BP_ConcreteBase.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Buildings\BP_ConcreteBase.uasset)
- [BP_P_PipeMaker.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Environment\Pipes\Parent\BP_P_PipeMaker.uasset)
- [BP_P_Pillar.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Environment\Pillars\Parent\BP_P_Pillar.uasset)
- [BP_MetalFence.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Environment\Splines\Exterior\BP_MetalFence.uasset)
- [BP_Quarantine_Tent.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Buildings\BP_Quarantine_Tent.uasset)

Oynanis amaci:

- daha yuksek degerli loot
- gorev ve anahtarli alanlar
- gorus hatti ve pusular icin daha sert mekanlar

## District 4: Nature Overgrowth / Collapse

Bu bolge insan kontrolunun cozuldugu, sehir ile doganin birbirine girdigi alan olmalidir. Ana tema:

- cokmus yollar
- kayalar
- sarmaşık ve yesil istila
- duzensiz traversal
- gizli loot / ambush hissi

Kullanilacak paket omurgasi:

- [BP_TrashField.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Environment\Scatters\Exterior\BP_TrashField.uasset)
- [BP_WallIvy.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Environment\Scatters\Exterior\BP_WallIvy.uasset)
- [BP_RoofIvy.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Environment\Scatters\Exterior\BP_RoofIvy.uasset)
- [BP_Highway1.uasset](C:\GercekProje\Gercek\Content\PostABundle\Blueprints\Environment\Splines\Exterior\BP_Highway1.uasset)
- `Geometry/MAAS/Nature` altindaki kaya ve arazi kirici assetler

Oynanis amaci:

- gizli zula ve quest item alanlari
- risk-odul dengesini arttiran dolasik geometri
- atmosferik gezi ve anlatim

## Istanbul Icin Onerilen Uretim Sirasi

1. Ana yol ve district sinirlarini kur.
2. Hero bina ve skyline siluetini yerlestir.
3. Trader/hub, loot, rota ve guvenli alanlari sabitle.
4. Spline ile yol, tel orgu, arac dizilimi ve direk omurgasini kur.
5. Scatter ile kirlenme, sarmaşık ve yikim dokusunu ekle.
6. Landscape ve yol gecislerini polish et.
7. Uzak alanlari `DistantBuildings` ve HLOD mantigiyla hafiflet.

## AAA Kalite Notlari

1. Her district ayni yogunlukta olmamali.
2. Her bina girilebilir olmamali.
3. Her sokakta bir "hero odak" olmasi yerine district bazli ritim kurulmalidir.
4. Yol kenarindaki detay yogunlugu merkezden cevreye degismelidir.
5. Trader ve quest alanlari mimari olarak uzaktan okunmalidir.
6. Industrial ve Commercial district birbiriyle karismamali; materyal dili farkli tutulmalidir.
