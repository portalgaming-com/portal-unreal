#pragma once

#include "CoreMinimal.h"
#include "Portal/Misc/PortalLogging.h"
#include "JsonObjectConverter.h"
// clang-format off
#include "PortalJSMessages.generated.h"
// clang-format on

UENUM()
enum class EPortalIdentityError : int8
{
	AuthenticationError,
	WalletConnectionError,
	UserRegistrationError,
	RefreshTokenError,
	OperationNotSupportedError
	// unknown, timeout?
};

USTRUCT()
struct PORTAL_API FPortalResponseError
{
	GENERATED_BODY()

	UPROPERTY()
	EPortalIdentityError errorType = static_cast<EPortalIdentityError>(-1);

	UPROPERTY()
	FString errorMessage;

	FString ToString()
	{
		const FString ErrType = StaticEnum<EPortalIdentityError>()->GetNameStringByValue(static_cast<int8>(errorType));
		return ErrType == "" ? errorMessage : ErrType + ": " + errorMessage;
	}
};

USTRUCT()
struct PORTAL_API FPortalJSRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString fxName;

	UPROPERTY()
	FString data;

	UPROPERTY()
	FString requestId;

	FString ToJsonString() const
	{
		FString OutString;
		FJsonObjectConverter::UStructToJsonObjectString<FPortalJSRequest>(*this, OutString, 0, 0, 0, nullptr, false);
		return OutString;
	}
};

USTRUCT()
struct PORTAL_API FPortalJSResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString responseFor;

	UPROPERTY()
	FString requestId;

	UPROPERTY()
	bool success = false;

	TOptional<FPortalResponseError> Error;

	// Store the parsed response object for later in case we need to access more
	// data from it (tokens, addresses, user email, etc)
	TSharedPtr<FJsonObject> JsonObject;

	static TOptional<FPortalJSResponse> FromJsonString(const FString &JsonString)
	{
		TOptional<FPortalJSResponse> Response;
		FPortalJSResponse JSResponse;

		// Parse the JSON
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
		if (!FJsonSerializer::Deserialize(JsonReader, JSResponse.JsonObject) || !JSResponse.JsonObject.IsValid())
		{
			PORTAL_ERR("Could not parse response from JavaScript -- invalid JSON: %s", *JsonString)
		}
		else if (!FJsonObjectConverter::JsonObjectToUStruct(JSResponse.JsonObject.ToSharedRef(), &JSResponse, 0, 0))
		{
			PORTAL_ERR("Could not parse response from JavaScript into the expected "
					   "response object format: %s",
					   *JsonString)
		}
		else
		{
			Response = JSResponse;
		}

		if (!JSResponse.success)
		{
			double ErrType = -1;
			FString Error;
			if (JSResponse.JsonObject.IsValid())
			{
				JSResponse.JsonObject->TryGetNumberField("errorType", ErrType);
				JSResponse.JsonObject->TryGetStringField("error", Error);
			}
			JSResponse.Error = FPortalResponseError{static_cast<EPortalIdentityError>(ErrType), Error};
		}

		return Response;
	}
};
