$root = 'C:\GercekProje\Gercek'
$exportJson = Join-Path $root 'postapoc_items_export.json'
$outputCsv = Join-Path $root 'MERYEM_DRINK_MEDICAL_ASSET_LIST.csv'

$rows = Get-Content -Raw -Path $exportJson | ConvertFrom-Json

function Get-DisplayLabel {
    param(
        [string]$DisplayNameValue,
        [string]$FallbackName
    )

    if ([string]::IsNullOrWhiteSpace($DisplayNameValue)) {
        return $FallbackName
    }

    if ($DisplayNameValue -match '"([^"]+)"\s*\)\s*$') {
        return $matches[1]
    }

    return $DisplayNameValue
}

$result = foreach ($row in ($rows | Where-Object { $_.Category -in @('Drink', 'Medical') } | Sort-Object Category, Name)) {
    $rowName = [string]$row.Name
    $category = [string]$row.Category
    $displayLabel = Get-DisplayLabel -DisplayNameValue ([string]$row.DisplayName) -FallbackName $rowName

    $iconAssetName = "T_Item_{0}_Icon" -f $rowName
    $meshAssetName = "SM_Item_{0}_Pickup" -f $rowName
    $iconFolder = "/Game/Gercek/Items/Icons/{0}" -f $category
    $meshFolder = "/Game/Gercek/Items/Meshes/{0}" -f $category
    $iconPath = "{0}/{1}.{1}" -f $iconFolder, $iconAssetName
    $meshPath = "{0}/{1}.{1}" -f $meshFolder, $meshAssetName

    [pscustomobject]@{
        RowName = $rowName
        Category = $category
        DisplayName = $displayLabel
        IconAssetName = $iconAssetName
        IconFolder = $iconFolder
        IconAssetPath = $iconPath
        PickupMeshAssetName = $meshAssetName
        PickupMeshFolder = $meshFolder
        PickupMeshAssetPath = $meshPath
    }
}

$result | Export-Csv -Path $outputCsv -NoTypeInformation -Encoding UTF8
