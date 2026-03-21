import unreal
asset_path = '/Game/Gercek/Datas/PostApocItems'
asset = unreal.load_asset(asset_path)
if not asset:
    raise RuntimeError(f'Asset not found: {asset_path}')
result = unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False)
unreal.log(f'SAVE_RESULT {result}')
