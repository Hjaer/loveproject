#pragma once

#include "CoreMinimal.h"
#include "PCGVolume.h"
#include "GercekPCGVolume.generated.h"

/**
 * Project-specific PCG volume shim.
 * Keeps legacy forest graph property lookups alive after content cleanup.
 */
UCLASS(BlueprintType)
class GERCEK_API AGercekPCGVolume : public APCGVolume
{
	GENERATED_BODY()

public:
	AGercekPCGVolume();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Legacy")
	TObjectPtr<UObject> CurrentSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Legacy", meta = (ClampMin = "0.0"))
	float GlobalTreesDensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Legacy", meta = (ClampMin = "0.0"))
	float GlobalRocksDensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Legacy", meta = (ClampMin = "0.0"))
	float GlobalDeadTreesDensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Legacy", meta = (ClampMin = "0.0"))
	float GlobalLargeCliffsAreaScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Legacy", meta = (ClampMin = "0.0"))
	float GlobalLargeCliffsScaleModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Legacy", meta = (ClampMin = "0.0"))
	float GlobalLargeCliffsScaleModifierRatio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Legacy")
	int32 GlobalLargeCliffsSeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Legacy", meta = (ClampMin = "0.0"))
	float GlobalLargeCliffsDensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Legacy", meta = (ClampMin = "0.0"))
	float GlobalTreesScale;
};
