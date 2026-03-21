# PostApocItems Doldurma Standardi

Bu dosya `FPostApocItemRow` icin icerik giris standardidir.

## Zorunlu Alanlar
- `ItemID`: Benzersiz, sabit, save/load guvenli kimlik. Sonradan degistirilmemeli.
- `DisplayName`: Oyuncuya gorunen lokalize ad.
- `Category`: Mekanik sinif. Economy, consume, loot ve trader akisini etkiler.
- `MaxStack`: Stack kurali. `bCanStack=true` ise `>1` olmali.
- `Weight`: Tasima ve hareket cezasi icin kullanilir.
- `BaseValue`: Economy ve trade icin baz deger.
- `bCanConsume`: Item kullanilabilir mi?
- `bCanEquip`: Kusanilabilir mi?
- `bCanStack`: Stack mantigi acik mi?
- `bQuestItem`: Gorev item'i mi?
- `ItemSize`: Grid envanterde kapladigi alan.
- `bCanBeRotated`: Gridde dondurulebilir mi?

## Uretim Kalitesi Icin Fiilen Zorunlu Alanlar
- `Description`: Bos birakilmamali. Tooltip, trader ve inspect panelleri icin gerekir.
- `PickupMesh`: Dunyada fiziksel temsil gerekiyorsa doldurulmali. Mevcut projede pickup/dropped item zinciri bunu bekliyor.
- `ItemIcon`: Inventory, trade ve loot UI icin doldurulmali.
- `ItemRarity`: Loot ve ekonomi katmani icin standardize edilmeli.

## Kosullu Alanlar
- `ConsumeEffectType`: `bCanConsume=true` ise zorunlu.
- `ConsumeAmount`: `bCanConsume=true` ise `>0` olmali.

## Alan Kurallari
- `Category=Drink` veya `Category=Food` ise `bCanConsume=true` olmali.
- `bCanConsume=true` olan itemlarda `ConsumeEffectType=None` kabul edilmemeli.
- `bCanConsume=true` olan itemlarda `ConsumeAmount=0` kabul edilmemeli.
- `bCanStack=true` iken `MaxStack=1` olmamali.
- `bQuestItem=true` olan itemlarda `BaseValue=0` tercih edilmeli ve tradera dusmemeli.
- `PickupMesh=None` sadece dunyada asla temsil edilmeyecek saf veri itemlarinda kabul edilmeli. Bu projede pratikte cogu item icin mesh gerekir.

## Onerilen Rarity Sozlugu
- `Common`
- `Uncommon`
- `Rare`
- `Epic`
- `Legendary`

## Onerilen Isimlendirme
- Hurda: `Scrap_*`
- Tibbi: `Med_*`
- Icecek: `Drink_*`
- Yiyecek: `Food_*`
- Cephane: `Ammo_*`
- Silah: `Wpn_*`
- Alet: `Tool_*`
- Ekipman: `Gear_*`
- Degerli: `Val_*`
- Gorev: `Quest_*`
