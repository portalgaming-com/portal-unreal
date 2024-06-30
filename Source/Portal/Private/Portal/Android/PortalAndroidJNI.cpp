#include "PortalAndroidJNI.h"

#if PLATFORM_ANDROID

#include "Portal/PortalIdentity.h"
#include "Portal/PortalSubsystem.h"
#include "Engine/GameEngine.h"

UPortalIdentity *GetIdentity()
{
	UGameEngine *GameEngine = Cast<UGameEngine>(GEngine);

	if (!GameEngine)
	{
		return nullptr;
	}

	UWorld *World = GameEngine ? GameEngine->GetGameWorld() : NULL;

	if (!World)
	{
		return nullptr;
	}

	auto PortalSubsystem = World->GetGameInstance()->GetSubsystem<UPortalSubsystem>();

	if (!PortalSubsystem)
	{
		return nullptr;
	}

	auto Identity = PortalSubsystem->GetIdentity();

	if (!Identity.IsValid())
	{
		return nullptr;
	}

	return Identity.Get();
}

JNI_METHOD void Java_com_epicgames_unreal_GameActivity_handleDeepLink(JNIEnv *env, jobject obj, jstring jDeeplink)
{
	if (env->IsSameObject(jDeeplink, NULL))
	{
		return;
	}

	const char *deeplinkCStr = env->GetStringUTFChars(jDeeplink, NULL);
	const FString deeplink = FString(UTF8_TO_TCHAR(deeplinkCStr));

	if (auto Identity = GetIdentity())
	{
		Identity->HandleDeepLink(deeplink);
	}
	env->ReleaseStringUTFChars(jDeeplink, deeplinkCStr);
}

JNI_METHOD void Java_com_epicgames_unreal_GameActivity_handleOnCustomTabsDismissed(JNIEnv *env, jobject obj, jstring jUrl)
{
	if (env->IsSameObject(jUrl, NULL))
	{
		return;
	}

	if (auto Identity = GetIdentity())
	{
		Identity->HandleCustomTabsDismissed(FString(UTF8_TO_TCHAR(env->GetStringUTFChars(jUrl, NULL))));
	}
}
#endif