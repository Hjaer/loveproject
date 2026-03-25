# Test Modları Rehberi

## Amaç
Bu doküman, projedeki hangi sistemin hangi test modunda doğrulanması gerektiğini açıklar.

Ana hedef:
- yanlış test modunda zaman kaybetmemek
- güvenilir sonuç almak
- tek oyuncu ve co-op testlerini doğru ayırmak

## Temel Kural
Bu proje için ana test modları:
- `PIE (Play In Editor)`
- `2 Oyunculu PIE`
- `Standalone`

`Simulate` yardımcı moddur, ana doğrulama modu değildir.

---

## 1. Simulate

### Ne İçin Uygun?
- landscape görünümü
- RVT / paint görsel tepkisi
- prop yerleşimi
- çevre görsel kontrolü
- kaba dünya davranışları

### Ne İçin Uygun Değil?
- HUD
- input doğrulama
- inventory
- loot
- trade
- save/load
- continue
- sleep sistemi
- co-op testleri
- reconnect
- gerçek oyuncu akışı

### Kural
`Simulate`, dünya ve görsel kurulumları hızlı görmek için kullanılır.
Oynanışın ana doğrulaması burada yapılmaz.

---

## 2. PIE (Tek Oyuncu)

### Ne İçin Uygun?
- hareket
- survival statları
- HUD
- inventory
- pickup
- loot container
- trade
- base storage
- bed interaction
- görev progression
- hostile NPC ile oyuncu etkileşimi

### Kullanım
Önerilen mod:
- `Selected Viewport`
veya
- `New Editor Window (PIE)`

### Kural
Çekirdek oynanış doğrulaması için ana mod budur.

---

## 3. 2 Oyunculu PIE

### Ne İçin Uygun?
- host HUD doğru mu
- client HUD doğru mu
- pickup sync
- loot container sync
- trade sync
- sleep vote
- reconnect öncesi/sonrası davranış
- hostile NPC’lerin host/client üzerindeki görünümü

### Kullanım
Önerilen ayar:
- `Number of Players = 2`
- mümkünse ayrı editor pencereleri

### Kural
1.0 kalite hedefi için en kritik mod budur.

Eğer bir sistem co-op’ta kullanılacaksa, bu modda test edilmeden bitmiş sayılmaz.

---

## 4. Standalone

### Ne İçin Uygun?
- menu akışı
- save/load
- continue
- başlangıç ve map load hissi
- input / UI odak davranışları
- daha gerçek kullanıcı deneyimi

### Özellikle Standalone'da Test Edilmesi Gerekenler
- `Main Menu`
- `Create Server`
- `Continue`
- `HUD açılışı`
- `base sleep`
- `save/load`

### Kural
Editor etkisini azaltmak ve daha gerçek oyun hissi görmek için bu mod gerekir.

---

## Sistem Bazlı Test Modu Önerisi

### Landscape / RVT / Paint
- `Simulate`
- `Tek Oyuncu PIE`

### HUD
- `Tek Oyuncu PIE`
- `2 Oyunculu PIE`

### Inventory / Loot / Trade
- `Tek Oyuncu PIE`
- `2 Oyunculu PIE`

### Save / Load / Continue
- `Standalone`

### Sleep Sistemi
- `Tek Oyuncu PIE`
- `2 Oyunculu PIE`
- gerekirse `Standalone`

### Hostile NPC Davranışı
- `Tek Oyuncu PIE`
- `2 Oyunculu PIE`

### Menu ve Session
- `Standalone`
- gerekirse `2 Oyunculu PIE`

---

## Önerilen Test Sırası

### Aşama 1
`Tek Oyuncu PIE`
- HUD
- survival
- inventory
- trade
- loot

### Aşama 2
`2 Oyunculu PIE`
- host/client HUD
- pickup
- trade
- sleep
- sync davranışları

### Aşama 3
`Standalone`
- menu
- continue
- save/load
- giriş ve map geçiş akışları

### Aşama 4
`Simulate`
- sadece world / art / landscape doğrulaması

---

## Kısa Özet

### Simulate
Yardımcı dünya testi

### Tek Oyuncu PIE
Ana gameplay testi

### 2 Oyunculu PIE
Ana co-op testi

### Standalone
Gerçek kullanıcı akışı testi

---

## Son Kural
Bu projede şu sistemler yalnızca `çalışıyor gibi` göründüğü için tamam sayılmaz:
- HUD
- inventory
- trade
- save/load
- continue
- sleep
- co-op senaryoları

Bunlar doğru modda test edilmeden kapatılmamalıdır.
