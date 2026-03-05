"""
DT_ItemDataBase Row Structure Repair Script
Calistirma: UE Editor -> Tools -> Execute Python Script -> bu dosyayi sec
"""
import unreal

# 1. Data Table asset'ini yukle
dt_path = "/Game/Gercek/Datas/DT_ItemDataBase"
dt = unreal.load_asset(dt_path)

if dt is None:
    unreal.log_error("Data Table yuklenemedi: " + dt_path)
else:
    unreal.log("Data Table bulundu: " + str(dt))

    # 2. Dogru struct tipini bul
    correct_struct = unreal.load_object(None, "/Script/Gercek.ItemDBRow")

    if correct_struct is None:
        unreal.log_error("FItemDBRow struct bulunamadi. Proje derlemesini kontrol edin.")
    else:
        unreal.log("Dogru struct bulundu: " + str(correct_struct))

        # 3. Mevcut satirlari yedekle
        rows = unreal.DataTableFunctionLibrary.get_data_table_row_names(dt)
        unreal.log("Mevcut satirlar: " + str(rows))

        # 4. Row structure'i guncelle (editor utility ile)
        # Data Table'i editor'de aciniz, RowStruct'i degistiriniz:
        # Row Structure dropdown -> FItemDBRow secin -> Save
        unreal.log("="*60)
        unreal.log("YAPI ONARILAMADI - EDITOR MANÜEL ADIMLARI:")
        unreal.log("1. Data Table'i 'Yes' ile aciniz")
        unreal.log("2. Ust kisimda 'Row Structure' dropdown'ini bulunuz")
        unreal.log("3. 'FItemDBRow' veya 'ItemDBRow' secin")
        unreal.log("4. Kaydedin")
        unreal.log("="*60)
        unreal.log("Mevcut row sayisi: " + str(len(rows)))
