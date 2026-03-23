#include "GercekPCGVolume.h"

AGercekPCGVolume::AGercekPCGVolume()
{
	CurrentSettings = nullptr;
	GlobalTreesDensity = 1.0f;
	GlobalRocksDensity = 1.0f;
	GlobalDeadTreesDensity = 1.0f;
	GlobalLargeCliffsAreaScale = 1.0f;
	GlobalLargeCliffsScaleModifier = 1.0f;
	GlobalLargeCliffsScaleModifierRatio = 1.0f;
	GlobalLargeCliffsSeed = 1337;
	GlobalLargeCliffsDensity = 1.0f;
	GlobalTreesScale = 1.0f;
}
