// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal/PortalSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Portal/PortalIdentity.h"
#include "Portal/Misc/PortalLogging.h"
#include "PortalBlui.h"
#include "PortalBrowserUserWidget.h"
#include "Portal/PortalJSConnector.h"

UPortalSubsystem::UPortalSubsystem() { PORTAL_LOG_FUNCSIG }

void UPortalSubsystem::Initialize(FSubsystemCollectionBase &Collection)
{
	PORTAL_LOG_FUNCSIG
	Super::Initialize(Collection);

#if PLATFORM_ANDROID
	// Enable DOM storage so we can use localStorage in the Android webview
	GConfig->SetBool(
		TEXT("/Script/AndroidRuntimeSettings.AndroidRuntimeSettings"),
		TEXT("bEnableDomStorage"), true, GEngineIni);
#endif

#if PLATFORM_ANDROID | PLATFORM_IOS
	EngineInitCompleteHandle =
		FCoreDelegates::OnFEngineLoopInitComplete.AddUObject(
			this, &UPortalSubsystem::OnViewportCreated);
#else
	ViewportCreatedHandle = UGameViewportClient::OnViewportCreated().AddUObject(this, &UPortalSubsystem::OnViewportCreated);
#endif
	WorldTickHandle = FWorldDelegates::OnWorldTickStart.AddUObject(this, &UPortalSubsystem::WorldTickStart);
}

void UPortalSubsystem::Deinitialize()
{
	PORTAL_LOG_FUNCSIG

	BrowserWidget = nullptr;

#if USING_BLUI_CEF
	PortalBlui->ConditionalBeginDestroy();
	PortalBlui = nullptr;
#endif

	Identity = nullptr;

#if PLATFORM_ANDROID | PLATFORM_IOS
	UGameViewportClient::OnViewportCreated().Remove(EngineInitCompleteHandle);
#else
	UGameViewportClient::OnViewportCreated().Remove(ViewportCreatedHandle);
#endif
	FWorldDelegates::OnWorldTickStart.Remove(WorldTickHandle);

	Super::Deinitialize();
}

template <class UserClass>
#if (ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1)
void UPortalSubsystem::WhenReady(UserClass *Object, typename FPortalSubsystemReadyDelegate::FDelegate::TMethodPtr<UserClass> Func)
#else
void UPortalSubsystem::WhenReady(UserClass *Object, typename FPortalSubsystemReadyDelegate::FDelegate::TUObjectMethodDelegate<UserClass>::FMethodPtr Func)
#endif
{
	OnReady.AddUObject(Object, Func);
}

void UPortalSubsystem::OnBridgeReady()
{
	// When the bridge is ready our subsystem is ready to be used by game code.
	// Set the bIsReady flag and broadcast the OnReady event for any waiting
	// delegates.
	bIsReady = true;
	ManageBridgeDelegateQueue();
}

void UPortalSubsystem::ManageBridgeDelegateQueue()
{
	if (bIsReady)
	{
#if USING_BLUI_CEF
		OnReady.Broadcast(PortalBlui->GetJSConnector());
#else
		OnReady.Broadcast(BrowserWidget->GetJSConnector());
#endif
		OnReady.Clear();
	}
}

void UPortalSubsystem::SetupGameBridge()
{
	if (bHasSetupGameBridge)
	{
		return;
	}
	bHasSetupGameBridge = true;

#if USING_BLUI_CEF
	// Create the Blui
	if (!PortalBlui)
	{
		PortalBlui = NewObject<UPortalBlui>();
		PortalBlui->Init();
	}

	if (!PortalBlui)
	{
		PORTAL_ERR("Failed to create UPortalBlui")
		return;
	}
	if (!PortalBlui->GetJSConnector().IsValid())
	{
		PORTAL_ERR(
			"JSConnector not available, can't set up subsystem-ready event chain")
		return;
	}
	// Set up ready event chain
	if (!IsReady())
	{
		PortalBlui->GetJSConnector()->AddCallbackWhenBridgeReady(
			UPortalJSConnector::FOnBridgeReadyDelegate::FDelegate::CreateUObject(
				this, &UPortalSubsystem::OnBridgeReady));
	}

	// Prepare Identity
	if (!Identity)
	{
		Identity = NewObject<UPortalIdentity>(this);
		if (Identity)
			Identity->Setup(PortalBlui->GetJSConnector());
	}

#else
	// Create the browser widget
	if (!BrowserWidget)
	{
		BrowserWidget = CreateWidget<UPortalBrowserUserWidget>(GetWorld());
	}
	if (!BrowserWidget)
	{
		PORTAL_ERR("Failed to create up BrowserWidget")
		return;
	}
	// Launch browser
	if (!BrowserWidget->IsInViewport())
	{
		PORTAL_LOG("Adding BrowserWidget to viewport")
		BrowserWidget->AddToViewport();
	}
	if (!BrowserWidget->GetJSConnector().IsValid())
	{
		PORTAL_ERR("JSConnector not available, can't set up subsystem-ready event chain")
		return;
	}
	// Set up ready event chain
	if (!IsReady())
	{
		BrowserWidget->GetJSConnector()->AddCallbackWhenBridgeReady(UPortalJSConnector::FOnBridgeReadyDelegate::FDelegate::CreateUObject(this, &UPortalSubsystem::OnBridgeReady));
	}

	// Prepare Identity
	if (!Identity)
	{
		Identity = NewObject<UPortalIdentity>(this);
		if (Identity)
		{
			Identity->Setup(BrowserWidget->GetJSConnector());
		}
	}
#endif
}

void UPortalSubsystem::OnViewportCreated()
{
	PORTAL_LOG_FUNCSIG
	SetupGameBridge();
}

void UPortalSubsystem::WorldTickStart(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
	ManageBridgeDelegateQueue();
}
