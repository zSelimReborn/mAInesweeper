// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FOnBoardRequestCompletedDelegate, FString);
DECLARE_DELEGATE_OneParam(FOnBoardRequestFailedDelegate, FString);

/**
 * 
 */
class SWEEPERPLUGIN_API SMinesweeperPrompt : public SCompoundWidget
{
public:
	static const FString GEMINI_PROMPT_BASE_URL;
	
	SLATE_BEGIN_ARGS(SMinesweeperPrompt) {}
		SLATE_EVENT(FOnBoardRequestCompletedDelegate, OnBoardRequestCompleted)
		SLATE_EVENT(FOnBoardRequestFailedDelegate, OnBoardRequestFailed)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	TSharedRef<IHttpRequest> BuildBoardRequest(const FString& Prompt);
	FString BuildRequestBody(const FString& Prompt) const;
	
	FReply OnPromptButtonClick();

	void OnBoardRequestCompletedCallback(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	static FString ClearResponse(FString Response);

	FString GetGeminiApiKey() const;

private:
	TSharedPtr<SEditableText> PromptEditableText;
	FString CurrentPromptText;

	FOnBoardRequestCompletedDelegate OnBoardRequestCompleted;
	FOnBoardRequestFailedDelegate OnBoardRequestFailed;
};
