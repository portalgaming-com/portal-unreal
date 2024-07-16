#pragma once

namespace PortalIdentityAction
{
	const FString INIT = TEXT("init");
	const FString INIT_DEVICE_FLOW = TEXT("initDeviceFlow");
	const FString REAUTHENTICATE = TEXT("reauthenticate");
	const FString AUTHENTICATE_CONFIRM_CODE = TEXT("authenticateConfirmCode");
	const FString LOGOUT = TEXT("logout");

#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC
	const FString GET_PKCE_AUTH_URL = TEXT("getPKCEAuthUrl");
	const FString AUTHENTICATE_PKCE = TEXT("authenticatePKCE");
#endif

	const FString GET_ACCESS_TOKEN = TEXT("getAccessToken");
	const FString GET_ID_TOKEN = TEXT("getIdToken");
	const FString REQUEST_WALLET_SESSION_KEY = TEXT("requestWalletSessionKey");
	const FString EXECUTE_TRANSACTION = TEXT("executeTransaction");
} // namespace PortalIdentityAction
