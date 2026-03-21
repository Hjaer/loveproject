import unreal
asset = unreal.load_asset('/Game/Gercek/Datas/PostApocItems')
legacy_columns = ['ItemName','ItemDescription','ItemType','ItemCategory','Rarity','MaxStackSize','ItemWeight','ItemValue','ExtraCapacity','MeshPath']
for column in legacy_columns:
    try:
        values = unreal.DataTableFunctionLibrary.get_data_table_column_as_string(asset, column)
        unreal.log(f'LEGACY_COLUMN {column} present count={len(values)}')
    except Exception as exc:
        unreal.log(f'LEGACY_COLUMN {column} missing')
