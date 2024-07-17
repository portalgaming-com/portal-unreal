// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Portal/PortalIdentity.h"
#include "PortalBlueprintAsyncAction.h"
#include "PortalIdentityExecuteTransactionAsyncAction.generated.h"

/**
 * Async action blueprint node for EVM Execute Transaction
 */
UCLASS()
class PORTAL_API UPortalIdentityExecuteTransactionAsyncAction : public UPortalBlueprintAsyncAction
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FIdentityExecuteTransactionOutputPin, FString, ErrorMessage, FString, Transaction);

public:
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"), Category = "Portal")
	static UPortalIdentityExecuteTransactionAsyncAction* ExecuteTransaction(UObject* WorldContextObject, const FPortalExecuteTransactionRequest& Request);

	virtual void Activate() override;

private:
	FPortalExecuteTransactionRequest TransactionRequest;

	UPROPERTY(BlueprintAssignable)
	FIdentityExecuteTransactionOutputPin TransactionSent;
	UPROPERTY(BlueprintAssignable)
	FIdentityExecuteTransactionOutputPin Failed;

	void DoExecuteTransaction(TWeakObjectPtr<class UPortalJSConnector> JSGetConnector);
	void OnExecuteTransactionResponse(FPortalIdentityResult Result);
};