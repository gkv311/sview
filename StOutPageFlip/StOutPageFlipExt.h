/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2007-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
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
    ST_CPPEXPORT virtual bool create();

    /**
     * Extra routines to be processed before window close.
     */
    ST_CPPEXPORT virtual void beforeClose();

    /**
     * Callback
     */
    ST_CPPEXPORT virtual void processEvents();

    /**
     * Retrieve options list.
     */
    ST_CPPEXPORT virtual void getOptions(StParamsList& theList) const;

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
    ST_LOCAL virtual void releaseResources();

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

    ST_LOCAL virtual void stglDrawExtra(unsigned int theView, int theMode);

};

#endif //__StOutPageFlipExt_h_
