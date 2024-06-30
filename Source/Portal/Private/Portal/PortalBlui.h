// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
// clang-format off
#include "PortalBlui.generated.h"
// clang-format on

/**
 *
 */
UCLASS()
class PORTAL_API UPortalBlui : public UObject
{
	GENERATED_BODY()

public:
	UPortalBlui();

	UFUNCTION()
	void OnScriptEvent(const FString &EventName, const FString &EventMessage);
	TWeakObjectPtr<class UPortalJSConnector> GetJSConnector() const;

	UFUNCTION()
	void OnLogEvent(const FString &LogText);

	virtual void BeginDestroy() override;
	void Init();

#if USING_BLUI_CEF
	void StopBluiEventLoop();
#endif

private:
	UPROPERTY()
	UObject *BluEyePtr = nullptr;

	bool bLoadedIndexJS = false;
	void WorldTickStart(UWorld *World, ELevelTick LevelTick, float X);

	UPROPERTY()
	class UPortalJSConnector *JSConnector = nullptr;
	FDelegateHandle WorldTickHandle;

	void ExecuteJS(const FString &ScriptText) const;

#if USING_BLUI_CEF
	class UBluEye *GetBluEye() const;
#endif
};
