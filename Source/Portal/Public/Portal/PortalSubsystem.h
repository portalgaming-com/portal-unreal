// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
// clang-format off
#include "PortalSubsystem.generated.h"
// clang-format on

DECLARE_MULTICAST_DELEGATE_OneParam(FPortalSubsystemReadyDelegate, TWeakObjectPtr<class UPortalJSConnector>);

/**
 *
 */
UCLASS()
class PORTAL_API UPortalSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPortalSubsystem();

	virtual void Initialize(FSubsystemCollectionBase &Collection) override;
	virtual void Deinitialize() override;

	TWeakObjectPtr<class UPortalIdentity> GetIdentity() const
	{
		return MakeWeakObjectPtr(Identity);
	}

	bool IsReady() const { return bIsReady; }

	// FOnGameViewportTick& OnGameViewportTick() { return GameViewportTickEvent; }

	// Execute a delegate when the subsystem is ready (i.e.: when the browser is
	// running and the Portal SDK game bridge has loaded).
	template <class UserClass>
#if (ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1)
	void WhenReady(UserClass *Object, typename FPortalSubsystemReadyDelegate::FDelegate::TMethodPtr<UserClass> Func);
#else
	void WhenReady(UserClass *Object, typename FPortalSubsystemReadyDelegate::FDelegate::TUObjectMethodDelegate<UserClass>::FMethodPtr Func);
#endif

private:
	UPROPERTY()
	class UPortalBrowserUserWidget *BrowserWidget = nullptr;

	UPROPERTY()
	class UPortalBlui *PortalBlui = nullptr;

	UPROPERTY()
	class UPortalIdentity *Identity = nullptr;

	bool bHasSetupGameBridge = false;
	bool bIsReady = false;
	FPortalSubsystemReadyDelegate OnReady;

	FDelegateHandle WorldTickHandle;
	FDelegateHandle ViewportCreatedHandle;
#if PLATFORM_ANDROID | PLATFORM_IOS
	FDelegateHandle EngineInitCompleteHandle;
#endif

	void SetupGameBridge();
	void OnBridgeReady();
	void ManageBridgeDelegateQueue();
	void OnViewportCreated();
	void WorldTickStart(UWorld *World, ELevelTick TickType, float DeltaSeconds);
};
