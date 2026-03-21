# Item Asset Naming Template

Bu dokuman, `PostApocItems` DataTable satirlari icin `ItemIcon` ve `PickupMesh` asset'lerinin tek tip isimlendirilmesi icin hazirlandi.

## Ana Kural
- DataTable `RowName` ana kimliktir.
- Asset isimleri `RowName` baz alinarak uretilir.
- Turkce karakter, bosluk ve rastgele suffix kullanilmaz.
- Tekil standard disina cikilmaz.

## RowName Formati
- Kategori prefix'i korunur:
  - `Drink_`
  - `Med_`
  - `Ammo_`
  - `Wpn_`
  - `Gear_`
  - `Tool_`
  - `Scrap_`
  - `Val_`
  - `Quest_`

## ItemIcon Standardi
- Asset tipi: `Texture2D`
- Asset ismi:
  - `T_Item_<RowName>_Icon`
- Ornek:
  - `T_Item_Drink_FilteredWater_Icon`
  - `T_Item_Med_FirstAidKit_Icon`
  - `T_Item_Ammo_556Round_Icon`

## PickupMesh Standardi
- Asset tipi: `StaticMesh`
- Asset ismi:
  - `SM_Item_<RowName>_Pickup`
- Ornek:
  - `SM_Item_Drink_FilteredWater_Pickup`
  - `SM_Item_Med_FirstAidKit_Pickup`
  - `SM_Item_Scrap_BrokenRadio_Pickup`

## Onerilen Klasor Yapisi
- Icon import klasoru:
  - `Content/Gercek/Items/Icons/<Category>`
- Pickup mesh klasoru:
  - `Content/Gercek/Items/Meshes/<Category>`

Ornek tam yollar:
- `/Game/Gercek/Items/Icons/Drink/T_Item_Drink_FilteredWater_Icon`
- `/Game/Gercek/Items/Icons/Medical/T_Item_Med_FirstAidKit_Icon`
- `/Game/Gercek/Items/Meshes/Drink/SM_Item_Drink_FilteredWater_Pickup`
- `/Game/Gercek/Items/Meshes/Junk/SM_Item_Scrap_BrokenRadio_Pickup`

## Kategori Klasor Adlari
- `Drink`
- `Medical`
- `Ammo`
- `Weapon`
- `Gear`
- `Tool`
- `Junk`
- `Valuable`
- `Quest`

## DataTable Alanina Ne Yazilacak
- `ItemIcon` alanina icon asset path'i yazilir.
- `PickupMesh` alanina static mesh asset path'i yazilir.

Ornek:
- `ItemIcon`:
  - `/Game/Gercek/Items/Icons/Drink/T_Item_Drink_FilteredWater_Icon.T_Item_Drink_FilteredWater_Icon`
- `PickupMesh`:
  - `/Game/Gercek/Items/Meshes/Drink/SM_Item_Drink_FilteredWater_Pickup.SM_Item_Drink_FilteredWater_Pickup`

## Uretim Kurallari
- Icon:
  - `256x256`
  - seffaf arka plan
  - tek obje merkezde
  - ayni kategori icinde benzer kadraj
- Pickup mesh:
  - oyun dunyasinda okunabilir olcek
  - gereksiz collision karmasasi olmadan temiz mesh
  - eldeki cevre paketinden alinacaksa isim yine bu standarda gore yeniden duzenlenir

## Reimport / Varyasyon Kurali
- Farkli versiyon gerekiyorsa asla ana isim bozulmaz.
- Gecici varyasyonlar:
  - `T_Item_<RowName>_Icon_WIP`
  - `SM_Item_<RowName>_Pickup_WIP`
- Final asset geldigi anda WIP kullanimi temizlenir.

## Meryem Icin Hizli Uygulama Akisi
1. DataTable `RowName` kopyalanir.
2. Icon asset'i `T_Item_<RowName>_Icon` olarak import edilir.
3. Pickup mesh `SM_Item_<RowName>_Pickup` olarak ayarlanir veya duplicate edilir.
4. Asset ilgili kategori klasorune tasinir.
5. `PostApocItems` satirindaki `ItemIcon` ve `PickupMesh` alanlari bu asset'lere baglanir.
6. Validation script tekrar calistirilir.

## Ornek Eslesme Tablosu
- `Drink_FilteredWater`
  - Icon: `T_Item_Drink_FilteredWater_Icon`
  - Mesh: `SM_Item_Drink_FilteredWater_Pickup`
- `Med_FirstAidKit`
  - Icon: `T_Item_Med_FirstAidKit_Icon`
  - Mesh: `SM_Item_Med_FirstAidKit_Pickup`
- `Ammo_9mmRound`
  - Icon: `T_Item_Ammo_9mmRound_Icon`
  - Mesh: `SM_Item_Ammo_9mmRound_Pickup`
- `Wpn_RattlerSMG`
  - Icon: `T_Item_Wpn_RattlerSMG_Icon`
  - Mesh: `SM_Item_Wpn_RattlerSMG_Pickup`

## Yasaklar
- `icon_final2`, `mesh_new`, `pickup_last`, `copy`, `temp` gibi isimler kullanilmaz.
- `RowName` ile birebir eslesmeyen asset adi kullanilmaz.
- HUD ikonlari item icon olarak tekrar kullanilmaz.
