// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "Core.h"
#include "Json.h"
#include "JsonUtilities.h"

bool SerializeJson(TSharedRef<FJsonObject> const& JsonObject, FString& OutString);

bool DeserializeJson(FString const& JsonString, TSharedPtr<FJsonObject>& JsonObject);

template <typename UObjectType>
static bool UObjectToJsonObject(UObjectType const* Object, TSharedRef<FJsonObject> JsonObject)
{
	return FJsonObjectConverter::UStructToJsonObject(UObjectType::StaticClass(), Object, JsonObject,
													 CPF_SaveGame, 0);
}

template <typename UStructType>
static bool UStructToJsonObject(UStructType const* Object, TSharedRef<FJsonObject> JsonObject)
{
	return FJsonObjectConverter::UStructToJsonObject(UStructType::StaticStruct(), Object,
													 JsonObject, CPF_SaveGame, 0);
}

template <typename UObjectType>
static bool JsonObjectToUObject(TSharedRef<FJsonObject> const& JsonObject, UObjectType* Object)
{
	return FJsonObjectConverter::JsonObjectToUStruct(JsonObject, UObjectType::StaticClass(), Object,
													 CPF_SaveGame, 0);
}

template <typename UStructType>
static bool JsonObjectToUStruct(TSharedRef<FJsonObject> const& JsonObject, UStructType* Object)
{
	return FJsonObjectConverter::JsonObjectToUStruct(JsonObject, UStructType::StaticStruct(),
													 Object, CPF_SaveGame, 0);
}
