/**
 * Copyright © 2011-2013 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#if (defined(__APPLE__))

#include <StSettings/StSettings.h>

#include <StStrings/StLogger.h>
#include <StThreads/StProcess.h>

#include <StCocoa/StCocoaLocalPool.h>
#include <StCocoa/StCocoaString.h>

#include <sys/stat.h>    // mkdir under Linux
#include <sys/types.h>   // mkdir under Linux

#import <Cocoa/Cocoa.h>

bool StSettings::load() {
    StCocoaLocalPool aLocalPool;
    StCocoaString aPath(myFilePath);
    myDict = [NSMutableDictionary dictionaryWithContentsOfFile: aPath.toStringNs()];
    if(myDict == nil) {
        ST_DEBUG_LOG("StConfig, failed to parse " + myFilePath);
        myDict = [[NSMutableDictionary alloc] init];
        return false;
    } else {
        [myDict retain];
    }
    return true;
}

bool StSettings::save() {
    StCocoaLocalPool aLocalPool;
    StCocoaString aPath(myFilePath);
    if([myDict writeToFile: aPath.toStringNs() atomically: NO] == NO) {
        ST_DEBUG_LOG("StConfig, failed write to " + myFilePath);
        return false;
    }
    return true;
}

StSettings::StSettings(const StString& theSettingsSet)
: myFilePath(),
  myDict(NULL){
    StString aRoot = StProcess::getEnv(StString("HOME")) + "/Library/Preferences";
    StString aSViewConfigs = aRoot + "/sview";
    myFilePath = aSViewConfigs + '/' + theSettingsSet + ".plist";

    // make sure directory exists...
    mkdir(aRoot.toCString(), 0700);
    mkdir(aSViewConfigs.toCString(), 0700);

    load();
}

StSettings::~StSettings() {
    save();
    [myDict release];
}

bool StSettings::loadInt32(const StString& theParamPath,
                           int32_t&        theValue) {
    StCocoaString aPath(theParamPath);
    NSObject* anObj = [myDict objectForKey: aPath.toStringNs()];
    if(anObj == nil || [anObj isKindOfClass: [NSNumber class]] == NO) {
        ST_DEBUG_LOG("StConfig, parameter \"" + theParamPath + "\" not found");
        return false;
    }
    NSNumber* aValue = (NSNumber* )anObj;
    theValue = [aValue intValue];
    return true;
}

bool StSettings::saveInt32(const StString& theParamPath,
                           const int32_t&  theValue) {
    StCocoaString aPath(theParamPath);
    NSNumber* aNumber = [[NSNumber alloc] initWithInt: theValue];
    [myDict setObject: aNumber forKey: aPath.toStringNs()];
    [aNumber release];
    return save();
}

bool StSettings::loadString(const StString& theParamPath,
                            StString&       theValue) {
    StCocoaLocalPool aLocalPool;
    StCocoaString aPath(theParamPath);
    NSObject* anObj = [myDict objectForKey: aPath.toStringNs()];
    if(anObj == nil || [anObj isKindOfClass: [NSString class]] == NO) {
        ST_DEBUG_LOG("StConfig, parameter \"" + theParamPath + "\" not found");
        return false;
    }
    NSString* aValue = (NSString* )anObj;
    theValue = [aValue UTF8String];
    return true;
}

bool StSettings::saveString(const StString& theParamPath,
                            const StString& theValue) {
    StCocoaString aPath(theParamPath);
    StCocoaString aValue(theValue);
    [myDict setObject: aValue.toStringNs() forKey: aPath.toStringNs()];
    return save();
}

#endif // __APPLE__
