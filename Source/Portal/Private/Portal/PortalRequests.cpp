#include "Portal/PortalRequests.h"
#include "JsonObjectConverter.h"
#include "JsonObjectWrapper.h"
#include "Portal/Misc/PortalLogging.h"
#include "Policies/CondensedJsonPrintPolicy.h"

FString FPortalExecuteTransactionRequest::ToJsonString() const
{
	FString OutString;
	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();

	if (!FJsonObjectConverter::UStructToJsonObject(FPortalExecuteTransactionRequest::StaticStruct(), this, JsonObject, 0, 0))
	{
		UE_LOG(LogTemp, Error, TEXT("Could not convert FPortalExecuteTransactionRequest to JSON"));
		return "";
	}

	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutString);
	if (!FJsonSerializer::Serialize(JsonObject, Writer))
	{
		UE_LOG(LogTemp, Error, TEXT("Could not serialize JSON object"));
		return "";
	}

	return OutString;
}