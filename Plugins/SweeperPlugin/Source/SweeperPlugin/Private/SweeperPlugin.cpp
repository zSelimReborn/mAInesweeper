// Copyright Epic Games, Inc. All Rights Reserved.

#include "SweeperPlugin.h"

#include "IPropertyTable.h"
#include "SweeperPluginStyle.h"
#include "SweeperPluginCommands.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "Templates/SharedPointer.h"
#include "Widgets/SMinesweeperTab.h"

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
		FExecuteAction::CreateRaw(this, &FSweeperPluginModule::MinesweeperButtonClicked),
		FCanExecuteAction()
	);

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSweeperPluginModule::RegisterMinesweeperButton));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(SweeperPluginTabName, FOnSpawnTab::CreateRaw(this, &FSweeperPluginModule::OnSpawnMinesweeperTab))
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

TSharedRef<SDockTab> FSweeperPluginModule::OnSpawnMinesweeperTab(const FSpawnTabArgs& SpawnTabArgs)
{
	MinesweeperTab = SNew(SMinesweeperTab);
	return MinesweeperTab.ToSharedRef();
}

void FSweeperPluginModule::MinesweeperButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(SweeperPluginTabName);
}

void FSweeperPluginModule::RegisterMinesweeperButton()
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

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSweeperPluginModule, SweeperPlugin)