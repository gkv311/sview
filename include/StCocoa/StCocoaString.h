/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if (defined(__APPLE__))

#ifndef __StCocoaString_h_
#define __StCocoaString_h_

#include <StStrings/StString.h>

#import <Cocoa/Cocoa.h>

/**
 * Simple class to simplify conversions between NSString and StString.
 */
class ST_LOCAL StCocoaString {

        public:

    /**
     * Create instance from StString.
     */
    StCocoaString(const StString& theStringSt)
    : myStringNs([[NSString alloc] initWithUTF8String: theStringSt.toCString()]) {
        //
    }

    /**
     * Create instance from C-String, assuming UTF-8.
     */
    StCocoaString(const char* theCString)
    : myStringNs([[NSString alloc] initWithUTF8String: theCString]) {
        //
    }

    /**
     * Create instance from NSString.
     * Does NOT perform copying!
     */
    StCocoaString(NSString* theStringNs)
    : myStringNs([theStringNs retain]) {
        //
    }

    /**
     * Release own NSString instance.
     */
    ~StCocoaString() {
        [myStringNs release];
    }

    /**
     * @return own NSString instance.
     */
    NSString* toStringNs() const {
        return myStringNs;
    }

    /**
     * This method require existing NSAutoreleasePool!
     * @return NSString copy as StString.
     */
    StString toStringSt() const {
        return StString([myStringNs UTF8String]);
    }

    /**
     * This method require existing NSAutoreleasePool!
     * @return C-String in UTF-8.
     */
    const char* toCString() const {
        return [myStringNs UTF8String];
    }

        private:

    NSString* myStringNs; //!< own NSString instance

};

#endif // __StCocoaString_h_
#endif // __APPLE__
