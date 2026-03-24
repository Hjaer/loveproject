#include "InventoryWidgetRepairCommandlet.h"

#include "BlueprintEditorLibrary.h"
#include "Blueprint/UserWidget.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "Engine/Blueprint.h"
#include "FileHelpers.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/PackageName.h"
#include "PostApocGridDragDropOperation.h"
#include "PostApocGridItem.h"
#include "PostApocInventoryGridWidget.h"

UInventoryWidgetRepairCommandlet::UInventoryWidgetRepairCommandlet()
{
	LogToConsole = true;
	IsClient = false;
	IsEditor = true;
	IsServer = false;
}

int32 UInventoryWidgetRepairCommandlet::Main(const FString& Params)
{
	UE_LOG(LogTemp, Display, TEXT("[InventoryWidgetRepair] Starting repair pass."));

	int32 FailureCount = 0;

	if (!RepairBlueprintAsset(TEXT("/Game/Gercek/Inventory/WBP_InventoryGrid"),
		UPostApocInventoryGridWidget::StaticClass(), true))
	{
		++FailureCount;
	}

	if (!RepairBlueprintAsset(TEXT("/Game/Gercek/Inventory/WBP_GridItem"),
		UPostApocGridItem::StaticClass(), true))
	{
		++FailureCount;
	}

	if (!RepairBlueprintAsset(TEXT("/Game/Gercek/Inventory/DDO_GridItem"),
		UPostApocGridDragDropOperation::StaticClass(), true))
	{
		++FailureCount;
	}

	UE_LOG(LogTemp, Display,
		TEXT("[InventoryWidgetRepair] Repair pass complete. Failures=%d"),
		FailureCount);

	return FailureCount == 0 ? 0 : 1;
}

bool UInventoryWidgetRepairCommandlet::RepairBlueprintAsset(
	const FString& AssetPath, UClass* DesiredParentClass,
	const bool bClearGraphs) const
{
	if (!DesiredParentClass)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[InventoryWidgetRepair] Missing desired parent class for %s"),
			*AssetPath);
		return false;
	}

	UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *AssetPath);
	if (!Blueprint)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[InventoryWidgetRepair] Failed to load blueprint asset %s"),
			*AssetPath);
		return false;
	}

	bool bModified = false;

	if (Blueprint->ParentClass != DesiredParentClass)
	{
		UE_LOG(LogTemp, Display,
			TEXT("[InventoryWidgetRepair] Reparenting %s -> %s"),
			*AssetPath, *DesiredParentClass->GetName());
		UBlueprintEditorLibrary::ReparentBlueprint(Blueprint, DesiredParentClass);
		bModified = true;
	}

	if (bClearGraphs)
	{
		ClearBlueprintGraphs(Blueprint);
		bModified = true;
	}

	if (bModified)
	{
		FKismetEditorUtilities::CompileBlueprint(
			Blueprint, EBlueprintCompileOptions::SkipGarbageCollection);
	}

	return SaveBlueprintAsset(Blueprint);
}

void UInventoryWidgetRepairCommandlet::ClearBlueprintGraphs(
	UBlueprint* Blueprint) const
{
	if (!Blueprint)
	{
		return;
	}

	TArray<UEdGraph*> Graphs;
	Graphs.Append(Blueprint->UbergraphPages);
	Graphs.Append(Blueprint->FunctionGraphs);
	Graphs.Append(Blueprint->MacroGraphs);
	Graphs.Append(Blueprint->DelegateSignatureGraphs);

	for (UEdGraph* Graph : Graphs)
	{
		if (!Graph)
		{
			continue;
		}

		TArray<UEdGraphNode*> Nodes = Graph->Nodes;
		for (UEdGraphNode* Node : Nodes)
		{
			if (!Node)
			{
				continue;
			}

			FBlueprintEditorUtils::RemoveNode(Blueprint, Node, true);
		}
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
}

bool UInventoryWidgetRepairCommandlet::SaveBlueprintAsset(UBlueprint* Blueprint) const
{
	if (!Blueprint)
	{
		return false;
	}

	UPackage* Package = Blueprint->GetOutermost();
	if (!Package)
	{
		return false;
	}

	TArray<UPackage*> PackagesToSave;
	PackagesToSave.Add(Package);

	const bool bSaved = UEditorLoadingAndSavingUtils::SavePackages(
		PackagesToSave, /*bOnlyDirty*/false);

	if (!bSaved)
	{
		UE_LOG(LogTemp, Error,
			TEXT("[InventoryWidgetRepair] Failed to save package %s"),
			*Package->GetName());
	}

	return bSaved;
}
