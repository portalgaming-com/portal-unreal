// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal/Actions/PortalIdentityGetEmailAsyncAction.h"

#include "Portal/PortalIdentity.h"
#include "Portal/PortalSubsystem.h"
#include "Portal/Misc/PortalLogging.h"

UPortalIdentityGetEmailAsyncAction *UPortalIdentityGetEmailAsyncAction::GetEmail(UObject *WorldContextObject)
{
	UPortalIdentityGetEmailAsyncAction *IdentityInitBlueprintNode = NewObject<UPortalIdentityGetEmailAsyncAction>();
	IdentityInitBlueprintNode->WorldContextObject = WorldContextObject;
	return IdentityInitBlueprintNode;
}

void UPortalIdentityGetEmailAsyncAction::Activate()
{
	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		FString Err = "GetEmail failed due to missing world or world context object.";
		PORTAL_WARN("%s", *Err)
		Failed.Broadcast(Err, TEXT(""));
		return;
	}

	GetSubsystem()->WhenReady(this, &UPortalIdentityGetEmailAsyncAction::DoGetEmail); //, /* timoutSec
																					  //*/ 15.0f);
}

void UPortalIdentityGetEmailAsyncAction::DoGetEmail(TWeakObjectPtr<UPortalJSConnector> JSConnector)
{
	// Get Identity
	auto Identity = GetSubsystem()->GetIdentity();
	// Run GetEmail
	Identity->GetEmail(UPortalIdentity::FPortalIdentityResponseDelegate::CreateUObject(this, &UPortalIdentityGetEmailAsyncAction::OnGetEmailResponse));
}

void UPortalIdentityGetEmailAsyncAction::OnGetEmailResponse(FPortalIdentityResult Result)
{
	if (Result.Success)
	{
		GotEmail.Broadcast(TEXT(""), Result.Message);
	}
	else
	{
		Failed.Broadcast(Result.Message, TEXT(""));
	}
}
