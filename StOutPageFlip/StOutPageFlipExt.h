/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
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

#include "StOutPageFlip.h"

#include "StGLColoredLine.h"
#include "StGLControlED.h"
#include "StDXInfo.h"

class ST_LOCAL StOutPageFlipExt : public StOutPageFlip {

        private:

    enum {
        DEVICE_OPTION_SHOWFPS    = 0,
        DEVICE_OPTION_EXTRA      = 1,
        DEVICE_OPTION_QUADBUFFER = 2,
        DEVICE_OPTION_CONTROL    = 3,
    } DeviceOption;

    typedef enum tagDeviceControlEnum {
        DEVICE_CONTROL_NONE      = 0,
        DEVICE_CONTROL_BLUELINE  = 1,
        DEVICE_CONTROL_WHITELINE = 2,
        DEVICE_CONTROL_ED_ON_OFF = 3,
    } DeviceControlEnum;

    StHandle<StMonitor> myMonitor;      //!< current monitor
    StGLColoredLine     myCodesLine;
    StGLControlED       myCodesEDOnOff;
    GLsizei             myVpSizeY;      //!< VIewPort Y size
    GLsizei             myVpSizeX;      //!< VIewPort X size
    DeviceControlEnum   myDeviceCtrl;
    bool                myIsQuiting;

    bool isControlOn() const {
        return myDeviceCtrl != DEVICE_CONTROL_NONE;
    }

    StGLDeviceControl* getDeviceControl() {
        switch(myDeviceCtrl) {
            case DEVICE_CONTROL_BLUELINE:  return &myCodesLine;
            case DEVICE_CONTROL_WHITELINE: return &myCodesLine;
            case DEVICE_CONTROL_ED_ON_OFF: return &myCodesEDOnOff;
            default: return NULL;
        }
    }

    void setDeviceControl(DeviceControlEnum newDeviceControl);
    void setSlavePosition(int thePositionId);

    virtual void optionsStructAlloc();
    virtual void updateOptions(const StSDOptionsList_t* theOptions,
                               StMessage_t&             theMsg);
    virtual void stglDrawExtra(unsigned int theView, int theMode);
    virtual void stglResize(const StRectI_t& theWinRect);
    virtual void parseKeys(bool* theKeysMap);

        public:

    StOutPageFlipExt(const StHandle<StSettings>& theSettings);
    virtual ~StOutPageFlipExt();
    virtual bool init(const StString& , const int& , const StNativeWin_t );
    virtual void callback(StMessage_t* theMessages);

};

#endif //__StOutPageFlipExt_h_
