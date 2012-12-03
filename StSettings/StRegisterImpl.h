/**
 * Copyright © 2007-2012 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StRegisterImpl_h_
#define __StRegisterImpl_h_

#include <StSettings/StConfig.h>

#if(defined(_WIN32) || defined(__WIN32__))

/**
 * Special class for load/save values into Window register.
 */
class StRegisterImpl : public StConfigInterface {

        public:

    StRegisterImpl(const StStringUtfWide& theSettingsSet);

    virtual ~StRegisterImpl();

    bool loadInt32(const StString& theParam,
                   int32_t&        theValue);

    bool saveInt32(const StString& theParam,
                   const int32_t&  theValue);

    bool loadString(const StString& theParamPath,
                    StString&       theValue);

    bool saveString(const StString& theParamPath,
                    const StString& theValue);

        private:

    StStringUtfWide mySettingsSet;
    StStringUtfWide myRegisterPath;

};

#endif // defined(_WIN32)
#endif //__StRegisterImpl_h_
