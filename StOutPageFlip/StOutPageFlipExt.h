/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutPageFlip library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutPageFlip library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
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
    ST_CPPEXPORT StOutPageFlipExt(const StNativeWin_t theParentWindow);

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
     * Callback
     */
    ST_CPPEXPORT virtual void processEvents(StMessage_t* theMessages);

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
    ST_LOCAL virtual void stglResize(const StRectI_t& theWinRect);

};

#endif //__StOutPageFlipExt_h_
