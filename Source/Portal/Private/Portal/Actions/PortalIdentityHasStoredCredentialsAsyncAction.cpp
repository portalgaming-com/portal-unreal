#include "Portal/Actions/PortalIdentityHasStoredCredentialsAsyncAction.h"

#include "Portal/PortalIdentity.h"
#include "Portal/PortalSubsystem.h"
#include "Portal/Misc/PortalLogging.h"

UPortalIdentityHasStoredCredentialsAsyncAction *UPortalIdentityHasStoredCredentialsAsyncAction::HasStoredCredentials(UObject *WorldContextObject)
{
	UPortalIdentityHasStoredCredentialsAsyncAction *IdentityInitBlueprintNode = NewObject<UPortalIdentityHasStoredCredentialsAsyncAction>();

	IdentityInitBlueprintNode->WorldContextObject = WorldContextObject;

	return IdentityInitBlueprintNode;
}

void UPortalIdentityHasStoredCredentialsAsyncAction::Activate()
{
	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		FString Err = "HasStoredCredentials failed due to missing world or world context object.";
		PORTAL_WARN("%s", *Err)
		OnFalse.Broadcast(Err);
		return;
	}

	GetSubsystem()->WhenReady(this, &UPortalIdentityHasStoredCredentialsAsyncAction::DoHasStoredCredentials);
}

void UPortalIdentityHasStoredCredentialsAsyncAction::DoHasStoredCredentials(TWeakObjectPtr<UPortalJSConnector> JSConnector)
{
	const auto Identity = GetSubsystem()->GetIdentity();

	if (Identity.IsValid())
	{
		Identity->HasStoredCredentials(UPortalIdentity::FPortalIdentityResponseDelegate::CreateUObject(this, &UPortalIdentityHasStoredCredentialsAsyncAction::OnHasStoredCredentialsResponse));
	}
}

void UPortalIdentityHasStoredCredentialsAsyncAction::OnHasStoredCredentialsResponse(FPortalIdentityResult Result)
{
	if (Result.Success)
	{
		OnTrue.Broadcast(TEXT(""));
	}
	else
	{
		OnFalse.Broadcast(Result.Message);
	}
}
