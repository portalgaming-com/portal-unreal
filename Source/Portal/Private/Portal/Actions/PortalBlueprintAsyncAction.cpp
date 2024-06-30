// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal/Actions/PortalBlueprintAsyncAction.h"

UPortalSubsystem *UPortalBlueprintAsyncAction::GetSubsystem() const
{
	if (!WorldContextObject || !WorldContextObject->GetWorld())
	{
		return nullptr;
	}
	return WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UPortalSubsystem>();
}
