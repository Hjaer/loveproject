import unreal
for name in dir(unreal.DataTableFunctionLibrary):
    if 'csv' in name.lower() or 'json' in name.lower() or 'data_table' in name.lower() or 'fill_' in name.lower():
        unreal.log(name)
