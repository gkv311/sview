/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
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

#include <StCore/StCore.h>       // header for Stereo Output Core
#include <StThreads/StThreads.h> // threads header (mutexes, threads,...)
#include <StThreads/StFPSControl.h>
#include <StGL/StGLProgram.h>
#include <StGL/StGLFrameBuffer.h>
#include <StGL/StGLVertexBuffer.h>

class StSettings;

/**
 * Just simple GLSL program.
 */
class ST_LOCAL StProgramFB : public StGLProgram {

        public:

    StProgramFB(const StString& theTitle);
    virtual bool link(StGLContext& theCtx);

};

class ST_LOCAL StOutInterlace : public StRendererInterface {

        private:

    enum {
        DEVICE_AUTO          =-1,
        DEVICE_HINTERLACE    = 0, //!< interlace (horizontal 1xPixel lines, full color from R or L)
        DEVICE_VINTERLACE    = 1, //!< interlace (vertical 1xPixel lines, full color from R or L)
        DEVICE_CHESSBOARD    = 2, //!< 1xPixel chessboard (some DLP devices)
        DEVICE_HINTERLACE_ED = 3, //!< interlace (horizontal 1xPixel lines) + EDimensional onscreen codes

        DEVICE_NB,
    };

    enum {
        DEVICE_OPTION_VSYNC   = 0,
        DEVICE_OPTION_SHOWFPS = 1,
        DEVICE_OPTION_REVERSE = 2,
        DEVICE_OPTION_BINDMON = 3,
    };

        private:

    static StAtomic<int32_t>  myInstancesNb;              //!< shared counter for all instances

        private:

    StHandle<StCore>          myStCore;
    StHandle<StSettings>      mySettings;
    StString                  myPluginPath;
    StHandle<StGLContext>     myContext;
    StHandle<StGLFrameBuffer> myFrmBuffer;                //!< OpenGL frame buffer object
    StHandle<StProgramFB>     myGlPrograms[DEVICE_NB];    //!< GLSL programs
    StHandle<StProgramFB>     myGlProgramsRev[DEVICE_NB]; //!< GLSL programs with reversed left/right condition
    StGLVertexBuffer          myQuadVertBuf;
    StGLVertexBuffer          myQuadTexCoordBuf;
    int                       myDeviceId;
    StHandle<StMonitor>       myMonitor;                  //!< current monitor

    StTimer                   myEDTimer;                  //!< EDimensional activator/disactivator timer
    StHandle<StGLProgram>     myEDIntelaceOn;             //!< ED interlace activate program
    StHandle<StGLProgram>     myEDOff;                    //!< ED disactivate program
    GLsizei                   myVpSizeY;                  //!< VIewPort Y size
    StGLVarLocation           myVpSizeYOnLoc;             //!< helper shader variables
    StGLVarLocation           myVpSizeYOffLoc;

    StSDOptionsList_t*        myOptionsStruct;
    StFPSControl              myFPSControl;
    bool                      myToSavePlacement;
    bool                      myToBindToMonitor;
    bool                      myIsVSync;
    bool                      myToShowFPS;
    bool                      myIsReversed;
    bool                      myIsStereo;
    bool                      myIsEDactive;
    bool                      myIsEDCodeFinished;
    bool                      myToCompressMem;            //!< reduce memory usage
    bool                      myIsBroken;                 //!< special flag for broke state - when FBO can not be allocated

        private:

    void setDevice(const int theDeviceId);
    void optionsStructAlloc();
    void stglDrawEDCodes();

        public:

    static StHandle<StMonitor> getHInterlaceMonitor();

    StOutInterlace();
    ~StOutInterlace();
    StRendererInterface* getLibImpl() { return this; }
    StWindowInterface* getStWindow() { return myStCore->getStWindow(); }
    bool init(const StString& , const int& , const StNativeWin_t );
    bool open(const StOpenInfo& stOpenInfo) { return myStCore->open(stOpenInfo); }
    void callback(StMessage_t* );
    void stglDraw(unsigned int );

};

#endif //__StOutInterlace_h_
