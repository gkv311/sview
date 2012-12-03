/**
 * Copyright © 2007-2010 Kirill Gavrilov <kirill@sview.ru>
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

#include <StCore/StCore.h> // header for Stereo Output Core
#include <StGL/StGLVertexBuffer.h>
#include <StThreads/StFPSControl.h>

class StSettings;
class StProgramMM;
class StGLFrameBuffer;

class ST_LOCAL StOutDual : public StRendererInterface {

        private:

    typedef enum tagDeviceEnum {
        DEVICE_AUTO       =-1,
        DUALMODE_SIMPLE   = 0, //!< no mirroring
        DUALMODE_XMIRROW  = 1, //!< mirror on X SLAVE  window
        DUALMODE_YMIRROW  = 2, //!< mirror on Y SLAVE  window
    } DeviceEnum;

    enum {
        DEVICE_OPTION_VSYNC   = 0,
        DEVICE_OPTION_SHOWFPS = 1,
        DEVICE_OPTION_SLAVEID = 2,
    };

        private:

    void replaceDualAttribute(const DeviceEnum theFrom,
                              const DeviceEnum theTo);
    void optionsStructAlloc();

        public:

    StOutDual();
    ~StOutDual();
    StRendererInterface* getLibImpl() { return this; }
    StWindowInterface* getStWindow() { return myStCore->getStWindow(); }
    bool init(const StString& , const int& , const StNativeWin_t* );
    bool open(const StOpenInfo& stOpenInfo) { return myStCore->open(stOpenInfo); }
    void callback(StMessage_t* );
    void stglDraw(unsigned int );

        private:

    static StAtomic<int32_t>  myInstancesNb;     //!< shared counter for all instances

        private:

    StHandle<StCore>          myStCore;
    StHandle<StSettings>      mySettings;
    StString                  myPluginPath;

    StHandle<StGLContext>     myContext;
    StHandle<StGLFrameBuffer> myFrBuffer;        //!< OpenGL frame buffer object
    StHandle<StProgramMM>     myProgram;
    StFPSControl              myFPSControl;
    StGLVertexBuffer          myVertFlatBuf;     //!< buffers to draw simple fullsreen quad
    StGLVertexBuffer          myVertXMirBuf;
    StGLVertexBuffer          myVertYMirBuf;
    StGLVertexBuffer          myTexCoordBuf;

    StSDOptionsList_t*        myOptions;         //!< options menu

    DeviceEnum                myDevice;
    int32_t                   mySlaveMonId;      //!< slave window placement
    bool                      myToSavePlacement; //!< to save window position on exit
    bool                      myIsVSyncOn;       //!< to turn VSync on
    bool                      myToShowFPS;       //!< to show FPS
    bool                      myToCompressMem;   //!< reduce memory usage
    bool                      myIsBroken;        //!< special flag for broke state - when FBO can not be allocated


};

#endif //__StOutDual_h_
