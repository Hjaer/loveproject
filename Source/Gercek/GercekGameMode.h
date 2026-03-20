#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GercekGameMode.generated.h"

UCLASS()
class GERCEK_API AGercekGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGercekGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
};
