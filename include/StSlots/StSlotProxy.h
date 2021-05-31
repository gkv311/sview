/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StSlotProxy_h_
#define __StSlotProxy_h_

#include "StSlotTypes.h"
#include "StSlot.h"

/**
 * This class implements item of sequence of multiple slots.
 */
template<typename slotMethod_t>
class StSlotProxy : public StSlot<slotMethod_t> {

        public:

    typedef StSlotTypes<stNoType, slotMethod_t> types; //!< fast link to all useful types

        public:

    /**
     * Main constructor.
     */
    StSlotProxy(const StHandle< StSlot<slotMethod_t> >& theSlot1,
                const StHandle< StSlot<slotMethod_t> >& theSlot2)
    : StSlot<slotMethod_t>(),
      mySlot(theSlot1),
      myNext(theSlot2) {}

    virtual ~StSlotProxy() {}

    /**
     * Validate the callback Slot.
     * @return true if main slot is valid
     */
    virtual bool isValid() const {
        return !mySlot.isNull()
             && mySlot->isValid();
    }

    /**
     * Only real slots should be compared!
     */
    virtual bool isEqual(const StSlot<slotMethod_t>& ) const {
        return false;
    }

    bool call() const {
        const bool isCalled1 = !mySlot.isNull() && mySlot->call();
        const bool isCalled2 = !myNext.isNull() && myNext->call();
        return isCalled1 || isCalled2;
    }

    bool call(typename types::arg1_t arg1) const {
        const bool isCalled1 = !mySlot.isNull() && mySlot->call(arg1);
        const bool isCalled2 = !myNext.isNull() && myNext->call(arg1);
        return isCalled1 || isCalled2;
    }

    bool call(typename types::arg1_t arg1,
              typename types::arg2_t arg2) const {
        const bool isCalled1 = !mySlot.isNull() && mySlot->call(arg1, arg2);
        const bool isCalled2 = !myNext.isNull() && myNext->call(arg1, arg2);
        return isCalled1 || isCalled2;
    }

    bool call(typename types::arg1_t arg1,
              typename types::arg2_t arg2,
              typename types::arg3_t arg3) const {
        const bool isCalled1 = !mySlot.isNull() && mySlot->call(arg1, arg2, arg3);
        const bool isCalled2 = !myNext.isNull() && myNext->call(arg1, arg2, arg3);
        return isCalled1 || isCalled2;
    }

        public:

    StHandle< StSlot<slotMethod_t> > mySlot; //!< handle to connected Slot
    StHandle< StSlot<slotMethod_t> > myNext; //!< handle to next      Slot

};

#endif // __StSlotProxy_h_
