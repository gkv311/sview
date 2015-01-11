/**
 * Copyright © 2013-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutDual library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutDual library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StOutDistorted_h_
#define __StOutDistorted_h_

#include <StCore/StWindow.h>
#include <StGL/StGLVertexBuffer.h>
#include <StThreads/StFPSControl.h>

class StSettings;
class StProgramBarrel;
class StProgramFlat;
class StGLFrameBuffer;
class StGLTexture;

struct ovrHmdDesc_;
typedef const ovrHmdDesc_* ovrHmd;

/**
 * This class implements stereoscopic rendering on displays
 * wich require software distortion correction.
 */
class StOutDistorted : public StWindow {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StOutDistorted(const StHandle<StResourceManager>& theResMgr,
                                const StNativeWin_t                theParentWindow);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StOutDistorted();

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
     * This methods returns device lost state.
     * @return true if rendering device requires reinitialization
     */
    ST_CPPEXPORT virtual bool isLostDevice() const;

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
     * Process callback.
     */
    ST_CPPEXPORT virtual void processEvents();

    /**
     * @param theFullScreen fullscreen state
     */
    ST_CPPEXPORT virtual void setFullScreen(const bool theFullScreen);

    /**
     * Stereo renderer.
     */
    ST_CPPEXPORT virtual void stglDraw();

    /**
     * Show/Hide mouse cursor.
     * @param theToShow true to show cursor
     */
    ST_CPPEXPORT virtual void showCursor(const bool theToShow);

    ST_CPPEXPORT virtual GLfloat getLensDist() const;

    /**
     * Downscale GUI for low-resolution Oculus Rift prototype
     */
    ST_CPPEXPORT virtual GLfloat getScaleFactor() const;

        private:

    /**
     * Release GL resources before window closing.
     */
    ST_LOCAL void releaseResources();

    /**
     * On/off VSync callback.
     */
    ST_LOCAL void doSwitchVSync(const int32_t theValue);

    ST_LOCAL void stglDrawCursor(const StPointD_t&  theCursorPos,
                                 const unsigned int theView);

        private:

    static StAtomic<int32_t> myInstancesNb; //!< shared counter for all instances

        private:

    enum {
        DEVICE_AUTO         =-1,
        DEVICE_DISTORTED    = 0, //!< general output
        DEVICE_OCULUS       = 1, //!< Oculus Rift
        DEVICE_NB,
    };

    enum Layout {
        LAYOUT_SIDE_BY_SIDE = 0, //!< anamorph side by side
        LAYOUT_OVER_UNDER   = 1, //!< anamorph over under
    };

    struct {

        StHandle<StInt32Param> Layout;   //!< pair layout
        StHandle<StBoolParam>  Anamorph; //!< anamorph filter
        StHandle<StBoolParam>  MonoClone;//!< display mono in stereo

    } params;

        private:

    StOutDevicesList          myDevices;
    StHandle<StSettings>      mySettings;
    StString                  myAbout;           //!< about string

    int                       myDevice;          //!< currently active device
    bool                      myToResetDevice;
    StHandle<StGLContext>     myContext;
    StHandle<StGLFrameBuffer> myFrBuffer;        //!< OpenGL frame buffer object
    StHandle<StGLTexture>     myCursor;          //!< cursor texture - we can not use normal cursor due to distortions
    StHandle<StProgramFlat>   myProgramFlat;
    StHandle<StProgramBarrel> myProgramBarrel;
    StFPSControl              myFPSControl;
    StGLVertexBuffer          myFrVertsBuf;      //!< buffers to draw simple fullsreen quad
    StGLVertexBuffer          myFrTCrdsBuf;
    StGLVertexBuffer          myCurVertsBuf;
    StGLVertexBuffer          myCurTCrdsBuf;
    StGLVec4                  myBarrelCoef;      //!< Barrel distortion coefficients
    StGLVec4                  myChromAb;         //!< chrome coefficients

    StMarginsI                myBarMargins;      //!< GUI margins

    ovrHmd                    myOvrHmd;

    bool                      myToReduceGui;     //!< scale down GUI
    bool                      myToShowCursor;    //!< cursor visibility flag
    bool                      myToCompressMem;   //!< reduce memory usage
    bool                      myIsBroken;        //!< special flag for broke state - when FBO can not be allocated
    bool                      myIsStereoOn;

};

#endif // __StOutDistorted_h_
