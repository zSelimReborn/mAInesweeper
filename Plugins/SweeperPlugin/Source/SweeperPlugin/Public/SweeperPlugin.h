// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;
class FUICommandList;

class SMinesweeperBoard;
class SMinesweeperPrompt;
class SDockTab;

class FSweeperPluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
	
private:

	void RegisterMenus();

	FReply OnPlayAgainClick();

	void OnGameOver();
	void OnGameWin();

	TSharedRef<SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	void OnBoardRequestCompleted(FString BoardText);

private:
	TSharedPtr<FUICommandList> PluginCommands;
	TSharedPtr<SMinesweeperBoard> MinesweeperBoard;
	TSharedPtr<SMinesweeperPrompt> MinesweeperPrompt;
};
