// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;
class FUICommandList;
class FSpawnTabArgs;

class FSweeperPluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	void MinesweeperButtonClicked();

// Callbacks
private:
	void RegisterMinesweeperButton();
	TSharedRef<SDockTab> OnSpawnMinesweeperTab(const FSpawnTabArgs& SpawnTabArgs);

// Properties
private:
	TSharedPtr<FUICommandList> PluginCommands;
};
