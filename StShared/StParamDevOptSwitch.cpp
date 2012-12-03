/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StSettings/StParamDevOptSwitch.h>

#include <StCore/StStereoDeviceInfo_t.h>
#include <StCore/StWindow.h>

StParamDevOptSwitch::StParamDevOptSwitch(StWindow*     theWindow,
                                         StSDSwitch_t* theOptionPtr)
: StInt32Param(0),
  myWindow(theWindow),
  myOption(theOptionPtr) {
    //
}

int32_t StParamDevOptSwitch::getValue() const {
    return int32_t(myOption->value);
}

bool StParamDevOptSwitch::setValue(const int32_t theValue) {
    if(myOption->value != size_t(theValue)) {
        myOption->value = size_t(theValue);
        StMessage_t aMsg; aMsg.uin = StMessageList::MSG_DEVICE_OPTION; aMsg.data = NULL;
        myWindow->appendMessage(aMsg);
        signals.onChanged(theValue);
        return true;
    }
    return false;
}
