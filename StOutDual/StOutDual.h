/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StOutDual_h_
#define __StOutDual_h_

#include <StCore/StWindow.h>
#include <StGL/StGLVertexBuffer.h>
#include <StThreads/StFPSControl.h>

class StSettings;
class StProgramMM;
class StGLFrameBuffer;

/**
 * This class implements stereoscopic rendering on displays
 * with independent connection to each view.
 */
class StOutDual : public StWindow {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StOutDual(const StNativeWin_t theParentWindow);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StOutDual();

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
    ST_CPPEXPORT virtual void processEvents(StMessage_t* theMessages);

    /**
     * Stereo renderer.
     */
    ST_CPPEXPORT virtual void stglDraw();

        private:

    typedef enum tagDeviceEnum {
        DEVICE_AUTO       =-1,
        DUALMODE_SIMPLE   = 0, //!< no mirroring
        DUALMODE_XMIRROW  = 1, //!< mirror on X SLAVE  window
        DUALMODE_YMIRROW  = 2, //!< mirror on Y SLAVE  window
    } DeviceEnum;

        private:

    ST_LOCAL void replaceDualAttribute(const DeviceEnum theValue);

    /**
     * Release GL resources before window closing.
     */
    ST_LOCAL void releaseResources();

    /**
     * On/off VSync callback.
     */
    ST_LOCAL void doVSync(const bool theValue);

    /**
     * Change slave window position callback.
     */
    ST_LOCAL void doSlaveMon(const int32_t theValue);

        private:

    static StAtomic<int32_t> myInstancesNb; //!< shared counter for all instances

        private:

    struct {

        StHandle<StBoolParam>  IsVSyncOn;  //!< flag to use VSync
        StHandle<StInt32Param> SlaveMonId; //!< slave window position

    } params;

        private:

    StOutDevicesList          myDevices;
    StHandle<StSettings>      mySettings;
    StString                  myAbout;           //!< about string

    StHandle<StGLContext>     myContext;
    StHandle<StGLFrameBuffer> myFrBuffer;        //!< OpenGL frame buffer object
    StHandle<StProgramMM>     myProgram;
    StFPSControl              myFPSControl;
    StGLVertexBuffer          myVertFlatBuf;     //!< buffers to draw simple fullsreen quad
    StGLVertexBuffer          myVertXMirBuf;
    StGLVertexBuffer          myVertYMirBuf;
    StGLVertexBuffer          myTexCoordBuf;

    DeviceEnum                myDevice;
    bool                      myToSavePlacement; //!< to save window position on exit
    bool                      myToCompressMem;   //!< reduce memory usage
    bool                      myIsBroken;        //!< special flag for broke state - when FBO can not be allocated

};

#endif //__StOutDual_h_
