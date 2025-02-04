// Copyright Epic Games, Inc. All Rights Reserved.

#include "SweeperPlugin.h"

#include "IPropertyTable.h"
#include "SweeperPluginStyle.h"
#include "SweeperPluginCommands.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "Dialog/SCustomDialog.h"
#include "Templates/SharedPointer.h"

static const FName SweeperPluginTabName("SweeperPlugin");

#define LOCTEXT_NAMESPACE "FSweeperPluginModule"

FMinesweeperCell::FMinesweeperCell(bool _bIsBomb)
	: bIsBomb(_bIsBomb), bDiscovered(false), BombCount(0)
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
}

void FMinesweeperCell::IncrementBombCount()
{
	BombCount++;
}

bool FMinesweeperCell::IsEmpty() const
{
	return BombCount == 0;
}

int FMinesweeperCell::GetCount() const
{
	return BombCount;
}

FMinesweeperBoard::FMinesweeperBoard()
	: RowCount(0), ColCount(0), CellToDiscover(0)
{
	InnerBoard.Empty();
}

void FMinesweeperBoard::Create(const FString& BoardText)
{
	InnerBoard.Empty();

	TArray<TPair<int, int>> BombIndexes;
	TArray<FString> Rows;
	BoardText.ParseIntoArray(Rows, TEXT("|"), true);

	RowCount = Rows.Num();
	for (int i = 0; i < Rows.Num(); ++i)
	{
		TArray<FMinesweeperCell> NewRow;
		const FString RowString = Rows[i];
		
		TArray<FString> Elements;
		RowString.ParseIntoArray(Elements, TEXT(","), true);
		ColCount = FMath::Max(Elements.Num(), ColCount);
		
		for (int j = 0; j < ColCount; ++j)
		{
			const FString Element = Elements[j];
			bool bIsBomb = Element.Equals("1");
			FMinesweeperCell Cell(bIsBomb);
			NewRow.Add(Cell);

			if (bIsBomb)
			{
				BombIndexes.Add(TPair<int, int>(i, j));
			}
			else
			{
				CellToDiscover++;
			}
		}

		InnerBoard.Add(NewRow);
	}


	TArray<TPair<int, int>> AroundBomb{
		{-1, -1}, //TopLeft
		{-1, 0}, //Top
		{-1, 1}, //TopRight
		{0, 1}, //Right
		{1, 1}, //BottomRight
		{1, 0}, //Bottom
		{1, -1}, //BottomLeft
		{0, -1}, //Left
	};
	for (const TPair<int, int>& BombIndex : BombIndexes)
	{
		for (const TPair<int, int>& Around : AroundBomb)
		{
			const int Row = BombIndex.Key + Around.Key;
			const int Col = BombIndex.Value + Around.Value;
			if (InnerBoard.IsValidIndex(Row) && InnerBoard[Row].IsValidIndex(Col))
			{
				InnerBoard[Row][Col].IncrementBombCount();
			}
		}
	}
	
	UE_LOG(LogTemp, Error, TEXT("[MineSweeper] - Created board. Rows: %d | Cols: %d | CellToDiscover: %d"), RowCount, ColCount, CellToDiscover);
	UE_LOG(LogTemp, Error, TEXT("[MineSweeper] - Original string: %s"), *BoardText);
	for (int i = 0; i < RowCount; ++i)
	{
		FString RowPrint;
		for (int j = 0; j < ColCount; ++j)
		{
			FString ElementString = InnerBoard[i][j].IsBomb() ? TEXT("x") : FString::Printf(TEXT("%d"), InnerBoard[i][j].GetCount());
			if (j != ColCount - 1)
			{
				ElementString.Append(TEXT(","));
			}
			RowPrint.Append(ElementString);
		}

		UE_LOG(LogTemp, Error, TEXT("[MineSweeper] - [%s]"), *RowPrint);
	}
}

int FMinesweeperBoard::Rows() const
{
	return RowCount;
}

int FMinesweeperBoard::Cols() const
{
	return ColCount;
}

bool FMinesweeperBoard::IsDiscovered(const int Row, const int Column) const
{
	if (InnerBoard.IsValidIndex(Row) && InnerBoard[Row].IsValidIndex(Column))
	{
		return InnerBoard[Row][Column].IsDiscovered();
	}

	return false;
}

void FMinesweeperBoard::Discover(const int Row, const int Column)
{
	if (InnerBoard.IsValidIndex(Row) && InnerBoard[Row].IsValidIndex(Column) && !InnerBoard[Row][Column].IsDiscovered())
	{
		CellToDiscover = FMath::Max(0, CellToDiscover - 1);
		return InnerBoard[Row][Column].Discover();
	}
}

bool FMinesweeperBoard::Exists(const int Row, const int Column) const
{
	return InnerBoard.IsValidIndex(Row) && InnerBoard[Row].IsValidIndex(Column);
}

bool FMinesweeperBoard::HasWon() const
{
	return CellToDiscover <= 0;
}

FMinesweeperCell FMinesweeperBoard::operator()(const int Row, const int Column) const
{
	return InnerBoard[Row][Column];
}

FMinesweeperCell& FMinesweeperBoard::operator()(const int Row, const int Column)
{
	return InnerBoard[Row][Column];
}

void FSweeperPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FSweeperPluginStyle::Initialize();
	FSweeperPluginStyle::ReloadTextures();

	FSweeperPluginCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FSweeperPluginCommands::Get().OpenMinesweeperWindow,
		FExecuteAction::CreateRaw(this, &FSweeperPluginModule::PluginButtonClicked),
		FCanExecuteAction()
	);

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSweeperPluginModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SweeperPluginTabName, FOnSpawnTab::CreateRaw(this, &FSweeperPluginModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FSweeperPluginTabTitle", "Minesweeper!"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FSweeperPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FSweeperPluginStyle::Shutdown();

	FSweeperPluginCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SweeperPluginTabName);
}

TSharedRef<SDockTab> FSweeperPluginModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FString Text = "0,0,0,1,0,0|0,1,0,0,0,0|0,0,0,0,0,1|0,0,0,0,1,0|0,0,1,0,0,0|0,0,0,0,0,0";
	Board.Create(Text);

	FText HintText = LOCTEXT("SweeperPromptHint", "Waiting your mAInesweeper request...");

	ButtonMapEnabled.Empty();
	ButtonMapText.Empty();
	Buttons.Empty();

	int ButtonId = 0;
	TSharedRef<SGridPanel> Grid = SNew(SGridPanel);
	for (int i = 0; i < Board.Rows(); ++i)
	{
		for (int j = 0; j < Board.Cols(); ++j)
		{
			ButtonMapEnabled.Add(ButtonId, true);
			ButtonMapText.Add(ButtonId, FText::GetEmpty());
			
			TSharedRef<SButton> Button = SNew(SButton)
					.OnClicked_Raw(this, &FSweeperPluginModule::OnGridButtonClicked, ButtonId, i, j)
					.IsEnabled_Lambda([this, ButtonId]() { return ButtonMapEnabled[ButtonId]; })
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text_Lambda([this, ButtonId]() { return ButtonMapText[ButtonId]; })
					]
				];

			Buttons.Add(ButtonId, Button);
			Grid->AddSlot(j, i)
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
	
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.Padding(10.f)
			[
				Grid
			]
			+SVerticalBox::Slot()
			[
				SNew(SBox)
				.Padding(10.f)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Bottom)
				[
					SNew(SEditableText)
					.HintText(HintText)
				]
			]
		];
}

FReply FSweeperPluginModule::OnGridButtonClicked(int ButtonId, int Row, int Col)
{
	// Make static
	TArray<TPair<int, int>> Around{
			{-1, -1}, //TopLeft
			{-1, 0}, //Top
			{-1, 1}, //TopRight
			{0, 1}, //Right
			{1, 1}, //BottomRight
			{1, 0}, //Bottom
			{1, -1}, //BottomLeft
			{0, -1}, //Left
		};
	
	const FMinesweeperCell StartingCell = Board(Row, Col);
	if (!StartingCell.IsBomb())
	{
		// Recursively discover empty point on board
		TQueue<TPair<int,int>> ToDiscover;
		ToDiscover.Enqueue(TPair<int,int>(Row, Col));
		while (!ToDiscover.IsEmpty())
		{
			TPair<int,int> Coordinate;
			if (ToDiscover.Dequeue(Coordinate))
			{
				const FMinesweeperCell Cell = Board(Coordinate.Key, Coordinate.Value);
				if (!Cell.IsBomb())
				{
					Board.Discover(Coordinate.Key, Coordinate.Value);
					const int CellButtonId = Coordinate.Key * Board.Rows() + Coordinate.Value;
					ButtonMapEnabled[CellButtonId] = false;

					FString CellButtonText = FString::Printf(TEXT("%d"), Board(Coordinate.Key, Coordinate.Value).GetCount()); 
					ButtonMapText[CellButtonId] = FText::FromString(CellButtonText);
					Buttons[CellButtonId]->Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
					if (!Cell.IsEmpty())
					{
						continue;
					}
					
					for (const TPair<int, int>& AdjacentOffset : Around)
					{
						const int AdjacentRow = AdjacentOffset.Key + Coordinate.Key;
						const int AdjacentCol = AdjacentOffset.Value + Coordinate.Value;
						if (Board.Exists(AdjacentRow, AdjacentCol)
							&& !Board(AdjacentRow, AdjacentCol).IsDiscovered()
							&& !Board(AdjacentRow, AdjacentCol).IsBomb()
						) {
							ToDiscover.Enqueue(TPair<int,int>(AdjacentRow, AdjacentCol));
						}
					}
				}
			}
		}

		UE_LOG(LogTemp, Error, TEXT("[Minesweeper] - CellToDiscover: %d"), Board.CellToDiscover);
		if (Board.HasWon())
		{
			TSharedRef<SCustomDialog> GameWonDialog = SNew(SCustomDialog)
			.Title(LOCTEXT("GameWonDialog", "Game Won!"))
			.Content()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GameWonText", "Congratulations! You won the game!"))
				.Justification(ETextJustify::Center)
			]
			.Buttons({
				SCustomDialog::FButton(LOCTEXT("CloseText", "Close")),
			});

			GameWonDialog->ShowModal();

			// TODO reveal board
		}
	}
	else
	{
		TSharedRef<SCustomDialog> GameOverDialog = SNew(SCustomDialog)
		.Title(LOCTEXT("GameOverDialog", "Game Over!"))
		.Content()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("GameOverText", "Game Over!"))
			.Justification(ETextJustify::Center)
		]
		.Buttons({
			SCustomDialog::FButton(LOCTEXT("CloseText", "Close")),
		});

		const int32 GameOverClosed = GameOverDialog->ShowModal();
		// Reveal Board
		for (const TTuple<int, TSharedRef<SButton>>& Button : Buttons)
		{
			int Id = Button.Key;
			ButtonMapEnabled[Id] = false;

			int CurrentRow = Id / Board.Rows();
			int CurrentCol = Id % Board.Cols();
			bool bIsBomb = Board(CurrentRow, CurrentCol).IsBomb();
			FString ButtonText = (bIsBomb)? TEXT("X") : FString::Printf(TEXT("%d"), Board(CurrentRow, CurrentCol).GetCount()); 
			ButtonMapText[Id] = FText::FromString(ButtonText);

			Buttons[Id]->Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
		}
	}

	return FReply::Handled();
}


void FSweeperPluginModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(SweeperPluginTabName);
}

void FSweeperPluginModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
		{
			FName ButtonName = TEXT("SweeperToolBarButton");
			FToolMenuSection& Section = Menu->FindOrAddSection("SweeperLayout");
			Section.AddEntry(FToolMenuEntry::InitToolBarButton(
				ButtonName,
				FToolUIActionChoice(FSweeperPluginCommands::Get().OpenMinesweeperWindow, *PluginCommands),
				LOCTEXT("SweeperToolBarButtonLabel", "Open MineSweeper"),
				LOCTEXT("SweeperToolBarButtonTooltip", "Open MineSweeper tab"),
				FSlateIcon(FSweeperPluginStyle::GetStyleSetName(), "SweeperPlugin.ToolBarButton")
			));
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSweeperPluginModule, SweeperPlugin)