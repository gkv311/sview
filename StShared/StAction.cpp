/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StSlots/StAction.h>

StAction::StAction(const StCString& theName)
: myName(theName),
  myHotKey1(0),
  myHotKey2(0),
  myToHoldKey(false) {
    //
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
