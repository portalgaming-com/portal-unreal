// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal/Actions/PortalIdentityLogoutAsyncAction.h"

#include "Portal/PortalIdentity.h"
#include "Portal/PortalSubsystem.h"
#include "Portal/Misc/PortalLogging.h"

UPortalIdentityLogoutAsyncAction *UPortalIdentityLogoutAsyncAction::Logout(UObject *WorldContextObject, bool DoHardLogout)
{
	UPortalIdentityLogoutAsyncAction *IdentityInitBlueprintNode = NewObject<UPortalIdentityLogoutAsyncAction>();

	IdentityInitBlueprintNode->WorldContextObject = WorldContextObject;
	IdentityInitBlueprintNode->bDoHardLogout = DoHardLogout;

	return IdentityInitBlueprintNode;
}

void UPortalIdentityLogoutAsyncAction::Activate()
{
	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		const FString ErrorMessage = "Logout failed due to missing world or world context object.";

		PORTAL_WARN("%s", *ErrorMessage)
		OnFailure.Broadcast(ErrorMessage);

		return;
	}

	GetSubsystem()->WhenReady(this, &UPortalIdentityLogoutAsyncAction::DoLogout);
}

void UPortalIdentityLogoutAsyncAction::DoLogout(TWeakObjectPtr<UPortalJSConnector> JSConnector)
{
	auto Identity = GetSubsystem()->GetIdentity();

	Identity->Logout(bDoHardLogout, UPortalIdentity::FPortalIdentityResponseDelegate::CreateUObject(this, &UPortalIdentityLogoutAsyncAction::OnLogoutResponse));
}

void UPortalIdentityLogoutAsyncAction::OnLogoutResponse(FPortalIdentityResult Result) const
{
	if (Result.Success)
	{
		OnSuccess.Broadcast(Result.Message);
	}
	else
	{
		OnFailure.Broadcast(Result.Message);
	}
}
