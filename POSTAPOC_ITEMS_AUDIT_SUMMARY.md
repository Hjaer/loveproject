# PostApocItems Audit Summary

## Genel Durum
- Toplam satir: 70
- Tuketilebilir olarak isaretli satir: 10
- Food kategorisi satiri: 0

## Consume Durumu
- Kritik consume satiri: 10
- Hata gerektiren consume satiri: 0
- Durum: Temiz

## Gorsel Veri Durumu
- PickupMesh eksik satir: 70/70
- ItemIcon eksik satir: 69/70
- Description eksik satir: 70/70
- ItemRarity eksik satir: 70/70

## Icon Analizi
- Projede aktif item icon kutuphanesi yok.
- Bulunan icon dosyalari HUD odakli.
- DataTable tarafinda yalnizca tek bir item icon referansi gorunuyor.
- Bu nedenle ikon bosluklari veri hatasindan cok, icerik hatti eksigi olarak ele alinmali.

## Uretilen Dosyalar
- `POSTAPOC_ITEMS_CONSUME_AUDIT.csv`
- `POSTAPOC_ITEMS_VISUAL_GAPS.csv`
- `POSTAPOC_ITEMS_VISUAL_PRODUCTION_PLAN.md`
