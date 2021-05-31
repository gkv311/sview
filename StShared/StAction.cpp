/**
 * Copyright © 2013-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StSlots/StAction.h>
#include <StCore/StEvent.h>

StAction::StAction(const StCString& theName)
: myName(theName),
  myToHoldKey(false) {
    myHotKeys[0] = 0;
    myHotKeys[1] = 0;
    myHotKeysDef[0] = 0;
    myHotKeysDef[1] = 0;
}

StAction::~StAction() {}

StActionBool::StActionBool(const StCString&             theName,
                           const StHandle<StBoolParam>& theParam)
: StAction(theName),
  myParam(theParam) {
    //
}

StActionBool::~StActionBool() {}

void StActionBool::doTrigger(const StEvent* ) {
    myParam->reverse();
}

StActionIntValue::StActionIntValue(const StCString&              theName,
                                   const StHandle<StInt32Param>& theParam,
                                   const int32_t                 theValue)
: StAction(theName),
  myParam(theParam),
  myValue(theValue) {
    //
}

StActionIntValue::~StActionIntValue() {}

void StActionIntValue::doTrigger(const StEvent* ) {
    myParam->setValue(myValue);
}

void StActionIntSlot::doTrigger(const StEvent* ) {
    mySlot->call(myValue);
}

StActionIntSlot::~StActionIntSlot() {}

void StActionHoldSlot::doTrigger(const StEvent* theEvent) {
    double aValue = 0.0;
    if(theEvent != NULL) {
        if(theEvent->Type == stEvent_KeyHold) {
            aValue = theEvent->Key.Progress;
        } else if(theEvent->Type == stEvent_Action) {
            aValue = theEvent->Action.Progress;
        }
    }
    mySlot->call(aValue);
}

StActionHoldSlot::~StActionHoldSlot() {}
