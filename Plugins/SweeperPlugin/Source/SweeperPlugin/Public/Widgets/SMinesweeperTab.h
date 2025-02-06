// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SMinesweeperPrompt;
class SMinesweeperBoard;

/**
 * 
 */
class SWEEPERPLUGIN_API SMinesweeperTab : public SDockTab
{
public:
	SLATE_BEGIN_ARGS(SMinesweeperTab) {	}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

// Callbacks
private:
	FReply OnPlayAgainClick();

	void OnGameOver();
	void OnGameWin();

	void OnBoardRequestCompleted(FString BoardText);

// Properties
private:
	TSharedPtr<SMinesweeperBoard> MinesweeperBoard;
	TSharedPtr<SMinesweeperPrompt> MinesweeperPrompt;
};
