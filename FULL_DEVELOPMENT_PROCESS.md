# Tam Geliştirme Süreci

## Amaç
Bu doküman, projenin mevcut durumundan stabil bir Erken Erişim sürümüne nasıl taşınacağını tanımlar.

Hedef, her fikri aynı anda yapmak değil; küçük, güvenilir, genişletilebilir bir oyunu doğru sırayla geliştirmektir.

## Oyunun Kimliği
- Kıyamet sonrası co-op hayatta kalma
- Tehlikeli dünya, düşük güven, kıt kaynaklar
- Loot, envanter, ticaret ve hayatta kalma baskısı
- Görevlerle açılan yüksek değerli alanlar
- Güvenlik, depolama ve uyku için küçük oyuncu üsleri
- Kilit bölgelerde düşman NPC tehditleri
- Geç oyunda dünyayı iyileştirmeye başlayan CORE AI ilerlemesi

## Ana Kural
Sistemleri katman katman bitirin.

Yapılmaması gerekenler:
- aynı anda çok fazla büyük sistem açmak
- bitmemiş sistemi parlatmaya çalışmak
- dengesiz temel üstüne ağır içerik inşa etmek

Her zaman yapılması gereken:
- bir katmanı stabil hale getirmek
- tek başına test etmek
- bağlı sistemlerle test etmek
- sonra sıradaki katmana geçmek

## Mevcut Temel
Projede şu büyük omurgalar halihazırda bulunuyor:
- `AGercekCharacter`
- Survival statları ve HUD olay akışı
- Grid envanter ve item instance kimlikleri
- Loot container ve pickup sistemleri
- Trade, kondisyon, değer ve TradeKnowledge
- Host-authoritative save/load ve continue
- Steam co-op session akışı
- Başlangıç seviyesinde world-state persistence
- Yeni dünya/harita üretim hattı

## Geliştirme Katmanları

### Katman 1: Çekirdek Oynanış Stabilitesi
Bu vazgeçilmez temel katmandır.

Stabil olması gerekenler:
- hareket
- HUD
- survival statları
- item use/drop
- envanter
- loot container
- trade
- save/load
- continue
- host + 1 client

Çıkış kriteri:
- küçük ama güvenilir oynanış döngüsü

### Katman 2: Dünya Temeli
Gerçek oynanabilir dünya, çekirdek stabil olduktan sonra kurulur.

Olması gerekenler:
- çalışan landscape material zinciri
- district blockout
- trader hub
- suburb loot alanı
- industrial loot alanı
- city center risk alanı
- bir lab/bunker/özel alan

Çıkış kriteri:
- dünya artık test alanı gibi değil

### Katman 3: Görev ve Erişim İlerlemesi
Oyuncu yalnızca rastgele loot yapmamalı.

Olması gerekenler:
- görev veri modeli
- görev veren NPC akışı
- kilitli kapılar / bunker / erişim kartları
- progression anahtarları ve özel loot kapıları

Çıkış kriteri:
- oyuncu yalnızca loot ile değil, amaçla ilerliyor

### Katman 4: Base Temeli
Oyuncunun güvenli dönüş döngüsüne ihtiyacı var.

Olması gerekenler:
- metal shack
- stone house
- storage
- bed
- co-op uyku hazırlık sistemi
- tüm aktif oyuncular hazırsa sabaha geçiş
- base storage persistence

Çıkış kriteri:
- oyuncular dönebiliyor, depolayabiliyor, dinlenebiliyor, hazırlanabiliyor

### Katman 5: Tehlikeli Dünya Tehdidi
Dünya oyuncuya karşılık vermeli.

Olması gerekenler:
- iki düşman NPC arketipi
- kontrollü encounter alanları
- city center threat zone
- oyuncunun da düşman hedef sayılması

Çıkış kriteri:
- dünya boş değil, tehlikeli hissettiriyor

### Katman 6: CORE AI İlerlemesi
Bu uzun vadeli anlam sistemidir.

Erken Erişim için küçük tutulmalı:
- üç collectible kategorisi
  - Seeds
  - Water Data
  - Cultural Relics
- ilk CORE AI teslim akışı
- ilk görünür dünya iyileşme işaretleri

Çıkış kriteri:
- oyuncu daha büyük bir amacı olduğunu anlıyor

## Teslim Stratejisi

### Önce Küçük Kur, Sonra Büyüt
Proje şu sırayla geliştirilmeli:
1. sistemleri stabil hale getir
2. oynanabilir dünyayı kur
3. görev ve ilerlemeyi bağla
4. küçük base döngüsünü ekle
5. sınırlı düşman NPC baskısı ekle
6. ilk CORE AI ilerlemesini ekle
7. polish ve optimizasyon yap

### Bitti Tanımı
Her özellik şu üç seviyeden birinde olmalı:
- Prototype
- Playable
- Early Access Ready

Sadece Early Access Ready seviyesindeki sistemler çıkış build’ine girer.

## Haftalık Çalışma Ritmi

### Haftalık Yapı
- 1 ana hedef
- 2-3 destek görevi
- 1 test bloğu

### Haftalık Sorular
Her haftanın sonunda şunları cevaplayın:
1. Gerçekte ne bitti?
2. Ne hâlâ kırık?
3. Gelecek haftanın tek ana hedefi ne?
4. Bilerek neyi sonraya bıraktık?
5. Şu an Erken Erişim hedefi için en büyük risk ne?

## Ekip Çalışma Biçimi

### Teknik Sorumluluk
Odak alanları:
- C++
- multiplayer authority
- save/load
- progression logic
- hostile NPC temeli
- test ve entegrasyon

### İçerik Sorumluluğu
Odak alanları:
- harita üretimi
- district yerleşimi
- UI görünümü
- DataTable görsel doldurma
- environment art
- base görselleri
- pickup/container/trader Blueprint üretimi

### Ortak Kararlar
- ton
- görev yapısı
- loot yoğunluğu
- city center tehlike seviyesi
- CORE AI progression eşikleri
- base kullanışlılığı

## Görev Yönetimi Kuralları
Aşağıdaki kolonları olan bir board kullanın:
- Backlog
- Next
- In Progress
- Needs Test
- Blocked
- Done

Her görev kartında şunlar olmalı:
- başlık
- kategori
- başarı kriteri
- test adımları
- sorumlu
- risk

## Kategoriler
- Core Gameplay
- Survival
- Inventory & Loot
- Trade & Economy
- Save/Load
- Co-op & Network
- World Building
- Base Building
- Quests
- NPC & Hostiles
- CORE AI
- Optimization
- Bugs

## Test Felsefesi
Her önemli özellik üç seviyeden geçmeli:

### 1. Tekil Test
Tek başına çalışıyor mu?

### 2. Bağlı Sistem Testi
Save/load, UI, progression ve envanter ile birlikte doğru çalışıyor mu?

### 3. Multiplayer Test
Host authority ve client tarafında doğru davranıyor mu?

## Scope Kontrolü
Yeni fikir geldiğinde şu soruları sorun:
1. Bu 1.0 için gerekli mi?
2. Gerekliyse hangi katmana ait?
3. Gerekli değilse hangi sonraki sürüme ait?

Bu kural projenin bitirilebilir kalmasını sağlar.

## Son Yönlendirme
Tam hayali hemen çıkarmaya çalışmayın.

Şunu çıkarın:
- güvenilir bir döngü
- net bir dünya
- anlamlı progression
- küçük ama işlevsel base sistemi
- kontrollü düşman baskısı
- ilk CORE AI kancası

Sonra büyütün.
