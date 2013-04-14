/**
 * Copyright © 2007-2013 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StSettings_h_
#define __StSettings_h_

#include <StSettings/StConfig.h>
#include <StThreads/StMutex.h>
#include <StLibrary.h>

class StSettings : public StConfig {

        public:

    StSettings(const StString& theSettingsSet)
    : StConfig(theSettingsSet) {
        //
    }

};

#endif //__StSettings_h_
