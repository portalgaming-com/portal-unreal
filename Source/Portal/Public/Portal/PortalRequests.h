#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PortalRequests.generated.h"

USTRUCT(BlueprintType)
struct PORTAL_API FPortalExecuteTransactionRequest
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Portal")
	int32 ChainId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Portal")
	FString ContractId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Portal")
	FString PolicyId;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Portal")
	FString FunctionName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Portal")
	TArray<FString> FunctionArgs;

	FPortalExecuteTransactionRequest()
		: ChainId(0)
	{
		FunctionArgs = TArray<FString>();
	}

	FString ToJsonString() const;
};