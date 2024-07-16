// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal/Actions/PortalIdentityInitializationAsyncAction.h"

#include "Portal/PortalIdentity.h"
#include "Portal/PortalSubsystem.h"

UPortalIdentityInitializationAsyncAction *UPortalIdentityInitializationAsyncAction::InitializeIdentity(UObject *WorldContextObject, const FString &ClientID, const FString &RedirectUri, const FString &LogoutUri)
{
	UPortalIdentityInitializationAsyncAction *IdentityInitBlueprintNode = NewObject<UPortalIdentityInitializationAsyncAction>();

	IdentityInitBlueprintNode->ClientId = ClientID;
	IdentityInitBlueprintNode->RedirectUri = RedirectUri;
	IdentityInitBlueprintNode->LogoutUri = LogoutUri;
	IdentityInitBlueprintNode->WorldContextObject = WorldContextObject;

	return IdentityInitBlueprintNode;
}

void UPortalIdentityInitializationAsyncAction::Activate()
{
	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		Failed.Broadcast("Initialization failed due to missing world or world context object.");
		return;
	}

	GetSubsystem()->WhenReady(this, &UPortalIdentityInitializationAsyncAction::DoInit);
}

void UPortalIdentityInitializationAsyncAction::DoInit(TWeakObjectPtr<UPortalJSConnector> JSConnector)
{
	// Get Identity
	auto Identity = GetSubsystem()->GetIdentity();
	// Run Initialize
	Identity->Initialize(FPortalIdentityInitData{ClientId, RedirectUri, LogoutUri}, UPortalIdentity::FPortalIdentityResponseDelegate::CreateUObject(this, &UPortalIdentityInitializationAsyncAction::OnInitialized));
}

void UPortalIdentityInitializationAsyncAction::OnInitialized(FPortalIdentityResult Result)
{
	if (Result.Success)
	{
		Initialized.Broadcast(Result.Message);
	}
	else
	{
		Failed.Broadcast(Result.Message);
	}
}
