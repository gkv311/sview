/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StCore/StSearchMonitors.h>

#include <StStrings/StLogger.h>
#include <StCocoa/StCocoaLocalPool.h>

#import <Cocoa/Cocoa.h>
#import <IOKit/graphics/IOGraphicsLib.h>

#if !defined(MAC_OS_X_VERSION_10_7) || (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_7)
@interface NSScreen (LionAPI)
    - (CGFloat )backingScaleFactor;
@end
#endif

namespace {

    /**
     * Retrieve number from NSNumber.
     */
    inline uint32_t getNumber(NSNumber* theNum) {
        if(theNum == NULL
        || ![theNum isKindOfClass: [NSNumber class]]) {
            return 0;
        }
        return [theNum intValue];
    }

    /**
     * Find low-level info for specified display (replacement for deprecated CGDisplayIOServicePort(theDispId)).
     */
    static NSDictionary* findDispInfo(CGDirectDisplayID theDispId) {
        io_iterator_t anIter;
        CFMutableDictionaryRef aMatching = IOServiceMatching("IODisplayConnect");
        if(IOServiceGetMatchingServices(kIOMasterPortDefault, aMatching, &anIter) != KERN_SUCCESS) {
            return NULL;
        }

        const uint32_t aVendId = CGDisplayVendorNumber(theDispId);
        const uint32_t aProdId = CGDisplayModelNumber (theDispId);
        const uint32_t aSerial = CGDisplaySerialNumber(theDispId);
        for(io_service_t aServ = IOIteratorNext(anIter); aServ != 0; aServ = IOIteratorNext(anIter)) {
            NSDictionary* aDevInfo = (NSDictionary* )IODisplayCreateInfoDictionary(aServ, kIODisplayOnlyPreferredName);

            NSNumber* aVendIdRef = [aDevInfo objectForKey: [NSString stringWithUTF8String: kDisplayVendorID]];
            NSNumber* aProdIdRef = [aDevInfo objectForKey: [NSString stringWithUTF8String: kDisplayProductID]];
            NSNumber* aSerialRef = [aDevInfo objectForKey: [NSString stringWithUTF8String: kDisplaySerialNumber]];
            if(getNumber(aVendIdRef) == aVendId
            && getNumber(aProdIdRef) == aProdId
            && getNumber(aSerialRef) == aSerial) {
                IOObjectRelease(anIter);
                return aDevInfo;
            }

            [aDevInfo release];
        }

        ST_ERROR_LOG("StSearchMonitors, no match between NS and CF display");
        IOObjectRelease(anIter);
        return NULL;
    }
}

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

        GLfloat aScale = 1.0f;
        if([aScreen respondsToSelector: @selector(backingScaleFactor)]) {
            aScale = [aScreen backingScaleFactor];
        }
    #if MAC_OS_X_VERSION_MIN_REQUIRED < 1070
        else {
            aScale = [aScreen userSpaceScaleFactor];
        }
    #endif
        aStMon.setScale(aScale);

        aStMon.setVRect(StRectI_t(aRectCg.origin.y, aRectCg.origin.y + aRectCg.size.height,
                                  aRectCg.origin.x, aRectCg.origin.x + aRectCg.size.width));

        //boolean_t isActive   = CGDisplayIsActive(aDispId);
        //boolean_t isStereoOn = CGDisplayIsStereo(aDispId);
        //double CGDisplayModeGetRefreshRate(CGDisplayModeRef mode);

        // retrieve low-level information about device
        NSDictionary* aDevInfo = (NSDictionary* )findDispInfo(aDispId);
        //NSDictionary* aDevInfo = (NSDictionary* )IODisplayCreateInfoDictionary(CGDisplayIOServicePort(aDispId), kIODisplayOnlyPreferredName);

        // retrieve display name
        NSDictionary* aLocalizedNames = NULL;
        if(aDevInfo != NULL) {
            aLocalizedNames = [aDevInfo objectForKey: [NSString stringWithUTF8String: kDisplayProductName]];
        }
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
