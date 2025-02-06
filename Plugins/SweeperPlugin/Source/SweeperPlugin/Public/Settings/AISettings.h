// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AISettings.generated.h"

/**
 * 
 */
UCLASS(Config=Secret, DefaultConfig, meta = (DisplayName="AI Api Settings"))
class SWEEPERPLUGIN_API UAISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	FString GetGeminiApiKey() const;

	static const UAISettings* Get();

private:
	UPROPERTY(Config, EditAnywhere, Category="Gemini")
	FString GeminiApiKey;
};
