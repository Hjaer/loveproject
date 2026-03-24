$root = if ($args.Count -gt 0) { $args[0] } else { "C:\GercekProje\Gercek\Content" }

Get-ChildItem -Path $root -Recurse -File -Include *.uasset,*.umap |
    Where-Object { $_.Length -lt 1024 } |
    ForEach-Object {
        try {
            $bytes = [System.IO.File]::ReadAllBytes($_.FullName)
            $prefixLength = [Math]::Min($bytes.Length, 80)
            $text = [System.Text.Encoding]::UTF8.GetString($bytes, 0, $prefixLength)
            if ($text -like 'version https://git-lfs.github.com/spec/v1*') {
                [PSCustomObject]@{
                    FullName = $_.FullName
                    Length = $_.Length
                    LastWriteTime = $_.LastWriteTime
                }
            }
        } catch {
        }
    } |
    Sort-Object FullName |
    Format-Table -AutoSize
