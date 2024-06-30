#pragma once

#include "CoreMinimal.h"
#include "Misc/GeneratedTypeName.h"

PORTAL_API DECLARE_LOG_CATEGORY_EXTERN(LogPortal, Log, All);

// Disable for shipping builds
#if NO_LOGGING
PORTAL_API DECLARE_LOG_CATEGORY_EXTERN(LogClean, Log, All);
#else
// Direct implementation of the DECLARE_LOG_CATEGORY_EXTERN macro
PORTAL_API extern struct FLogCategoryLogClean : public FLogCategory<ELogVerbosity::Log, ELogVerbosity::All>
{
  FORCEINLINE FLogCategoryLogClean() : FLogCategory(TEXT(""))
  {
  }
} LogClean;
#endif

#define PORTAL_LOG_CLEAN(Format, ...) \
  UE_LOG(LogClean, Log, TEXT(Format), ##__VA_ARGS__)

// PORTAL_API DECLARE_LOG_CATEGORY_EXTERN(LogFuncSig, Log, All);

#define PORTAL_FUNCNAME __FUNCTION__
#if defined(_MSC_VER) && !defined(__clang__)
#define PORTAL_FUNCSIG __FUNCSIG__
#else
#define PORTAL_FUNCSIG __PRETTY_FUNCTION__
#endif

#define PORTAL_LOG_FUNCSIG \
  UE_LOG(LogPortal, Log, TEXT("%s"), *FString(PORTAL_FUNCSIG))

#define PORTAL_LOG(Format, ...) \
  {UE_LOG(LogPortal, Log, TEXT(Format), ##__VA_ARGS__)}

#define PORTAL_LOG_FUNC(Format, ...)                                         \
  {                                                                          \
    const FString _Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__);       \
    UE_LOG(LogPortal, Log, TEXT("%s: %s"), *FString(PORTAL_FUNCNAME), *_Msg) \
  }

#define PORTAL_DISPLAY(Format, ...) \
  {UE_LOG(LogPortal, Display, TEXT(Format), ##__VA_ARGS__)}

#define PORTAL_VERBOSE(Format, ...) \
  {UE_LOG(LogPortal, Verbose, TEXT(Format), ##__VA_ARGS__)}

#define PORTAL_VERBOSE_FUNC(Format, ...)                                 \
  {                                                                      \
    const FString _Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__);   \
    UE_LOG(LogPortal, Verbose, TEXT("%s: %s"), *FString(PORTAL_FUNCSIG), \
           *_Msg)                                                        \
  }

#define PORTAL_WARN(Format, ...) \
  {UE_LOG(LogPortal, Warning, TEXT(Format), ##__VA_ARGS__)}

#define PORTAL_WARN_FUNC(Format, ...)                                     \
  {                                                                       \
    const FString _Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__);    \
    UE_LOG(LogPortal, Warning, TEXT("%s: %s"), *FString(PORTAL_FUNCNAME), \
           *_Msg)                                                         \
  }

#define PORTAL_ERR(Format, ...) \
  {UE_LOG(LogPortal, Error, TEXT(Format), ##__VA_ARGS__)}

#define PORTAL_ERR_FUNC(Format, ...)                                    \
  {                                                                     \
    const FString _Msg = FString::Printf(TEXT(Format), ##__VA_ARGS__);  \
    UE_LOG(LogPortal, Error, TEXT("%s: %s"), *FString(PORTAL_FUNCNAME), \
           *_Msg)                                                       \
  }
