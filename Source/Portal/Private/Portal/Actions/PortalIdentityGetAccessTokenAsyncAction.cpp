// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal/Actions/PortalIdentityGetAccessTokenAsyncAction.h"

#include "Portal/PortalIdentity.h"
#include "Portal/PortalSubsystem.h"
#include "Portal/Misc/PortalLogging.h"

UPortalIdentityGetAccessTokenAsyncAction *UPortalIdentityGetAccessTokenAsyncAction::GetAccessToken(UObject *WorldContextObject)
{
	UPortalIdentityGetAccessTokenAsyncAction *IdentityInitBlueprintNode = NewObject<UPortalIdentityGetAccessTokenAsyncAction>();
	IdentityInitBlueprintNode->WorldContextObject = WorldContextObject;
	return IdentityInitBlueprintNode;
}

void UPortalIdentityGetAccessTokenAsyncAction::Activate()
{
	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		FString Err = "GetAccessToken failed due to missing world or world context object.";
		PORTAL_WARN("%s", *Err)
		Failed.Broadcast(Err, TEXT(""));
		return;
	}

	GetSubsystem()->WhenReady(this, &UPortalIdentityGetAccessTokenAsyncAction::DoGetAccessToken);
}

void UPortalIdentityGetAccessTokenAsyncAction::DoGetAccessToken(TWeakObjectPtr<UPortalJSConnector> JSConnector)
{
	// Get Identity
	auto Identity = GetSubsystem()->GetIdentity();
	// Run GetAccessToken
	Identity->GetAccessToken(UPortalIdentity::FPortalIdentityResponseDelegate::CreateUObject(this, &UPortalIdentityGetAccessTokenAsyncAction::OnGetAccessTokenResponse));
}

void UPortalIdentityGetAccessTokenAsyncAction::OnGetAccessTokenResponse(FPortalIdentityResult Result)
{
	if (Result.Success)
	{
		GotAccessToken.Broadcast(TEXT(""), Result.Message);
	}
	else
	{
		Failed.Broadcast(Result.Message, TEXT(""));
	}
}
