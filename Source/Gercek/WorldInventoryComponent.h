#pragma once

#include "CoreMinimal.h"
#include "PostApocInventoryTypes.h"
#include "WorldInventoryComponent.generated.h"

UCLASS(ClassGroup = (PostApoc), meta = (BlueprintSpawnableComponent))
class GERCEK_API UWorldInventoryComponent : public UPostApocInventoryComponent {
  GENERATED_BODY()

public:
  UWorldInventoryComponent();
};
