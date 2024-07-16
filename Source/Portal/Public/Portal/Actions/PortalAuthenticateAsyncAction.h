// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Portal/PortalIdentity.h"
#include "PortalBlueprintAsyncAction.h"
#include "PortalAuthenticateAsyncAction.generated.h"

/**
 *
 */
UCLASS()
class PORTAL_API UPortalConnectionAsyncActions : public UPortalBlueprintAsyncAction
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIdentityConnectOutputPin, FString, ErrorMessage);

public:
	/**
	 * Log into Identity using Device Code Authorisation.
	 *
	 * @param	WorldContextObject	World context
	 * @param	UseCachedSession	Whether to use stored credentials for relogin
	 *
	 * @return	A reference to the object represented by this node
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"), Category = "Portal")
	static UPortalConnectionAsyncActions *Authenticate(UObject *WorldContextObject, bool UseCachedSession = false);

	/**
	 * Log into Identity using PKCE
	 *
	 * @param	WorldContextObject	World context
	 *
	 * @return	A reference to the object represented by this node
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"), Category = "Portal")
	static UPortalConnectionAsyncActions *AuthenticatePKCE(UObject *WorldContextObject);

private:
	FPortalIdentityInitDeviceFlowData InitDeviceFlowData;

	void DoConnect(TWeakObjectPtr<class UPortalJSConnector> JSConnector);
	void OnConnect(FPortalIdentityResult Result);

	UPROPERTY(BlueprintAssignable)
	FIdentityConnectOutputPin Success;
	UPROPERTY(BlueprintAssignable)
	FIdentityConnectOutputPin Failed;

	bool bUseCachedSession = false;
	bool bIsAuthenticate = false;
	bool bIsPKCE = false;
};
