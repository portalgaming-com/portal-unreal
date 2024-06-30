// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Portal/PortalIdentity.h"
#include "PortalBlueprintAsyncAction.h"
#include "PortalIdentityGetAccessTokenAsyncAction.generated.h"

/**
 *
 */
UCLASS()
class PORTAL_API UPortalIdentityGetAccessTokenAsyncAction : public UPortalBlueprintAsyncAction
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FIdentityGetAccessTokenOutputPin, FString, ErrorMessage, FString, AccessToken);

public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"), Category = "Portal")
	static UPortalIdentityGetAccessTokenAsyncAction *GetAccessToken(UObject *WorldContextObject);

	virtual void Activate() override;

private:
	void DoGetAccessToken(TWeakObjectPtr<class UPortalJSConnector> JSGetConnector);
	void OnGetAccessTokenResponse(FPortalIdentityResult Result);

	UPROPERTY(BlueprintAssignable)
	FIdentityGetAccessTokenOutputPin GotAccessToken;
	UPROPERTY(BlueprintAssignable)
	FIdentityGetAccessTokenOutputPin Failed;
};
