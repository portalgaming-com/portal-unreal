// Fill out your copyright notice in the Description page of Project Settings.

#include "PortalBrowserWidget.h"

#include "Portal/Misc/PortalLogging.h"
#include "Portal/PortalJSConnector.h"
#if USING_BUNDLED_CEF
#include "SWebBrowser.h"
#endif
#include "Portal/Assets/PortalSDKResource.h"
#include "Portal/PortalSubsystem.h"
#include "Interfaces/IPluginManager.h"

UPortalBrowserWidget::UPortalBrowserWidget()
{
	PORTAL_LOG_FUNCSIG

	JSConnector = NewObject<UPortalJSConnector>(this, "JSConnector");
	JSConnector->ExecuteJs = UPortalJSConnector::FOnExecuteJsDelegate::CreateUObject(this, &UPortalBrowserWidget::ExecuteJS);

	// WebBrowserWidget->LoadString("<html><head><title>Test</title></head><body><h1>Test</h1></body></html>",
	// TEXT("http://www.google.com")); InitialURL = TEXT("http://www.google.com");
	// InitialURL = TEXT("chrome://version");
	// IPluginManager& PluginManager = IPluginManager::Get();
	// if (const TSharedPtr<IPlugin> Plugin =
	// PluginManager.FindPlugin("Portal"))
	// {
	//     InitialURL = FString::Printf(TEXT("%s%s"), TEXT("file:///"),
	//     *FPaths::ConvertRelativePathToFull(FPaths::Combine(Plugin->GetContentDir(),
	//     TEXT("index.html")))); PORTAL_LOG("Loading initial url: %s",
	//     *InitialURL)
	// }
	InitialURL = TEXT("about:blank");
}

void UPortalBrowserWidget::BindConnector()
{
	if (JSConnector && JSConnector->IsBound())
	{
		return;
	}

	PORTAL_LOG("Setting up %s...", *UPortalJSConnector::StaticClass()->GetName())

	if (JSConnector)
	{
		if (BindUObject(UPortalJSConnector::JSObjectName(), JSConnector))
		{
			JSConnector->Init(IsPageLoaded());
		}
	}
}

TWeakObjectPtr<UPortalJSConnector> UPortalBrowserWidget::GetJSConnector() const
{
	return JSConnector;
}

bool UPortalBrowserWidget::IsPageLoaded() const
{
#if USING_BUNDLED_CEF
	return WebBrowserWidget.IsValid() && WebBrowserWidget->IsLoaded();
#endif
	return false;
}

void UPortalBrowserWidget::ExecuteJS(const FString &ScriptText) const
{
#if USING_BUNDLED_CEF
	if (WebBrowserWidget.IsValid())
	{
		WebBrowserWidget->ExecuteJavascript(ScriptText);
	}
#endif
}

void UPortalBrowserWidget::SetBrowserContent()
{
#if USING_BUNDLED_CEF
	FSoftObjectPath AssetRef(TEXT("/Script/Portal.PortalSDKResource'/Portal/"
								  "PackagedResources/index.index'"));
	if (UObject *LoadedAsset = AssetRef.TryLoad())
	{
		if (auto Resource = Cast<UPortalSDKResource>(LoadedAsset))
		{
			if (!WebBrowserWidget.IsValid())
			{
				PORTAL_ERR("no browser")
				return;
			}

			const FString IndexHtml = FString("<!doctype html><html lang='en'><head><meta "
											  "charset='utf-8'><title>GameSDK Bridge</title><script>") +
									  Resource->Js + FString("</script></head><body><h1>Bridge Running</h1></body></html>");

			// PORTAL_LOG("Loaded resource: %s", *Resource->GetName())
			WebBrowserWidget->LoadString(IndexHtml, TEXT("file:///portal/index.html"));
			// WebBrowserWidget->LoadURL(FString::Printf(TEXT("%s%s"),
			// TEXT("file:///"),
			// *FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectContentDir(),
			// TEXT("html"), TEXT("index.html")))));
		}
	}
#endif
}

bool UPortalBrowserWidget::BindUObject(const FString &Name, UObject *Object, const bool bIsPermanent) const
{
#if USING_BUNDLED_CEF
	if (!WebBrowserWidget)
	{
		PORTAL_WARN_FUNC("Could not bind UObject '%s' to browser, WebBrowserWidget is null", *Object->GetName())
		return false;
	}

	WebBrowserWidget->BindUObject(Name, Object, bIsPermanent);
#endif
	return true;
}

void UPortalBrowserWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
#if USING_BUNDLED_CEF
	WebBrowserWidget.Reset();
#endif
}

TSharedRef<SWidget> UPortalBrowserWidget::RebuildWidget()
{
	if (IsDesignTime())
	{
		return SNew(SBox).HAlign(HAlign_Center).VAlign(VAlign_Center)[SNew(STextBlock).Text(NSLOCTEXT("Portal", "Portal Web Browser", "Portal Web Browser"))];
	}
	else
	{
#if USING_BUNDLED_CEF
		WebBrowserWidget = SNew(SWebBrowser).InitialURL(InitialURL).ShowControls(false).SupportsTransparency(bSupportsTransparency).ShowInitialThrobber(bShowInitialThrobber)
#if PLATFORM_ANDROID | PLATFORM_IOS
							   .OnLoadCompleted(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnLoadCompleted))
#endif
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
							   .OnConsoleMessage(BIND_UOBJECT_DELEGATE(FOnConsoleMessageDelegate, HandleOnConsoleMessage))
#endif
			;

		return WebBrowserWidget.ToSharedRef();
#else
		return SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)[SNew(STextBlock)
									   .Text(NSLOCTEXT("Portal",
													   "Portal Web Browser",
													   "Portal Web Browser"))];
#endif
	}
}

#if PLATFORM_ANDROID | PLATFORM_IOS
void UPortalBrowserWidget::HandleOnLoadCompleted()
{
	FString indexUrl = "file:///portal/index.html";

#if USING_BUNDLED_CEF
	if (WebBrowserWidget->GetUrl() == indexUrl)
	{
		JSConnector->SetMobileBridgeReady();
	}
	else
	{
		PORTAL_ERR("Portal Browser Widget Url don't match: (loaded : %s, required: %s)", *WebBrowserWidget->GetUrl(), *indexUrl);
	}
#endif
}
#endif

void UPortalBrowserWidget::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();
	BindConnector();
	SetBrowserContent();
}

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
void UPortalBrowserWidget::HandleOnConsoleMessage(const FString &Message, const FString &Source, int32 Line, EWebBrowserConsoleLogSeverity Severity)
{
	// TODO: add severity to log and callback
	PORTAL_LOG("Browser console message: %s, Source: %s, Line: %d", *Message, *Source, Line);
	OnConsoleMessage.Broadcast(Message, Source, Line);
}
#endif
