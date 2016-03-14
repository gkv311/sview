/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

#ifndef __StCADMsgPrinter_h_
#define __StCADMsgPrinter_h_

#include <Message_Printer.hxx>
#include <StStrings/StMsgQueue.h>

/**
 * Printer redirecting OCCT messages to GUI.
 */
class StCADMsgPrinter : public Message_Printer {

        public:

    /**
     * Main constructor.
     */
    ST_LOCAL StCADMsgPrinter(const StHandle<StMsgQueue> theMsgQueue);

    /**
     * Send specified message - redirector.
     */
    ST_LOCAL virtual void Send(const TCollection_ExtendedString& theString,
                               const Message_Gravity             theGravity,
                               const Standard_Boolean            theToPutEndl) const Standard_OVERRIDE;

    /**
     * Send specified message - redirector.
     */
    ST_LOCAL virtual void Send(const Standard_CString theString,
                               const Message_Gravity  theGravity,
                               const Standard_Boolean theToPutEndl) const Standard_OVERRIDE;

    /**
     * Send specified message - redirector.
     */
    ST_LOCAL virtual void Send(const TCollection_AsciiString& theString,
                               const Message_Gravity          theGravity,
                               const Standard_Boolean         theToPutEndl) const Standard_OVERRIDE;

    /**
     * Send specified message - main implementation.
     */
    ST_LOCAL void send(const StString&        theString,
                       const Message_Gravity  theGravity,
                       const Standard_Boolean theToPutEndl) const;

        private:

    StHandle<StMsgQueue> myMsgQueue;

        public:

    DEFINE_STANDARD_RTTI_INLINE(StCADMsgPrinter, Message_Printer)

};

DEFINE_STANDARD_HANDLE(StCADMsgPrinter, Message_Printer)

#endif // __StCADMsgPrinter_h_
