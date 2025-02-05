// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SGridPanel;

DECLARE_DELEGATE(FOnGameOverDelegate);
DECLARE_DELEGATE(FOnGameWinDelegate);

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

/**
 * 
 */
class SWEEPERPLUGIN_API SMinesweeperBoard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMinesweeperBoard) { }
		SLATE_EVENT(FOnGameOverDelegate, OnGameOver);
		SLATE_EVENT(FOnGameWinDelegate, OnGameWin);
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	void BuildFromString(const FString& BoardText);

private:
	void PopulateGrid();
	TSharedRef<SButton> CreateButton(int32 ButtonId, int32 Row, int32 Column);
	FReply OnGridButtonClick(int32 ButtonId, int32 Row, int32 Col);
	
// Properties
private:
	TSharedPtr<SGridPanel> GridPanel;
	TMap<int32, TSharedRef<SButton>> Buttons;

	FMinesweeperBoard BoardModel;

	FOnGameOverDelegate OnGameOver;
	FOnGameWinDelegate OnGameWin;
};
