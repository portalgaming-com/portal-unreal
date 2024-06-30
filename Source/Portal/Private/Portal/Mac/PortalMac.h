#pragma once

#import <AuthenticationServices/ASWebAuthenticationSession.h>
#import <Foundation/Foundation.h>

@interface PortalMac
    : NSObject <ASWebAuthenticationPresentationContextProviding>
+ (PortalMac *)instance;
- (void)launchUrl:(const char *)url forRedirectUri:(const char *)redirectUri;
@end
