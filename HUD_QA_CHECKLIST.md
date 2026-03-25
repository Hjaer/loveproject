# HUD QA Checklist

## Amaç
Bu checklist, HUD sisteminin tek oyuncu ve co-op senaryolarında doğru karaktere bağlı çalıştığını doğrulamak için hazırlanmıştır.

Kontrol edilen sistemler:
- `UPostApocHUDWidget`
- `WBP_HUD_Main`
- `AGercekCharacter`
- survival stat event zinciri

---

## 1. Tek Oyuncu Testi

### Amaç
HUD tek oyuncuda doğru widget’a bağlı mı ve stat değişimlerini doğru gösteriyor mu?

### Adımlar
1. Oyunu tek oyuncu başlat.
2. HUD ekranda geliyor mu kontrol et.
3. Koş.
4. `Stamina` barı düşüyor mu kontrol et.
5. `TXT_Stamina` yüzdesi düşüyor mu kontrol et.
6. Dur.
7. `Stamina` tekrar doluyor mu kontrol et.
8. Bir süre bekle.
9. `Hunger` ve `Thirst` yavaş azalıyor mu kontrol et.
10. Hasar al.
11. `Health` barı ve text’i değişiyor mu kontrol et.

### Beklenen Sonuç
- başlık yazıları sabit kalır
- yüzde text’leri canlı değişir
- barlar ve text’ler birlikte güncellenir
- boş HUD oluşmaz

---

## 2. Host Testi

### Amaç
Listen server host kendi karakterine doğru HUD ile bağlı mı?

### Adımlar
1. Oyunu host olarak başlat.
2. Host karakter spawn olduktan sonra HUD geliyor mu kontrol et.
3. Koş.
4. `Stamina` host tarafında doğru düşüyor mu kontrol et.
5. Bekle.
6. `Hunger` ve `Thirst` hostta akıyor mu kontrol et.
7. Hasar al.
8. `Health` doğru güncelleniyor mu kontrol et.
9. Inventory / trade / loot container açıp kapat.
10. HUD kayboluyor mu veya bozuluyor mu kontrol et.

### Beklenen Sonuç
- host yalnızca kendi statlarını görür
- HUD client’a karışmaz
- viewport’ta tek HUD kalır

---

## 3. Client Testi

### Amaç
Client kendi pawn’ına doğru bağlanıyor mu?

### Adımlar
1. Host oyunu açsın.
2. Client bağlansın.
3. Client HUD açılıyor mu kontrol et.
4. Client koşsun.
5. `Stamina` yalnızca client’ın kendi HUD’ında değişiyor mu kontrol et.
6. Host koşsun.
7. Client HUD’ı hostun statına kayıyor mu kontrol et.
8. Client hasar alsın.
9. `Health` client tarafında doğru güncelleniyor mu kontrol et.

### Beklenen Sonuç
- client yalnızca kendi statlarını görür
- host statları client HUD’ına karışmaz
- yanlış pawn bind durumu olmaz

---

## 4. Reconnect Testi

### Amaç
Yeniden bağlanınca HUD doğru karaktere tekrar bağlanıyor mu?

### Adımlar
1. Host oyunu açsın.
2. Client bağlansın.
3. Client biraz oynasın.
4. Client oyundan çıksın.
5. Aynı hesapla tekrar bağlansın.
6. Spawn sonrası HUD geliyor mu kontrol et.
7. Client tekrar koşsun.
8. Client tekrar hasar alsın.

### Beklenen Sonuç
- HUD yeniden bağlanır
- boş kalmaz
- eski karakter referansında takılı kalmaz

---

## 5. Respawn Testi

### Amaç
Pawn değişirse HUD yeni pawn’a bağlanıyor mu?

### Adımlar
1. Oyuncuyu öldür veya respawn tetikle.
2. Yeni pawn spawn olduktan sonra HUD’ı kontrol et.
3. Koş.
4. Hasar al.
5. Bekle.

### Beklenen Sonuç
- HUD yeni pawn’a bağlanır
- eski pawn verisinde donup kalmaz
- statlar akmaya devam eder

---

## 6. Map Restart / Yeniden Başlatma Testi

### Amaç
Map yeniden açıldığında HUD zinciri doğru kuruluyor mu?

### Adımlar
1. Oyunu başlat.
2. HUD çalıştığını doğrula.
3. Map’i yeniden yükle veya oyun akışını yeniden başlat.
4. Spawn sonrası HUD’ı kontrol et.
5. Koş / hasar al / bekle.

### Beklenen Sonuç
- HUD yeniden oluşur
- doğru karaktere bind olur
- bind kaçırması yaşanmaz

---

## Hızlı Hata İşaretleri
Aşağıdakilerden biri olursa HUD zinciri sorunludur:

1. Başlık yazıları yüzdeye dönüşüyor
2. Bar değişiyor ama text değişmiyor
3. HUD hiç gelmiyor
4. Host kendi yerine client statlarını görüyormuş gibi davranıyor
5. Client reconnect sonrası HUD boş kalıyor
6. Respawn sonrası eski değerlerde donup kalıyor

---

## Test Sonucu Not Formatı

### Test
- Tek Oyuncu / Host / Client / Reconnect / Respawn / Map Restart

### Sonuç
- Geçti / Kaldı

### Sorun
- Kısa açıklama

### Tekrar Üretim
1. ...
2. ...
3. ...

### Not
- Log / ekran görüntüsü / video varsa eklenir

---

## Son Kural
HUD yalnızca tek oyuncuda çalışıyor diye tamam sayılmaz.

Aşağıdaki dördü geçmeden HUD sistemi kapatılmaz:
- tek oyuncu
- host
- client
- reconnect / respawn
