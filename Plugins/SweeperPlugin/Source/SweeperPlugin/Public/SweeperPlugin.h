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
	int32 BombCount;
	FText Text;
	
	FMinesweeperCell(bool _bIsBomb);
	bool IsBomb() const;
	bool IsEmpty() const;
	bool IsDiscovered() const;
	void Discover();
	void IncrementBombCount();
	int32 GetCount() const;
	FText GetText() const;
};

struct FMinesweeperBoard
{
	typedef TArray<TArray<FMinesweeperCell>> Board;
	typedef TPair<int32, int32> Coordinate;
	
	Board InnerBoard;
	int32 RowCount;
	int32 ColCount;
	int32 CellToDiscover;
	int32 TotalBombCount;
	
	FMinesweeperBoard();
	void Create(const FString& BoardText);
	int32 Rows() const;
	int32 Cols() const;
	int32 GetTotalBombCount() const;
	bool IsDiscovered(const int32 Row, const int32 Column) const;
	bool IsDiscovered(const int32 Index) const;
	bool IsBomb(const int32 Row, const int32 Column) const;
	bool IsBomb(const int32 Index) const;
	TArray<int32> Discover(const int32 Row, const int32 Column);
	TArray<int32> Reveal();
	bool Exists(const int32 Row, const int32 Column) const;
	bool Exists(const int32 Index) const;
	FText GetCellText(const int32 Row, const int32 Column) const;
	FText GetCellText(const int32 Index) const;
	static TArray<Coordinate> GetAroundOffset();
	bool HasWon() const;
	FMinesweeperCell operator()(const int32 Row, const int32 Column) const;
	FMinesweeperCell& operator()(const int32 Row, const int32 Column);
	FMinesweeperCell operator()(const int32 Index) const;
	FMinesweeperCell& operator()(const int32 Index);
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

	FReply OnGridButtonClicked(int32 ButtonId, int32 Row, int32 Col);

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<class FUICommandList> PluginCommands;

	FMinesweeperBoard Board;
	TMap<int32, TSharedRef<SButton>> Buttons;
};
