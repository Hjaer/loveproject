#pragma once

#include "Commandlets/Commandlet.h"
#include "InventoryWidgetRepairCommandlet.generated.h"

class UBlueprint;
class UClass;

UCLASS()
class GERCEK_API UInventoryWidgetRepairCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UInventoryWidgetRepairCommandlet();

	virtual int32 Main(const FString& Params) override;

private:
	bool RepairBlueprintAsset(const FString& AssetPath, UClass* DesiredParentClass,
		bool bClearGraphs) const;
	bool SaveBlueprintAsset(UBlueprint* Blueprint) const;
	void ClearBlueprintGraphs(UBlueprint* Blueprint) const;
};
