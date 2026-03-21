import csv
import json
import os
import unreal

ASSET_PATH = '/Game/Gercek/Datas/PostApocItems'
REPORT_DIR = r'C:\GercekProje\Gercek\Saved\Reports'
REPORT_PATH = os.path.join(REPORT_DIR, 'PostApocItemsValidation.csv')

asset = unreal.load_asset(ASSET_PATH)
if not asset:
    raise RuntimeError(f'Asset not found: {ASSET_PATH}')

rows = json.loads(unreal.DataTableFunctionLibrary.export_data_table_to_json_string(asset))
os.makedirs(REPORT_DIR, exist_ok=True)
issues = []

for row in rows:
    name = row.get('Name', 'Unknown')
    category = row.get('Category', 'Unknown')
    can_consume = bool(row.get('bCanConsume', False))
    max_stack = int(row.get('MaxStack', 0) or 0)
    consume_type = str(row.get('ConsumeEffectType', 'None'))
    consume_amount = float(row.get('ConsumeAmount', 0) or 0)
    pickup_mesh = str(row.get('PickupMesh', 'None'))
    item_icon = str(row.get('ItemIcon', 'None'))
    description = str(row.get('Description', ''))
    rarity = str(row.get('ItemRarity', 'None'))
    can_stack = bool(row.get('bCanStack', False))
    base_value = float(row.get('BaseValue', 0) or 0)
    is_quest = bool(row.get('bQuestItem', False))

    def add_issue(severity, field, message):
        issues.append({
            'Name': name,
            'Category': category,
            'Severity': severity,
            'Field': field,
            'Message': message,
        })

    if not row.get('ItemID'):
        add_issue('Error', 'ItemID', 'Bos birakilmis.')
    if not row.get('DisplayName'):
        add_issue('Error', 'DisplayName', 'Bos birakilmis.')
    if pickup_mesh == 'None':
        add_issue('Warning', 'PickupMesh', 'Dunya temsil meshi yok.')
    if item_icon == 'None':
        add_issue('Warning', 'ItemIcon', 'Inventory/trade iconu yok.')
    if not description.strip():
        add_issue('Warning', 'Description', 'Tooltip/aciklama bos.')
    if rarity == 'None':
        add_issue('Warning', 'ItemRarity', 'Rarity tanimli degil.')
    if can_stack and max_stack <= 1:
        add_issue('Error', 'MaxStack', 'bCanStack=true iken MaxStack <= 1.')
    if category in ('Food', 'Drink') and not can_consume:
        add_issue('Error', 'bCanConsume', f'{category} kategorisinde ama tuketilebilir degil.')
    if can_consume and consume_type == 'None':
        add_issue('Error', 'ConsumeEffectType', 'Tuketilebilir itemda effect tipi yok.')
    if can_consume and consume_amount <= 0:
        add_issue('Error', 'ConsumeAmount', 'Tuketilebilir itemda consume amount <= 0.')
    if is_quest and base_value > 0:
        add_issue('Warning', 'BaseValue', 'Quest item icin trade degeri sifir degil.')

food_count = sum(1 for row in rows if row.get('Category') == 'Food')
if food_count == 0:
    issues.append({
        'Name': '__TABLE__',
        'Category': 'Table',
        'Severity': 'Warning',
        'Field': 'FoodCategory',
        'Message': 'Food kategorisinde hic satir yok.',
    })

with open(REPORT_PATH, 'w', newline='', encoding='utf-8') as handle:
    writer = csv.DictWriter(handle, fieldnames=['Name', 'Category', 'Severity', 'Field', 'Message'])
    writer.writeheader()
    writer.writerows(issues)

unreal.log(f'VALIDATION_REPORT {REPORT_PATH}')
unreal.log(f'VALIDATION_ISSUE_COUNT {len(issues)}')
