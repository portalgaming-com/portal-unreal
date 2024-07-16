// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal/PortalIdentity.h"

#include "Portal/Misc/PortalLogging.h"
#include "Portal/PortalResponses.h"
#include "Portal/PortalJSConnector.h"
#include "JsonObjectConverter.h"
#include "Portal/PortalSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Policies/CondensedJsonPrintPolicy.h"

#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC
#include "GenericPlatform/GenericPlatformHttp.h"
#endif

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/PortalAndroidJNI.h"
#elif PLATFORM_IOS
#include "IOS/PortalIOS.h"
#elif PLATFORM_MAC
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Mac/PortalMac.h"
#endif

#define IDENTITY_SAVE_GAME_SLOT_NAME TEXT("Portal")

void UPortalIdentity::Initialize(const FPortalIdentityInitData &Data, const FPortalIdentityResponseDelegate &ResponseDelegate)
{
	check(JSConnector.IsValid());

	LoadIdentitySettings();
	// we check saved settings in case if player has not logged out properly
	if (Data.logoutRedirectUri.IsEmpty() && IsStateFlagsSet(IPS_PKCE))
	{
		PORTAL_ERR("Logout URI is empty. Previously logged in via PKCE.")
		ResetStateFlags(IPS_PKCE);
		SaveIdentitySettings();
	}

	InitData = Data;

	CallJS(PortalIdentityAction::INIT, InitData.ToJsonString(), ResponseDelegate, FPortalJSResponseDelegate::CreateUObject(this, &UPortalIdentity::OnInitializeResponse), false);
}

void UPortalIdentity::Logout(bool DoHardLogout, const FPortalIdentityResponseDelegate &ResponseDelegate)
{
#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC
	if (IsStateFlagsSet(IPS_PKCE))
	{
		PKCELogoutResponseDelegate = ResponseDelegate;
	}
#endif
	if (IsStateFlagsSet(IPS_CONNECTED))
	{
		if (DoHardLogout)
		{
			SetStateFlags(IPS_HARDLOGOUT);
		}
		CallJS(PortalIdentityAction::LOGOUT, TEXT(""), ResponseDelegate, FPortalJSResponseDelegate::CreateUObject(this, &UPortalIdentity::OnLogoutResponse));
	}
	else
	{
		PORTAL_WARN("Identity is not connected to execute logout.");
	}
}

void UPortalIdentity::ConfirmCode(const FString &DeviceCode, const float Interval, const FPortalIdentityResponseDelegate &ResponseDelegate)
{
	FPortalIdentityCodeConfirmRequestData Data{DeviceCode, Interval};

	CallJS(PortalIdentityAction::AUTHENTICATE_CONFIRM_CODE, UStructToJsonString(Data), ResponseDelegate, FPortalJSResponseDelegate::CreateUObject(this, &UPortalIdentity::OnConfirmCodeResponse));
}

void UPortalIdentity::RequestWalletSessionKey(const FPortalIdentityResponseDelegate &ResponseDelegate)
{
	CallJS(PortalIdentityAction::REQUEST_WALLET_SESSION_KEY, TEXT(""), ResponseDelegate, FPortalJSResponseDelegate::CreateUObject(this, &UPortalIdentity::OnRequestWalletSessionKeyResponse));
}

void UPortalIdentity::ExecuteTransaction(const FPortalExecuteTransactionRequest &RequestData, const FPortalIdentityResponseDelegate &ResponseDelegate)
{
	PORTAL_LOG("ExecuteTransaction Request: %s", *UStructToJsonString(RequestData))

	CallJS(PortalIdentityAction::EXECUTE_TRANSACTION, UStructToJsonString(RequestData), ResponseDelegate, FPortalJSResponseDelegate::CreateUObject(this, &UPortalIdentity::OnExecuteTransactionResponse));
}

void UPortalIdentity::GetIdToken(const FPortalIdentityResponseDelegate &ResponseDelegate)
{
	CallJS(PortalIdentityAction::GET_ID_TOKEN, TEXT(""), ResponseDelegate, FPortalJSResponseDelegate::CreateUObject(this, &UPortalIdentity::OnGetIdTokenResponse));
}

void UPortalIdentity::GetAccessToken(const FPortalIdentityResponseDelegate &ResponseDelegate)
{
	CallJS(PortalIdentityAction::GET_ACCESS_TOKEN, TEXT(""), ResponseDelegate, FPortalJSResponseDelegate::CreateUObject(this, &UPortalIdentity::OnGetAccessTokenResponse));
}

void UPortalIdentity::HasStoredCredentials(const FPortalIdentityResponseDelegate &ResponseDelegate)
{
	// we do check credentials into two steps, we check accessToken and then IdToken
	// check access token
	CallJS(PortalIdentityAction::GET_ACCESS_TOKEN, TEXT(""), ResponseDelegate, FPortalJSResponseDelegate::CreateLambda([=](FPortalJSResponse Response)
																													   {
		FString AccessToken;

		Response.JsonObject->TryGetStringField(TEXT("result"), AccessToken);
		if (!Response.success || AccessToken.IsEmpty())
		{
			ResponseDelegate.ExecuteIfBound(FPortalIdentityResult{false, Response.JsonObject->HasField(TEXT("error")) ? Response.JsonObject->GetStringField(TEXT("error")) : "Failed to retrieve Access Token.", Response});
		}
		else
		{
			// check for id token
			CallJS(PortalIdentityAction::GET_ID_TOKEN, TEXT(""), ResponseDelegate, FPortalJSResponseDelegate::CreateLambda([ResponseDelegate](FPortalJSResponse Response)
			{
				FString IdToken;

				Response.JsonObject->TryGetStringField(TEXT("result"), IdToken);
				if (!Response.success || IdToken.IsEmpty())
				{
					ResponseDelegate.ExecuteIfBound(FPortalIdentityResult{false, Response.JsonObject->HasField(TEXT("error")) ? Response.JsonObject->GetStringField(TEXT("error")) : "Failed to retrieve Id Token.", Response});
				}
				else
				{
					ResponseDelegate.ExecuteIfBound(FPortalIdentityResult{Response.success, "", Response});
				}
			}));
		} }));
}

void UPortalIdentity::Setup(const TWeakObjectPtr<UPortalJSConnector> Connector)
{
	PORTAL_LOG_FUNCSIG

	if (!Connector.IsValid())
	{
		PORTAL_ERR("Invalid JSConnector passed to UPortalIdentity::Setup.")
		return;
	}

	JSConnector = Connector.Get();
}

void UPortalIdentity::ReinstateConnection(FPortalJSResponse Response)
{
	ResetStateFlags(IPS_CONNECTING);

	if (auto ResponseDelegate = GetResponseDelegate(Response))
	{
		const FString CallbackName = "Reauthenticate";

		if (Response.JsonObject->GetBoolField(TEXT("result")))
		{
			SetStateFlags(IPS_CONNECTED);
			ResponseDelegate->ExecuteIfBound(FPortalIdentityResult{true, "", Response});
		}
		else
		{
#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC
			if (IsStateFlagsSet(IPS_PKCE))
			{
				PKCEResponseDelegate = ResponseDelegate.GetValue();
				CallJS(PortalIdentityAction::GET_PKCE_AUTH_URL, TEXT(""), PKCEResponseDelegate, FPortalJSResponseDelegate::CreateUObject(this, &UPortalIdentity::OnGetPKCEAuthUrlResponse));
			}
			else
#endif
			{
				CallJS(PortalIdentityAction::INIT_DEVICE_FLOW, TEXT(""), ResponseDelegate.GetValue(), FPortalJSResponseDelegate::CreateUObject(this, &UPortalIdentity::OnInitDeviceFlowResponse));
			}
		}
	}
}

bool UPortalIdentity::CheckIsInitialized(const FString &Action, const FPortalIdentityResponseDelegate &ResponseDelegate) const
{
	const bool IsInitialized = IsStateFlagsSet(IPS_INITIALIZED);

	if (!IsInitialized)
	{
		PORTAL_WARN("Attempting action '%s' before Identity is initialized", *Action)
		ResponseDelegate.ExecuteIfBound(FPortalIdentityResult{false, "Identity is not initialized"});
	}

	return IsInitialized;
}

void UPortalIdentity::CallJS(const FString &Action, const FString &Data, const FPortalIdentityResponseDelegate &ClientResponseDelegate, const FPortalJSResponseDelegate &HandleJSResponse, const bool bCheckInitialized /*= true*/)
{
	if (bCheckInitialized && !CheckIsInitialized(Action, ClientResponseDelegate))
	{
		return;
	}

	check(JSConnector.IsValid());
	const FString Guid = JSConnector->CallJS(Action, Data, HandleJSResponse);
	ResponseDelegates.Add(Guid, ClientResponseDelegate);
}

TOptional<UPortalIdentity::FPortalIdentityResponseDelegate> UPortalIdentity::GetResponseDelegate(const FPortalJSResponse &Response)
{
	FPortalIdentityResponseDelegate ResponseDelegate;
	if (!ResponseDelegates.RemoveAndCopyValue(Response.requestId, ResponseDelegate))
	{
		PORTAL_WARN("Couldn't find delegate for %s response", *Response.responseFor)
		return TOptional<FPortalIdentityResponseDelegate>();
	}
	return ResponseDelegate;
}

void UPortalIdentity::OnInitializeResponse(FPortalJSResponse Response)
{
	if (auto ResponseDelegate = GetResponseDelegate(Response))
	{
		FString Msg;
		if (Response.success)
		{
			SetStateFlags(IPS_INITIALIZED);
			PORTAL_LOG("Identity initialization succeeded.")
		}
		else
		{
			PORTAL_ERR("Identity initialization failed.")
			Response.Error.IsSet() ? Msg = Response.Error->ToString() : Msg = Response.JsonObject->GetStringField(TEXT("error"));
		}
		ResponseDelegate->ExecuteIfBound(FPortalIdentityResult{Response.success, Msg, Response});
	}
}

void UPortalIdentity::OnInitDeviceFlowResponse(FPortalJSResponse Response)
{
	if (auto ResponseDelegate = GetResponseDelegate(Response))
	{
		const auto InitDeviceFlowData = JsonObjectToUStruct<FPortalIdentityInitDeviceFlowData>(Response.JsonObject);

		if (!Response.success || !InitDeviceFlowData || !InitDeviceFlowData->code.Len())
		{
			FString Msg;

			PORTAL_WARN("Authenticate device flow initialization attempt failed.");
			Response.Error.IsSet() ? Msg = Response.Error->ToString() : Msg = Response.JsonObject->GetStringField(TEXT("error"));
			ResponseDelegate->ExecuteIfBound(FPortalIdentityResult{false, Msg, Response});

			return;
		}
		FString Err;

		FPlatformProcess::LaunchURL(*InitDeviceFlowData->url, nullptr, &Err);
		if (Err.Len())
		{
			FString Msg = "Failed to connect to Browser: " + Err;

			PORTAL_ERR("%s", *Msg);
			ResponseDelegate->ExecuteIfBound(FPortalIdentityResult{false, Msg, Response});
			return;
		}
		ConfirmCode(InitDeviceFlowData->deviceCode, InitDeviceFlowData->interval, ResponseDelegate.GetValue());
	}
}

void UPortalIdentity::OnLogoutResponse(FPortalJSResponse Response)
{
	auto ResponseDelegate = GetResponseDelegate(Response);

	if (!ResponseDelegate)
	{
		return;
	}

	FString Message;

	if (!Response.success)
	{
		Message = Response.Error.IsSet() ? Response.Error->ToString() : Response.JsonObject->GetStringField(TEXT("error"));

		PORTAL_ERR("%s", *Message)
		ResponseDelegate->ExecuteIfBound(FPortalIdentityResult{Response.success, Message, Response});

		return;
	}

	if (!IsStateFlagsSet(IPS_HARDLOGOUT))
	{
		Message = "Logged out without clearing browser session";

		PORTAL_LOG("%s", *Message)
		ResponseDelegate->ExecuteIfBound(FPortalIdentityResult{true, Message});

		return;
	}

	FString Url;
	FString ErrorMessage;

	ResetStateFlags(IPS_HARDLOGOUT);
	Response.JsonObject->TryGetStringField(TEXT("result"), Url);
	if (!Url.IsEmpty())
	{
#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC
		if (IsStateFlagsSet(IPS_PKCE))
		{
			OnHandleDeepLink = FPortalIdentityHandleDeepLinkDelegate::CreateUObject(this, &UPortalIdentity::OnDeepLinkActivated);
#if PLATFORM_ANDROID
			LaunchAndroidUrl(Url);
#elif PLATFORM_IOS
			[[PortalIOS instance] launchUrl:TCHAR_TO_ANSI(*Url)];
#elif PLATFORM_MAC
			[[PortalMac instance] launchUrl:TCHAR_TO_ANSI(*Url)
							 forRedirectUri:TCHAR_TO_ANSI(*InitData.logoutRedirectUri)];
#endif
		}
		else
		{
#endif
			FPlatformProcess::LaunchURL(*Url, nullptr, &ErrorMessage);
			if (ErrorMessage.Len())
			{
				Message = "Failed to connect to Browser: " + ErrorMessage;

				PORTAL_ERR("%s", *Message);
				ResponseDelegate->ExecuteIfBound(FPortalIdentityResult{false, Message, Response});

				return;
			}
			Message = "Logged out";
			PORTAL_LOG("%s", *Message)
			ResponseDelegate->ExecuteIfBound(FPortalIdentityResult{Response.success, Message});
#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC
		}
#endif
	}
	else
	{
		ResponseDelegate->ExecuteIfBound(FPortalIdentityResult{false, "Logout Url is empty", Response});
	}
	ResetStateFlags(IPS_CONNECTED);
}

#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC
void UPortalIdentity::OnGetPKCEAuthUrlResponse(FPortalJSResponse Response)
{
	if (PKCEResponseDelegate.IsBound())
	{
		FString Msg;
		bool bSuccess = true;

		if (!Response.success || !Response.JsonObject->HasTypedField<EJson::String>(TEXT("result")))
		{
			PORTAL_LOG("Could not get PKCE auth URL from Identity.");
		}
		else
		{
			// Handle deeplink calls
			OnHandleDeepLink = FPortalIdentityHandleDeepLinkDelegate::CreateUObject(this, &UPortalIdentity::OnDeepLinkActivated);

			Msg = Response.JsonObject->GetStringField(TEXT("result")).Replace(TEXT(" "), TEXT("+"));
#if PLATFORM_ANDROID
			OnPKCEDismissed = FPortalIdentityOnPKCEDismissedDelegate::CreateUObject(this, &UPortalIdentity::HandleOnAuthenticatePKCEDismissed);
			LaunchAndroidUrl(Msg);
#elif PLATFORM_IOS
			[[PortalIOS instance] launchUrl:TCHAR_TO_ANSI(*Msg)];
#elif PLATFORM_MAC
			[[PortalMac instance] launchUrl:TCHAR_TO_ANSI(*Msg)
							 forRedirectUri:TCHAR_TO_ANSI(*InitData.redirectUri)];
#endif
		}
	}
	else
	{
		PORTAL_ERR("Unable to return a response for Connect PKCE.");
	}
}

void UPortalIdentity::OnConnectPKCEResponse(FPortalJSResponse Response)
{
	if (PKCEResponseDelegate.IsBound())
	{
		FString Msg;

		if (Response.success)
		{
			PORTAL_LOG("Successfully connected via PKCE")
			SetStateFlags(IPS_CONNECTED);
		}
		else
		{
			PORTAL_WARN("Connect PKCE attempt failed.");
			ResetStateFlags(IPS_PKCE);
			Response.Error.IsSet() ? Msg = Response.Error->ToString() : Msg = Response.JsonObject->GetStringField(TEXT("error"));
		}
		PKCEResponseDelegate.ExecuteIfBound(FPortalIdentityResult{Response.success, Msg});
		PKCEResponseDelegate = nullptr;

		// we save identity state for PKCE flow in case if we decide to close a game
		// and reopen it later with Identity is still being connected and we decided to logout.
		// In this case, we logout using PKCE flow
		SaveIdentitySettings();
	}
	else
	{
		PORTAL_ERR("Unable to return a response for Connect PKCE.");
	}
	ResetStateFlags(IPS_COMPLETING_PKCE);
}
#endif

void UPortalIdentity::OnGetIdTokenResponse(FPortalJSResponse Response)
{
	if (auto ResponseDelegate = GetResponseDelegate(Response))
	{
		FString IdToken;

		Response.JsonObject->TryGetStringField(TEXT("result"), IdToken);

		if (!Response.success || IdToken.IsEmpty())
		{
			PORTAL_LOG("Failed to retrieve Id Token.");

			const FString Msg = Response.JsonObject->HasField(TEXT("error")) ? Response.JsonObject->GetStringField(TEXT("error")) : "Failed to retrieve Id Token.";

			ResponseDelegate->ExecuteIfBound(FPortalIdentityResult{false, Msg, Response});
		}
		else
		{
			PORTAL_LOG("Retrieved Id Token.");
			ResponseDelegate->ExecuteIfBound(FPortalIdentityResult{true, IdToken});
		}
	}
}

void UPortalIdentity::OnGetAccessTokenResponse(FPortalJSResponse Response)
{
	if (auto ResponseDelegate = GetResponseDelegate(Response))
	{
		FString AccessToken;

		Response.JsonObject->TryGetStringField(TEXT("result"), AccessToken);

		if (!Response.success || AccessToken.IsEmpty())
		{
			PORTAL_LOG("Failed to retrieve Access Token");

			const FString Msg = Response.JsonObject->HasField(TEXT("error")) ? Response.JsonObject->GetStringField(TEXT("error")) : "Failed to retrieve Access Token.";

			ResponseDelegate->ExecuteIfBound(FPortalIdentityResult{false, Msg, Response});
		}
		else
		{
			PORTAL_LOG("Retrieved Access Token.");
			ResponseDelegate->ExecuteIfBound(FPortalIdentityResult{true, AccessToken});
		}
	}
}

void UPortalIdentity::OnExecuteTransactionResponse(FPortalJSResponse Response)
{
	if (auto ResponseDelegate = GetResponseDelegate(Response))
	{
		FString Msg;
		bool bSuccess = true;

		if (!Response.success)
		{
			PORTAL_WARN("EVM transaction receipt retrieval failed.");
			Response.Error.IsSet() ? Msg = Response.Error->ToString() : Msg = Response.JsonObject->GetStringField(TEXT("error"));
			bSuccess = false;
		}
		ResponseDelegate->ExecuteIfBound(FPortalIdentityResult{bSuccess, Msg, Response});
	}
}

void UPortalIdentity::OnRequestWalletSessionKeyResponse(FPortalJSResponse Response)
{
	auto ResponseDelegate = GetResponseDelegate(Response);

	if (!ResponseDelegate)
	{
		return;
	}

	if (!Response.success)
	{
		const FString Message = Response.Error.IsSet() ? Response.Error->ToString() : Response.JsonObject->GetStringField(TEXT("error"));

		PORTAL_ERR("%s", *Message);
		ResponseDelegate->ExecuteIfBound(FPortalIdentityResult{Response.success, Message, Response});

		return;
	}
}

void UPortalIdentity::OnConfirmCodeResponse(FPortalJSResponse Response)
{
	if (auto ResponseDelegate = GetResponseDelegate(Response))
	{
		FString Msg;
		FString TypeOfConnection = TEXT("authenticate");

		ResetStateFlags(IPS_CONNECTING);
		if (Response.success)
		{
			PORTAL_LOG("Code confirmed for %s operation.", *TypeOfConnection)
			SetStateFlags(IPS_CONNECTED);
		}
		else
		{
			PORTAL_LOG("%s code not confirmed.", *TypeOfConnection)
			Response.Error.IsSet() ? Msg = Response.Error->ToString() : Msg = Response.JsonObject->GetStringField(TEXT("error"));
		}
		ResponseDelegate->ExecuteIfBound(FPortalIdentityResult{Response.success, Msg, Response});
	}
}

void UPortalIdentity::LogAndIgnoreResponse(FPortalJSResponse Response)
{
	if (Response.success && !Response.Error)
	{
		PORTAL_LOG("Received success response from Identity for action %s", *Response.responseFor);
	}
	else
	{
		FString Msg;
		Response.Error.IsSet() ? Msg = Response.Error->ToString() : Msg = Response.JsonObject->GetStringField(TEXT("error"));
		PORTAL_WARN("Received error response from Identity for action %s -- %s", *Response.responseFor, *Msg);
	}
}

void UPortalIdentity::SetStateFlags(uint8 StateIn)
{
	StateFlags |= StateIn;
}

void UPortalIdentity::ResetStateFlags(uint8 StateIn)
{
	StateFlags &= ~StateIn;
}

bool UPortalIdentity::IsStateFlagsSet(uint8 StateIn) const
{
	return (StateFlags & StateIn) == StateIn;
}

void UPortalIdentity::SaveIdentitySettings()
{
	UPortalSaveGame *SaveGameInstance = Cast<UPortalSaveGame>(UGameplayStatics::CreateSaveGameObject(UPortalSaveGame::StaticClass()));

	SaveGameInstance->bWasConnectedViaPKCEFlow = IsStateFlagsSet(IPS_PKCE | IPS_CONNECTED);

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, IDENTITY_SAVE_GAME_SLOT_NAME, 0);
}

void UPortalIdentity::LoadIdentitySettings()
{
	UPortalSaveGame *SaveGameInstance = Cast<UPortalSaveGame>(UGameplayStatics::LoadGameFromSlot(IDENTITY_SAVE_GAME_SLOT_NAME, 0));

	if (SaveGameInstance)
	{
		SaveGameInstance->bWasConnectedViaPKCEFlow ? SetStateFlags(IPS_PKCE) : ResetStateFlags(IPS_PKCE);
	}
}

#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC
void UPortalIdentity::OnDeepLinkActivated(FString DeepLink)
{
	PORTAL_LOG_FUNC("URL : %s", *DeepLink);
	OnHandleDeepLink = nullptr;
	if (DeepLink.StartsWith(InitData.logoutRedirectUri))
	{
		// execute on game thread to prevent call to Identity instance from another thread
		if (FTaskGraphInterface::IsRunning())
		{
			FGraphEventRef GameThreadTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this]()
																						   {
				PKCELogoutResponseDelegate.ExecuteIfBound(FPortalIdentityResult{true, "Logged out"});
				PKCELogoutResponseDelegate = nullptr;
				ResetStateFlags(IPS_CONNECTED | IPS_PKCE);
				SaveIdentitySettings(); }, TStatId(), nullptr, ENamedThreads::GameThread);
		}
	}
	else if (DeepLink.StartsWith(InitData.redirectUri))
	{
		CompleteAuthenticatePKCEFlow(DeepLink);
	}
}

void UPortalIdentity::CompleteAuthenticatePKCEFlow(FString Url)
{
	// Required mainly for Android to detect when Chrome Custom tabs is dismissed
	// See HandleOnAuthenticatePKCEDismissed
	SetStateFlags(IPS_COMPLETING_PKCE);

	// Get code and state from deeplink URL
	TOptional<FString> Code, State;
	FString Endpoint, Params;
	Url.Split(TEXT("?"), &Endpoint, &Params);
	TArray<FString> ParamsArray;

	Params.ParseIntoArray(ParamsArray, TEXT("&"));
	for (FString Param : ParamsArray)
	{
		FString Key, Value;

		if (Param.StartsWith("code"))
		{
			Param.Split(TEXT("="), &Key, &Value);
			Code = Value;
		}
		else if (Param.StartsWith("state"))
		{
			Param.Split(TEXT("="), &Key, &Value);
			State = Value;
		}
	}

	if (!Code.IsSet() || !State.IsSet())
	{
		const FString ErrorMsg = "Uri was missing state and/or code. Please call ConnectPKCE() again";

		PORTAL_ERR("%s", *ErrorMsg);
		PKCEResponseDelegate.ExecuteIfBound(FPortalIdentityResult{false, ErrorMsg});
		PKCEResponseDelegate = nullptr;
		ResetStateFlags(IPS_PKCE | IPS_CONNECTING | IPS_COMPLETING_PKCE);
		SaveIdentitySettings();
	}
	else
	{
		FPortalIdentityConnectPKCEData Data = FPortalIdentityConnectPKCEData{Code.GetValue(), State.GetValue()};

		CallJS(PortalIdentityAction::AUTHENTICATE_PKCE, UStructToJsonString(Data), PKCEResponseDelegate,
			   FPortalJSResponseDelegate::CreateUObject(this, &UPortalIdentity::OnConnectPKCEResponse));
	}
}

#endif

#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC
#if PLATFORM_ANDROID
// Called from Android JNI
void UPortalIdentity::HandleDeepLink(FString DeepLink) const
{
#elif PLATFORM_IOS | PLATFORM_MAC
// Called from iOS Objective C
void UPortalIdentity::HandleDeepLink(NSString *sDeepLink) const
{
	FString DeepLink = FString(UTF8_TO_TCHAR([sDeepLink UTF8String]));
	PORTAL_LOG("Handle Deep Link: %s", *DeepLink);
#endif

	if (!OnHandleDeepLink.ExecuteIfBound(DeepLink))
	{
		PORTAL_WARN("OnHandleDeepLink delegate was not called");
	}
}
#endif

#if PLATFORM_ANDROID
void UPortalIdentity::HandleOnAuthenticatePKCEDismissed()
{
	PORTAL_LOG("Handle On Authenticate PKCE Dismissed");
	OnPKCEDismissed = nullptr;

	// If the second part of PKCE (CompleteAuthenticatePKCEFlow) has not started yet and custom tabs is dismissed,
	// this means the user manually dismissed the custom tabs before entering all
	// all required details (e.g. email address) into Identity
	// Cannot use IPS_CONNECTING as that is set when PKCE flow is initiated. Here we are checking against the second
	// half of the PKCE flow.
	if (!IsStateFlagsSet(IPS_COMPLETING_PKCE))
	{
		// User hasn't entered all required details (e.g. email address) into
		// Identity yet
		PORTAL_LOG("Authenticate PKCE dismissed before completing the flow");
		if (FTaskGraphInterface::IsRunning())
		{
			FGraphEventRef GameThreadTask = FFunctionGraphTask::CreateAndDispatchWhenReady([this]()
																						   {
					if (!PKCEResponseDelegate.ExecuteIfBound(FPortalIdentityResult{ false, "Cancelled" }))
					{
						PORTAL_WARN("Authenticate PKCEResponseDelegate delegate was not called");
					}
					PKCEResponseDelegate = nullptr; }, TStatId(), nullptr, ENamedThreads::GameThread);
		}
	}
	else
	{
		PORTAL_LOG("PKCE dismissed by user or SDK");
	}
}

void UPortalIdentity::HandleCustomTabsDismissed(FString Url)
{
	PORTAL_LOG("On PKCE Dismissed");

	if (!OnPKCEDismissed.ExecuteIfBound())
	{
		PORTAL_WARN("OnPKCEDismissed delegate was not called");
	}
}

void UPortalIdentity::CallJniStaticVoidMethod(JNIEnv *Env, const jclass Class, jmethodID Method, ...)
{
	va_list Args;

	va_start(Args, Method);
	Env->CallStaticVoidMethodV(Class, Method, Args);
	va_end(Args);
	Env->DeleteLocalRef(Class);
}

void UPortalIdentity::LaunchAndroidUrl(FString Url)
{
	if (JNIEnv *Env = FAndroidApplication::GetJavaEnv())
	{
		jstring jurl = Env->NewStringUTF(TCHAR_TO_UTF8(*Url));
		jclass jportalAndroidClass = FAndroidApplication::FindJavaClass("com/portal/unreal/PortalActivity");
		static jmethodID jlaunchUrl = FJavaWrapper::FindStaticMethod(Env, jportalAndroidClass, "startActivity", "(Landroid/app/Activity;Ljava/lang/String;)V", false);

		CallJniStaticVoidMethod(Env, jportalAndroidClass, jlaunchUrl, FJavaWrapper::GameActivityThis, jurl);
	}
}
#endif
