#pragma once

#include "CoreMinimal.h"
#include "Portal/PortalIdentity.h"
#include "PortalBlueprintAsyncAction.h"
#include "PortalIdentityLogoutAsyncAction.generated.h"

/**
 *
 */
// UCLASS(meta = (HasDedicatedAsyncNode))
UCLASS()
class PORTAL_API UPortalIdentityLogoutAsyncAction : public UPortalBlueprintAsyncAction
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIdentityLogoutOutPin, FString, Message);

public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"), Category = "Portal")
	static UPortalIdentityLogoutAsyncAction *Logout(UObject *WorldContextObject, bool DoHardLogout = true);

	virtual void Activate() override;

private:
	void DoLogout(TWeakObjectPtr<class UPortalJSConnector> JSConnector);
	void OnLogoutResponse(FPortalIdentityResult Result) const;

private:
	bool bDoHardLogout = true;

	UPROPERTY(BlueprintAssignable)
	FIdentityLogoutOutPin OnSuccess;
	UPROPERTY(BlueprintAssignable)
	FIdentityLogoutOutPin OnFailure;
};
