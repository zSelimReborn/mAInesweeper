// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SMinesweeperBoard.h"

#include "SlateOptMacros.h"
#include "Widgets/Layout/SGridPanel.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

FMinesweeperCell::FMinesweeperCell(bool _bIsBomb)
	: bIsBomb(_bIsBomb), bDiscovered(false), BombCount(0), Text(FText::GetEmpty())
{
}

bool FMinesweeperCell::IsBomb() const
{
	return bIsBomb;
}

bool FMinesweeperCell::IsDiscovered() const
{
	return bDiscovered;
}

void FMinesweeperCell::Discover()
{
	bDiscovered = true;

	const FString TextString = IsBomb()? TEXT("X") : FString::Printf(TEXT("%d"), BombCount);
	Text = FText::FromString(TextString);
}

void FMinesweeperCell::IncrementBombCount()
{
	BombCount++;
}

bool FMinesweeperCell::IsEmpty() const
{
	return BombCount == 0;
}

int32 FMinesweeperCell::GetCount() const
{
	return BombCount;
}

FText FMinesweeperCell::GetText() const
{
	return Text;
}

FMinesweeperBoard::FMinesweeperBoard()
	: RowCount(0), ColCount(0), CellToDiscover(0), TotalBombCount(0)
{
	InnerBoard.Empty();
}

void FMinesweeperBoard::Create(const FString& BoardText)
{
	CellToDiscover = 0;
	InnerBoard.Empty();

	TArray<Coordinate> BombIndexes;
	TArray<FString> Rows;
	BoardText.ParseIntoArray(Rows, TEXT("|"), true);

	// Parsing and board creation
	RowCount = Rows.Num();
	for (int32 i = 0; i < Rows.Num(); ++i)
	{
		TArray<FMinesweeperCell> NewRow;
		const FString RowString = Rows[i];
		
		TArray<FString> Elements;
		RowString.ParseIntoArray(Elements, TEXT(","), true);
		ColCount = Elements.Num();
		
		for (int32 j = 0; j < ColCount; ++j)
		{
			const FString Element = Elements[j];
			bool bIsBomb = Element.Equals("1");
			FMinesweeperCell Cell(bIsBomb);
			NewRow.Add(Cell);

			if (bIsBomb)
			{
				BombIndexes.Add(Coordinate(i, j));
				TotalBombCount++;
			}
			else
			{
				CellToDiscover++;
			}
		}

		InnerBoard.Add(NewRow);
	}

	// Bomb counting
	for (const Coordinate& BombIndex : BombIndexes)
	{
		for (const Coordinate& Around : GetAroundOffset())
		{
			const int32 Row = BombIndex.Key + Around.Key;
			const int32 Col = BombIndex.Value + Around.Value;
			if (Exists(Row, Col))
			{
				InnerBoard[Row][Col].IncrementBombCount();
			}
		}
	}

	// Debug logging
	UE_LOG(LogSlate, Display, TEXT("[MineSweeper] - Created board. Rows: %d | Cols: %d | CellToDiscover: %d | BombCount: %d"), RowCount, ColCount, CellToDiscover, TotalBombCount);
	UE_LOG(LogSlate, Display, TEXT("[MineSweeper] - Original string: %s"), *BoardText);
	for (int32 i = 0; i < RowCount; ++i)
	{
		FString RowPrint;
		for (int32 j = 0; j < ColCount; ++j)
		{
			FString ElementString = InnerBoard[i][j].IsBomb() ? TEXT("x") : FString::Printf(TEXT("%d"), InnerBoard[i][j].GetCount());
			if (j != ColCount - 1)
			{
				ElementString.Append(TEXT(","));
			}
			RowPrint.Append(ElementString);
		}

		UE_LOG(LogSlate, Display, TEXT("[MineSweeper] - [%s]"), *RowPrint);
	}
}

int32 FMinesweeperBoard::Rows() const
{
	return RowCount;
}

int32 FMinesweeperBoard::Cols() const
{
	return ColCount;
}

int32 FMinesweeperBoard::GetTotalBombCount() const
{
	return TotalBombCount;
}

bool FMinesweeperBoard::IsDiscovered(const int32 Row, const int32 Column) const
{
	if (InnerBoard.IsValidIndex(Row) && InnerBoard[Row].IsValidIndex(Column))
	{
		return InnerBoard[Row][Column].IsDiscovered();
	}

	return false;
}

bool FMinesweeperBoard::IsDiscovered(const int32 Index) const
{
	const int32 Row = Index / ColCount;
	const int32 Column = Index % ColCount;

	return IsDiscovered(Row, Column);
}

bool FMinesweeperBoard::IsBomb(const int32 Index) const
{
	const int32 Row = Index / ColCount;
	const int32 Column = Index % ColCount;

	return IsBomb(Row, Column);
}

bool FMinesweeperBoard::Exists(const int32 Index) const
{
	const int32 Row = Index / ColCount;
	const int32 Column = Index % ColCount;

	return Exists(Row, Column);
}

FText FMinesweeperBoard::GetCellText(const int32 Row, const int32 Column) const
{
	if (InnerBoard.IsValidIndex(Row) && InnerBoard[Row].IsValidIndex(Column))
	{
		return InnerBoard[Row][Column].GetText();
	}

	return FText::GetEmpty();
}

FText FMinesweeperBoard::GetCellText(const int32 Index) const
{
	const int32 Row = Index / ColCount;
	const int32 Column = Index % ColCount;
	return GetCellText(Row, Column);
}

TArray<FMinesweeperBoard::Coordinate> FMinesweeperBoard::GetAroundOffset()
{
	static TArray<Coordinate> Around{
		{-1, -1}, //TopLeft
		{-1, 0}, //Top
		{-1, 1}, //TopRight
		{0, 1}, //Right
		{1, 1}, //BottomRight
		{1, 0}, //Bottom
		{1, -1}, //BottomLeft
		{0, -1}, //Left
	};

	return Around;
}


bool FMinesweeperBoard::IsBomb(const int32 Row, const int32 Column) const
{
	if (InnerBoard.IsValidIndex(Row) && InnerBoard[Row].IsValidIndex(Column))
	{
		return InnerBoard[Row][Column].IsBomb();
	}

	return false;
}

TArray<int32> FMinesweeperBoard::Discover(const int32 Row, const int32 Column)
{
	TArray<int32> Discovered;
	if (!InnerBoard.IsValidIndex(Row) || !InnerBoard[Row].IsValidIndex(Column) || InnerBoard[Row][Column].IsDiscovered())
	{
		return Discovered;	
	}

	if (InnerBoard[Row][Column].IsBomb())
	{
		InnerBoard[Row][Column].Discover();
		Discovered.Add(Row * RowCount + Column);
		return Discovered;
	}

	// Recursively discover empty point on board
	TQueue<Coordinate> ToDiscover;
	ToDiscover.Enqueue(Coordinate(Row, Column));
	while (!ToDiscover.IsEmpty())
	{
		Coordinate CurrentCoordinate;
		if (ToDiscover.Dequeue(CurrentCoordinate))
		{
			FMinesweeperCell& Cell = InnerBoard[CurrentCoordinate.Key][CurrentCoordinate.Value];
			if (!Cell.IsBomb())
			{
				Cell.Discover();

				const int32 CellIndex = CurrentCoordinate.Key * RowCount + CurrentCoordinate.Value;
				Discovered.AddUnique(CellIndex);
				
				if (!Cell.IsEmpty())
				{
					continue;
				}
					
				for (const Coordinate& AdjacentOffset : GetAroundOffset())
				{
					const int32 AdjacentRow = AdjacentOffset.Key + CurrentCoordinate.Key;
					const int32 AdjacentCol = AdjacentOffset.Value + CurrentCoordinate.Value;
					if (Exists(AdjacentRow, AdjacentCol)
						&& !IsDiscovered(AdjacentRow, AdjacentCol)
						&& !IsBomb(AdjacentRow, AdjacentCol)
					) {
						ToDiscover.Enqueue(Coordinate(AdjacentRow, AdjacentCol));
					}
				}
			}
		}
	}

	CellToDiscover = FMath::Max(0, CellToDiscover - Discovered.Num());
	return Discovered;
}

TArray<int32> FMinesweeperBoard::Reveal()
{
	TArray<int32> Revealed;
	for (int32 i = 0; i < RowCount; ++i)
	{
		for (int32 j = 0; j < ColCount; ++j)
		{
			if (!IsDiscovered(i, j))
			{
				InnerBoard[i][j].Discover();
				Revealed.Add(i * RowCount + j);
			}
		}
	}

	return Revealed;
}

bool FMinesweeperBoard::Exists(const int32 Row, const int32 Column) const
{
	return InnerBoard.IsValidIndex(Row) && InnerBoard[Row].IsValidIndex(Column);
}

bool FMinesweeperBoard::HasWon() const
{
	return CellToDiscover <= 0;
}

FMinesweeperCell FMinesweeperBoard::operator()(const int32 Row, const int32 Column) const
{
	return InnerBoard[Row][Column];
}

FMinesweeperCell& FMinesweeperBoard::operator()(const int32 Row, const int32 Column)
{
	return InnerBoard[Row][Column];
}

FMinesweeperCell& FMinesweeperBoard::operator()(const int32 Index)
{
	const int32 Row = Index / ColCount;
	const int32 Column = Index % ColCount;

	return InnerBoard[Row][Column];
}

FMinesweeperCell FMinesweeperBoard::operator()(const int32 Index) const
{
	const int32 Row = Index / ColCount;
	const int32 Column = Index % ColCount;

	return InnerBoard[Row][Column];
}

void SMinesweeperBoard::Construct(const FArguments& InArgs)
{
	OnGameOver = InArgs._OnGameOver;
	OnGameWin = InArgs._OnGameWin;
	
	GridPanel = SNew(SGridPanel);
	ChildSlot
	[
		GridPanel.ToSharedRef()
	];
}

void SMinesweeperBoard::BuildFromString(const FString& BoardText)
{
	CurrentBoardText = BoardText;
	BoardModel.Create(CurrentBoardText);
	PopulateGrid();
}

void SMinesweeperBoard::Rebuild()
{
	if (CurrentBoardText.IsEmpty())
	{
		return;
	}
	
	BuildFromString(CurrentBoardText);
}

FString SMinesweeperBoard::GetCurrentBoardText() const
{
	return CurrentBoardText;
}

void SMinesweeperBoard::PopulateGrid()
{
	Buttons.Empty();
	GridPanel->ClearChildren();
	int32 ButtonId = 0;
	for (int32 i = 0; i < BoardModel.Rows(); ++i)
	{
		for (int32 j = 0; j < BoardModel.Cols(); ++j)
		{
			TSharedRef<SButton> Button = CreateButton(ButtonId, i, j);

			Buttons.Add(ButtonId, Button);
			GridPanel->AddSlot(j, i)
			[
				SNew(SBox)
				.WidthOverride(50)
				.HeightOverride(50)
				[
					Button
				]
			];

			ButtonId++;
		}
	}
}

TSharedRef<SButton> SMinesweeperBoard::CreateButton(int32 ButtonId, int32 Row, int32 Column)
{
	TSharedRef<SButton> Button = SNew(SButton)
		.OnClicked_Raw(this, &SMinesweeperBoard::OnGridButtonClick, ButtonId, Row, Column)
		.IsEnabled_Lambda([this, ButtonId]() { return !BoardModel.IsDiscovered(ButtonId); })
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Justification(ETextJustify::Center)
			.Text_Lambda([this, ButtonId]() { return BoardModel.GetCellText(ButtonId); })
		]
	];

	Buttons.Add(ButtonId, Button);
	return Button;
}

FReply SMinesweeperBoard::OnGridButtonClick(int32 ButtonId, int32 Row, int32 Col)
{
	TSet<int32> UpdateButtons;
	
	const FMinesweeperCell StartingCell = BoardModel(Row, Col);
	if (!BoardModel.IsBomb(Row, Col))
	{
		const TArray<int32> DiscoveredIds = BoardModel.Discover(Row, Col);
		UpdateButtons.Append(DiscoveredIds);
		
		if (BoardModel.HasWon())
		{
			const TArray<int32> RevealedIds = BoardModel.Reveal();
			UpdateButtons.Append(RevealedIds);
			OnGameWin.ExecuteIfBound();
		}
	}
	else
	{
		// Reveal Board
		const TArray<int32> RevealedIds = BoardModel.Reveal();
		UpdateButtons.Append(RevealedIds);
		OnGameOver.ExecuteIfBound();
	}

	for (const int32& Id : UpdateButtons)
	{
		if (Buttons.Contains(Id))
		{
			Buttons[Id]->Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
		}
	}
	
	return FReply::Handled();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
