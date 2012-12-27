/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StRendererInterface_h_
#define __StRendererInterface_h_

#include <stTypes.h>
#include "StMessageList.h"
#include "StOpenInfo.h"
#include "StNativeWin_t.h"

class StWindowInterface;

/**
 * Output (rendering) Plugins should implement this interface.
 * Each plugin should provide support for some stereoscopic device(s)
 * in terms of OpenGL rendering cycle.
 */
class StRendererInterface {

        public:

    virtual StRendererInterface* getLibImpl() = 0;

    virtual ~StRendererInterface() {} // never forget it!

    /**
     * @return stWindow (StWindow* ) - stereo window.
     */
    virtual StWindowInterface* getStWindow() = 0;

    /**
     * Function initialize current stereo-output mode.
     * @param nativeParent - handle with native window information (to create embedded StWindow);
     * @return true if success.
     */
    virtual bool init(const StString&     rendererPath,
                      const int&          deviceId,
                      const StNativeWin_t nativeParent = (StNativeWin_t )NULL) = 0;

    /**
     * Special commands:
     *  - if MIME type is StDrawerInfo::DRAWER_MIME(), then specified StDrawer plugin will be loaded;
     *  - if MIME type is StDrawerInfo::CLOSE_MIME(), then current StDrawer plugin will be unloaded.
     * If MIME type is general (open file) and:
     *  - StDrawer plugin already loaded, then StOpenInfo will be sent to current StDrawer instance;
     *  - no StDrawer loaded, then best plugin will be detected by file MIME type and will be loaded.
     * @return true on success.
     */
    virtual bool open(const StOpenInfo& stOpenInfo = StOpenInfo()) = 0;

    /**
     * Callback function.
     * @param stMessages (StMessage_t* ) - buffer to get new messages;
     */
    virtual void callback(StMessage_t* ) = 0;

    /**
     * Function draw stereo.
     * @param views (enum ) - views to draw.
     */
    virtual void stglDraw(unsigned int ) = 0;

};

#endif //__StRendererInterface_h_
