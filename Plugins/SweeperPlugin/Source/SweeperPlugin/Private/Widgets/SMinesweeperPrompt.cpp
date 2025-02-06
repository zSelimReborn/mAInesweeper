// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/SMinesweeperPrompt.h"

#include "HttpModule.h"
#include "SlateOptMacros.h"
#include "Interfaces/IHttpResponse.h"
#include "Settings/AISettings.h"

#define LOCTEXT_NAMESPACE "FSweeperPluginModule"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

const FString SMinesweeperPrompt::GEMINI_PROMPT_BASE_URL = TEXT("https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent");

void SMinesweeperPrompt::Construct(const FArguments& InArgs)
{
	OnBoardRequestCompleted = InArgs._OnBoardRequestCompleted;
	OnBoardRequestFailed = InArgs._OnBoardRequestFailed;
	
	FText HintText = LOCTEXT("SweeperPromptHint", "Waiting your mAInesweeper request...");
	PromptEditableText = SNew(SEditableText)
				.HintText(HintText);

	ChildSlot
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		.FillWidth(.9f)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			PromptEditableText.ToSharedRef()
		]
		+SHorizontalBox::Slot()
		.AutoWidth()
		.FillWidth(.1f)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.OnClicked_Raw(this, &SMinesweeperPrompt::OnPromptButtonClick)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PromptButtonText", "Send"))
				.Justification(ETextJustify::Center)
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
	const FText Prompt = PromptEditableText->GetText();
	if (Prompt.IsEmpty())
	{
		UE_LOG(LogSlate, Error, TEXT("[Minesweeper] - Empty prompt. AI Request blocked."));
		return FReply::Handled();
	}

	CurrentPromptText = Prompt.ToString();
	UE_LOG(LogSlate, Display, TEXT("[Minesweeper] - Prompt: %s"), *CurrentPromptText);
	PromptEditableText->SetText(FText::GetEmpty());

	TSharedRef<IHttpRequest> Request = BuildBoardRequest(CurrentPromptText);
	Request->ProcessRequest();
	
	return FReply::Handled();
}

void SMinesweeperPrompt::OnBoardRequestCompletedCallback(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	check(IsInGameThread());
	
	if (!bWasSuccessful || !Response.IsValid() || Response->GetResponseCode() > EHttpResponseCodes::PartialContent)
	{
		UE_LOG(LogSlate, Error, TEXT("[Minesweeper] - Error contacting Gemini: %d"), Response->GetResponseCode());
		OnBoardRequestFailed.ExecuteIfBound(FString::Printf(TEXT("Error contacting Gemini: %d"), Response->GetResponseCode()));
		return;
	}
	
	TSharedPtr<FJsonObject> BodyJson;
	const FString Body = Response->GetContentAsString();
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	FJsonSerializer::Deserialize(Reader, BodyJson);
	if (!BodyJson.IsValid())
	{
		UE_LOG(LogSlate, Error, TEXT("[Minesweeper] - AI response not valid, no JSON"));
		OnBoardRequestFailed.ExecuteIfBound(FString::Printf(TEXT("Failed to deserialize Gemini response: %s"), *Body));
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

					OnBoardRequestCompleted.ExecuteIfBound(BoardText);
					return;
				}
			}
		}
	}

	OnBoardRequestFailed.ExecuteIfBound(FString::Printf(TEXT("Gemini response malformed.")));
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