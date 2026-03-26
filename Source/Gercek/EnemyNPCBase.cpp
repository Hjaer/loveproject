#include "EnemyNPCBase.h"

#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyNPCBase::AEnemyNPCBase()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnemyNPCBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyEnemyDataFromRow();
}

void AEnemyNPCBase::BeginPlay()
{
	Super::BeginPlay();
	ApplyEnemyDataFromRow();
}

const FEnemyNPCRow* AEnemyNPCBase::GetEnemyDataRow() const
{
	if (!EnemyDataTable || EnemyRowName.IsNone())
	{
		return nullptr;
	}

	return EnemyDataTable->FindRow<FEnemyNPCRow>(
		EnemyRowName, TEXT("AEnemyNPCBase::GetEnemyDataRow"));
}

bool AEnemyNPCBase::ApplyEnemyDataFromRow()
{
	const FEnemyNPCRow* EnemyRow = GetEnemyDataRow();
	if (!EnemyRow)
	{
		bHasCachedEnemyData = false;
		return false;
	}

	CachedEnemyData = *EnemyRow;
	bHasCachedEnemyData = true;

	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		if (!EnemyRow->CharacterMesh.IsNull())
		{
			if (USkeletalMesh* LoadedMesh = EnemyRow->CharacterMesh.LoadSynchronous())
			{
				MeshComponent->SetSkeletalMesh(LoadedMesh);
			}
		}

		if (!EnemyRow->AnimBlueprintClass.IsNull())
		{
			if (UClass* LoadedAnimClass = EnemyRow->AnimBlueprintClass.LoadSynchronous())
			{
				MeshComponent->SetAnimInstanceClass(LoadedAnimClass);
			}
		}
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = EnemyRow->WalkSpeed;
	}

	return true;
}
