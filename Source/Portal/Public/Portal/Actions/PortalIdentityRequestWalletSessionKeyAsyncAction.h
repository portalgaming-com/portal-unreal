// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Portal/PortalIdentity.h"
#include "PortalBlueprintAsyncAction.h"
#include "PortalIdentityRequestWalletSessionKeyAsyncAction.generated.h"


/**
 * Async action blueprint node to request a session key.
 */
UCLASS()
class PORTAL_API UPortalIdentityRequestWalletSessionKey : public UPortalBlueprintAsyncAction
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIdentityRequestWalletSessionKeyOutputPin, FString, ErrorMessage);

public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"), Category = "Portal")
	static UPortalIdentityRequestWalletSessionKey* RequestWalletSessionKey(UObject* WorldContextObject);

	virtual void Activate() override;

private:
	UPROPERTY(BlueprintAssignable)
	FIdentityRequestWalletSessionKeyOutputPin Success;
	UPROPERTY(BlueprintAssignable)
	FIdentityRequestWalletSessionKeyOutputPin Failed;

	void DoRequestWalletSessionKey(TWeakObjectPtr<class UPortalJSConnector> JSGetConnector);
	void OnRequestWalletSessionKeyResponse(FPortalIdentityResult Result);
};
