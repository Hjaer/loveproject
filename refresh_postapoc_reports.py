import csv
import json
from collections import defaultdict
from pathlib import Path


ROOT = Path(r"C:\GercekProje\Gercek")
EXPORT_JSON = ROOT / "postapoc_items_export.json"
CONSUME_AUDIT = ROOT / "POSTAPOC_ITEMS_CONSUME_AUDIT.csv"
VISUAL_GAPS = ROOT / "POSTAPOC_ITEMS_VISUAL_GAPS.csv"
SUMMARY_MD = ROOT / "POSTAPOC_ITEMS_AUDIT_SUMMARY.md"

CRITICAL_CONSUME_ROWS = {
    "Med_DirtyBandage",
    "Med_CleanBandage",
    "Med_Antibiotic",
    "Med_FirstAidKit",
    "Med_VitaminTabs",
    "Drink_DirtyWater",
    "Drink_FilteredWater",
    "Drink_EnergyDrink",
    "Drink_MilitaryFlask",
    "Drink_ElectrolyteMix",
}


def is_missing_soft_ref(value):
    return str(value or "None") in {"", "None"}


rows = json.loads(EXPORT_JSON.read_text(encoding="utf-8"))

consume_rows = []
visual_by_category = defaultdict(
    lambda: {
        "Rows": 0,
        "MissingPickupMesh": 0,
        "MissingItemIcon": 0,
        "MissingDescription": 0,
        "MissingItemRarity": 0,
    }
)

missing_icons = 0
missing_meshes = 0
missing_descriptions = 0
missing_rarity = 0
food_count = 0
consume_enabled_count = 0
consume_error_count = 0

for row in rows:
    name = row.get("Name", "Unknown")
    category = row.get("Category", "Unknown")
    can_consume = bool(row.get("bCanConsume", False))
    consume_type = str(row.get("ConsumeEffectType", "None"))
    consume_amount = float(row.get("ConsumeAmount", 0) or 0)
    pickup_mesh_missing = is_missing_soft_ref(row.get("PickupMesh"))
    item_icon_missing = is_missing_soft_ref(row.get("ItemIcon"))
    description_missing = not str(row.get("Description", "")).strip()
    rarity_missing = str(row.get("ItemRarity", "None")) == "None"

    if category == "Food":
        food_count += 1

    if can_consume:
        consume_enabled_count += 1

    if pickup_mesh_missing:
        missing_meshes += 1
    if item_icon_missing:
        missing_icons += 1
    if description_missing:
        missing_descriptions += 1
    if rarity_missing:
        missing_rarity += 1

    bucket = visual_by_category[category]
    bucket["Rows"] += 1
    bucket["MissingPickupMesh"] += int(pickup_mesh_missing)
    bucket["MissingItemIcon"] += int(item_icon_missing)
    bucket["MissingDescription"] += int(description_missing)
    bucket["MissingItemRarity"] += int(rarity_missing)

    if name in CRITICAL_CONSUME_ROWS:
        issue = ""
        if not can_consume:
            issue = "bCanConsume=false"
            consume_error_count += 1
        elif consume_type == "None":
            issue = "ConsumeEffectType missing"
            consume_error_count += 1
        elif consume_amount <= 0:
            issue = "ConsumeAmount <= 0"
            consume_error_count += 1

        consume_rows.append(
            {
                "Name": name,
                "Category": category,
                "bCanConsume": str(can_consume),
                "ConsumeEffectType": consume_type,
                "ConsumeAmount": str(int(consume_amount) if consume_amount.is_integer() else consume_amount),
                "BaseValue": str(int(float(row.get("BaseValue", 0) or 0))),
                "Status": "OK" if not issue else "FixRequired",
                "Issue": issue,
            }
        )


consume_rows.sort(key=lambda item: item["Name"])
with CONSUME_AUDIT.open("w", newline="", encoding="utf-8") as handle:
    writer = csv.DictWriter(
        handle,
        fieldnames=[
            "Name",
            "Category",
            "bCanConsume",
            "ConsumeEffectType",
            "ConsumeAmount",
            "BaseValue",
            "Status",
            "Issue",
        ],
    )
    writer.writeheader()
    writer.writerows(consume_rows)


with VISUAL_GAPS.open("w", newline="", encoding="utf-8") as handle:
    writer = csv.DictWriter(
        handle,
        fieldnames=[
            "Category",
            "Rows",
            "MissingPickupMesh",
            "MissingItemIcon",
            "MissingDescription",
            "MissingItemRarity",
        ],
    )
    writer.writeheader()
    for category in sorted(visual_by_category.keys()):
        row = {"Category": category}
        row.update(visual_by_category[category])
        writer.writerow(row)


summary = f"""# PostApocItems Audit Summary

## Genel Durum
- Toplam satir: {len(rows)}
- Tuketilebilir olarak isaretli satir: {consume_enabled_count}
- Food kategorisi satiri: {food_count}

## Consume Durumu
- Kritik consume satiri: {len(consume_rows)}
- Hata gerektiren consume satiri: {consume_error_count}
- Durum: {"Temiz" if consume_error_count == 0 else "Duzeltme gerekli"}

## Gorsel Veri Durumu
- PickupMesh eksik satir: {missing_meshes}/{len(rows)}
- ItemIcon eksik satir: {missing_icons}/{len(rows)}
- Description eksik satir: {missing_descriptions}/{len(rows)}
- ItemRarity eksik satir: {missing_rarity}/{len(rows)}

## Icon Analizi
- Projede aktif item icon kutuphanesi yok.
- Bulunan icon dosyalari HUD odakli.
- DataTable tarafinda yalnizca tek bir item icon referansi gorunuyor.
- Bu nedenle ikon bosluklari veri hatasindan cok, icerik hatti eksigi olarak ele alinmali.

## Uretilen Dosyalar
- `POSTAPOC_ITEMS_CONSUME_AUDIT.csv`
- `POSTAPOC_ITEMS_VISUAL_GAPS.csv`
- `POSTAPOC_ITEMS_VISUAL_PRODUCTION_PLAN.md`
"""

SUMMARY_MD.write_text(summary, encoding="utf-8")
