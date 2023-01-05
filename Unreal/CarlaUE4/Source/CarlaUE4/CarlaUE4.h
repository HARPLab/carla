// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"

#include "Util/NonCopyable.h"
#include <string_view>

DECLARE_LOG_CATEGORY_EXTERN(LogDReyeVR, Log, All);

constexpr inline const char *file_name(const char *path)
{
    const char *file = path;
    while (*path)
    {
        if (*path++ == '/')
        {
            file = path;
        }
    }
    return file;
}

#define LOG(msg, ...)                                                                                                  \
    UE_LOG(LogDReyeVR, Log, TEXT("[%s::%s:%d] %s"), UTF8_TO_TCHAR(file_name(__FILE__)), UTF8_TO_TCHAR(__func__),       \
           __LINE__, *FString::Printf(TEXT(msg), ##__VA_ARGS__));