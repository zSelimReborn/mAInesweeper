// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "SweeperPluginStyle.h"

class FSweeperPluginCommands : public TCommands<FSweeperPluginCommands>
{
public:

	FSweeperPluginCommands();

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenMinesweeperWindow;
};