# İlk 90 Gün Planı

## Amaç
Yaklaşık 90 gün içinde Erken Erişim adayı bir build seviyesine ulaşmak. Bunu yapmak için içerik genişletmek yerine stabil, oynanabilir milestone’lara odaklanacağız.

## 1. Ay: Çekirdeği Stabil Hale Getir

### Ana Hedef
Tek oyuncuda ve host + 1 client senaryosunda güvenilir bir gameplay loop kurmak.

### Hafta 1
- landscape / RVT / paint zincirini düzelt
- HUD güncellemelerini doğrula
- inventory, loot ve trade döngüsünü test et
- save/load/continue hattını doğrula

### Hafta 2
- autosave ve continue akışını tekrar tekrar test et
- reconnect ve inventory restore doğrulaması yap
- loot container ve trade sync testlerini tek client ile yap
- kalan inventory UI kırıklarını kapat

### Hafta 3
- resmi bir gameplay test checklist’i oluştur
- host + 1 client oturum testleri yap
- engelleyici sync sorunlarını kapat
- base sistemine geçmeden önce save/load gereksinimlerini netleştir

### Hafta 4
- çekirdek sistemlerde feature freeze uygula
- sadece bug fix, cleanup ve test kaynaklı düzeltmeler yap
- milestone’u ancak gerçekten güvenilir olduğunda bitmiş say

### 1. Ay Çıkış Kriteri
- stabil host + 1 client
- güvenilir save/load
- güvenilir inventory/trade loop
- çözülmüş landscape zinciri

## 2. Ay: Oynanabilir Dünya ve Progression

### Ana Hedef
Yeni haritayı gerçek progression alanına dönüştürmek.

### Hafta 5
- district blockout
- trader hub yerleşimi
- suburb loot alanı
- industrial zone kurulumu

### Hafta 6
- city center risk alanı kurulumu
- özel lab/bunker alanı kurulumu
- traversal okunabilirlik düzenlemesi
- loot dağılım pass’i

### Hafta 7
- quest veri modeli
- 3-5 görevlik zincirin temeli
- locked access sistemi
- access card / key progression

### Hafta 8
- görevleri özel alanlara bağla
- ilk yüksek değerli progression ödüllerini ekle
- oyuncunun güvenli alandan riskli alana amaçla gidebildiğini doğrula

### 2. Ay Çıkış Kriteri
- gerçek oynanabilir harita akışı
- görev odaklı progression
- oyuncunun rastgele loot dışında amacı olması

## 3. Ay: Base Döngüsü, Tehdit Döngüsü ve Erken Erişim Adayı

### Ana Hedef
Oyunun duygusal döngüsünü tamamlayan sistemleri eklemek.

### Hafta 9
- metal shack
- stone house
- base storage
- bed interaction

### Hafta 10
- sleep vote sistemi
- tüm oyuncular hazırsa sabaha geçiş
- base storage save/load
- co-op sleep flow doğrulaması

### Hafta 11
- iki düşman NPC arketipi ekle
- kontrollü encounter noktaları yerleştir
- city center’ı gerçekten tehlikeli hissettir
- combat baskısının progression loop’a etkisini test et

### Hafta 12
- ilk CORE AI teslim akışını ekle
- üç collectible kategorisini bağla
- ilk restoration işaretini göster
- 10 saatlik içerik akışını gözden geçir

### 3. Ay Çıkış Kriteri
- oyuncu hayatta kalabiliyor, loot yapabiliyor, trade yapabiliyor, ilerleyebiliyor, base kurabiliyor, uyuyabiliyor ve ilk CORE AI loop’una girebiliyor
- proje Erken Erişim aday build seviyesine gelmiş oluyor

## Son Düzlük: Aday Build İncelemesi

Eğer ilk 90 günden sonra zaman kalırsa yalnızca şunlara harcayın:
- bug fixing
- performance pass
- stability pass
- UX cleanup
- Steam sayfası hazırlığı
- trailer/screenshot hazırlığı

Bu aşamada gerçekten kritik bir eksik değilse yeni büyük sistem açmayın.

## Haftalık Kurallar
- haftada bir ana hedef
- en fazla üç destek görevi
- bir zorunlu test bloğu
- scope dışına çıkan her şey durdurulmalı

## Kırmızı Bayraklar
Aşağıdakilerden biri olursa durup scope’u yeniden değerlendirin:
- save/load stabil değilse
- co-op temel progression’ı bozuyorsa
- harita üretimi tüm mühendislik zamanını yiyorsa
- base sistemi erken aşamada freeform building’e dönüşüyorsa
- NPC sistemi çok erken tam simülasyona kayıyorsa

## Nihai Standart
90 günün sonunda proje:
- tam hayaliniz kadar büyük olmayabilir
- ama ağır içerikli bir prototipten çok daha sağlam olmalı
- dürüstçe pazarlanabilir olmalı
- yaklaşık 10 saatlik anlamlı oynanış taşımalı
