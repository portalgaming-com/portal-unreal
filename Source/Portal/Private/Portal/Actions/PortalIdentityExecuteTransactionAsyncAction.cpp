// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal/Actions/PortalIdentityExecuteTransactionAsyncAction.h"

#include "Portal/PortalIdentity.h"
#include "Portal/PortalSubsystem.h"
#include "Portal/Misc/PortalLogging.h"

UPortalIdentityExecuteTransactionAsyncAction* UPortalIdentityExecuteTransactionAsyncAction::ExecuteTransaction(UObject* WorldContextObject, const FPortalExecuteTransactionRequest& Request)
{
	UPortalIdentityExecuteTransactionAsyncAction* IdentityExecuteTransactionBlueprintNode = NewObject<UPortalIdentityExecuteTransactionAsyncAction>();
	IdentityExecuteTransactionBlueprintNode->WorldContextObject = WorldContextObject;
	IdentityExecuteTransactionBlueprintNode->TransactionRequest = Request;
	return IdentityExecuteTransactionBlueprintNode;
}

void UPortalIdentityExecuteTransactionAsyncAction::Activate()
{
	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		FString Err = "EVM Execute Transaction failed due to missing world or world " "context object.";
		PORTAL_WARN("%s", *Err)
		Failed.Broadcast(Err, TEXT(""));
		return;
	}

	GetSubsystem()->WhenReady(this, &UPortalIdentityExecuteTransactionAsyncAction::DoExecuteTransaction);
}

void UPortalIdentityExecuteTransactionAsyncAction::DoExecuteTransaction(TWeakObjectPtr<UPortalJSConnector> JSConnector)
{
	// Get Identity
	auto Identity = GetSubsystem()->GetIdentity();
	// Run ExecuteTransaction
	Identity->ExecuteTransaction(TransactionRequest, UPortalIdentity::FPortalIdentityResponseDelegate::CreateUObject(this, &UPortalIdentityExecuteTransactionAsyncAction::OnExecuteTransactionResponse));
}

void UPortalIdentityExecuteTransactionAsyncAction::OnExecuteTransactionResponse(FPortalIdentityResult Result)
{
	if (Result.Success)
	{
		PORTAL_LOG("EVM Send Transaction success")
		TransactionSent.Broadcast(TEXT(""), Result.Message);
	}
	else
	{
		PORTAL_LOG("EVM Send Transaction failed")
		Failed.Broadcast(Result.Message, TEXT(""));
	}
}