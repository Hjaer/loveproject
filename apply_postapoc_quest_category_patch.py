import json
import unreal

ASSET_PATH = '/Game/Gercek/Datas/PostApocItems'
TEMP_JSON_PATH = r'C:\GercekProje\Gercek\postapoc_items_quest_category_patched.json'

QUEST_ROWS = {
    'Quest_BunkerKey',
    'Quest_AccessCard',
    'Quest_EncryptedRadio',
    'Quest_MysteriousLetter',
    'Quest_FamilyPhoto',
}

asset = unreal.load_asset(ASSET_PATH)
if not asset:
    raise RuntimeError(f'Asset not found: {ASSET_PATH}')

json_data = unreal.DataTableFunctionLibrary.export_data_table_to_json_string(asset)
rows = json.loads(json_data)
patched = []

for row in rows:
    name = row.get('Name')
    if name in QUEST_ROWS:
        row['Category'] = 'Quest'
        patched.append(name)

missing = sorted(QUEST_ROWS - set(patched))
if missing:
    raise RuntimeError('Missing rows for patch: ' + ', '.join(missing))

with open(TEMP_JSON_PATH, 'w', encoding='utf-8') as handle:
    json.dump(rows, handle, ensure_ascii=False, indent=2)

unreal.DataTableFunctionLibrary.fill_data_table_from_json_file(asset, TEMP_JSON_PATH)
saved = unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)
unreal.log('PATCHED_QUEST_ROWS ' + ','.join(sorted(patched)))
unreal.log(f'SAVE_RESULT {saved}')
