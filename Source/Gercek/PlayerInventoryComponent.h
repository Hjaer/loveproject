#pragma once

#include "CoreMinimal.h"
#include "PostApocInventoryTypes.h"
#include "PlayerInventoryComponent.generated.h"

UCLASS(ClassGroup = (PostApoc), meta = (BlueprintSpawnableComponent))
class GERCEK_API UPlayerInventoryComponent : public UPostApocInventoryComponent {
  GENERATED_BODY()

public:
  UPlayerInventoryComponent();
};
