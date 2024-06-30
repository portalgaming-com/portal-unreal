
// Fill out your copyright notice in the Description page of Project Settings.

#include "PortalSDKResourceAssetFactory.h"

#include "Portal/Assets/PortalSDKResource.h"
#include "Portal/Misc/PortalLogging.h"
#include "Misc/FileHelper.h"

UPortalSDKResourceAssetFactory::UPortalSDKResourceAssetFactory()
{
  Formats.Add(FString(TEXT("js; Portal SDK Resource")));
  SupportedClass = UPortalSDKResource::StaticClass();
  bCreateNew = false;
  bEditorImport = true;
}

bool UPortalSDKResourceAssetFactory::FactoryCanImport(const FString &Filename)
{
  return FPaths::GetCleanFilename(Filename) == TEXT("index.js");
}

UObject *UPortalSDKResourceAssetFactory::FactoryCreateBinary(
    UClass *InClass, UObject *InParent, FName InName, EObjectFlags Flags,
    UObject *Context, const TCHAR *Type, const uint8 *&Buffer,
    const uint8 *BufferEnd, FFeedbackContext *Warn)
{
  UPortalSDKResource *Resource =
      NewObject<UPortalSDKResource>(InParent, InClass, InName, Flags);

  if (CurrentFilename.IsEmpty() || !FPaths::FileExists(CurrentFilename))
  {
    PORTAL_LOG("Invalid input js file: %s", *CurrentFilename)
    return nullptr;
  }

  FFileHelper::LoadFileToString(Resource->Js, *CurrentFilename,
                                FFileHelper::EHashOptions::EnableVerify,
                                FILEREAD_NoFail);

  return Resource;
}
