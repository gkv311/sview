/**
 * Copyright © 2007-2012 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StSettings/StConfig.h>

#if(!defined(_WIN32) && !defined(__WIN32__))

#include <StStrings/StLogger.h>
#include <StThreads/StProcess.h>

#include <sys/stat.h>    // mkdir under Linux
#include <sys/types.h>   // mkdir under Linux

#include <libconfig.h++>
using namespace libconfig;

class ST_LOCAL StConfigImpl : public StConfigInterface {

        private:

    bool load() {
        if(myIsLoaded) {
            return true;
        }

        try {
            myConfig.readFile(myFullFileName.toCString()); // load the configuration
        } catch(...) {
            ST_DEBUG_LOG("StConfig, failed to parse " + myFullFileName);
            return false;
        }
        return true;
    }

    bool save() {
        try {
            myConfig.writeFile(myFullFileName.toCString());
        } catch(...) {
            ST_DEBUG_LOG("StConfig, failed write to " + myFullFileName);
            return false;
        }
        return true;
    }

        public:

    StConfigImpl(const StString& theSettingsSet)
    : myFullFileName(),
      myConfig(),
      myIsLoaded(false) {
        //
    #if(defined(__linux__) || defined(__linux))
        StString aConfigRootPath  = StProcess::getEnv(StString("HOME")) + "/.config";
        StString aConfigSViewPath = aConfigRootPath + "/sview";
        myFullFileName = aConfigSViewPath + '/' + theSettingsSet + ".cfg";

        // make sure directory exists...
        mkdir(aConfigRootPath.toCString(),  0755);
        mkdir(aConfigSViewPath.toCString(), 0755);
    #endif
    }

    virtual ~StConfigImpl() {
        //
    }

    bool loadInt32(const StString& theParamPath,
                   int32_t&        theValue) {
        if(!load()) {
            return false;
        }
        try {
            if(!myConfig.lookupValue(theParamPath.toCString(), theValue)) {
                ST_DEBUG_LOG("StConfig, parameter \"" + theParamPath + "\" not founded");
                return false;
            }
        } catch(...) {
            ST_DEBUG_LOG(myFullFileName + " failed");
            return false;
        }
        return true;
    }

    /**
     * Split parameter path into group and parameter name (theParamPath = 'group.paramName')
     */
    bool groupName(const StString& theParamPath,
                   StString&       theGroup,
                   StString&       theParam) {
        for(StUtf8Iter anIter = theParamPath.iterator(); *anIter != 0; ++anIter) {
            if(*anIter == stUtf32_t('.')) {
                theGroup = theParamPath.subString(0, anIter.getIndex());
                theParam = theParamPath.subString(anIter.getIndex() + 1, theParamPath.getLength());
                return true;
            }
        }
        return false;
    }

    bool saveInt32(const StString& theParamPath,
                   const int32_t&  theValue) {
        bool isLoaded = load(); // try to load config firstly
        try {
            if(!isLoaded || !myConfig.exists(theParamPath.toCString())) {
                Setting& aRoot = myConfig.getRoot();
                StString aParamGroup, aParamName;
                if(groupName(theParamPath, aParamGroup, aParamName)) {
                    if(!myConfig.exists(aParamGroup.toCString())) {
                        Setting& aRootGrp = aRoot.add(aParamGroup.toCString(), Setting::TypeGroup);
                        aRootGrp.add(aParamName.toCString(), Setting::TypeInt);
                    } else {
                        myConfig.lookup(aParamGroup.toCString()).add(aParamName.toCString(), Setting::TypeInt);
                    }
                } else {
                    aRoot.add(theParamPath.toCString(), Setting::TypeInt);
                }
                /**ST_DEBUG_LOG(
                    "aRoot.add, " + theParamPath.toCString()
                    + "= " + theValue
                );*/
            }
            myConfig.lookup(theParamPath.toCString()) = theValue;
            save();
        } catch(ParseException& ex) {
            ST_DEBUG_LOG("StConfig, error on line " + ex.getLine() + ": " + ex.getError());
            return false;
        } catch(SettingNotFoundException nfex) {
            ST_DEBUG_LOG("StConfig, setting not found: " + nfex.getPath());
            return false;
        } catch(ConfigException& cex) {
            ST_DEBUG_LOG("StConfig, config exception!");
            return false;
        }
        return true;
    }

    bool loadString(const StString& theParamPath,
                    StString&       theValue) {
        if(!load()) {
            return false;
        }
        try {
            std::string aBuff;
            if(!myConfig.lookupValue(theParamPath.toCString(), aBuff)) {
                ST_DEBUG_LOG("StConfig, parameter \"" + theParamPath + "\" not founded");
                return false;
            }
            theValue = StString(aBuff.c_str());
        } catch(...) {
            ST_DEBUG_LOG("StConfig, " + myFullFileName + " failed");
            return false;
        }
        return true;
    }

    bool saveString(const StString& theParamPath,
                    const StString& theValue) {
        bool isLoaded = load(); // try to load config firstly
        try {
            if(!isLoaded || !myConfig.exists(theParamPath.toCString())) {
                Setting& aRoot = myConfig.getRoot();
                StString aParamGroup, aParamName;
                if(groupName(theParamPath, aParamGroup, aParamName)) {
                    if(!myConfig.exists(aParamGroup.toCString())) {
                        Setting& rootGrp = aRoot.add(aParamGroup.toCString(), Setting::TypeGroup);
                        rootGrp.add(aParamName.toCString(), Setting::TypeInt);
                    } else {
                        myConfig.lookup(aParamGroup.toCString()).add(aParamName.toCString(), Setting::TypeString);
                    }
                } else {
                    aRoot.add(theParamPath.toCString(), Setting::TypeString);
                }
                /**ST_DEBUG_LOG(
                    "aRoot.add, " + theParamPath
                    + "= " + theValue
                );*/
            }
            myConfig.lookup(theParamPath.toCString()) = theValue.toCString();
            save();
        } catch(ParseException& ex) {
            ST_DEBUG_LOG("StConfig, error on line " + ex.getLine() + ": " + ex.getError());
            return false;
        } catch(SettingNotFoundException nfex) {
            ST_DEBUG_LOG("StConfig, setting not found: " + nfex.getPath());
            return false;
        } catch(ConfigException& cex) {
            ST_DEBUG_LOG("StConfig, config exception!");
            return false;
        }
        return true;
    }

        private:

    StString myFullFileName;
    Config   myConfig;
    bool     myIsLoaded;

};

// Exported class-methods wpappers.
ST_EXPORT StConfigInterface* StConfig_new(const stUtf8_t* theSettingsSet) {
    return new StConfigImpl(theSettingsSet);
}

ST_EXPORT void StConfig_del(StConfigInterface* theInst) {
    delete (StConfigImpl* )theInst;
}

ST_EXPORT stBool_t StConfig_loadInt32(StConfigInterface* theInst,
                                      const stUtf8_t*    theParam,
                                      int32_t&           theValue) {
    return ((StConfigImpl* )theInst)->loadInt32(theParam, theValue);
}

ST_EXPORT stBool_t StConfig_saveInt32(StConfigInterface* theInst,
                                      const stUtf8_t*    theParam,
                                      const int32_t&     theValue) {
    return ((StConfigImpl* )theInst)->saveInt32(theParam, theValue);
}

ST_EXPORT stBool_t StConfig_loadString(StConfigInterface* theInst,
                                       const stUtf8_t*    theParam,
                                       stUtf8_t*          theValue) {
    StString aBuff;
    if(!((StConfigImpl* )theInst)->loadString(theParam, aBuff)) {
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
    return ((StConfigImpl* )theInst)->saveString(theParam, StString(theValue));
}

#endif // !defined(_WIN32)
