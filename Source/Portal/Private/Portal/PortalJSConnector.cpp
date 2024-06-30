// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal/PortalJSConnector.h"

#include "Portal/Misc/PortalLogging.h"
#include "PortalBrowserWidget.h"
#include "Portal/PortalJSMessages.h"

UPortalJSConnector::UPortalJSConnector() { PORTAL_LOG_FUNCSIG }

void UPortalJSConnector::Init(bool bPageLoaded)
{
	PORTAL_LOG("JSConnect::Init called, bPageloaded %d", bPageLoaded);
	if (bPageLoaded)
	{
		PORTAL_LOG("Browser finished loading the bridge document before the "
				   "Portal JSConnector was created & bound")
		FPortalJSResponse InitResponse{};
		InitResponse.responseFor = "init";
		InitResponse.requestId = "1";
		InitResponse.success = true;
		HandleInitResponse(InitResponse);
	}
	else
	{
		// Add init function to "waiting for response" queue to handle the init
		// message
		RequestResponseDelegates.Add("1", FPortalJSResponseDelegate::CreateUObject(this, &UPortalJSConnector::HandleInitResponse));
	}
}

void UPortalJSConnector::PostInitProperties() { UObject::PostInitProperties(); }

bool UPortalJSConnector::IsBound() const { return bIsBound; }

bool UPortalJSConnector::IsBridgeReady() const { return bIsBridgeReady; }

void UPortalJSConnector::AddCallbackWhenBridgeReady(const FOnBridgeReadyDelegate::FDelegate &Delegate)
{
	if (IsBridgeReady())
	{
		Delegate.Execute();
	}
	else
	{
		OnBridgeReady.Add(Delegate);
	}
}

FString UPortalJSConnector::CallJS(const FString &Function, const FString &Data, const FPortalJSResponseDelegate &HandleResponse, const float ResponseTimeout)
{
	const FString Guid = FGuid::NewGuid().ToString();
	CallJS(FPortalJSRequest{Function, Data, Guid}, HandleResponse, ResponseTimeout);
	return Guid;
}

void UPortalJSConnector::CallJS(const FPortalJSRequest &Request, FPortalJSResponseDelegate HandleResponse, float ResponseTimeout)
{
	if (!IsBridgeReady())
	{
		// PORTAL_WARN("Attempt to call Portal JS bridge before bridge ready")
		AddCallbackWhenBridgeReady(FOnBridgeReadyDelegate::FDelegate::CreateLambda([Request, HandleResponse, ResponseTimeout, this]()
																				   { CallJS(Request, HandleResponse, ResponseTimeout); }));
		return;
	}

	// First, add response callback to map
	RequestResponseDelegates.Add(Request.requestId, HandleResponse);

	// Convert the request to a string
	FString RequestString = Request.ToJsonString();
	RequestString.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
	RequestString.ReplaceInline(TEXT("\""), TEXT("\\\""));
	const FString JS = FString::Printf(TEXT("callFunction(\"%s\");"), *RequestString);
	// const FString JS = FString::Printf(TEXT("callFunction(%s);"),
	// *RequestString);

	// Execute
	PORTAL_LOG("Executing JS: %s", *JS)
	const bool Result = ExecuteJs.ExecuteIfBound(JS);
	if (!Result)
	{
		PORTAL_WARN("ExecuteJS delegate was not called");
	}

	// TODO: add timeout callback
}

void UPortalJSConnector::HandleInitResponse(FPortalJSResponse Response)
{
	PORTAL_LOG_FUNCSIG
	bIsBridgeReady = true;
	OnBridgeReady.Broadcast();
	OnBridgeReady.Clear();
}

void UPortalJSConnector::SendToGame(FString Message)
{
	PORTAL_LOG_FUNC("Received message from JS: %s", *Message);

	// Parse response

	const TOptional<FPortalJSResponse> Response = FPortalJSResponse::FromJsonString(Message);
	if (!Response.IsSet())
	{
		PORTAL_WARN("Received unexpected response from browser: %s", *Message);
		return;
	}

	// Handle response

	if (!RequestResponseDelegates.Contains(Response->requestId))
	{
		PORTAL_WARN("No delegate found for response with id %s", *Response->requestId);
		return;
	}

	if (!RequestResponseDelegates[Response->requestId].ExecuteIfBound(Response.GetValue()))
	{
		PORTAL_WARN("Delegate for response with id %s failed to execute", *Response->requestId);
	}

	RequestResponseDelegates.Remove(Response->requestId);
}

#if PLATFORM_ANDROID | PLATFORM_IOS
void UPortalJSConnector::SetMobileBridgeReady()
{
	PORTAL_LOG_FUNCSIG
	bIsBridgeReady = true;
	OnBridgeReady.Broadcast();
	OnBridgeReady.Clear();
}
#endif
