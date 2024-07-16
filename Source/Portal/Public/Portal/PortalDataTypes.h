#pragma once

#include "JsonObjectConverter.h"
#include "Portal/PortalJSMessages.h"
#include "Portal/PortalNames.h"

#include "PortalDataTypes.generated.h"

// This is the version of the Unreal Portal SDK that is being used. This is not the version of the engine.
// This hardcoded value will be updated by a workflow during the release process.
#define ENGINE_SDK_VERSION TEXT("0.0.1")

USTRUCT()
struct FPortalEngineVersionData
{
	GENERATED_BODY()

	UPROPERTY()
	FString engine = TEXT("unreal");

	UPROPERTY()
	FString engineSdkVersion = ENGINE_SDK_VERSION;

	// cannot have spaces
	UPROPERTY()
	FString engineVersion = FEngineVersion::Current().ToString().Replace(TEXT(" "), TEXT("_"));

	// cannot have spaces
	UPROPERTY()
	FString platform = FString(FPlatformProperties::IniPlatformName()).Replace(TEXT(" "), TEXT("_"));

	// cannot have spaces
	UPROPERTY()
	FString platformVersion = FPlatformMisc::GetOSVersion().Replace(TEXT(" "), TEXT("_"));

	// Information on device. Examples of expected results: Apple|Apple M3 Max, GenuineIntel|13th Gen Intel(R) Core(TM) i7-13700H, Apple|iPhone15&#44;4.
	UPROPERTY()
	FString deviceModel = FGenericPlatformMisc::GetDeviceMakeAndModel();
};

USTRUCT()
struct PORTAL_API FPortalIdentityInitData
{
	GENERATED_BODY()

	UPROPERTY()
	FString clientId;

	UPROPERTY()
	FString redirectUri;

	UPROPERTY()
	FString logoutRedirectUri;

	FString ToJsonString() const;
};

USTRUCT()
struct FPortalIdentityInitDeviceFlowData
{
	GENERATED_BODY()

	UPROPERTY()
	FString code;

	UPROPERTY()
	FString deviceCode;

	UPROPERTY()
	FString url;

	UPROPERTY()
	float interval = 0;

	static TOptional<FPortalIdentityInitDeviceFlowData> FromJsonString(const FString &JsonObjectString);
};

USTRUCT()
struct FPortalUserProfile
{
	GENERATED_BODY()

	UPROPERTY()
	FString email;

	UPROPERTY()
	FString nickname;

	UPROPERTY()
	FString sub;
};

USTRUCT()
struct FPortalIdentityCodeConfirmRequestData
{
	GENERATED_BODY()

	UPROPERTY()
	FString deviceCode;

	UPROPERTY()
	float interval = 5;

	UPROPERTY()
	float timeoutMs = 15 * 60 * 1000;
};

USTRUCT()
struct FPortalIdentityConnectPKCEData
{
	GENERATED_BODY()

	UPROPERTY()
	FString authorizationCode;

	UPROPERTY()
	FString state;
};

USTRUCT()
struct PORTAL_API FPortalIdentityResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool Success = false;

	UPROPERTY()
	FString Message;

	FPortalJSResponse Response;
};

USTRUCT(BlueprintType)
struct FPortalAccessListItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString address;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> storageKeys;
};

USTRUCT(BlueprintType)
struct FNftTransferDetails
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString receiver;

	UPROPERTY(BlueprintReadWrite)
	FString tokenId;

	UPROPERTY(BlueprintReadWrite)
	FString tokenAddress;
};
