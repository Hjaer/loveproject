# Bug Listesi

## Kullanım Amacı
Bu dosya, projede tespit edilen aktif bugları ve riskleri takip etmek için kullanılır.

Her bug için şu format kullanılmalı:
- Başlık
- Öncelik
- Durum
- Tekrar Üretim Adımı
- Beklenen Sonuç
- Mevcut Sonuç
- Etkilenen Sistem
- Not

## Öncelik Tanımı
- `P1`: oyunu veya temel ilerlemeyi kırıyor
- `P2`: önemli ama geçici workaround var
- `P3`: polish veya düşük riskli sorun

## Durum Tanımı
- `Açık`
- `İnceleniyor`
- `Düzeltildi`
- `Test Bekliyor`
- `Ertelendi`

---

## Aktif Buglar

### 1. Landscape paint zinciri güvenilir değil
- Öncelik: `P1`
- Durum: `Açık`
- Tekrar Üretim:
  1. yeni open world haritayı aç
  2. `MI_Landscape` ata
  3. layer info’ları doğrula
  4. `Grass Fill Layer` uygula
  5. `Sand` veya `SmoothSand` boya
- Beklenen Sonuç:
  - yüzey katmanlara göre görünür değişmeli
- Mevcut Sonuç:
  - grass spawn olabiliyor ama yüzey boyama güvenilir tepki vermiyor
- Etkilenen Sistem:
  - World Building / Landscape / RVT
- Not:
  - ilk hafta ana blocker

### 2. Save / Load / Continue saha doğrulaması eksik
- Öncelik: `P1`
- Durum: `Açık`
- Tekrar Üretim:
  1. oyuna gir
  2. loot al
  3. save tetikle
  4. çık
  5. continue ile dön
- Beklenen Sonuç:
  - karakter, inventory ve statlar doğru geri gelmeli
- Mevcut Sonuç:
  - kod omurgası var, saha doğrulaması tamamlanmadı
- Etkilenen Sistem:
  - Save/Load / Progression
- Not:
  - erken erişim için kritik

### 3. Host + 1 client canlı senaryo doğrulaması eksik
- Öncelik: `P1`
- Durum: `Açık`
- Tekrar Üretim:
  1. host oyun açar
  2. client bağlanır
  3. pickup, trade, loot container ve reconnect test edilir
- Beklenen Sonuç:
  - bütün ana döngü senkron çalışmalı
- Mevcut Sonuç:
  - teknik omurga var, canlı saha doğrulaması eksik
- Etkilenen Sistem:
  - Co-op & Network
- Not:
  - kalite eşiği için şart

### 4. Inventory / loot / trade tam saha testi eksik
- Öncelik: `P1`
- Durum: `Açık`
- Tekrar Üretim:
  1. pickup
  2. envanter aç
  3. item kullan
  4. loot container’a taşı
  5. trader ile takas yap
- Beklenen Sonuç:
  - tüm akışlar kırılmadan çalışmalı
- Mevcut Sonuç:
  - sistemler büyük ölçüde hazır ama tam checklist doğrulaması eksik
- Etkilenen Sistem:
  - Inventory & Loot / Trade

### 5. Yeni harita district blockout henüz yok
- Öncelik: `P2`
- Durum: `Açık`
- Tekrar Üretim:
  - yeni harita açıldığında oynanabilir district akışı bulunmuyor
- Beklenen Sonuç:
  - trader hub, loot zone, city center ve özel alanlar okunabilir olmalı
- Mevcut Sonuç:
  - teknik kurulum var, içerik akışı henüz oluşmadı
- Etkilenen Sistem:
  - World Building

---

## Kapatma Kuralı
Bir bug şu üç durumda kapatılır:
1. düzeltildi
2. tekrar üretilemedi
3. ilgili sistemle birlikte test edildi

Sadece “düzelmiş gibi görünüyor” durumu kapatma için yeterli değildir.
