// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SMinesweeperTab.h"

#include "SlateOptMacros.h"
#include "Dialog/SCustomDialog.h"
#include "Widgets/SMinesweeperBoard.h"
#include "Widgets/SMinesweeperPrompt.h"

#define LOCTEXT_NAMESPACE "FSweeperPluginModule"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SMinesweeperTab::Construct(const FArguments& InArgs)
{
	FText HintText = LOCTEXT("SweeperPromptHint", "Waiting your mAInesweeper request...");

	SDockTab::Construct(SDockTab::FArguments()
	.TabRole(ETabRole::NomadTab)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillWidth(0.5f)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.VAlign(VAlign_Fill)
				.FillHeight(0.1f)
				[
					SNew(SBox)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					.Padding(5)
					[
						SNew(SButton)
						.OnClicked_Raw(this, &SMinesweeperTab::OnPlayAgainClick)
						.Visibility_Lambda([this]() { return MinesweeperBoard->GetCurrentBoardText().IsEmpty()? EVisibility::Collapsed : EVisibility::Visible; })
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("PlayAgainButtonText", "Play Again"))
								.Justification(ETextJustify::Center)
							]
						]
					]
				]
				+SVerticalBox::Slot()
				.VAlign(VAlign_Fill)
				.FillHeight(0.9f)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(5)
					[
						SNew(SBox)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						[
							SAssignNew(MinesweeperBoard, SMinesweeperBoard)
							.OnGameOver_Raw(this, &SMinesweeperTab::OnGameOver)
							.OnGameWin_Raw(this, &SMinesweeperTab::OnGameWin)
						]
					]
				]
			]
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillWidth(0.5f)
			[
				SNew(SBox)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.Padding(5)
				[
					SAssignNew(MinesweeperPrompt, SMinesweeperPrompt)
					.OnBoardRequestCompleted_Raw(this, &SMinesweeperTab::OnBoardRequestCompleted)
				]
			]
		]
	);
}

FReply SMinesweeperTab::OnPlayAgainClick()
{
	MinesweeperBoard->Rebuild();
	return FReply::Handled();
}

void SMinesweeperTab::OnGameOver()
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

	GameOverDialog->ShowModal();
}

void SMinesweeperTab::OnGameWin()
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
}

void SMinesweeperTab::OnBoardRequestCompleted(FString BoardText)
{
	MinesweeperBoard->BuildFromString(BoardText);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE