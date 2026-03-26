#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Texture2D.h"
#include "Animation/AnimBlueprint.h"
#include "EnemyNPCTypes.generated.h"

UENUM(BlueprintType)
enum class EEnemyNPCType : uint8
{
	Scavenger UMETA(DisplayName = "Scavenger"),
	Raider UMETA(DisplayName = "Raider"),
	Guard UMETA(DisplayName = "Guard"),
	Elite UMETA(DisplayName = "Elite")
};

UENUM(BlueprintType)
enum class ENPCFaction : uint8
{
	Player UMETA(DisplayName = "Player"),
	Scavengers UMETA(DisplayName = "Scavengers"),
	Raiders UMETA(DisplayName = "Raiders"),
	Merchants UMETA(DisplayName = "Merchants"),
	Neutrals UMETA(DisplayName = "Neutrals")
};

UENUM(BlueprintType)
enum class EEnemyThreatTier : uint8
{
	Low UMETA(DisplayName = "Low"),
	Medium UMETA(DisplayName = "Medium"),
	High UMETA(DisplayName = "High")
};

USTRUCT(BlueprintType)
struct GERCEK_API FEnemyNPCRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName NPCId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	EEnemyNPCType EnemyType = EEnemyNPCType::Scavenger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	ENPCFaction Faction = ENPCFaction::Scavengers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	EEnemyThreatTier ThreatTier = EEnemyThreatTier::Low;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TSoftObjectPtr<USkeletalMesh> CharacterMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TSoftClassPtr<UAnimInstance> AnimBlueprintClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TSoftObjectPtr<UTexture2D> PortraitIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0"))
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0"))
	float AttackRange = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.1"))
	float AttackCooldown = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float WalkSpeed = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = "0.0"))
	float SprintSpeed = 420.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception", meta = (ClampMin = "0.0"))
	float SightRadius = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception", meta = (ClampMin = "0.0"))
	float LoseSightRadius = 1600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception", meta = (ClampMin = "0.0"))
	float HearingRadius = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception", meta = (ClampMin = "0.0"))
	float AggroRange = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception", meta = (ClampMin = "0.0"))
	float LeashDistance = 2500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	FName LootTableId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "0"))
	int32 DropCurrencyMin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "0"))
	int32 DropCurrencyMax = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Usage")
	bool bCanSpawnInCityCenter = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Usage")
	bool bCanSpawnInIndustrial = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Usage")
	bool bCanSpawnInSuburb = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Usage")
	bool bQuestRelated = false;
};
