// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

struct FMinesweeperCell
{
	bool bIsBomb;
	bool bDiscovered;
	int BombCount;
	
	FMinesweeperCell(bool _bIsBomb);
	bool IsBomb() const;
	bool IsEmpty() const;
	bool IsDiscovered() const;
	void Discover();
	void IncrementBombCount();
	int GetCount() const;
};

struct FMinesweeperBoard
{
	typedef TArray<TArray<FMinesweeperCell>> Board;
	
	Board InnerBoard;
	int RowCount;
	int ColCount;
	int CellToDiscover;
	
	FMinesweeperBoard();
	void Create(const FString& BoardText);
	int Rows() const;
	int Cols() const;
	bool IsDiscovered(const int Row, const int Column) const;
	void Discover(const int Row, const int Column);
	bool Exists(const int Row, const int Column) const;
	bool HasWon() const;
	FMinesweeperCell operator()(const int Row, const int Column) const;
	FMinesweeperCell& operator()(const int Row, const int Column);
};

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

	FReply OnGridButtonClicked(int ButtonId, int Row, int Col);

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<class FUICommandList> PluginCommands;

	FMinesweeperBoard Board;
	TMap<int, bool> ButtonMapEnabled;
	TMap<int, FText> ButtonMapText;
	TMap<int, TSharedRef<SButton>> Buttons;
};
