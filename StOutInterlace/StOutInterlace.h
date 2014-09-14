/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutInterlace library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutInterlace library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StOutInterlace_h_
#define __StOutInterlace_h_

#include <StCore/StWindow.h>
#include <StCore/StMonitor.h>
#include <StThreads/StFPSControl.h>
#include <StGL/StGLProgram.h>
#include <StGL/StGLFrameBuffer.h>
#include <StGL/StGLVertexBuffer.h>

class StSettings;

/**
 * Simple GLSL program.
 */
class StProgramFB : public StGLProgram {

        public:

    ST_LOCAL StProgramFB(const StString& theTitle);
    ST_LOCAL virtual bool link(StGLContext& theCtx);

};

/**
 * This class implements stereoscopic rendering on Interlaced monitors.
 */
class StOutInterlace : public StWindow {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StOutInterlace(const StHandle<StResourceManager>& theResMgr,
                                const StNativeWin_t                theParentWindow);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StOutInterlace();

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
     * Activate Device.
     */
    ST_CPPEXPORT virtual bool setDevice(const StString& theDevice);

    /**
     * Devices list.
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
     * Show up the window.
     */
    ST_CPPEXPORT virtual void show();

    /**
     * Process callback.
     */
    ST_CPPEXPORT virtual void processEvents();

    /**
     * Stereo renderer.
     */
    ST_CPPEXPORT virtual void stglDraw();

        private:

    enum {
        DEVICE_AUTO          =-1,
        DEVICE_HINTERLACE    = 0, //!< interlace (horizontal 1xPixel lines, full color from R or L)
        DEVICE_VINTERLACE    = 1, //!< interlace (vertical 1xPixel lines, full color from R or L)
        DEVICE_CHESSBOARD    = 2, //!< 1xPixel chessboard (some DLP devices)
        DEVICE_HINTERLACE_ED = 3, //!< interlace (horizontal 1xPixel lines) + EDimensional onscreen codes

        DEVICE_NB,
    };

        private:

    ST_LOCAL static StHandle<StMonitor> getHInterlaceMonitor(const StArrayList<StMonitor>& theMonitors,
                                                             bool& theIsReversed);

    ST_LOCAL void stglDrawEDCodes();

    /**
     * Release GL resources before window closing.
     */
    ST_LOCAL void releaseResources();

    /**
     * On/off VSync callback.
     */
    ST_LOCAL void doSwitchVSync(const int32_t theValue);

    /**
     * Bind to monitor callback.
     */
    ST_LOCAL void doSetBindToMonitor(const bool theValue);

    /**
     * Process monitor change event.
     */
    ST_LOCAL void doNewMonitor(const StSizeEvent& theEvent);

        private:

    static StAtomic<int32_t>  myInstancesNb;              //!< shared counter for all instances

        private:

    struct {

        StHandle<StBoolParam> ToReverse; //!< configurable flag to reverse rows order
        StHandle<StBoolParam> BindToMon; //!< flag to bind to monitor

    } params;

        private:

    StOutDevicesList          myDevices;
    StHandle<StSettings>      mySettings;
    StString                  myAbout;                    //!< about string
    StHandle<StGLContext>     myContext;
    StHandle<StGLFrameBuffer> myFrmBuffer;                //!< OpenGL frame buffer object
    StHandle<StProgramFB>     myGlPrograms[DEVICE_NB];    //!< GLSL programs
    StHandle<StProgramFB>     myGlProgramsRev[DEVICE_NB]; //!< GLSL programs with reversed left/right condition
    StGLVertexBuffer          myQuadVertBuf;
    StGLVertexBuffer          myQuadTexCoordBuf;
    int                       myDevice;
    StHandle<StMonitor>       myMonitor;                  //!< current monitor

    StRectI_t                 myWinRect;
    StRectI_t                 myEDRect;
    StTimer                   myEDTimer;                  //!< EDimensional activator/disactivator timer
    StHandle<StGLProgram>     myEDIntelaceOn;             //!< ED interlace activate program
    StHandle<StGLProgram>     myEDOff;                    //!< ED disactivate program
    GLsizei                   myVpSizeY;                  //!< VIewPort Y size
    StGLVarLocation           myVpSizeYOnLoc;             //!< helper shader variables
    StGLVarLocation           myVpSizeYOffLoc;

    StFPSControl              myFPSControl;
    bool                      myToSavePlacement;
    bool                      myIsMonReversed;            //!< indicates (known) monitor model with reversed rows order
    bool                      myIsStereo;
    bool                      myIsEDactive;
    bool                      myIsEDCodeFinished;
    bool                      myToCompressMem;            //!< reduce memory usage
    bool                      myIsBroken;                 //!< special flag for broke state - when FBO can not be allocated

};

#endif //__StOutInterlace_h_
