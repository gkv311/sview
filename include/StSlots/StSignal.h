/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StSignal_h_
#define __StSignal_h_

#include <StTemplates/StHandle.h>

#include "StSlotFunction.h"
#include "StSlotMethod.h"
#include "StSlotMethodUnsafe.h"

/** @page signals_and_slots Signals and Slots

There are many techniques have been invented to define callbacks per event.
The main problem is that most of them were implemented for some local logic
and can not be reused in other places (doesn't provide interface for classes
for example or can not be used outside the API for your own classes).

Signals and Slots is a unified mechanism for interconnection between classes.
When one class generate some events it should declare the Signals objects for each event.
Another class that should provide callback for some event should declare Slots methods
and register them to Signals in first class (connect Slots to the Signals).
Than when event occur Signal object emit the Slot method and interconnection is done.
Thus second class can process this event.

Some APIs already implements such kind of logic (like Qt framework).
sView SDK provides its own implementation represented by classes templates StSlot and StSignal.
API doesn't requires strong classes hierarchy (in fact it could be used outside the classes)
and can be used for interconnection between classes (providing strong compile-time types check)
or even the libraries.

Usage sample:

@code
// SenderClass.h
class SenderClass {

        public:

    // some worker
    void loopIter() {
        double d = 33.3;
        onSignal(22, &d);
    }

        public: // Signals

    StSignal<void (int , const double* )> onSignal;

};
@endcode

@code
// ReceiverClass.h
class ReceiverClass {

        private:

    int myPrivateInt;
    SenderClass mySender;

        public:

    ReceiverClass();

        public: // Slots

    void doCallback(int arg1, const double* arg2);

};
@endcode

@code
// ReceiverClass.cpp
ReceiverClass::ReceiverClass()
: myPrivateInt(4), mySender() {
   mySender.onSignal.connect(this, &ReceiverClass::doCallback);
   mySender.loopIter();
}

void ReceiverClass::doCallback(int arg1, const double* arg2) {
   myPrivateInt = (int )(*arg2 + arg1);
}
@endcode
 */

/**
 * This class defines the Signal object.
 * The template argument is a function definition like 'void (int , double )'
 * and should be set explicitly.
 * Class implements ::emit() methods with all possible arguments count,
 * however only one of them may be used (which corresponds to the function definition).
 * If user will try to call another ::emit function it will give compilation-time error.
 */
template<typename slotMethod_t>
class StSignal {

        public:

    typedef StSlotTypes<stNoType, slotMethod_t> types; //!< fast link to all useful types

        private:

    StHandle< StSlot<slotMethod_t> > mySlot;           //!< connected Slot (stored as handle)

        public:

    /**
     * Empty constructor.
     */
    StSignal() : mySlot() {}
    virtual ~StSignal() {}

    /**
     * Disconnect all slots.
     */
    void disconnect() {
        mySlot.nullify();
    }

    /**
     * Major method to connect Slot - the receiver of this Signal.
     * @param theClassPtr (class_t* ) - pointer to the class instance;
     * @param theMethod - pointer to the class method;
     * @return true if parameters are valid.
     */
    template<typename class_t>
    bool connect(class_t* theClassPtr,
                 typename StSlotMethod<class_t, slotMethod_t>::method_t theMethod) {
        mySlot = new StSlotMethod<class_t, slotMethod_t> (theClassPtr, theMethod);
        return !mySlot.isNull() && mySlot->isValid();
    }

    /**
     * Method to connect Slot, doesn't provide compiler-time types check.
     * @param theClassPtr (void* ) - pointer to the class instance;
     * @param theMethod - pointer to the static class method which retrieves the class pointer as first argument;
     * @return true if parameters are valid.
     */
    bool connectUnsafe(void* theClassPtr,
                       typename StSlotMethodUnsafe<slotMethod_t>::method_t theMethod) {
        mySlot = new StSlotMethodUnsafe<slotMethod_t> (theClassPtr, theMethod);
        return !mySlot.isNull() && mySlot->isValid();
    }

    /**
     * Method to connect Slot.
     * @param theFunction - pointer to the static function;
     * @return true if parameters are valid.
     */
    bool connectStatic(slotMethod_t theFunction) {
        mySlot = new StSlotFunction<slotMethod_t> (theFunction);
        return !mySlot.isNull() && mySlot->isValid();
    }

        public:

    /**
     * Emit callback Slot without arguments.
     */
    bool emit() const {
        StHandle< StSlot<slotMethod_t> > aSlot = mySlot;
        return !aSlot.isNull() ? aSlot->call() : false;
    }

    /**
     * Emit callback Slot with 1 argument.
     */
    bool emit(typename types::arg1_t arg1) const {
        StHandle< StSlot<slotMethod_t> > aSlot = mySlot;
        return !aSlot.isNull() ? aSlot->call(arg1) : false;
    }

    /**
     * Emit callback Slot with 2 arguments.
     */
    bool emit(typename types::arg1_t arg1,
              typename types::arg2_t arg2) const {
        StHandle< StSlot<slotMethod_t> > aSlot = mySlot;
        return !aSlot.isNull() ? aSlot->call(arg1, arg2) : false;
    }

    /**
     * Emit callback Slot with 3 arguments.
     */
    bool emit(typename types::arg1_t arg1,
              typename types::arg2_t arg2,
              typename types::arg3_t arg3) const {
        StHandle< StSlot<slotMethod_t> > aSlot = mySlot;
        return !aSlot.isNull() ? aSlot->call(arg1, arg2, arg3) : false;
    }

    /**
     * Function-style calls to ::emit() methods.
     */
    bool operator()() const { return emit(); }
    bool operator()(typename types::arg1_t arg1) const { return emit(arg1); }
    bool operator()(typename types::arg1_t arg1,
                    typename types::arg2_t arg2) const { return emit(arg1, arg2); }
    bool operator()(typename types::arg1_t arg1,
                    typename types::arg2_t arg2,
                    typename types::arg3_t arg3) const { return emit(arg1, arg2, arg3); }

};

#endif //__StSignal_h_
