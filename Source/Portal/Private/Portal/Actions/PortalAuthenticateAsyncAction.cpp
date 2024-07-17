// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal/Actions/PortalAuthenticateAsyncAction.h"

#include "Portal/PortalIdentity.h"
#include "Portal/PortalSubsystem.h"
#include "Portal/Misc/PortalLogging.h"

UPortalConnectionAsyncActions *UPortalConnectionAsyncActions::Authenticate(UObject *WorldContextObject, bool UseCachedSession)
{
	UPortalConnectionAsyncActions *IdentityInitBlueprintNode = NewObject<UPortalConnectionAsyncActions>();

	IdentityInitBlueprintNode->WorldContextObject = WorldContextObject;
	IdentityInitBlueprintNode->bUseCachedSession = UseCachedSession;
	IdentityInitBlueprintNode->bIsAuthenticate = false;

	return IdentityInitBlueprintNode;
}

UPortalConnectionAsyncActions *UPortalConnectionAsyncActions::AuthenticatePKCE(UObject *WorldContextObject)
{
	UPortalConnectionAsyncActions *IdentityInitBlueprintNode = NewObject<UPortalConnectionAsyncActions>();

	IdentityInitBlueprintNode->WorldContextObject = WorldContextObject;
	IdentityInitBlueprintNode->bIsAuthenticate = false;
	IdentityInitBlueprintNode->bIsPKCE = true;

	return IdentityInitBlueprintNode;
}

void UPortalConnectionAsyncActions::Activate()
{
	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		FString Error = "Connect failed due to missing world or world context object.";
		PORTAL_WARN("%s", *Error)
		Failed.Broadcast(Error);

		return;
	}

	GetSubsystem()->WhenReady(this, &UPortalConnectionAsyncActions::DoConnect);
}

void UPortalConnectionAsyncActions::DoConnect(TWeakObjectPtr<UPortalJSConnector> JSConnector)
{
	auto Identity = GetSubsystem()->GetIdentity();

	if (Identity.IsValid())
	{
		if (bIsPKCE)
		{
#if PLATFORM_ANDROID | PLATFORM_IOS | PLATFORM_MAC
			Identity->AuthenticatePKCE(UPortalIdentity::FPortalIdentityResponseDelegate::CreateUObject(this, &UPortalConnectionAsyncActions::OnConnect));
#endif
		}
		else
		{
			Identity->Authenticate(bUseCachedSession, UPortalIdentity::FPortalIdentityResponseDelegate::CreateUObject(this, &UPortalConnectionAsyncActions::OnConnect));
		}
	}
	else
	{
		PORTAL_ERR("Identity was not valid while trying to connect")
	}
}

void UPortalConnectionAsyncActions::OnConnect(FPortalIdentityResult Result)
{
	if (Result.Success)
	{
		Success.Broadcast(TEXT(""));
	}
	else
	{
		Failed.Broadcast(Result.Message);
	}
}
