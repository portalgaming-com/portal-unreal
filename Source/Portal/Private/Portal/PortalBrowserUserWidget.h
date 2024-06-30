// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "PortalBrowserUserWidget.generated.h"

UCLASS()
class PORTAL_API UPortalBrowserUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void BeginDestroy() override;
	virtual void RemoveFromParent() override;
	virtual void OnWidgetRebuilt() override;

	TWeakObjectPtr<class UPortalJSConnector> GetJSConnector() const;

private:
	UPROPERTY()
	class UPortalBrowserWidget *Browser = nullptr;

	bool bIsBrowserAppInitialized = false;

	FTimerHandle Timer;

	FMargin DefaultOffsets = FMargin(150, 150, 150, 150);
};
