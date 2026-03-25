# Erken Erişim Kapsamı

## Çıkış Hedefi
Erken Erişim sürümü küçük ama güven veren bir oyun olmalı.

Oyuncu şunu hissetmeli:
- oyunun net bir kimliği var
- çekirdek döngü çalışıyor
- ilerleme anlamlı
- co-op güvenilir
- dünya tehlikeli ve amaçlı

## 1.0 İçinde Olacak Sistemler

### Çekirdek Oynanış
- Hareket
- Health, stamina, hunger, thirst, radiation
- HUD

### Envanter ve Loot
- Grid envanter
- Pickup
- Drop
- Item use
- Loot container
- Item condition
- Item value

### Trade
- Stabil trader akışı
- Trade UI
- Kondisyon/değer odaklı takas
- TradeKnowledge etkisi

### Kayıt ve İlerleme
- Host-authoritative save/load
- Autosave
- Continue
- Player restore
- Inventory restore
- Base storage restore

### Co-op
- Stabil host + 1 client
- Join
- Reconnect
- Trade sync
- Loot sync
- Sleep sync

### Base
- Metal shack
- Stone house
- Storage
- Bed
- Co-op sleep vote
- Tüm aktif oyuncular hazırsa sabaha geçiş

### Dünya
- Tek ana oynanabilir harita
- Trader hub
- Suburb loot alanı
- Industrial loot alanı
- City center risk alanı
- Bir lab veya bunker

### Görevler
- 3-5 anlamlı görev
- Kilitli alan progression’ı
- Access card / key / bunker akışı

### Düşman Tehdidi
- İki düşman NPC arketipi
- Kontrollü encounter noktaları
- City center tehdit baskısı

### CORE AI
- İlk progression sürümü
- Üç collectible kategorisi:
  - Seeds
  - Water Data
  - Cultural Relics
- İlk görünür restoration işaretleri

## Erken Erişim İçin Şart Olmayanlar

### Base Genişleme Sistemleri
- Tam serbest inşa
- gelişmiş duvar duvar yapı sistemi
- büyük modüler snap sistemi

### NPC Simülasyon Genişlemesi
- büyük ölçekli kalıcı faction savaşı
- derin reputation sistemleri
- geniş sosyal ilişki sistemleri

### Görev Genişlemesi
- çok büyük dallanan görev ağları
- ağır diyalog ağaçları
- quest reputation etrafında ek yan sistemler

### CORE AI Genişlemesi
- tam world restoration simülasyonu
- iklim/tarım/ekoloji tarafında tam dönüşüm

### İçerik Genişlemesi
- devasa harita ölçeği
- çok sayıda NPC fraksiyonu
- gelişmiş araç sistemi
- büyük craft ağaçları
- tarım simülasyonu

Bunlar geçersiz fikirler değil; sadece 1.0 için zorunlu değiller.

## Erken Erişim Hazırlık Şartları

### Güvenilir Olmalı
- save/load
- continue
- inventory
- trade
- loot containers
- base storage
- co-op sync

### Anlaşılır Olmalı
- oyuncu ne yapıyor
- sırada nereye gidiyor
- görevler neden önemli
- base neden önemli
- CORE AI neden önemli

### Tekrar Oynanabilir Olmalı
- loot run’ları
- trade hazırlığı
- base’e dönüş döngüsü
- tehlikeli alan koşuları
- özel item toplama

## Başarı Tanımı
Erken Erişim build’i şu durumda hazır sayılır:
- oyuncular yaklaşık 10 saat oynayabiliyor
- oyun boş hissettirmiyor
- oyun sahte şekilde fazla büyük görünmüyor
- sistemler oyuncuya güven veriyor

## Scope Kuralı
Bir özellik aşağıdaki döngüyü güçlendirmiyorsa sonraya bırakılmalı:

Loot -> Hayatta Kal -> Trade Yap -> İlerle -> Base'e Dön -> Uyu -> Tehlikeli Dünyaya Yeniden Çık -> CORE AI'ı İlerle
