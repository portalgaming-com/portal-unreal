// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Portal/PortalIdentity.h"
#include "PortalBlueprintAsyncAction.h"

#include "PortalIdentityInitializationAsyncAction.generated.h"

/**
 * Async action to instantiate
 */
UCLASS()
class PORTAL_API UPortalIdentityInitializationAsyncAction : public UPortalBlueprintAsyncAction
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIdentityInitializationOutputPin, FString, Message);

public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"), Category = "Portal")
	static UPortalIdentityInitializationAsyncAction *InitializeIdentity(UObject *WorldContextObject, const FString &ClientID, const FString &RedirectUri, const FString &LogoutUri, const FString &Environment);

	virtual void Activate() override;

private:
	FString ClientId;
	FString RedirectUri;
	FString LogoutUri;
	FString Environment;

	UPROPERTY(BlueprintAssignable)
	FIdentityInitializationOutputPin Initialized;
	UPROPERTY(BlueprintAssignable)
	FIdentityInitializationOutputPin Failed;

	void DoInit(TWeakObjectPtr<class UPortalJSConnector> JSConnector);
	void OnInitialized(FPortalIdentityResult Result);
};
