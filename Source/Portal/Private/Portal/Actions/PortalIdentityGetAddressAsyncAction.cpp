// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal/Actions/PortalIdentityGetAddressAsyncAction.h"

#include "Portal/PortalIdentity.h"
#include "Portal/PortalSubsystem.h"
#include "Portal/Misc/PortalLogging.h"

UPortalIdentityGetAddressAsyncAction *UPortalIdentityGetAddressAsyncAction::GetAddress(UObject *WorldContextObject)
{
	UPortalIdentityGetAddressAsyncAction *IdentityInitBlueprintNode = NewObject<UPortalIdentityGetAddressAsyncAction>();
	IdentityInitBlueprintNode->WorldContextObject = WorldContextObject;
	return IdentityInitBlueprintNode;
}

void UPortalIdentityGetAddressAsyncAction::Activate()
{
	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		FString Err = "GetAddress failed due to missing world or world context object.";
		PORTAL_WARN("%s", *Err)
		Failed.Broadcast(Err, TEXT(""));
		return;
	}

	GetSubsystem()->WhenReady(this, &UPortalIdentityGetAddressAsyncAction::DoGetAddress); //, /* timoutSec
																						  //*/ 15.0f);
}

void UPortalIdentityGetAddressAsyncAction::DoGetAddress(TWeakObjectPtr<UPortalJSConnector> JSConnector)
{
	// Get Identity
	auto Identity = GetSubsystem()->GetIdentity();
	// Run GetAddress
	Identity->GetAddress(UPortalIdentity::FPortalIdentityResponseDelegate::CreateUObject(this, &UPortalIdentityGetAddressAsyncAction::OnGetAddressResponse));
}

void UPortalIdentityGetAddressAsyncAction::OnGetAddressResponse(FPortalIdentityResult Result)
{
	if (Result.Success)
	{
		GotAddress.Broadcast(TEXT(""), Result.Message);
	}
	else
	{
		Failed.Broadcast(Result.Message, TEXT(""));
	}
}
