import unreal

asset_path = "/Game/Gercek/Datas/PostApocItems"
asset = unreal.load_asset(asset_path)
if not asset:
    unreal.log_error(f"ASSET_NOT_FOUND {asset_path}")
    raise RuntimeError("Asset not found")

row_struct = asset.get_editor_property("row_struct")
struct_name = row_struct.get_name() if row_struct else "None"
unreal.log(f"ROW_STRUCT {struct_name}")

props = []
if row_struct:
    try:
        props = [p.get_name() for p in row_struct.properties()]
    except Exception:
        props = []
unreal.log("ROW_PROPERTIES " + ",".join(props))

legacy = [name for name in props if name in [
    "ItemName", "ItemDescription", "ItemType", "ItemCategory", "Rarity",
    "MaxStackSize", "ItemWeight", "ItemValue", "ExtraCapacity", "MeshPath"
]]
unreal.log("LEGACY_PROPERTIES " + (",".join(legacy) if legacy else "NONE"))

row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(asset)
unreal.log(f"ROW_COUNT {len(row_names)}")

interesting_columns = [
    "ItemID", "DisplayName", "Description", "Category", "PickupMesh", "ItemIcon",
    "ItemRarity", "MaxStack", "Weight", "BaseValue", "bCanConsume", "bCanEquip",
    "bCanStack", "bQuestItem", "ConsumeEffectType", "ConsumeAmount", "ItemSize",
    "bCanBeRotated"
]

for column in interesting_columns:
    try:
        values = unreal.DataTableFunctionLibrary.get_data_table_column_as_string(asset, column)
        empty_count = 0
        non_empty = 0
        for value in values:
            normalized = str(value).strip()
            if normalized == "" or normalized == "None" or normalized == "()":
                empty_count += 1
            else:
                non_empty += 1
        unreal.log(f"COLUMN {column} non_empty={non_empty} empty={empty_count}")
    except Exception as exc:
        unreal.log_error(f"COLUMN_ERROR {column} {exc}")
