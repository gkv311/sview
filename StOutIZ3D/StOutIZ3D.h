/**
 * StOutInterlace, class providing stereoscopic output for iZ3D monitors using StCore toolkit.
 * Copyright © 2009-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
    ST_CPPEXPORT virtual StString getRendererAbout() const ST_ATTR_OVERRIDE;

    /**
     * Renderer id.
     */
    ST_CPPEXPORT virtual const char* getRendererId() const ST_ATTR_OVERRIDE;

    /**
     * Active Device id.
     */
    ST_CPPEXPORT virtual const char* getDeviceId() const ST_ATTR_OVERRIDE;

    /**
     * Devices list.
     * This class supports only 1 device type - iZ3D monitor.
     * Different glasses filters provided as device options.
     */
    ST_CPPEXPORT virtual void getDevices(StOutDevicesList& theList) const ST_ATTR_OVERRIDE;

    /**
     * Retrieve options list.
     */
    ST_CPPEXPORT virtual void getOptions(StParamsList& theList) const ST_ATTR_OVERRIDE;

    /**
     * Create and show window.
     * @return false if any critical error appeared
     */
    ST_CPPEXPORT virtual bool create() ST_ATTR_OVERRIDE;

    /**
     * Close the window.
     */
    ST_CPPEXPORT virtual void close() ST_ATTR_OVERRIDE;

    /**
     * Extra routines to be processed before window close.
     */
    ST_CPPEXPORT virtual void beforeClose() ST_ATTR_OVERRIDE;

    /**
     * Process callback.
     */
    ST_CPPEXPORT virtual void processEvents() ST_ATTR_OVERRIDE;

    /**
     * Stereo renderer.
     */
    ST_CPPEXPORT virtual void stglDraw() ST_ATTR_OVERRIDE;

    /**
     * Update strings.
     */
    ST_LOCAL virtual void doChangeLanguage() ST_ATTR_OVERRIDE { updateStrings(); }

        private:

    /**
     * Release GL resources before window closing.
     */
    ST_LOCAL void releaseResources();

    /**
     * Update strings.
     */
    ST_LOCAL void updateStrings();

    /**
     * On/off VSync callback.
     */
    ST_LOCAL void doSwitchVSync(const int32_t theValue);

        private:

    static StAtomic<int32_t> myInstancesNb; //!< shared counter for all instances

        private:

    struct {

        StHandle<StEnumParam> Glasses; //!< glasses filter

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
