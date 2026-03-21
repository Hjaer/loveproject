"""
PostApocItems Row Structure Verification Script
Calistirma: UE Editor -> Tools -> Execute Python Script -> bu dosyayi sec
"""
import unreal

# 1. Data Table asset'ini yukle
dt_path = "/Game/Gercek/Datas/PostApocItems"
dt = unreal.load_asset(dt_path)

if dt is None:
    unreal.log_error("Data Table yuklenemedi: " + dt_path)
else:
    unreal.log("Data Table bulundu: " + str(dt))

    # 2. Dogru struct tipini bul
    correct_struct = unreal.load_object(None, "/Script/Gercek.PostApocItemRow")

    if correct_struct is None:
        unreal.log_error("FPostApocItemRow struct bulunamadi. Proje derlemesini kontrol edin.")
    else:
        unreal.log("Dogru struct bulundu: " + str(correct_struct))

        # 3. Mevcut satirlari yedekle
        rows = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt)
        unreal.log("Mevcut satirlar: " + str(rows))

        # 4. Row structure'i guncelle (editor utility ile)
        # Data Table'i editor'de aciniz, RowStruct'i degistiriniz:
        # Row Structure dropdown -> FPostApocItemRow secin -> Save
        unreal.log("=" * 60)
        unreal.log("YAPI OTOMATIK GUNCELLENEMEDI - EDITOR MANUEL ADIMLARI:")
        unreal.log("1. Data Table'i 'Yes' ile aciniz")
        unreal.log("2. Ust kisimda 'Row Structure' dropdown'ini bulunuz")
        unreal.log("3. 'FPostApocItemRow' veya 'PostApocItemRow' secin")
        unreal.log("4. Kaydedin")
        unreal.log("=" * 60)
        unreal.log("Mevcut row sayisi: " + str(len(rows)))
