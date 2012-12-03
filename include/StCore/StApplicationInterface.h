/**
 * Copyright © 2009 Kirill Gavrilov <kirill@sview.ru>
 *
 * StCore library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StCore library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StApplicationInterface_h_
#define __StApplicationInterface_h_

#include <stTypes.h>
#include "StMessageList.h"
#include "StNativeWin_t.h"
#include "StOpenInfo.h"

#ifdef __cplusplus

class StApplicationInterface {

        public:

    virtual StApplicationInterface* getLibImpl() = 0;

    virtual ~StApplicationInterface() {} // never forget it!

    /**
     * @return true if application active.
     */
    virtual bool isOpened() = 0;

    /**
     * @return true if application is in fullscreen mode.
     */
    virtual bool isFullscreen() = 0;

    /**
     * Function load Output (rendering) plugin.
     * @param nativeParent (const StNativeWin_t* ) - handle with native window information (to create embedded StWindow);
     * @return false on any critical error.
     */
    virtual bool create(const StNativeWin_t* nativeParent = NULL) = 0;

    /**
     * Force application to exit.
     */
    ///virtual void quit() = 0;

    /**
     * Function automatically load Drawer plugin and open file.
     * @return true on success.
     */
    virtual bool open(const StOpenInfo& stOpenInfo = StOpenInfo()) = 0;

    /**
     * Callback order:
     *   - StWindow      (intial -> buffer empting and filled with window's messages);
     *   - StDrawer      (could parse StWindow messages, add own messages);
     *   - StRenderer    (could parse StWindow and StDrawer messages, add own messages);
     *   - StCore        (do nothing at this moment);
     *   - StApplication (could only parse messages!).
     * @param stMessages (StMessage_t* ) - buffer to get new messages;
     */
    virtual void callback(StMessage_t* stMessages = NULL) = 0;

};

#endif //__cplusplus
#endif //__StApplicationInterface_h_
