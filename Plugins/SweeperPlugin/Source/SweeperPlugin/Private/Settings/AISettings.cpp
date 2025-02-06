// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/AISettings.h"

FString UAISettings::GetGeminiApiKey() const
{
	return GeminiApiKey;
}

const UAISettings* UAISettings::Get()
{
	return GetDefault<UAISettings>();
}
