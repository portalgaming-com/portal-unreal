#include "PortalIOS.h"
#include "Portal/PortalIdentity.h"
#include "Portal/PortalSubsystem.h"
#include "Engine/GameEngine.h"

API_AVAILABLE(ios(12.0))
ASWebAuthenticationSession *_authSession;

@implementation PortalIOS

- (instancetype)init
{
  self = [super init];
  return self;
}

+ (PortalIOS *)instance
{
  static PortalIOS *staticPortalIOS;
  static dispatch_once_t once;
  dispatch_once(&once, ^{
    staticPortalIOS = [[self alloc] init];
  });
  return staticPortalIOS;
}

+ (UPortalIdentity *)getIdentity
{
  UGameEngine *GameEngine = Cast<UGameEngine>(GEngine);

  if (!GameEngine)
  {
    return nil;
  }

  UWorld *World = GameEngine ? GameEngine->GetGameWorld() : NULL;

  if (!World)
  {
    return nil;
  }

  auto PortalSubsystem = World->GetGameInstance()->GetSubsystem<UPortalSubsystem>();

  if (!PortalSubsystem)
  {
    return nil;
  }

  auto Identity = PortalSubsystem->GetIdentity();

  if (!Identity.IsValid())
  {
    return nil;
  }

  return Identity.Get();
}

- (void)launchUrl:(const char *)url
{
  NSURL *URL = [NSURL URLWithString:[[NSString alloc] initWithUTF8String:url]];
  NSString *bundleIdentifier = [[NSBundle mainBundle] bundleIdentifier];

  ASWebAuthenticationSession *authSession = [[ASWebAuthenticationSession alloc]
            initWithURL:URL
      callbackURLScheme:bundleIdentifier
      completionHandler:^(NSURL *_Nullable callbackURL,
                          NSError *_Nullable error) {
        _authSession = nil;
        if (callbackURL)
        {
          UPortalIdentity *identity = [PortalIOS getIdentity];

          if (identity)
          {
            identity->HandleDeepLink(callbackURL.absoluteString);
          }
        }
        else
        {
          return;
        }
      }];

  _authSession = authSession;
  _authSession.presentationContextProvider = (id)self;

  [_authSession start];
}

- (nonnull ASPresentationAnchor)presentationAnchorForWebAuthenticationSession:
    (nonnull ASWebAuthenticationSession *)session API_AVAILABLE(ios(13.0))
{
  return [[[UIApplication sharedApplication] windows] firstObject];
}

@end
