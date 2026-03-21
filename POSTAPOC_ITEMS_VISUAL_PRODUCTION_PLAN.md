# PostApocItems Visual Production Plan

## Durum
- Hazir item icon kutuphanesi yok.
- Mevcut projede item icon tarafinda yalnizca tek kullanilan referans gorunuyor: `Ammo_9mmRound`.
- HUD iconlari var, fakat bunlar item icon pipeline'i degil.
- Tum kategorilerde `PickupMesh` eksigi var.

## Faz 0: Pipeline Karari
- Tek isimlendirme standardi sec: `T_Item_<RowName>_Icon`
- Icon boyutu standardi: `256x256`
- Arka plan standardi: seffaf PNG, merkez kadraj, tek obje
- Ithal klasor standardi: `Content/Gercek/Items/Icons`
- Mesh referans standardi: dunya pickup icin `SM_` veya uygun static mesh asset

## Faz 1: Gameplay Kritik Kategoriler
### Drink
- Satir sayisi: 5
- Oncelik: En yuksek
- Gerekce: Survival dongusu ve consume UI dogrudan bunlara bagli
- Uretilecekler:
  - Tum rowlar icin icon
  - Tum rowlar icin pickup mesh
  - Tum rowlar icin description
  - Tum rowlar icin rarity

### Medical
- Satir sayisi: 6
- Oncelik: En yuksek
- Gerekce: Heal/AntiRad davranisi aktif; okunabilirlik kritik
- Uretilecekler:
  - Tum rowlar icin icon
  - Tum rowlar icin pickup mesh
  - Tum rowlar icin description
  - Tum rowlar icin rarity
  - `Med_FilterMask` icin equip okunurlugu ayrica gozden gecirilmeli

## Faz 2: Combat ve Equipment
### Weapon
- Satir sayisi: 8
- Oncelik: Yuksek
- Gerekce: Silah secimi, loot taninabilirligi ve trader ekraninda onemli

### Ammo
- Satir sayisi: 7
- Oncelik: Yuksek
- Gerekce: Stack okunurlugu ve cephane ayrimi kritik
- Not: `Ammo_9mmRound` disinda icon altyapisi yok denecek kadar az

### Gear
- Satir sayisi: 11
- Oncelik: Yuksek
- Gerekce: Equip edilebilir olduklari icin icon ve mesh eksigi oyuncu kararini zayiflatir

### Tool
- Satir sayisi: 5
- Oncelik: Orta-Yuksek
- Gerekce: Utility itemlar trader ve exploration tarafinda okunur olmali

## Faz 3: Economy ve World Flavor
### Valuable
- Satir sayisi: 7
- Oncelik: Orta
- Gerekce: Trader ve loot cazibesi icin ikon gerekli

### Junk
- Satir sayisi: 21
- Oncelik: Orta
- Gerekce: Sayi en fazla burada; batch uretim en verimli bu kategoride olur
- Oneri: Benzer hurdalari tek cekim setleriyle seri icon pipeline'ina sok

## Faz 4: Quest Ayrimi
### Quest Rows
- Satirlar:
  - `Quest_BunkerKey`
  - `Quest_AccessCard`
  - `Quest_EncryptedRadio`
  - `Quest_MysteriousLetter`
  - `Quest_FamilyPhoto`
- Oncelik: Orta
- Gerekce: Oyuncu bu itemlari diger junk ile karistirmamali
- Oneri: Ayri rarity veya ayri gorunur renk kodu tanimla

## Uretim Stratejisi
1. Once `Drink` ve `Medical`
2. Sonra `Weapon`, `Ammo`, `Gear`
3. Sonra `Tool`, `Valuable`
4. En son `Junk` ve `Quest`

## Batch Is Akisi
1. Kategori sec
2. Mesh referanslarini eslestir
3. Icon render veya PNG uret
4. Description yaz
5. Rarity ata
6. Validation script ile tekrar kontrol et
