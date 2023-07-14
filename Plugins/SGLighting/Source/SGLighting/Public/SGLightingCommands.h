// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "SGLightingStyle.h"

class FSGLightingCommands : public TCommands<FSGLightingCommands>
{
public:

	FSGLightingCommands()
		: TCommands<FSGLightingCommands>(TEXT("SGLighting"), NSLOCTEXT("Contexts", "SGLighting", "SGLighting Plugin"), NAME_None, FSGLightingStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > BakeSGLightingAction;
	TSharedPtr< FUICommandInfo > CleanSGLightingAction;
};
