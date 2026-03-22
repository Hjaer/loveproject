#include "WorldInventoryComponent.h"

UWorldInventoryComponent::UWorldInventoryComponent() {
  SetIsReplicatedByDefault(true);
  SetIsReplicated(true);
}
