## Big Diff Reference Audit

Date: 2026-03-23

### Scope
- Modified assets:
  - `/Game/Istanbul`
  - `/Game/Gercek/BP_GercekCharacter`
- Deleted content roots:
  - `Content/MetaHumans` (`488`)
  - `Content/PostApocBuildings` (`331`)
  - `Content/insaat` (`7`)

### Confirmed Findings

1. `/Game/Istanbul` still loads and PIE starts.
   - This means the map is not fully broken at package-load level.

2. `/Game/Istanbul` has direct missing dependencies under `/Game/PostApocBuildings`.
   - Confirmed in log output while loading the map.
   - Examples:
     - `/Game/PostApocBuildings/09_-_Default`
     - `/Game/PostApocBuildings/Building_12_001`
     - `/Game/PostApocBuildings/Building_12_064`
     - `/Game/PostApocBuildings/Building_12_082`
     - `/Game/PostApocBuildings/Building_12_090`
     - `/Game/PostApocBuildings/Building_12_145`
     - `/Game/PostApocBuildings/Building_12_219`
     - `/Game/PostApocBuildings/Building_12_243`
     - `/Game/PostApocBuildings/Building_12_246`
     - `/Game/PostApocBuildings/Building_12_250`
     - `/Game/PostApocBuildings/Building_12_258`
     - `/Game/PostApocBuildings/Building_12_285`
     - `/Game/PostApocBuildings/Building_12_288`
     - `/Game/PostApocBuildings/Building_12_297`

3. A generated content path also still references deleted PostApocBuildings assets.
   - `/Game/_GENERATED/Hazar/4Walls_21467C67`
   - `/Game/_GENERATED/Hazar/Suburb_FoundationPre__B0CEA658`

4. There is a separate PCG breakage in the map.
   - `PCGVolume_5` is trying to read actor properties that do not exist on `PCGVolume`.
   - Examples:
     - `CurrentSettings`
     - `GlobalTreesDensity`
     - `GlobalRocksDensity`
     - `GlobalDeadTreesDensity`
     - `GlobalLargeCliffsAreaScale`
     - `GlobalLargeCliffsScaleModifier`
     - `GlobalLargeCliffsScaleModifierRatio`
     - `GlobalLargeCliffsSeed`
     - `GlobalLargeCliffsDensity`
     - `GlobalTreesScale`

5. No Blueprint compile failure was observed for the main gameplay chain during PIE startup.
   - PIE log reports: `No blueprints needed recompiling`
   - This is useful, but it does not clear asset-level runtime issues.

### Practical Meaning

- The current big diff is valid as an intentional content reduction, but `/Game/Istanbul` still contains references to removed building assets.
- The map can open, yet some placed actors, generated geometry, or materials may be missing visually.
- The PCG error is independent and should be treated as an active map logic bug.

### Recommended Next Fix Order

1. Open `/Game/Istanbul` and clean or replace all missing `/Game/PostApocBuildings/*` references.
2. Inspect generated assets under `/Game/_GENERATED/Hazar/*` and regenerate or remove stale content.
3. Inspect `PCGVolume_5` and either:
   - replace it with the expected custom actor that owns those properties, or
   - remove/update the graph nodes that query missing actor properties.
4. Re-save the map and re-run load validation.
