/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StSlotMethodUnsafe_h_
#define __StSlotMethodUnsafe_h_

#include "StSlotTypes.h"
#include "StSlot.h"

/**
 * This is an implementation template for class StSlot
 * designed for callback Slot has a static-method wrapper in a class.
 * Because it store class pointer as a generic void* the compile-time
 * types check is impossible and errors in usage may cause run-time errors
 * (if input pointer is not an instance of the desired class). However
 * generic or custom RTTI mechanism could be used to provide runtime safe-check
 * (majority for debugging purposes).
 *
 * The main purpose of this unsafe Slot definition is to provide a way for
 * C-style interconnection between the libraries (thus - intra-compilers
 * and intra-languages compatible).
 *
 * Class implements ::call() methods for all possible arguments list,
 * however only one of them will be callable from StSlot interface.
 */
template<typename slotMethod_t>
class StSlotMethodUnsafe : public StSlot<slotMethod_t> {

        public:

    typedef StSlotTypes<stNoType, slotMethod_t> types; //!< fast link to all useful types
    typedef typename types::methodUnsafe_t method_t;   //!< fast link to method type definition

        private:

    void*    myClassPtr;  //!< pointer to the class instance
    method_t myMethodPtr; //!< pointer to the method (static!)

    bool isValidInline() const {
        return myClassPtr != NULL && myMethodPtr != NULL;
    }

        public:

    /**
     * Main constructor.
     */
    StSlotMethodUnsafe(void* theClassPtr, method_t theMethodPtr)
    : StSlot<slotMethod_t>(),
      myClassPtr(theClassPtr),
      myMethodPtr(theMethodPtr) {}

    virtual ~StSlotMethodUnsafe() {}

    /**
     * Validate the callback Slot.
     * @return true if pointer to the class instance and static function are set.
     */
    virtual bool isValid() const {
        return isValidInline();
    }

    /**
     * Compare two slots.
     */
    virtual bool isEqual(const StSlot<slotMethod_t>& theOther) const {
        const StSlotMethodUnsafe* anOther = dynamic_cast<const StSlotMethodUnsafe*>(&theOther);
        return anOther     != NULL
            && myClassPtr  == anOther->myClassPtr
            && myMethodPtr == anOther->myMethodPtr;
    }

    bool call() const {
        if(isValidInline()) {
            myMethodPtr(myClassPtr);
            return true;
        }
        return false;
    }

    bool call(typename types::arg1_t arg1) const {
        if(isValidInline()) {
            myMethodPtr(myClassPtr, arg1);
            return true;
        }
        return false;
    }

    bool call(typename types::arg1_t arg1,
              typename types::arg2_t arg2) const {
        if(isValidInline()) {
            myMethodPtr(myClassPtr, arg1, arg2);
            return true;
        }
        return false;
    }

    bool call(typename types::arg1_t arg1,
              typename types::arg2_t arg2,
              typename types::arg3_t arg3) const {
        if(isValidInline()) {
            myMethodPtr(myClassPtr, arg1, arg2, arg3);
            return true;
        }
        return false;
    }

};

#endif //__StSlotMethodUnsafe_h_
