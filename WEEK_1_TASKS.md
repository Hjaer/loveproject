# Hafta 1 Görevleri

## Ana Hedef
Tek oyuncuda ve host + 1 client senaryosunda güvenilir bir gameplay loop kurmak.

Bu hafta yalnızca çekirdeği sabitlemeye odaklanılacak.

Yeni büyük sistem açılmayacak:
- base expansion yok
- NPC faction sistemi yok
- quest genişletmesi yok
- world polish yok

## Öncelik Sırası
1. Landscape / RVT / paint zinciri
2. HUD doğrulama
3. Inventory / loot / trade döngüsü
4. Save / load / continue doğrulama

---

## 1. Landscape / RVT / Paint Zinciri

### Hedef
Yeni haritada landscape gerçekten boyanabilir ve güvenilir hale gelsin.

### Yapılacaklar
- `MI_Landscape` ve `M_Landscape` zincirini doğrula
- landscape actor üzerinde `Draw in Virtual Textures` ayarını doğrula
- `RVT_Landscape` volume kurulumunu doğrula
- `Grass`, `Sand`, `SmoothSand`, `Road` katmanlarını test et
- `Fill Layer` sonrası yüzeyin gerçekten değiştiğini doğrula

### Bitti Sayılma Kriteri
- katmanlar görünür fark oluşturuyor
- landscape yüzeyi gri/boş kalmıyor
- paint tepkisi güvenilir
- tekrar uygulanabilir kurulum sırası netleşmiş

### Test Adımları
1. `Grass Fill Layer`
2. `Sand` boya
3. `SmoothSand` boya
4. `Road` boya
5. editor yeniden açıldıktan sonra tekrar doğrula

---

## 2. HUD Doğrulama

### Hedef
HUD stat değişimlerini doğru ve stabil göstersin.

### Yapılacaklar
- `WBP_HUD_Main` parent class kontrolü
- `PB_*` ve `TXT_*` isimlerinin doğrulanması
- `AGercekCharacter` event zincirinin doğrulanması
- koşma, bekleme, hasar alma ve stat değişimlerinde HUD tepkisinin test edilmesi

### Bitti Sayılma Kriteri
- `Health`, `Stamina`, `Hunger`, `Thirst` hem bar hem text olarak güncelleniyor
- başlık text’leri sabit kalıyor
- değer text’leri canlı değişiyor
- HUD açılmama veya bind hatası yok

### Test Adımları
1. oyunu başlat
2. koş
3. stamina değişimini kontrol et
4. bekle
5. hunger/thirst değişimini kontrol et
6. hasar al
7. health değişimini kontrol et

---

## 3. Inventory / Loot / Trade Döngüsü

### Hedef
Oyuncunun ana loot döngüsü güvenilir hale gelsin.

### Yapılacaklar
- pickup akışını test et
- loot container açılışını test et
- item transfer akışını test et
- trade screen açılışını test et
- trade offer panel davranışını test et
- item condition/value ve TradeKnowledge etkisini doğrula

### Bitti Sayılma Kriteri
- pickup çalışıyor
- loot container’dan item alınabiliyor
- item bırakılabiliyor
- trade ekranı açılıyor
- teklif listeleri doğru çalışıyor
- drag/drop ve click davranışı bozuk değil

### Test Adımları
1. item al
2. envantere gir
3. item kullan
4. item drop et
5. loot container aç
6. item transfer et
7. trader aç
8. takas yap

---

## 4. Save / Load / Continue

### Hedef
İlerleme güvenilir şekilde geri yüklenebilsin.

### Yapılacaklar
- autosave süresini pratikte doğrula
- host save zincirini doğrula
- continue davranışını doğrula
- player inventory restore kontrolü yap
- player stat restore kontrolü yap
- world-state restore sınırlarını not et

### Bitti Sayılma Kriteri
- autosave oluşuyor
- continue son save’den başlatıyor
- inventory geri geliyor
- statlar geri geliyor
- karakter pozisyonu mantıklı şekilde restore oluyor

### Test Adımları
1. oyuna gir
2. loot al
3. item taşı
4. kısa süre oynayıp save tetikle
5. oyundan çık
6. continue ile geri dön
7. inventory ve statları karşılaştır

---

## Günlük Plan

### Gün 1
- landscape / RVT / paint zinciri

### Gün 2
- HUD doğrulama
- landscape tekrar testi

### Gün 3
- inventory / loot / trade testi

### Gün 4
- save / load / continue testi

### Gün 5
- host + 1 client mini doğrulama
- hafta sonu bug kapama listesi çıkarma

---

## Hafta Sonu Çıkış Kriteri
Hafta sonunda elimizde şunlar olmalı:
- çalışan harita zemini
- çalışan HUD
- çalışan loot/trade loop
- güvenilir save/load başlangıcı

Eğer bunlardan biri tamam değilse yeni sisteme geçilmeyecek.
