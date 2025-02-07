// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_DELEGATE_OneParam(FOnBoardRequestCompletedDelegate, FString);
DECLARE_DELEGATE_OneParam(FOnBoardRequestFailedDelegate, FString);

struct FPromptMessage
{
	FText Content;
	bool bIsUser;

	FPromptMessage() : Content(FText::GetEmpty()), bIsUser(false) {}
	FPromptMessage(const FText& InContent, bool InIsUser) : Content(InContent), bIsUser(InIsUser) {}
};

/**
 * 
 */
class SWEEPERPLUGIN_API SMinesweeperPrompt : public SCompoundWidget
{
public:
	typedef TArray<TSharedPtr<FPromptMessage>> TPromptList;
	typedef SListView<TSharedPtr<FPromptMessage>> TPromptListWidget;

	static const FString NOT_RELATED_RESPONSE;
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
	void OnPromptCommit(const FText& PromptText, ETextCommit::Type CommitType);
	bool HandlePrompt();

	void OnBoardRequestCompletedCallback(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	static FString ClearResponse(FString Response);

	TSharedRef<ITableRow> OnGenerateChatRow(TSharedPtr<FPromptMessage> Message, const TSharedRef<STableViewBase>& Owner); 

	FString GetGeminiApiKey() const;

private:
	TSharedPtr<SEditableText> PromptEditableText;

	TPromptList PromptMessages;
	TSharedPtr<TPromptListWidget> ChatListView;
	TSharedPtr<FPromptMessage> LastServerMessage;
	
	FString CurrentPromptText;

	FOnBoardRequestCompletedDelegate OnBoardRequestCompleted;
	FOnBoardRequestFailedDelegate OnBoardRequestFailed;
};
