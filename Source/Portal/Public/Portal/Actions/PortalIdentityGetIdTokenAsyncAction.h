// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Portal/PortalIdentity.h"
#include "PortalBlueprintAsyncAction.h"
#include "PortalIdentityGetIdTokenAsyncAction.generated.h"

/**
 *
 */
UCLASS()
class PORTAL_API UPortalIdentityGetIdTokenAsyncAction : public UPortalBlueprintAsyncAction
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FIdentityGetIdTokenOutputPin, FString, ErrorMessage, FString, IdToken);

public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"), Category = "Portal")
	static UPortalIdentityGetIdTokenAsyncAction *GetIdToken(UObject *WorldContextObject);

	virtual void Activate() override;

private:
	void DoGetIdToken(TWeakObjectPtr<class UPortalJSConnector> JSGetConnector);
	void OnGetIdTokenResponse(FPortalIdentityResult Result);

	UPROPERTY(BlueprintAssignable)
	FIdentityGetIdTokenOutputPin GotIdToken;
	UPROPERTY(BlueprintAssignable)
	FIdentityGetIdTokenOutputPin Failed;
};
