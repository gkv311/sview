/**
 * Copyright © 2007-2013 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StConfig_h_
#define __StConfig_h_

#include <StSettings/StConfigInterface.h>
#include <StTemplates/StHandle.h>

class StLibrary;

class StConfig : public StConfigInterface {

        public:

    ST_CPPEXPORT StConfig(const StString& theSettingsSet);

    ST_CPPEXPORT virtual ~StConfig();

    bool isValid() const {
        return myLibInstance != NULL;
    }

    ST_CPPEXPORT bool loadInt32(const StString& theParam,
                                int32_t&        theOutValue);

    ST_CPPEXPORT bool saveInt32(const StString& theParam,
                                const int32_t&  theValue);

    ST_CPPEXPORT bool loadString(const StString& theParam,
                                 StString&       theOutValue);

    ST_CPPEXPORT bool saveString(const StString& theParam,
                                 const StString& theValue);

        private:

    // typedef pointer-to-class
    typedef void* StConfig_t;

    // types definitions - needed for each exported function
    typedef StConfig_t (*StConfig_new_t)(const stUtf8_t* );
    typedef void (*StConfig_del_t)(StConfig_t );
    typedef stBool_t (*StConfig_loadInt32_t) (StConfig_t , const stUtf8_t* , int32_t& );
    typedef stBool_t (*StConfig_saveInt32_t) (StConfig_t , const stUtf8_t* , const int32_t& );
    typedef stBool_t (*StConfig_loadString_t)(StConfig_t , const stUtf8_t* , stUtf8_t* );
    typedef stBool_t (*StConfig_saveString_t)(StConfig_t , const stUtf8_t* , const stUtf8_t* );

        private:

    StHandle<StLibrary>   myLib;
    StConfigInterface*    myLibInstance;
    StConfig_new_t        StConfig_new;
    StConfig_del_t        StConfig_del;
    StConfig_loadInt32_t  StConfig_loadInt32;
    StConfig_saveInt32_t  StConfig_saveInt32;
    StConfig_loadString_t StConfig_loadString;
    StConfig_saveString_t StConfig_saveString;

};

#endif //__StConfig_h_
