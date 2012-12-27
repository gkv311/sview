/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutAnaglyph library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutAnaglyph library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StOutAnaglyph_h_
#define __StOutAnaglyph_h_

#include <StCore/StCore.h>     // header for Stereo Output Core
#include <StGLStereo/StGLStereoFrameBuffer.h>
#include <StThreads/StFPSControl.h>

class StSettings;
class StGLContext;

class ST_LOCAL StOutAnaglyph : public StRendererInterface {

        private:

    enum {
        DEVICE_AUTO     =-1,
        DEVICE_ANAGLYPH = 0, // Anaglyph glasses
    };

    enum {
        GLASSES_TYPE_REDCYAN = 0, // Red-Cyan glasses (R + GB)
        GLASSES_TYPE_YELLOW  = 1, // Yellow glasses (RG + B)
        GLASSES_TYPE_GREEN   = 2, // Green-Magenta glasses (G + RB)
    };

    enum {
        REDCYAN_MODE_SIMPLE = 0, // simple Red-Cyan anaglyph
        REDCYAN_MODE_OPTIM  = 1, // optimized Red-Cyan anaglyph
        REDCYAN_MODE_GRAY   = 2, // grayed Red-Cyan anaglyph
        REDCYAN_MODE_DARK   = 3, // dark Red-Cyan anaglyph
    };

    enum {
        AMBERBLUE_MODE_SIMPLE = 0, // simple Amber-Blue anaglyph
        AMBERBLUE_MODE_DUBOIS = 1, // optimized Amber-Blue anaglyph
    };

    enum {
        DEVICE_OPTION_VSYNC   = 0,
        DEVICE_OPTION_SHOWFPS = 1,
        DEVICE_OPTION_GLASSES = 2,
        DEVICE_OPTION_REDCYAN = 3,
        DEVICE_OPTION_YELLOW  = 4,
    };

        private:

    void setShader(const int theGlasses,
                   const int theOptionRedCyan,
                   const int theOptionAmberBlue);
    void optionsStructAlloc();

        public:

    StOutAnaglyph();
    ~StOutAnaglyph();
    StRendererInterface* getLibImpl() { return this; }
    StWindowInterface* getStWindow() { return myStCore->getStWindow(); }
    bool init(const StString& , const int& , const StNativeWin_t );
    bool open(const StOpenInfo& stOpenInfo) { return myStCore->open(stOpenInfo); }
    void callback(StMessage_t* );
    void stglDraw(unsigned int );

        private:

    static StAtomic<int32_t>        myInstancesNb;          //!< shared counter for all instances

        private:

    typedef StGLStereoFrameBuffer::StGLStereoProgram StStereoProgram_t;

    StHandle<StCore>                myStCore;
    StHandle<StSettings>            mySettings;
    StString                        myPluginPath;

    StHandle<StGLContext>           myContext;
    StHandle<StGLStereoFrameBuffer> myFrBuffer;
    StStereoProgram_t*              myStereoProgram;        //!< pointer to current anaglyph program
    StStereoProgram_t               mySimpleAnaglyph;       //!< all available anaglyph programs
    StStereoProgram_t               myGrayAnaglyph;
    StStereoProgram_t               myTrueAnaglyph;
    StStereoProgram_t               myOptimAnaglyph;
    StStereoProgram_t               myYellowAnaglyph;
    StStereoProgram_t               myYellowDubiosAnaglyph;
    StStereoProgram_t               myGreenAnaglyph;
    int                             myGlasses;
    int                             myOptionRedCyan;
    int                             myOptionAmberBlue;

    StSDOptionsList_t*              myOptions;

    StFPSControl                    myFPSControl;
    bool                            myToSavePlacement;
    bool                            myIsVSyncOn;
    bool                            myToShowFPS;
    bool                            myToCompressMem;        //!< reduce memory usage
    bool                            myIsBroken;             //!< special flag for broke state - when FBO can not be allocated

};

#endif //__StOutAnaglyph_h_
