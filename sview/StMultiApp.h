/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2013
 */

#ifndef __StMultiApp_h_
#define __StMultiApp_h_

#include <StCore/StApplication.h>

namespace StMultiApp {

    ST_LOCAL StHandle<StApplication> getInstance(const StHandle<StOpenInfo>& theInfo = NULL);

};

#endif // __StMultiApp_h_
