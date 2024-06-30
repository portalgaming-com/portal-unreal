#pragma once

#include "CoreMinimal.h"
#include "PortalBlueprintAsyncAction.h"

#include "PortalIdentityHasStoredCredentialsAsyncAction.generated.h"

/**
 *
 */
UCLASS()
class PORTAL_API UPortalIdentityHasStoredCredentialsAsyncAction : public UPortalBlueprintAsyncAction
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIdentityHasStoredCredentialsOutputPin, FString, ErrorMessage);

public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"), Category = "Portal")
	static UPortalIdentityHasStoredCredentialsAsyncAction *HasStoredCredentials(UObject *WorldContextObject);

	virtual void Activate() override;

private:
	void DoHasStoredCredentials(TWeakObjectPtr<class UPortalJSConnector> JSGetConnector);
	void OnHasStoredCredentialsResponse(struct FPortalIdentityResult Result);

	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "True"))
	FIdentityHasStoredCredentialsOutputPin OnTrue;
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "False"))
	FIdentityHasStoredCredentialsOutputPin OnFalse;
};
