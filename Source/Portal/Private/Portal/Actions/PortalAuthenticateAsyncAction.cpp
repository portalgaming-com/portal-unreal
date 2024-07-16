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