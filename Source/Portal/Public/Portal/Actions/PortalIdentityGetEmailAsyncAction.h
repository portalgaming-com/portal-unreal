// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Portal/PortalIdentity.h"
#include "PortalBlueprintAsyncAction.h"
#include "PortalIdentityGetEmailAsyncAction.generated.h"

/**
 *
 */
UCLASS()
class PORTAL_API UPortalIdentityGetEmailAsyncAction : public UPortalBlueprintAsyncAction
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FIdentityGetEmailOutputPin, FString, ErrorMessage, FString, Email);

public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"), Category = "Portal")
	static UPortalIdentityGetEmailAsyncAction *GetEmail(UObject *WorldContextObject);

	virtual void Activate() override;

private:
	void DoGetEmail(TWeakObjectPtr<class UPortalJSConnector> JSGetConnector);
	void OnGetEmailResponse(FPortalIdentityResult Result);

	UPROPERTY(BlueprintAssignable)
	FIdentityGetEmailOutputPin GotEmail;
	UPROPERTY(BlueprintAssignable)
	FIdentityGetEmailOutputPin Failed;
};
