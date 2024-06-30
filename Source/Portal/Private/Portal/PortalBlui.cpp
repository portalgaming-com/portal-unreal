// Fill out your copyright notice in the Description page of Project Settings.

#include "PortalBlui.h"
#include "Portal/Assets/PortalSDKResource.h"
#include "Portal/Misc/PortalLogging.h"
#include "Portal/PortalJSConnector.h"
#include <utility>

#if USING_BLUI_CEF
#include "BluEye.h"
#endif

UPortalBlui::UPortalBlui()
{
  PORTAL_LOG_FUNCSIG
#if USING_BLUI_CEF
  if (!BluEyePtr)
  {
    PORTAL_LOG("Creating BluEye")
    BluEyePtr = NewObject<UBluEye>(this, "BluEye");
  }
#endif

  JSConnector = NewObject<UPortalJSConnector>(this, "JSConnector");
  JSConnector->ExecuteJs = UPortalJSConnector::FOnExecuteJsDelegate::CreateUObject(this, &UPortalBlui::ExecuteJS);
}

#if USING_BLUI_CEF
UBluEye *UPortalBlui::GetBluEye() const { return Cast<UBluEye>(BluEyePtr); }
#endif

void UPortalBlui::OnLogEvent(const FString &LogText)
{
  PORTAL_LOG_FUNC("LogEvent: %s", *LogText);
}

void UPortalBlui::WorldTickStart(UWorld *World, ELevelTick LevelTick, float X)
{
#if USING_BLUI_CEF
  if (!GetBluEye()->IsBrowserLoading() && !bLoadedIndexJS)
  {
    const FSoftObjectPath AssetRef(
        TEXT("/Script/Portal.PortalSDKResource'/Portal/PackagedResources/"
             "index.index'"));

    PORTAL_LOG("Browser loaded");
    bLoadedIndexJS = true;
    if (UObject *LoadedAsset = AssetRef.TryLoad())
    {
      if (const auto Resource = Cast<UPortalSDKResource>(LoadedAsset))
      {
        GetBluEye()->ExecuteJS(Resource->Js);
        PORTAL_VERBOSE("Loaded index.js")
      }
      else
      {
        PORTAL_ERR("Error in loading index.js")
      }
    }
    else
    {
      PORTAL_ERR("Error in loading index.js")
    }
  }
#endif
}

void UPortalBlui::BeginDestroy()
{
  PORTAL_LOG_FUNCSIG
#if USING_BLUI_CEF
  if (GetBluEye())
  {
    GetBluEye()->CloseBrowser();
  }
  BluEyePtr = nullptr;
#endif
  Super::BeginDestroy();
}

void UPortalBlui::OnScriptEvent(const FString &EventName, const FString &EventMessage)
{
  PORTAL_LOG_FUNC("EventName: %s, EventMessage: %s", *EventName, *EventMessage);
  JSConnector->SendToGame(EventMessage);
}

TWeakObjectPtr<UPortalJSConnector> UPortalBlui::GetJSConnector() const
{
  return JSConnector;
}

void UPortalBlui::ExecuteJS(const FString &ScriptText) const
{
#if USING_BLUI_CEF
  if (GetBluEye())
  {
    GetBluEye()->ExecuteJS(ScriptText);
  }
#endif
}

void UPortalBlui::Init()
{
  PORTAL_LOG_FUNCSIG

#if USING_BLUI_CEF
  // Todo: Add comment here why GetBluEye
  UBluEye *BluEye = GetBluEye();

  BluEye->LogEventEmitter.AddUniqueDynamic(this, &UPortalBlui::OnLogEvent);
  BluEye->ScriptEventEmitter.AddUniqueDynamic(this, &UPortalBlui::OnScriptEvent);

  BluEye->bEnabled = true;
  PORTAL_LOG("Events subscribed")

  BluEye->Init();
  PORTAL_LOG("BluEye Initialised")

  FSoftObjectPath AssetRef(TEXT("/Script/Portal.PortalSDKResource'/Portal/"
                                "PackagedResources/index.index'"));
  if (UObject *LoadedAsset = AssetRef.TryLoad())
  {
    if (auto Resource = Cast<UPortalSDKResource>(LoadedAsset))
    {
      // We're attempting to replicate the process that Unreal's WebBrowser
      // widget uses to load a page from a string. Unfortunately this doesn't
      // work correctly, but it still solves our issue. LocalStorage can't be
      // accessed from about:blank or data URIs, so we still need to load a
      // page.  Despite this failing to load our custom html, this approach
      // still allows us to access LocalStorage and use the game bridge.  If
      // there was more time in the future it would probably be worth
      // investigating the issues here.

      // PostData
      CefRefPtr<CefPostData> PostData = CefPostData::Create();
      CefRefPtr<CefPostDataElement> Element = CefPostDataElement::Create();
      FTCHARToUTF8 UTF8String(TEXT(
          "<!doctype html><html lang='en'><head><meta "
          "charset='utf-8'><title>GameSDK "
          "Bridge</title></head><body><h1>Bridge Running</h1></body></html>"));
      Element->SetToBytes(UTF8String.Length(), UTF8String.Get());
      PostData->AddElement(Element);

      CefRequest::HeaderMap HeaderMap;
      HeaderMap.insert(std::pair<CefString, CefString>(
          TCHAR_TO_WCHAR(TEXT("Content-Type")), "html"));

      const FString CustomContentMethod(TEXT("X-GET-CUSTOM-CONTENT"));

      const auto Request = CefRequest::Create();
      Request->Set("file:///Portal/index.html", *CustomContentMethod,
                   PostData, HeaderMap);

      GetBluEye()->Browser->GetMainFrame()->LoadRequest(Request);
      PORTAL_VERBOSE("LoadRequest'ed for Index.html")

      WorldTickHandle = FWorldDelegates::OnWorldTickStart.AddUObject(
          this, &UPortalBlui::WorldTickStart);
      PORTAL_LOG("Waiting for Browser to load...");
    }
  }
  else
  {
    PORTAL_ERR("Failed to load Portal bridge asset.")
  }

  // Do this after the the page is given to the browser and being loaded...
  JSConnector->Init(!BluEye->IsBrowserLoading());
#endif
}

#if USING_BLUI_CEF
void UPortalBlui::StopBluiEventLoop()
{
  if (UBluEye *BluEye = GetBluEye())
  {
    BluEye->SetShouldTickEventLoop(false);
  }
}
#endif
