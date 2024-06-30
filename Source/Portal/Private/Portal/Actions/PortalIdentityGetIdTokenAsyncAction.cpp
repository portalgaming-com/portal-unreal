// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal/Actions/PortalIdentityGetIdTokenAsyncAction.h"

#include "Portal/PortalIdentity.h"
#include "Portal/PortalSubsystem.h"
#include "Portal/Misc/PortalLogging.h"

UPortalIdentityGetIdTokenAsyncAction *UPortalIdentityGetIdTokenAsyncAction::GetIdToken(UObject *WorldContextObject)
{
	UPortalIdentityGetIdTokenAsyncAction *IdentityInitBlueprintNode = NewObject<UPortalIdentityGetIdTokenAsyncAction>();
	IdentityInitBlueprintNode->WorldContextObject = WorldContextObject;
	return IdentityInitBlueprintNode;
}

void UPortalIdentityGetIdTokenAsyncAction::Activate()
{
	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		FString Err = "GetIdToken failed due to missing world or world context object.";
		PORTAL_WARN("%s", *Err)
		Failed.Broadcast(Err, TEXT(""));
		return;
	}

	GetSubsystem()->WhenReady(this, &UPortalIdentityGetIdTokenAsyncAction::DoGetIdToken);
}

void UPortalIdentityGetIdTokenAsyncAction::DoGetIdToken(TWeakObjectPtr<UPortalJSConnector> JSConnector)
{
	// Get Identity
	auto Identity = GetSubsystem()->GetIdentity();
	// Run GetIdToken
	Identity->GetIdToken(UPortalIdentity::FPortalIdentityResponseDelegate::CreateUObject(this, &UPortalIdentityGetIdTokenAsyncAction::OnGetIdTokenResponse));
}

void UPortalIdentityGetIdTokenAsyncAction::OnGetIdTokenResponse(FPortalIdentityResult Result)
{
	if (Result.Success)
	{
		GotIdToken.Broadcast(TEXT(""), Result.Message);
	}
	else
	{
		Failed.Broadcast(Result.Message, TEXT(""));
	}
}
