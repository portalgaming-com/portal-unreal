// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "Misc/EngineVersion.h"
#include "Runtime/Core/Public/HAL/Platform.h"
#include "UObject/Object.h"
#include "Portal/PortalJSConnector.h"
#include "Portal/PortalDataTypes.h"
#include "Portal/PortalRequests.h"

#include "PortalIdentity.generated.h"

template <typename UStructType>
FString UStructToJsonString(const UStructType &InStruct)
{
	FString OutString;
	FJsonObjectConverter::UStructToJsonObjectString(InStruct, OutString, 0, 0, 0, nullptr, false);
	return OutString;
}

template <typename UStructType>
TOptional<UStructType> JsonObjectToUStruct(const TSharedPtr<FJsonObject> &JsonObject)
{
	if (!JsonObject.IsValid())
	{
		return TOptional<UStructType>();
	}

	// Parse the JSON
	UStructType StructInstance;
	if (!FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &StructInstance, 0, 0))
	{
		PORTAL_ERR("Could not parse response from JavaScript into the expected Ustruct")
		return TOptional<UStructType>();
	}
	return StructInstance;
}

/**
 * Portal sdk Identity object
 */
UCLASS()
class PORTAL_API UPortalIdentity : public UObject
{
	GENERATED_BODY()
	friend class UPortalSubsystem;

public:
	DECLARE_MULTICAST_DELEGATE(FOnIdentityReadyDelegate);

	DECLARE_DELEGATE_OneParam(FPortalIdentityResponseDelegate, FPortalIdentityResult);

#if PLATFORM_ANDROID
	void HandleDeepLink(FString DeepLink) const;
	void HandleCustomTabsDismissed(FString Url);
#elif PLATFORM_IOS | PLATFORM_MAC
	void HandleDeepLink(NSString *sDeepLink) const;
#endif

	void Initialize(const FPortalIdentityInitData &InitData, const FPortalIdentityResponseDelegate &ResponseDelegate);

	void Logout(bool DoHardLogout, const FPortalIdentityResponseDelegate &ResponseDelegate);
	void RequestWalletSessionKey(const FPortalIdentityResponseDelegate &ResponseDelegate);
	void ExecuteTransaction(const FPortalExecuteTransactionRequest &RequestData, const FPortalIdentityResponseDelegate &ResponseDelegate);
	void GetIdToken(const FPortalIdentityResponseDelegate &ResponseDelegate);
	void GetAccessToken(const FPortalIdentityResponseDelegate &ResponseDelegate);
	/**
	 * Checks if the user's credentials have been stored
	 * @param ResponseDelegate The response delegate of type
	 * FPortalIdentityResponseDelegate to call on response from JS.
	 */
	void HasStoredCredentials(const FPortalIdentityResponseDelegate &ResponseDelegate);

protected:
	void Setup(TWeakObjectPtr<class UPortalJSConnector> Connector);
	void ReinstateConnection(FPortalJSResponse Response);

#if PLATFORM_ANDROID
	DECLARE_DELEGATE(FPortalIdentityOnPKCEDismissedDelegate);

	FPortalIdentityOnPKCEDismissedDelegate OnPKCEDismissed;
#endif

	TWeakObjectPtr<UPortalJSConnector> JSConnector;
	FPortalIdentityInitData InitData;
	FDelegateHandle BridgeReadyHandle;
	TMap<FString, FPortalIdentityResponseDelegate> ResponseDelegates;
#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC
	DECLARE_DELEGATE_OneParam(FPortalIdentityHandleDeepLinkDelegate, FString);

	FPortalIdentityHandleDeepLinkDelegate OnHandleDeepLink;
	// Since the second part of PKCE is triggered by deep links, saving the
	// response delegate here so it's easier to get
	FPortalIdentityResponseDelegate PKCEResponseDelegate;
	FPortalIdentityResponseDelegate PKCELogoutResponseDelegate;
	// bool IsPKCEConnected = false;
#endif

	// Ensures that Identity has been initialized before calling JS
	bool CheckIsInitialized(const FString &Action, const FPortalIdentityResponseDelegate &ResponseDelegate) const;
	// Calls JS with the given Action and Data, and registers the given
	// ResponseDelegate to be called when JS responds
	void CallJS(const FString &Action, const FString &Data, const FPortalIdentityResponseDelegate &ClientResponseDelegate, const FPortalJSResponseDelegate &HandleJSResponse, const bool bCheckInitialized = true);
	// Pulls the ResponseDelegate from the ResponseDelegates map and returns it
	TOptional<FPortalIdentityResponseDelegate> GetResponseDelegate(const FPortalJSResponse &Response);
	void ConfirmCode(const FString &DeviceCode, const float Interval, const FPortalIdentityResponseDelegate &ResponseDelegate);

	void OnInitializeResponse(FPortalJSResponse Response);

	void OnInitDeviceFlowResponse(FPortalJSResponse Response);

	void OnLogoutResponse(FPortalJSResponse Response);
	void OnConfirmCodeResponse(FPortalJSResponse Response);
#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC
	void OnGetPKCEAuthUrlResponse(FPortalJSResponse Response);
	void OnConnectPKCEResponse(FPortalJSResponse Response);
#endif
	void OnGetIdTokenResponse(FPortalJSResponse Response);
	void OnRequestWalletSessionKeyResponse(FPortalJSResponse Response);
	void OnExecuteTransactionResponse(FPortalJSResponse Response);
	void OnGetAccessTokenResponse(FPortalJSResponse Response);

	void LogAndIgnoreResponse(FPortalJSResponse Response);

#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC
	void OnDeepLinkActivated(FString DeepLink);
	void CompleteAuthenticatePKCEFlow(FString Url);
#endif

#if PLATFORM_ANDROID
	void HandleOnAuthenticatePKCEDismissed();
	void CallJniStaticVoidMethod(JNIEnv *Env, const jclass Class, jmethodID Method, ...);
	void LaunchAndroidUrl(FString Url);
#endif

	void SetStateFlags(uint8 StateIn);
	void ResetStateFlags(uint8 StateIn);
	bool IsStateFlagsSet(uint8 StateIn) const;

private:
	void SaveIdentitySettings();
	void LoadIdentitySettings();

private:
	enum EPortalIdentityStateFlags : uint8
	{
		IPS_NONE = 0,
		IPS_CONNECTING = 1 << 0,
		IPS_CONNECTED = 1 << 1,
		IPS_PKCE = 1 << 3,
		IPS_COMPLETING_PKCE = 1 << 4,
		IPS_INITIALIZED = 1 << 5,
		IPS_HARDLOGOUT = 1 << 6
	};

	uint8 StateFlags = IPS_NONE;
};
