// Copyright 2017 Oh-Hyun Kwon. All Rights Reserved.

#pragma once

#include "Engine.h"

#define __KW_CURRENT_CLASS_FUNC (FString(__FUNCTION__))

#define __KW_CURRENT_CLASS__ (FString(__FUNCTION__).Left(FString(__FUNCTION__).Find(TEXT(":"))))

#define __KW_CURRENT_FUNC__ \
	(FString(__FUNCTION__)  \
		 .Right(FString(__FUNCTION__).Len() - FString(__FUNCTION__).Find(TEXT("::")) - 2))

#define __KW_CURRENT_LINE__ (FString::FromInt(__LINE__))

#define __KW_CURRENT_CLASS_LINE__ (__KW_CURRENT_CLASS__ + "(" + __KW_CURRENT_LINE__ + ")")

#define __KW_CURRENT_FUNCSIG__ (FString(__FUNCSIG__))

#define KW_LOG_S(LogCategory, LogVerbosity, FormatString, ...)                    \
	{                                                                             \
		FString const __Message__ =                                               \
			*FString::Printf(TEXT("%s %s"), *__KW_CURRENT_CLASS_LINE__,           \
							 *FString::Printf(FormatString, ##__VA_ARGS__));      \
		UE_LOG(LogCategory, LogVerbosity, TEXT("%s"), *__Message__);              \
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, *__Message__); \
	}

#define KW_LOG(LogCategory, LogVerbosity, FormatString, ...)                     \
	UE_LOG(LogCategory, LogVerbosity, TEXT("%s %s"), *__KW_CURRENT_CLASS_LINE__, \
		   *FString::Printf(FormatString, ##__VA_ARGS__))
