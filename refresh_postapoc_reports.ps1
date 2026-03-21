$root = 'C:\GercekProje\Gercek'
$exportJson = Join-Path $root 'postapoc_items_export.json'
$consumeAudit = Join-Path $root 'POSTAPOC_ITEMS_CONSUME_AUDIT.csv'
$visualGaps = Join-Path $root 'POSTAPOC_ITEMS_VISUAL_GAPS.csv'
$summaryPath = Join-Path $root 'POSTAPOC_ITEMS_AUDIT_SUMMARY.md'

$criticalConsumeRows = @(
    'Med_DirtyBandage',
    'Med_CleanBandage',
    'Med_Antibiotic',
    'Med_FirstAidKit',
    'Med_VitaminTabs',
    'Drink_DirtyWater',
    'Drink_FilteredWater',
    'Drink_EnergyDrink',
    'Drink_MilitaryFlask',
    'Drink_ElectrolyteMix'
)

$rows = Get-Content -Raw -Path $exportJson | ConvertFrom-Json

$consumeAuditRows = @()
$visualMap = @{}
$missingIcons = 0
$missingMeshes = 0
$missingDescriptions = 0
$missingRarity = 0
$foodCount = 0
$consumeEnabledCount = 0
$consumeErrorCount = 0

foreach ($row in $rows) {
    $name = [string]$row.Name
    $category = [string]$row.Category
    $pickupMesh = [string]$row.PickupMesh
    $itemIcon = [string]$row.ItemIcon
    $description = [string]$row.Description
    $rarity = [string]$row.ItemRarity
    $canConsume = [bool]$row.bCanConsume
    $consumeType = [string]$row.ConsumeEffectType
    $consumeAmountRaw = if ($null -ne $row.ConsumeAmount) { $row.ConsumeAmount } else { 0 }
    $baseValueRaw = if ($null -ne $row.BaseValue) { $row.BaseValue } else { 0 }
    $consumeAmount = [double]$consumeAmountRaw
    $baseValue = [int]([double]$baseValueRaw)

    $pickupMeshMissing = [string]::IsNullOrWhiteSpace($pickupMesh) -or $pickupMesh -eq 'None'
    $itemIconMissing = [string]::IsNullOrWhiteSpace($itemIcon) -or $itemIcon -eq 'None'
    $descriptionMissing = [string]::IsNullOrWhiteSpace($description)
    $rarityMissing = $rarity -eq 'None'

    if ($category -eq 'Food') { $foodCount++ }
    if ($canConsume) { $consumeEnabledCount++ }
    if ($pickupMeshMissing) { $missingMeshes++ }
    if ($itemIconMissing) { $missingIcons++ }
    if ($descriptionMissing) { $missingDescriptions++ }
    if ($rarityMissing) { $missingRarity++ }

    if (-not $visualMap.ContainsKey($category)) {
        $visualMap[$category] = [ordered]@{
            Category = $category
            Rows = 0
            MissingPickupMesh = 0
            MissingItemIcon = 0
            MissingDescription = 0
            MissingItemRarity = 0
        }
    }

    $bucket = $visualMap[$category]
    $bucket.Rows++
    if ($pickupMeshMissing) { $bucket.MissingPickupMesh++ }
    if ($itemIconMissing) { $bucket.MissingItemIcon++ }
    if ($descriptionMissing) { $bucket.MissingDescription++ }
    if ($rarityMissing) { $bucket.MissingItemRarity++ }

    if ($criticalConsumeRows -contains $name) {
        $issue = ''
        if (-not $canConsume) {
            $issue = 'bCanConsume=false'
            $consumeErrorCount++
        } elseif ($consumeType -eq 'None') {
            $issue = 'ConsumeEffectType missing'
            $consumeErrorCount++
        } elseif ($consumeAmount -le 0) {
            $issue = 'ConsumeAmount <= 0'
            $consumeErrorCount++
        }

        $displayConsumeAmount = if ($consumeAmount % 1 -eq 0) { [int]$consumeAmount } else { $consumeAmount }
        $status = if ([string]::IsNullOrEmpty($issue)) { 'OK' } else { 'FixRequired' }

        $consumeAuditRows += [pscustomobject]@{
            Name = $name
            Category = $category
            bCanConsume = $canConsume
            ConsumeEffectType = $consumeType
            ConsumeAmount = $displayConsumeAmount
            BaseValue = $baseValue
            Status = $status
            Issue = $issue
        }
    }
}

$consumeAuditRows |
    Sort-Object Name |
    Export-Csv -Path $consumeAudit -NoTypeInformation -Encoding UTF8

$visualGapRows = foreach ($entry in $visualMap.GetEnumerator()) {
    $bucket = $entry.Value
    [pscustomobject]@{
        Category = [string]$bucket.Category
        Rows = [int]$bucket.Rows
        MissingPickupMesh = [int]$bucket.MissingPickupMesh
        MissingItemIcon = [int]$bucket.MissingItemIcon
        MissingDescription = [int]$bucket.MissingDescription
        MissingItemRarity = [int]$bucket.MissingItemRarity
    }
}

$visualGapRows |
    Sort-Object Category |
    Export-Csv -Path $visualGaps -NoTypeInformation -Encoding UTF8

$summaryLines = @(
    '# PostApocItems Audit Summary',
    '',
    '## Genel Durum',
    "- Toplam satir: $($rows.Count)",
    "- Tuketilebilir olarak isaretli satir: $consumeEnabledCount",
    "- Food kategorisi satiri: $foodCount",
    '',
    '## Consume Durumu',
    "- Kritik consume satiri: $($consumeAuditRows.Count)",
    "- Hata gerektiren consume satiri: $consumeErrorCount",
    "- Durum: $(if ($consumeErrorCount -eq 0) { 'Temiz' } else { 'Duzeltme gerekli' })",
    '',
    '## Gorsel Veri Durumu',
    "- PickupMesh eksik satir: $missingMeshes/$($rows.Count)",
    "- ItemIcon eksik satir: $missingIcons/$($rows.Count)",
    "- Description eksik satir: $missingDescriptions/$($rows.Count)",
    "- ItemRarity eksik satir: $missingRarity/$($rows.Count)",
    '',
    '## Icon Analizi',
    '- Projede aktif item icon kutuphanesi yok.',
    '- Bulunan icon dosyalari HUD odakli.',
    '- DataTable tarafinda yalnizca tek bir item icon referansi gorunuyor.',
    '- Bu nedenle ikon bosluklari veri hatasindan cok, icerik hatti eksigi olarak ele alinmali.',
    '',
    '## Uretilen Dosyalar',
    '- `POSTAPOC_ITEMS_CONSUME_AUDIT.csv`',
    '- `POSTAPOC_ITEMS_VISUAL_GAPS.csv`',
    '- `POSTAPOC_ITEMS_VISUAL_PRODUCTION_PLAN.md`'
)

Set-Content -Path $summaryPath -Value $summaryLines -Encoding UTF8
