// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#include "KWJson.h"

bool SerializeJson(TSharedRef<FJsonObject> const& JsonObject, FString& OutString)
{
	auto JsonWriter =
		TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutString);
	return FJsonSerializer::Serialize(JsonObject, JsonWriter);
}

bool DeserializeJson(FString const& JsonString, TSharedPtr<FJsonObject>& JsonObject)
{
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
	return FJsonSerializer::Deserialize(JsonReader, JsonObject);
}
