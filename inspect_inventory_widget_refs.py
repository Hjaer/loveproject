import unreal

assets = [
    "/Game/Gercek/Inventory/DDO_GridItem",
    "/Game/Gercek/Inventory/WBP_GridItem",
    "/Game/Gercek/Inventory/WBP_InventoryGrid",
]

registry = unreal.AssetRegistryHelpers.get_asset_registry()
report_lines = []

for asset_path in assets:
    report_lines.append("=" * 80)
    report_lines.append(asset_path)
    data = registry.get_asset_by_object_path(asset_path)
    report_lines.append(f"asset_found={data.is_valid()}")
    if data.is_valid():
        report_lines.append(f"asset_class={data.asset_class_path.asset_name}")
    refs = registry.get_referencers(
        unreal.Name(asset_path),
        unreal.AssetRegistryDependencyOptions(
            include_soft_package_references=True,
            include_hard_package_references=True,
            include_searchable_names=True,
            include_soft_management_references=True,
            include_hard_management_references=True,
        ),
    )
    report_lines.append("referencers:")
    for ref in refs:
        report_lines.append(f"  {ref}")

report_path = r"C:\GercekProje\Gercek\Saved\Reports\InventoryWidgetRefReport.txt"
with open(report_path, "w", encoding="utf-8") as handle:
    handle.write("\n".join(report_lines))
print(report_path)
