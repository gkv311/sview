/**
 * Copyright © 2009-2010 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StVersion.h>

namespace {
    static bool HAS_LOGGER_ID = StLogger::IdentifyModule("StSettings");
};

// SDK version was used
ST_EXPORT void getSDKVersion(StVersion* theVer) {
    *theVer = StVersionInfo::getSDKVersion();
}
