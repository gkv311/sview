/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutIZ3D library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutIZ3D library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StOutIZ3D_h_
#define __StOutIZ3D_h_

#include <StCore/StWindow.h>
#include <StThreads/StFPSControl.h>
#include <StGL/StGLTexture.h>

#include "StOutIZ3DShaders.h"

class StSettings;

/**
 * This class implements stereoscopic rendering on iZ3D monitors.
 */
class StOutIZ3D : public StWindow {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StOutIZ3D(const StHandle<StResourceManager>& theResMgr,
                           const StNativeWin_t                theParentWindow);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StOutIZ3D();

    /**
     * Renderer about string.
     */
    ST_CPPEXPORT virtual StString getRendererAbout() const;

    /**
     * Renderer id.
     */
    ST_CPPEXPORT virtual const char* getRendererId() const;

    /**
     * Active Device id.
     */
    ST_CPPEXPORT virtual const char* getDeviceId() const;

    /**
     * Devices list.
     * This class supports only 1 device type - iZ3D monitor.
     * Different glasses filters provided as device options.
     */
    ST_CPPEXPORT virtual void getDevices(StOutDevicesList& theList) const;

    /**
     * Retrieve options list.
     */
    ST_CPPEXPORT virtual void getOptions(StParamsList& theList) const;

    /**
     * Create and show window.
     * @return false if any critical error appeared
     */
    ST_CPPEXPORT virtual bool create();

    /**
     * Close the window.
     */
    ST_CPPEXPORT virtual void close();

    /**
     * Extra routines to be processed before window close.
     */
    ST_CPPEXPORT virtual void beforeClose();

    /**
     * Process callback.
     */
    ST_CPPEXPORT virtual void processEvents();

    /**
     * Stereo renderer.
     */
    ST_CPPEXPORT virtual void stglDraw();

        private:

    /**
     * Release GL resources before window closing.
     */
    ST_LOCAL void releaseResources();

    /**
     * On/off VSync callback.
     */
    ST_LOCAL void doSwitchVSync(const int32_t theValue);

        private:

    static StAtomic<int32_t> myInstancesNb; //!< shared counter for all instances

        private:

    struct {

        StHandle<StInt32Param> Glasses; //!< glasses filter

    } params;

        private:

    StOutDevicesList                myDevices;
    StHandle<StSettings>            mySettings;
    StString                        myAbout;           //!< about string

    StHandle<StGLContext>           myContext;
    StHandle<StGLStereoFrameBuffer> myFrBuffer;        //!< frame buffer to draw
    StOutIZ3DShaders                myShaders;         //!< IZ3D shaders
    StGLTexture                     myTexTableOld;     //!< table textures
    StGLTexture                     myTexTableNew;

    StFPSControl                    myFPSControl;
    bool                            myToCompressMem;   //!< reduce memory usage
    bool                            myIsBroken;        //!< special flag for broke state - when FBO can not be allocated

};

#endif //__StOutIZ3D_h_
