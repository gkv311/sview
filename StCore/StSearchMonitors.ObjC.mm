/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StCore/StSearchMonitors.h>

#include <StStrings/StLogger.h>
#include <StCocoa/StCocoaCoords.h>
#include <StCocoa/StCocoaLocalPool.h>

#import <Cocoa/Cocoa.h>
#import <IOKit/graphics/IOGraphicsLib.h>

namespace {

    /**
     * Retrieve number from NSNumber.
     */
    static uint32_t getNumber(NSNumber* theNum) {
        if (theNum == nullptr
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
        if (IOServiceGetMatchingServices(kIOMasterPortDefault, aMatching, &anIter) != KERN_SUCCESS) {
            return nullptr;
        }

        const uint32_t aVendId = CGDisplayVendorNumber(theDispId);
        const uint32_t aProdId = CGDisplayModelNumber (theDispId);
        const uint32_t aSerial = CGDisplaySerialNumber(theDispId);
        for (io_service_t aServ = IOIteratorNext(anIter); aServ != 0; aServ = IOIteratorNext(anIter)) {
            NSDictionary* aDevInfo = (NSDictionary* )IODisplayCreateInfoDictionary(aServ, kIODisplayOnlyPreferredName);

            NSNumber* aVendIdRef = [aDevInfo objectForKey: [NSString stringWithUTF8String: kDisplayVendorID]];
            NSNumber* aProdIdRef = [aDevInfo objectForKey: [NSString stringWithUTF8String: kDisplayProductID]];
            NSNumber* aSerialRef = [aDevInfo objectForKey: [NSString stringWithUTF8String: kDisplaySerialNumber]];
            if (getNumber(aVendIdRef) == aVendId
             && getNumber(aProdIdRef) == aProdId
             && getNumber(aSerialRef) == aSerial) {
                IOObjectRelease(anIter);
                return aDevInfo;
            }

            [aDevInfo release];
        }

        // unfortunately, on Apple ARM processors this API doesn't work anymore,
        // hense report a warning, and not an error message
        ST_DEBUG_LOG("StSearchMonitors, no match between NS and CF display");
        IOObjectRelease(anIter);
        return nullptr;
    }
}

void StSearchMonitors::findMonitorsCocoa() {
    if (NSApp == nullptr) {
        return;
    }

    StCocoaLocalPool aLocalPool;
    NSArray* aScreens = [NSScreen screens];
    if (aScreens == nullptr || [aScreens count] == 0) {
        return;
    }

    StCocoaCoords aConverter;
    for (NSUInteger aScrId = 0; aScrId < [aScreens count]; ++aScrId) {
        StMonitor aStMon;
        aStMon.setId(aScrId);

        NSScreen* aScreen = (NSScreen* )[aScreens objectAtIndex: aScrId];
        NSDictionary* aDict = [aScreen deviceDescription];
        NSNumber* aNumber = [aDict objectForKey: @"NSScreenNumber"];
        if (aNumber == nullptr
        || [aNumber isKindOfClass: [NSNumber class]] == NO) {
            ST_DEBUG_LOG("StSearchMonitors, invalid Cocoa screen #" + aScrId);
            continue;
        }

        if ([aScreen respondsToSelector: @selector(backingScaleFactor)]) {
            aStMon.setScale([aScreen backingScaleFactor]);
        }

        const NSRect    aRectCg = [aScreen frame];
        const StRectI_t aRectI  = aConverter.cocoaToNormal(aRectCg);
        aStMon.setVRect(aRectI);

        if (@available(macOS 10.15, *)) {
            aStMon.setName(StString([[aScreen localizedName] UTF8String]));
        }
        aStMon.setWideGamut([aScreen canRepresentDisplayGamut: NSDisplayGamutP3]);

        const CGDirectDisplayID aDispId = [aNumber unsignedIntValue];

        // retrieve low-level information about device
        NSDictionary* aDevInfo = (NSDictionary* )findDispInfo(aDispId);
        //NSDictionary* aDevInfo = (NSDictionary* )IODisplayCreateInfoDictionary(CGDisplayIOServicePort(aDispId), kIODisplayOnlyPreferredName);

        // retrieve display name
        NSDictionary* aLocalizedNames = nullptr;
        if (aDevInfo != nullptr) {
            aLocalizedNames = [aDevInfo objectForKey: [NSString stringWithUTF8String: kDisplayProductName]];
        }
        if (aLocalizedNames != nullptr
        && [aLocalizedNames isKindOfClass: [NSDictionary class]]
        && [aLocalizedNames count] > 0) {
            NSString* aScreenName = [aLocalizedNames objectForKey: [[aLocalizedNames allKeys] objectAtIndex: 0]];
            if (aScreenName != nullptr
            && [aScreenName isKindOfClass: [NSString class]]) {
                aStMon.setName(StString([aScreenName UTF8String]));
            }
        }

        // retrieve EDID
        NSData* anEDID = [aDevInfo objectForKey: [NSString stringWithUTF8String: kIODisplayEDIDKey]];
        if (anEDID != nullptr
        && [anEDID isKindOfClass: [NSData class]]
        && [anEDID length] != 0) {
            aStMon.changeEdid().init((unsigned char* )[anEDID bytes], [anEDID length]);
            if (aStMon.getEdid().isValid()) {
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
