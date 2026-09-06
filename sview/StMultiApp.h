/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2013
 */

#ifndef __StMultiApp_h_
#define __StMultiApp_h_

#include <StCore/StApplication.h>

namespace StMultiApp {

    ST_LOCAL std::shared_ptr<StApplication> getInstance(const std::shared_ptr<StResourceManager>& theResMgr,
                                                        const std::shared_ptr<StOpenInfo>&        theInfo = std::shared_ptr<StOpenInfo>());

}

#endif // __StMultiApp_h_
