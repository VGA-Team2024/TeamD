#pragma once

#include "CoreMinimal.h"

TEAMD_API DECLARE_LOG_CATEGORY_EXTERN(LogInfo, Log, All);

// ログ出力マクロ（ファイル名と行番号を自動で付与）
#define LOG_INFO(Verbosity, Format, ...) \
UE_LOG(LogInfo, Verbosity, Format TEXT(" [%s:%d]"), ##__VA_ARGS__, \
*FString(FPaths::GetCleanFilename(__FILE__)), __LINE__)