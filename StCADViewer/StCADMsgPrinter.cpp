/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

#include "StCADMsgPrinter.h"

#include <TCollection_AsciiString.hxx>
#include <TCollection_ExtendedString.hxx>

StCADMsgPrinter::StCADMsgPrinter(const StHandle<StMsgQueue> theMsgQueue)
: myMsgQueue(theMsgQueue) {}

void StCADMsgPrinter::Send(const TCollection_ExtendedString& theString,
                           const Message_Gravity             theGravity,
                           const Standard_Boolean            theToPutEndl) const {
    send(StString((const stUtf16_t* )theString.ToExtString()), theGravity, theToPutEndl);
}

void StCADMsgPrinter::Send(const Standard_CString theString,
                           const Message_Gravity  theGravity,
                           const Standard_Boolean theToPutEndl) const {
    send(StString(theString), theGravity, theToPutEndl);
}

void StCADMsgPrinter::Send(const TCollection_AsciiString& theString,
                           const Message_Gravity          theGravity,
                           const Standard_Boolean         theToPutEndl) const {
    send(StString(theString.ToCString()), theGravity, theToPutEndl);
}

void StCADMsgPrinter::send(const StString&        theString,
                           const Message_Gravity  theGravity,
                           const Standard_Boolean theToPutEndl) const {
    if(theGravity < myTraceLevel) {
        return;
    }

    switch(theGravity) {
        case Message_Trace:
        case Message_Info:
        case Message_Warning: {
            myMsgQueue->pushInfo(theString);
            break;
        }
        case Message_Alarm:
        case Message_Fail: {
            myMsgQueue->pushError(theString);
            break;
        }
    }
}
