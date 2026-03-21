import unreal
asset = unreal.load_asset('/Game/Gercek/Datas/PostApocItems')
if not asset:
    raise RuntimeError('PostApocItems not found')
try:
    csv_data = unreal.DataTableFunctionLibrary.export_data_table_to_csv_string(asset)
    unreal.log('CSV_EXPORT_OK')
    with open(r'C:\GercekProje\Gercek\postapoc_items_export.csv', 'w', encoding='utf-8') as f:
        f.write(csv_data)
except Exception as exc:
    unreal.log_error(f'CSV_EXPORT_FAIL {exc}')
try:
    json_data = unreal.DataTableFunctionLibrary.export_data_table_to_json_string(asset)
    unreal.log('JSON_EXPORT_OK')
    with open(r'C:\GercekProje\Gercek\postapoc_items_export.json', 'w', encoding='utf-8') as f:
        f.write(json_data)
except Exception as exc:
    unreal.log_error(f'JSON_EXPORT_FAIL {exc}')
