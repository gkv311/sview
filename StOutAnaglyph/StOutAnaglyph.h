/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
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

#include <StCore/StWindow.h>
#include <StGLStereo/StGLStereoFrameBuffer.h>
#include <StThreads/StFPSControl.h>

class StSettings;
class StGLContext;

/**
 * This class implements anaglyph stereoscopic rendering.
 */
class StOutAnaglyph : public StWindow {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StOutAnaglyph(const StNativeWin_t theParentWindow);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StOutAnaglyph();

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
     * This class supports only 1 device type - anaglyph glasses.
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
     * Process callback.
     */
    ST_CPPEXPORT virtual void processEvents();

    /**
     * Stereo renderer.
     */
    ST_CPPEXPORT virtual void stglDraw();

        private:

    enum {
        DEVICE_AUTO     =-1,
        DEVICE_ANAGLYPH = 0, // Anaglyph glasses
    };

    enum {
        GLASSES_TYPE_REDCYAN  = 0, //!< Red-Cyan glasses (R + GB)
        GLASSES_TYPE_YELLOW   = 1, //!< Yellow glasses (RG + B)
        GLASSES_TYPE_GREEN    = 2, //!< Green-Magenta glasses (G + RB)
    };

    enum {
        REDCYAN_MODE_SIMPLE   = 0, //!< simple Red-Cyan anaglyph
        REDCYAN_MODE_OPTIM    = 1, //!< optimized Red-Cyan anaglyph
        REDCYAN_MODE_GRAY     = 2, //!< grayed Red-Cyan anaglyph
        REDCYAN_MODE_DARK     = 3, //!< dark Red-Cyan anaglyph
    };

    enum {
        AMBERBLUE_MODE_SIMPLE = 0, //!< simple Amber-Blue anaglyph
        AMBERBLUE_MODE_DUBOIS = 1, //!< optimized Amber-Blue anaglyph
    };

        private:

    /**
     * Switch anaglyph program.
     */
    ST_LOCAL void doSetShader(const int32_t );

    /**
     * On/off VSync callback.
     */
    ST_LOCAL void doVSync(const bool theValue);

    /**
     * Release GL resources before window closing.
     */
    ST_LOCAL void releaseResources();

        private:

    static StAtomic<int32_t> myInstancesNb; //!< shared counter for all instances

        private:

    struct {

        StHandle<StBoolParam>  IsVSyncOn; //!< flag to use VSync
        StHandle<StInt32Param> Glasses;   //!< glasses type
        StHandle<StInt32Param> RedCyan;   //!< Red-Cyan   filter
        StHandle<StInt32Param> AmberBlue; //!< Amber-Blue filter

    } params;

        private:

    typedef StGLStereoFrameBuffer::StGLStereoProgram StStereoProgram_t;

    StOutDevicesList                myDevices;
    StHandle<StSettings>            mySettings;
    StString                        myAbout;                //!< about string

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

    StFPSControl                    myFPSControl;
    bool                            myToSavePlacement;
    bool                            myToCompressMem;        //!< reduce memory usage
    bool                            myIsBroken;             //!< special flag for broke state - when FBO can not be allocated

};

#endif //__StOutAnaglyph_h_
