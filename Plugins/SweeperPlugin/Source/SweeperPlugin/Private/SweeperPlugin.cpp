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
#include "Widgets/SMinesweeperBoard.h"
#include "Widgets/SMinesweeperPrompt.h"

static const FName SweeperPluginTabName("SweeperPlugin");

#define LOCTEXT_NAMESPACE "FSweeperPluginModule"

void FSweeperPluginModule::StartupModule()
{
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
	FText HintText = LOCTEXT("SweeperPromptHint", "Waiting your mAInesweeper request...");
	
	MinesweeperBoard = SNew(SMinesweeperBoard)
		.OnGameOver_Raw(this, &FSweeperPluginModule::OnGameOver)
		.OnGameWin_Raw(this, &FSweeperPluginModule::OnGameWin);

	MinesweeperPrompt = SNew(SMinesweeperPrompt)
		.OnBoardRequestCompleted_Raw(this, &FSweeperPluginModule::OnBoardRequestCompleted);
	
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.VAlign(VAlign_Fill)
			.FillHeight(0.5f)
			.Padding(10.f)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(10.f)
				[
					SNew(SBox)
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Center)
					[
						MinesweeperBoard.ToSharedRef()
					]
				]
				+SHorizontalBox::Slot()
				.Padding(10.f)
				[
					SNew(SBox)
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Top)
					[
						SNew(SButton)
						.OnClicked_Raw(this, &FSweeperPluginModule::OnPlayAgainClick)
						.IsEnabled_Lambda([this]() { return !MinesweeperBoard->GetCurrentBoardText().IsEmpty(); })
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
			]
			+SVerticalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillHeight(0.3f)
			[
				SNew(SBox)
				.Padding(10.f)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Bottom)
				[
					MinesweeperPrompt.ToSharedRef()
				]
			]
		];
}

void FSweeperPluginModule::OnGameOver()
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

void FSweeperPluginModule::OnGameWin()
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

void FSweeperPluginModule::OnBoardRequestCompleted(FString BoardText)
{
	MinesweeperBoard->BuildFromString(BoardText);
}

void FSweeperPluginModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(SweeperPluginTabName);
}

void FSweeperPluginModule::RegisterMenus()
{
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

FReply FSweeperPluginModule::OnPlayAgainClick()
{
	MinesweeperBoard->Rebuild();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSweeperPluginModule, SweeperPlugin)