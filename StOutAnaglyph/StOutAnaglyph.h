/**
 * StOutAnaglyph, class providing stereoscopic output in Anaglyph format using StCore toolkit.
 * Copyright © 2007-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
    ST_CPPEXPORT StOutAnaglyph(const StHandle<StResourceManager>& theResMgr,
                               const StNativeWin_t                theParentWindow);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StOutAnaglyph();

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
     * This class supports only 1 device type - anaglyph glasses.
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
    ST_LOCAL void doSwitchVSync(const int32_t theValue);

    /**
     * Release GL resources before window closing.
     */
    ST_LOCAL void releaseResources();

    /**
     * Update strings.
     */
    ST_LOCAL void updateStrings();

        private:

    static StAtomic<int32_t> myInstancesNb; //!< shared counter for all instances

        private:

    struct {

        StHandle<StEnumParam> Glasses;   //!< glasses type
        StHandle<StEnumParam> RedCyan;   //!< Red-Cyan   filter
        StHandle<StEnumParam> AmberBlue; //!< Amber-Blue filter

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
    bool                            myToCompressMem;        //!< reduce memory usage
    bool                            myIsBroken;             //!< special flag for broke state - when FBO can not be allocated

};

#endif //__StOutAnaglyph_h_
