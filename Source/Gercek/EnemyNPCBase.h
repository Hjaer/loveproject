#pragma once

#include "CoreMinimal.h"
#include "EnemyNPCTypes.h"
#include "GameFramework\Character.h"
#include "EnemyNPCBase.generated.h"

class UDataTable;

UCLASS()
class GERCEK_API AEnemyNPCBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyNPCBase();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Enemy NPC")
	bool ApplyEnemyDataFromRow();

	const FEnemyNPCRow* GetEnemyDataRow() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy NPC")
	TObjectPtr<UDataTable> EnemyDataTable = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy NPC")
	FName EnemyRowName;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy NPC")
	FEnemyNPCRow CachedEnemyData;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy NPC")
	bool bHasCachedEnemyData = false;
};
