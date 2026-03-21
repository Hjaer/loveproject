import json
import unreal

ASSET_PATH = '/Game/Gercek/Datas/PostApocItems'
TEMP_JSON_PATH = r'C:\GercekProje\Gercek\postapoc_items_consume_patched.json'

PATCHES = {
    'Med_DirtyBandage': {'ConsumeEffectType': 'Heal', 'ConsumeAmount': 10.0},
    'Med_CleanBandage': {'ConsumeEffectType': 'Heal', 'ConsumeAmount': 18.0},
    'Med_Antibiotic': {'ConsumeEffectType': 'Heal', 'ConsumeAmount': 15.0},
    'Med_FirstAidKit': {'ConsumeEffectType': 'Heal', 'ConsumeAmount': 45.0},
    'Med_VitaminTabs': {'ConsumeEffectType': 'AntiRad', 'ConsumeAmount': 8.0},
    'Drink_DirtyWater': {'ConsumeEffectType': 'Drink', 'ConsumeAmount': 10.0},
    'Drink_FilteredWater': {'ConsumeEffectType': 'Drink', 'ConsumeAmount': 25.0},
    'Drink_EnergyDrink': {'ConsumeEffectType': 'Drink', 'ConsumeAmount': 20.0},
    'Drink_MilitaryFlask': {'ConsumeEffectType': 'Drink', 'ConsumeAmount': 30.0},
    'Drink_ElectrolyteMix': {'ConsumeEffectType': 'Drink', 'ConsumeAmount': 35.0},
}

asset = unreal.load_asset(ASSET_PATH)
if not asset:
    raise RuntimeError(f'Asset not found: {ASSET_PATH}')

json_data = unreal.DataTableFunctionLibrary.export_data_table_to_json_string(asset)
rows = json.loads(json_data)
patched = []

for row in rows:
    name = row.get('Name')
    if name in PATCHES:
        row.update(PATCHES[name])
        patched.append(name)

missing = sorted(set(PATCHES.keys()) - set(patched))
if missing:
    raise RuntimeError('Missing rows for patch: ' + ', '.join(missing))

with open(TEMP_JSON_PATH, 'w', encoding='utf-8') as handle:
    json.dump(rows, handle, ensure_ascii=False, indent=2)

unreal.DataTableFunctionLibrary.fill_data_table_from_json_file(asset, TEMP_JSON_PATH)
saved = unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)
unreal.log('PATCHED_ROWS ' + ','.join(sorted(patched)))
unreal.log(f'SAVE_RESULT {saved}')
