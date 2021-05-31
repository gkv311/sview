/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2007-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StOutPageFlipExt_h_
#define __StOutPageFlipExt_h_

#include <StCore/StMonitor.h>

#include "StOutPageFlip.h"

#include "StGLColoredLine.h"
#include "StGLControlED.h"
#include "StDXInfo.h"

/**
 * Stereoscopic renderer with compatibility options for old hardware.
 */
class StOutPageFlipExt : public StOutPageFlip {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StOutPageFlipExt(const StHandle<StResourceManager>& theResMgr,
                                  const StNativeWin_t                theParentWindow);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StOutPageFlipExt();

    /**
     * Create and show window.
     * @return false if any critical error appeared
     */
    ST_CPPEXPORT virtual bool create() ST_ATTR_OVERRIDE;

    /**
     * Extra routines to be processed before window close.
     */
    ST_CPPEXPORT virtual void beforeClose() ST_ATTR_OVERRIDE;

    /**
     * Callback
     */
    ST_CPPEXPORT virtual void processEvents() ST_ATTR_OVERRIDE;

    /**
     * @param theFullScreen fullscreen state
     */
    ST_CPPEXPORT virtual void setFullScreen(const bool theFullScreen) ST_ATTR_OVERRIDE;

    /**
     * Retrieve options list.
     */
    ST_CPPEXPORT virtual void getOptions(StParamsList& theList) const ST_ATTR_OVERRIDE;

    /**
     * Update strings.
     */
    ST_LOCAL virtual void doChangeLanguage() ST_ATTR_OVERRIDE {
        StOutPageFlip::doChangeLanguage();
        updateStringsExt();
    }

        private:

    typedef enum tagDeviceControlEnum {
        DEVICE_CONTROL_NONE      = 0,
        DEVICE_CONTROL_BLUELINE  = 1,
        DEVICE_CONTROL_WHITELINE = 2,
        DEVICE_CONTROL_ED_ON_OFF = 3,
    } DeviceControlEnum;

    /**
     * Release GL resources before window closing.
     */
    ST_LOCAL virtual void releaseResources() ST_ATTR_OVERRIDE;

    /**
     * Update strings.
     */
    ST_LOCAL void updateStringsExt();

        protected:

    struct {

        StHandle<StEnumParam> ControlCode; //!< control code option

    } params;

        private:

    StHandle<StMonitor> myMonitor;      //!< current monitor
    StRectI_t           myWinRect;
    StGLColoredLine     myCodesLine;
    StGLControlED       myCodesEDOnOff;
    GLsizei             myVpSizeY;      //!< VIewPort Y size
    GLsizei             myVpSizeX;      //!< VIewPort X size
    bool                myIsQuiting;

    ST_LOCAL bool isControlOn() const {
        return params.ControlCode->getValue() != DEVICE_CONTROL_NONE;
    }

    ST_LOCAL StGLDeviceControl* getDeviceControl() {
        switch(params.ControlCode->getValue()) {
            case DEVICE_CONTROL_BLUELINE:  return &myCodesLine;
            case DEVICE_CONTROL_WHITELINE: return &myCodesLine;
            case DEVICE_CONTROL_ED_ON_OFF: return &myCodesEDOnOff;
            default: return NULL;
        }
    }

    ST_LOCAL void doSetDeviceControl(const int32_t theValue);
    ST_LOCAL void setSlavePosition(int thePositionId);

    ST_LOCAL virtual void stglDrawExtra(unsigned int theView, int theMode) ST_ATTR_OVERRIDE;

};

#endif //__StOutPageFlipExt_h_
