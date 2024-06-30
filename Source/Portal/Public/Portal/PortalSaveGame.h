// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/SaveGame.h"

#include "PortalSaveGame.generated.h"

/**
 * Portal sdk save game object
 */
UCLASS(BlueprintType, Blueprintable)
class PORTAL_API UPortalSaveGame : public USaveGame
{
	GENERATED_UCLASS_BODY()

public:
	/** check if player logged in/connected with PKCE flow previously */
	UPROPERTY(VisibleAnywhere, Category = "Portal")
	bool bWasConnectedViaPKCEFlow;
};
