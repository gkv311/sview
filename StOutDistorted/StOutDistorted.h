/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
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

/**
 * This class implements stereoscopic rendering on displays
 * wich require software distortion correction.
 */
class StOutDistorted : public StWindow {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StOutDistorted(const StNativeWin_t theParentWindow);

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
     * Stereo renderer.
     */
    ST_CPPEXPORT virtual void stglDraw();

    /**
     * Show/Hide mouse cursor.
     * @param theToShow true to show cursor
     */
    ST_CPPEXPORT virtual void showCursor(const bool theToShow);

        private:

    /**
     * Release GL resources before window closing.
     */
    ST_LOCAL void releaseResources();

    /**
     * On/off VSync callback.
     */
    ST_LOCAL void doVSync(const bool theValue);

    ST_LOCAL void stglDrawCursor();

        private:

    static StAtomic<int32_t> myInstancesNb; //!< shared counter for all instances

        private:

    enum Layout {
        LAYOUT_SIDE_BY_SIDE = 0, //!< anamorph side by side
        LAYOUT_OVER_UNDER   = 1, //!< anamorph over under
    };

    enum Distortion {
        DISTORTION_OFF    = 0, //!< no extra distortion
        DISTORTION_BARREL = 1, //!< Barrel distortion for Oculus Rift
    };

    struct {

        StHandle<StBoolParam>  IsVSyncOn;  //!< flag to use VSync
        StHandle<StInt32Param> Layout;     //!< pair layout
        StHandle<StInt32Param> Distortion; //!< distortion shader

    } params;

        private:

    StOutDevicesList          myDevices;
    StHandle<StSettings>      mySettings;
    StString                  myAbout;           //!< about string

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

    bool                      myToShowCursor;    //!< cursor visibility flag
    bool                      myToSavePlacement; //!< to save window position on exit
    bool                      myToCompressMem;   //!< reduce memory usage
    bool                      myIsBroken;        //!< special flag for broke state - when FBO can not be allocated

};

#endif // __StOutDistorted_h_
