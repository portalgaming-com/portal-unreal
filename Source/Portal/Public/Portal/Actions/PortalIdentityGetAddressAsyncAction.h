// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Portal/PortalIdentity.h"
#include "PortalBlueprintAsyncAction.h"
#include "PortalIdentityGetAddressAsyncAction.generated.h"

/**
 *
 */
UCLASS()
class PORTAL_API UPortalIdentityGetAddressAsyncAction : public UPortalBlueprintAsyncAction
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FIdentityGetAddressOutputPin, FString, ErrorMessage, FString, Address);

public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"), Category = "Portal")
	static UPortalIdentityGetAddressAsyncAction *GetAddress(UObject *WorldContextObject);

	virtual void Activate() override;

private:
	void DoGetAddress(TWeakObjectPtr<class UPortalJSConnector> JSGetConnector);
	void OnGetAddressResponse(FPortalIdentityResult Result);

	UPROPERTY(BlueprintAssignable)
	FIdentityGetAddressOutputPin GotAddress;
	UPROPERTY(BlueprintAssignable)
	FIdentityGetAddressOutputPin Failed;
};
