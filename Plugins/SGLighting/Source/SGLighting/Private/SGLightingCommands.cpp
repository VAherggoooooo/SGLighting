// Copyright Epic Games, Inc. All Rights Reserved.

#include "SGLightingCommands.h"

#define LOCTEXT_NAMESPACE "FSGLightingModule"

void FSGLightingCommands::RegisterCommands()
{
	UI_COMMAND(BakeSGLightingAction, "Bake SG Lighting", "Execute Bake SG Lighting action", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(CleanSGLightingAction, "Clean SG Lighting", "Execute Clean SG Lighting action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
