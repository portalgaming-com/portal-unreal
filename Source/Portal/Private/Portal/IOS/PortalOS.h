#pragma once

#import <AuthenticationServices/ASWebAuthenticationSession.h>
#import <Foundation/Foundation.h>

@interface PortalIOS
    : NSObject <ASWebAuthenticationPresentationContextProviding>
+ (PortalIOS *)instance;
- (void)launchUrl:(const char *)url;
@end