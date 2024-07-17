// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal/Actions/PortalIdentityRequestWalletSessionKeyAsyncAction.h"

#include "Portal/PortalIdentity.h"
#include "Portal/PortalSubsystem.h"
#include "Portal/Misc/PortalLogging.h"


UPortalIdentityRequestWalletSessionKey* UPortalIdentityRequestWalletSessionKey::RequestWalletSessionKey(
	UObject* WorldContextObject)
{
	UPortalIdentityRequestWalletSessionKey* RequestWalletSessionKeyBPNode = NewObject<UPortalIdentityRequestWalletSessionKey>();
	
	RequestWalletSessionKeyBPNode->WorldContextObject = WorldContextObject;
	
	return RequestWalletSessionKeyBPNode;
}

void UPortalIdentityRequestWalletSessionKey::Activate()
{
	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		FString Err = "Request Wallet Session Key failed due to missing world context object.";

		PORTAL_WARN("%s", *Err)
		Failed.Broadcast(Err);
		
		return;
	}

	GetSubsystem()->WhenReady(this, &UPortalIdentityRequestWalletSessionKey::DoRequestWalletSessionKey);
}

void UPortalIdentityRequestWalletSessionKey::DoRequestWalletSessionKey(TWeakObjectPtr<UPortalJSConnector> JSConnector)
{
	auto Identity = GetSubsystem()->GetIdentity();

	if (Identity.IsValid())
	{
		Identity->RequestWalletSessionKey(UPortalIdentity::FPortalIdentityResponseDelegate::CreateUObject(this, &UPortalIdentityRequestWalletSessionKey::OnRequestWalletSessionKeyResponse));
	}
}

void UPortalIdentityRequestWalletSessionKey::OnRequestWalletSessionKeyResponse(FPortalIdentityResult Result)
{
	if (Result.Success)
	{
		PORTAL_LOG("Request Wallet Session Key successful")
		Success.Broadcast(TEXT(""));
	}
	else
	{
		PORTAL_LOG("Request Wallet Session Key failed")
		Failed.Broadcast(Result.Message);
	}
}
