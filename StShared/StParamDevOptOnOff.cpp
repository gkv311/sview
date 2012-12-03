/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StSettings/StParamDevOptOnOff.h>

#include <StCore/StStereoDeviceInfo_t.h>
#include <StCore/StWindow.h>

StParamDevOptOnOff::StParamDevOptOnOff(StWindow*    theWindow,
                                       StSDOnOff_t* theOptionPtr)
: StBoolParam(0),
  myWindow(theWindow),
  myOption(theOptionPtr) {
    //
}

bool StParamDevOptOnOff::getValue() const {
    return myOption->value;
}

bool StParamDevOptOnOff::setValue(const bool theValue) {
    if(myOption->value != theValue) {
        myOption->value = theValue;
        StMessage_t aMsg; aMsg.uin = StMessageList::MSG_DEVICE_OPTION; aMsg.data = NULL;
        myWindow->appendMessage(aMsg);
        signals.onChanged(theValue);
        return true;
    }
    return false;
}
