/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * StCore library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StCore library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <StCore/StSearchMonitors.h>

#include <StStrings/StLogger.h>
#include <StCocoa/StCocoaLocalPool.h>

#import <Cocoa/Cocoa.h>
#import <IOKit/graphics/IOGraphicsLib.h>

void StSearchMonitors::findMonitorsCocoa() {
    if(NSApp == NULL) {
        return;
    }
    StCocoaLocalPool aLocalPool;
    NSArray* aScreens = [NSScreen screens];
    if(aScreens == NULL) {
        return;
    }

    for(NSUInteger aScrId = 0; aScrId < [aScreens count]; ++aScrId) {
        StMonitor aStMon;
        aStMon.setId(aScrId);

        NSScreen* aScreen = (NSScreen* )[aScreens objectAtIndex: aScrId];
        NSDictionary* aDict = [aScreen deviceDescription];
        NSNumber* aNumber = [aDict objectForKey: @"NSScreenNumber"];
        if(aNumber == NULL
        || [aNumber isKindOfClass: [NSNumber class]] == NO) {
            ST_DEBUG_LOG("StSearchMonitors, invalid Cocoa screen #" + aScrId);
            continue;
        }

        CGDirectDisplayID aDispId = [aNumber unsignedIntValue];
        CGRect aRectCg = CGDisplayBounds(aDispId);
        aStMon.setVRect(StRectI_t(aRectCg.origin.y, aRectCg.origin.y + aRectCg.size.height,
                                  aRectCg.origin.x, aRectCg.origin.x + aRectCg.size.width));

        //boolean_t isActive   = CGDisplayIsActive(aDispId);
        //boolean_t isStereoOn = CGDisplayIsStereo(aDispId);
        //double CGDisplayModeGetRefreshRate(CGDisplayModeRef mode);

        // retrieve low-level information about device
        NSDictionary* aDevInfo = (NSDictionary* )IODisplayCreateInfoDictionary(CGDisplayIOServicePort(aDispId), kIODisplayOnlyPreferredName);

        // retrieve display name
        NSDictionary* aLocalizedNames = [aDevInfo objectForKey: [NSString stringWithUTF8String: kDisplayProductName]];
        if(aLocalizedNames != NULL
        && [aLocalizedNames isKindOfClass: [NSDictionary class]]
        && [aLocalizedNames count] > 0) {
            NSString* aScreenName = [aLocalizedNames objectForKey: [[aLocalizedNames allKeys] objectAtIndex: 0]];
            if(aScreenName != NULL
            && [aScreenName isKindOfClass: [NSString class]]) {
                aStMon.setName(StString([aScreenName UTF8String]));
            }
        }

        // retrieve EDID
        NSData* anEDID = [aDevInfo objectForKey: [NSString stringWithUTF8String: kIODisplayEDIDKey]];
        if(anEDID != NULL
        && [anEDID isKindOfClass: [NSData class]]
        && [anEDID length] != 0) {
            aStMon.changeEdid().init((unsigned char* )[anEDID bytes], [anEDID length]);
            if(aStMon.getEdid().isValid()) {
                aStMon.setPnPId(aStMon.getEdid().getPnPId());
            }
        }

        // release the device info
        [aDevInfo release];

        //aStMon.setFreq();
        //aStMon.setFreqMax();
        //aStMon.setGpuName();
        add(aStMon);
    }
}
