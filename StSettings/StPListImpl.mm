/**
 * Copyright © 2011-2012 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#if (defined(__APPLE__))

#include <StSettings/StConfig.h>

#include <StStrings/StLogger.h>
#include <StThreads/StProcess.h>

#include <StCocoa/StCocoaLocalPool.h>
#include <StCocoa/StCocoaString.h>

#include <sys/stat.h>    // mkdir under Linux
#include <sys/types.h>   // mkdir under Linux

#import <Cocoa/Cocoa.h>

class ST_LOCAL StPListImpl : public StConfigInterface {

        private:

    bool load() {
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

    bool save() {
        StCocoaLocalPool aLocalPool;
        StCocoaString aPath(myFilePath);
        if([myDict writeToFile: aPath.toStringNs() atomically: NO] == NO) {
            ST_DEBUG_LOG("StConfig, failed write to " + myFilePath);
            return false;
        }
        return true;
    }

        public:

    StPListImpl(const StString& theSettingsSet)
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

    virtual ~StPListImpl() {
        save();
        [myDict release];
    }

    bool loadInt32(const StString& theParamPath,
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

    bool saveInt32(const StString& theParamPath,
                   const int32_t&  theValue) {
        StCocoaString aPath(theParamPath);
        NSNumber* aNumber = [[NSNumber alloc] initWithInt: theValue];
        [myDict setObject: aNumber forKey: aPath.toStringNs()];
        [aNumber release];
        return save();
    }

    bool loadString(const StString& theParamPath,
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

    bool saveString(const StString& theParamPath,
                    const StString& theValue) {
        StCocoaString aPath(theParamPath);
        StCocoaString aValue(theValue);
        [myDict setObject: aValue.toStringNs() forKey: aPath.toStringNs()];
        return save();
    }

        private:

    StString             myFilePath;
    NSMutableDictionary* myDict;

};

// Exported class-methods wpappers.
ST_EXPORT StConfigInterface* StConfig_new(const stUtf8_t* theSettingsSet) {
    return new StPListImpl(theSettingsSet);
}

ST_EXPORT void StConfig_del(StConfigInterface* theInst) {
    delete (StPListImpl* )theInst;
}

ST_EXPORT stBool_t StConfig_loadInt32(StConfigInterface* theInst,
                                      const stUtf8_t*    theParam,
                                      int32_t&           theValue) {
    return ((StPListImpl* )theInst)->loadInt32(theParam, theValue);
}

ST_EXPORT stBool_t StConfig_saveInt32(StConfigInterface* theInst,
                                      const stUtf8_t*    theParam,
                                      const int32_t&     theValue) {
    return ((StPListImpl* )theInst)->saveInt32(theParam, theValue);
}

ST_EXPORT stBool_t StConfig_loadString(StConfigInterface* theInst,
                                       const stUtf8_t*    theParam,
                                       stUtf8_t*          theValue) {
    StString aBuff;
    if(!((StPListImpl* )theInst)->loadString(theParam, aBuff)) {
        return ST_FALSE;
    }
    size_t aSize = stMin(StConfigInterface::MAX_STRING_LENGHT - 1, aBuff.getSize());
    stMemCpy(theValue, aBuff.toCString(), aSize);
    theValue[aSize] = stUtf8_t('\0');
    return ST_TRUE;
}

ST_EXPORT stBool_t StConfig_saveString(StConfigInterface* theInst,
                                       const stUtf8_t*    theParam,
                                       const stUtf8_t*    theValue) {
    return ((StPListImpl* )theInst)->saveString(theParam, StString(theValue));
}

#endif // __APPLE__
