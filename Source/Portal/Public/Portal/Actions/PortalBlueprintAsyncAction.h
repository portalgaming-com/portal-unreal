// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "PortalBlueprintAsyncAction.generated.h"

/**

 * base class for asynchronous actions
 */
UCLASS()
class PORTAL_API UPortalBlueprintAsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

protected:
	UObject *WorldContextObject;

	// Get subsystem
	class UPortalSubsystem *GetSubsystem() const;
};
