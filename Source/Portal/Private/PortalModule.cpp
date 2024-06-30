// Copyright Epic Games, Inc. All Rights Reserved.

#include "PortalModule.h"

#include "Portal/Misc/PortalLogging.h"
#include "Interfaces/IPluginManager.h"
#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include <windows.h>
#include <psapi.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif

#define LOCTEXT_NAMESPACE "FPortalModule"

void FPortalModule::StartupModule()
{
  // This code will execute after your module is loaded into memory; the exact
  // timing is specified in the .uplugin file per-module

#if USING_BLUI_CEF
  if (IPluginManager::Get().FindPlugin("WebBrowserWidget") != nullptr &&
      IPluginManager::Get().FindPlugin("WebBrowserWidget")->IsEnabled())
  {
    PORTAL_ERR("Cannot enable both BLUI and WebBrowserWidget plugin at the same "
               "time as it crashes.  In Portal.uplugin file, "
               "'Plugins->WebBrowserWidget->Enabled' to 'false' and ensure the "
               "WebBrowserWidget is disabled in your project to use BLUI.")
  }
#endif
}

void FPortalModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPortalModule, Portal)
