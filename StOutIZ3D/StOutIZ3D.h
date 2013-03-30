/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
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

#include <StCore/StCore.h>       // header for Stereo Output Core
#include <StThreads/StThreads.h> // threads header (mutexes, threads,...)
#include <StThreads/StFPSControl.h>
#include <StGL/StGLTexture.h>

#include "StOutIZ3DShaders.h"

class StSettings;

class ST_LOCAL StOutIZ3D : public StRendererInterface {

        private:

    enum {
        DEVICE_OPTION_VSYNC   = 0,
        DEVICE_OPTION_SHADER  = 1,
    };

        private:

    void optionsStructAlloc();

        public:

    StOutIZ3D();
    ~StOutIZ3D();
    StRendererInterface* getLibImpl() { return this; }
    StWindowInterface* getStWindow() { return myStCore->getStWindow(); }
    bool init(const StString& , const int& , const StNativeWin_t );
    bool open(const StOpenInfo& stOpenInfo) { return myStCore->open(stOpenInfo); }
    void callback(StMessage_t* );
    void stglDraw(unsigned int );

        private:

    static StAtomic<int32_t>        myInstancesNb;     //!< shared counter for all instances

        private:

    StHandle<StCore>                myStCore;
    StHandle<StSettings>            mySettings;
    StString                        myPluginPath;

    StHandle<StGLContext>           myContext;
    StHandle<StGLStereoFrameBuffer> myFrBuffer;        //!< frame buffer to draw
    StOutIZ3DShaders                myShaders;         //!< IZ3D shaders
    StGLTexture                     myTexTableOld;     //!< table textures
    StGLTexture                     myTexTableNew;

    StSDOptionsList_t*              myOptions;

    StFPSControl                    myFPSControl;
    bool                            myToSavePlacement;
    bool                            myIsVSyncOn;
    bool                            myToCompressMem;   //!< reduce memory usage
    bool                            myIsBroken;        //!< special flag for broke state - when FBO can not be allocated

};

#endif //__StOutIZ3D_h_
