// Copyright Epic Games, Inc. All Rights Reserved.

#include "SweeperPluginCommands.h"

#define LOCTEXT_NAMESPACE "FSweeperPluginModule"

FSweeperPluginCommands::FSweeperPluginCommands()
	: TCommands<FSweeperPluginCommands>(TEXT("SweeperPlugin"), NSLOCTEXT("Contexts", "SweeperPlugin", "SweeperPlugin Plugin"), NAME_None, FSweeperPluginStyle::GetStyleSetName())
{
}

void FSweeperPluginCommands::RegisterCommands()
{
	UI_COMMAND(OpenMinesweeperWindow, "SweeperPlugin", "Open Minesweeper", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
