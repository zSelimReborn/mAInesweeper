// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SMinesweeperPrompt.h"

#include "HttpModule.h"
#include "SlateOptMacros.h"
#include "SweeperPluginStyle.h"
#include "Interfaces/IHttpResponse.h"
#include "Settings/AISettings.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Views/SListView.h"

#define LOCTEXT_NAMESPACE "FSweeperPluginModule"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

const FString SMinesweeperPrompt::GEMINI_PROMPT_BASE_URL = TEXT("https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent");

void SMinesweeperPrompt::Construct(const FArguments& InArgs)
{
	OnBoardRequestCompleted = InArgs._OnBoardRequestCompleted;
	OnBoardRequestFailed = InArgs._OnBoardRequestFailed;
	
	FText HintText = LOCTEXT("SweeperPromptHint", "Waiting your mAInesweeper request...");
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.VAlign(VAlign_Bottom)
		.HAlign(HAlign_Fill)
		.FillHeight(1.f)
		[
			SNew(SScrollBox)
			.Orientation(Orient_Vertical)
			.Style(&FScrollBoxStyle()
				.SetTopShadowBrush(FSlateNoResource())
				.SetBottomShadowBrush(FSlateNoResource()) 
				.SetLeftShadowBrush(FSlateNoResource())
				.SetRightShadowBrush(FSlateNoResource())
			)
			.Clipping(EWidgetClipping::ClipToBounds)
			+SScrollBox::Slot()
			[
				SAssignNew(ChatListView, SListView<TSharedPtr<FPromptMessage>>)
					.ListItemsSource(&PromptMessages)
					.Orientation(Orient_Vertical)
					.SelectionMode(ESelectionMode::Type::None)
					.OnGenerateRow_Raw(this, &SMinesweeperPrompt::OnGenerateChatRow)
			]
		]
		+SVerticalBox::Slot()
		.HAlign(HAlign_Fill)
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderBackgroundColor(FSweeperPluginStyle::Get().GetSlateColor(TEXT("SweeperPlugin.DefaultBorderColor")))
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.Padding(5)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Center)
				[
					SAssignNew(PromptEditableText, SEditableText)
					.HintText(HintText)
					.OnTextCommitted_Raw(this,&SMinesweeperPrompt::OnPromptCommit)
				]
				+SHorizontalBox::Slot()
				.Padding(5)
				.AutoWidth()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.OnClicked_Raw(this, &SMinesweeperPrompt::OnPromptButtonClick)
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SImage)
							.Image(FAppStyle::GetBrush("Icons.ChevronRight")) 
						]
					]
				]
			]
		]
	];
}

TSharedRef<IHttpRequest> SMinesweeperPrompt::BuildBoardRequest(const FString& Prompt)
{
	const FString ApiKey = GetGeminiApiKey();
	
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindRaw(this, &SMinesweeperPrompt::OnBoardRequestCompletedCallback);

	const FString Url = FString::Printf(TEXT("%s?key=%s"), *GEMINI_PROMPT_BASE_URL, *ApiKey);
	const FString RequestBody = BuildRequestBody(Prompt);
	
	Request->SetURL(Url);
	Request->SetVerb("POST");
	Request->SetHeader("User-Agent", "X-UnrealEngine-Agent");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(RequestBody);

	UE_LOG(LogSlate, Display, TEXT("[Minesweeper] - AI Request: %s"), *RequestBody);
	return Request;
}

FString SMinesweeperPrompt::BuildRequestBody(const FString& Prompt) const
{
	FString Base = TEXT("{ "
	"\"contents\": [ "
		"{ "
			"\"role\": \"model\", "
			"\"parts\": [ "
				"{ \"text\": \"You are an assistant specialized in generating grids for the Minesweeper game. "
					"Respond with only 0 (empty) and 1 (mine), with each cell separated by commas and each row separated by a |. "
					"No extra text or explanations. Each time, generate a different field. "
					"If the request is not related to Minesweeper, respond with an empty JSON: [].\" "
				"} "
			"] "
		"}, "
		"{ \"role\": \"user\", "
			"\"parts\": [ { \"text\": \"{0}\" } ] "
		"} "
	"] "
	"}");
	return FString::Format(*Base, {Prompt});
}

FReply SMinesweeperPrompt::OnPromptButtonClick()
{
	HandlePrompt();
	return FReply::Handled();
}

void SMinesweeperPrompt::OnPromptCommit(const FText& PromptText, ETextCommit::Type CommitType)
{
	if (CommitType != ETextCommit::Type::OnEnter)
	{
		return;
	}

	HandlePrompt();
}

bool SMinesweeperPrompt::HandlePrompt()
{
	FText Prompt = PromptEditableText->GetText();
	Prompt = FText::TrimPrecedingAndTrailing(Prompt);
	if (Prompt.IsEmpty())
	{
		UE_LOG(LogSlate, Error, TEXT("[Minesweeper] - Empty prompt. AI Request blocked."));
		return false;
	}

	CurrentPromptText = Prompt.ToString();
	UE_LOG(LogSlate, Display, TEXT("[Minesweeper] - Prompt: %s"), *CurrentPromptText);
	PromptEditableText->SetText(FText::GetEmpty());

	TSharedPtr<FPromptMessage> UserMessage = MakeShared<FPromptMessage>(Prompt, true);
	TSharedPtr<FPromptMessage> ServerMessage = MakeShared<FPromptMessage>(LOCTEXT("GeminiGeneratingText", "Generating..."), false);
	LastServerMessage = ServerMessage;
	PromptMessages.Add(UserMessage);
	PromptMessages.Add(ServerMessage);

	ChatListView->RequestListRefresh();

	TSharedRef<IHttpRequest> Request = BuildBoardRequest(CurrentPromptText);
	Request->ProcessRequest();
	
	return true;
}

void SMinesweeperPrompt::OnBoardRequestCompletedCallback(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	check(IsInGameThread());
	
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() > EHttpResponseCodes::PartialContent)
	{
		UE_LOG(LogSlate, Error, TEXT("[Minesweeper] - Error contacting Gemini: %d"), Response->GetResponseCode());
		
		const FText ErrorText = FText::Format(LOCTEXT("GeminiGenericError", "Error contacting Gemini: {0}"), Response->GetResponseCode());
		LastServerMessage->Content = ErrorText;
		ChatListView->RequestListRefresh();
		
		OnBoardRequestFailed.ExecuteIfBound(ErrorText.ToString());
		return;
	}
	
	TSharedPtr<FJsonObject> BodyJson;
	const FString Body = Response->GetContentAsString();
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	FJsonSerializer::Deserialize(Reader, BodyJson);
	if (!BodyJson.IsValid())
	{
		UE_LOG(LogSlate, Error, TEXT("[Minesweeper] - AI response not valid, no JSON"));
		
		const FText ErrorText = FText::Format(LOCTEXT("GeminiJsonFailed", "Failed to deserialize Gemini response: {0}"), FText::FromString(Body));
		LastServerMessage->Content = ErrorText;
		ChatListView->RequestListRefresh();

		OnBoardRequestFailed.ExecuteIfBound(ErrorText.ToString());
		return;
	}

	if (BodyJson->HasField(TEXT("candidates")))
	{
		TArray<TSharedPtr<FJsonValue>> Candidates = BodyJson->GetArrayField(TEXT("candidates"));
		if (Candidates.Num() > 0)
		{
			TSharedPtr<FJsonValue> Candidate = Candidates[0];
			TSharedPtr<FJsonObject> Content = Candidate->AsObject()->GetObjectField(TEXT("content"));
			if (Content.IsValid())
			{
				TArray<TSharedPtr<FJsonValue>> Parts = Content->GetArrayField(TEXT("parts"));
				if (Parts.Num() > 0)
				{
					TSharedPtr<FJsonObject> Text = Parts[0]->AsObject();
					FString BoardText = Text->GetStringField(TEXT("text"));
					BoardText = ClearResponse(BoardText);
					UE_LOG(LogSlate, Display, TEXT("[MineSweeper] - Board: %s"), *BoardText);

					LastServerMessage->Content = LOCTEXT("GeminiGeneratedText", "Board generated correctly.");
					ChatListView->RequestListRefresh();
					
					OnBoardRequestCompleted.ExecuteIfBound(BoardText);
					return;
				}
			}
		}
	}

	FText ErrorText = LOCTEXT("GeminiMalformedResponseText", "Gemini response malformed.");
	LastServerMessage->Content = ErrorText;
	ChatListView->RequestListRefresh();

	OnBoardRequestFailed.ExecuteIfBound(ErrorText.ToString());
}

TSharedRef<ITableRow> SMinesweeperPrompt::OnGenerateChatRow(TSharedPtr<FPromptMessage> Message, const TSharedRef<STableViewBase>& Owner)
{
	const EHorizontalAlignment Alignment = Message->bIsUser ? HAlign_Right : HAlign_Left;
	const FText Author = Message->bIsUser ? LOCTEXT("PromptUserAuthorText", "You say:") : LOCTEXT("PromptAgentAuthorText", "Gemini says:");
	
	return SNew(STableRow<TSharedPtr<FPromptMessage>>, Owner)
		.Padding(5)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(Alignment)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(Author)
					.Font(FSweeperPluginStyle::Get().GetFontStyle(TEXT("SweeperPlugin.FontItalic")))
				]
			]
			+SVerticalBox::Slot()
			[
				SNew(SBox)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.HAlign(Alignment)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text_Lambda([Message]() { return Message->Content; })
					]
				]
			]
		];
}


FString SMinesweeperPrompt::ClearResponse(FString Response)
{
	return Response.Replace(TEXT("\n"), TEXT(""));
}

FString SMinesweeperPrompt::GetGeminiApiKey() const
{
	return UAISettings::Get()->GetGeminiApiKey();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE