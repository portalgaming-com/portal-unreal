// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal/PortalDataTypes.h"

FString FPortalIdentityInitData::ToJsonString() const
{
	FString OutString;
	FJsonObjectWrapper Wrapper;
	Wrapper.JsonObject = MakeShared<FJsonObject>();
	FJsonObjectConverter::UStructToJsonObject(StaticStruct(), this, Wrapper.JsonObject.ToSharedRef(), 0, 0);

	if (!Wrapper.JsonObject.IsValid())
	{
		PORTAL_ERR("Could not convert FPortalIdentityInitData to JSON")
		return "";
	}
	// Remove redirectUri field if it's empty so that the bridge doesn't try to
	// use it
	if (Wrapper.JsonObject->HasField("redirectUri") && Wrapper.JsonObject->GetStringField("redirectUri").IsEmpty())
	{
		Wrapper.JsonObject->RemoveField("redirectUri");
	}
	Wrapper.JsonObjectToString(OutString);

	return OutString;
}

TOptional<FPortalIdentityInitDeviceFlowData> FPortalIdentityInitDeviceFlowData::FromJsonString(const FString &JsonObjectString)
{
	FPortalIdentityInitDeviceFlowData IdentityConnect;

	if (!FJsonObjectConverter::JsonObjectStringToUStruct(JsonObjectString, &IdentityConnect, 0, 0))
	{
		PORTAL_WARN("Could not parse response from JavaScript into the expected "
					"Identity connect format")
		return TOptional<FPortalIdentityInitDeviceFlowData>();
	}

	return IdentityConnect;
}