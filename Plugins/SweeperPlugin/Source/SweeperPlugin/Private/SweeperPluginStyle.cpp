// Copyright Epic Games, Inc. All Rights Reserved.

#include "SweeperPluginStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FSweeperPluginStyle::StyleInstance = nullptr;

void FSweeperPluginStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FSweeperPluginStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FSweeperPluginStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("SweeperPluginStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FSweeperPluginStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("SweeperPluginStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("SweeperPlugin")->GetBaseDir() / TEXT("Resources"));

	Style->Set("SweeperPlugin.ToolBarButton", new IMAGE_BRUSH(TEXT("Bomb"), Icon20x20));

	return Style;
}

void FSweeperPluginStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FSweeperPluginStyle::Get()
{
	return *StyleInstance;
}
