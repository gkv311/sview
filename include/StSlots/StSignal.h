/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StSignal_h_
#define __StSignal_h_

#include <StTemplates/StHandle.h>

#include "StSlotFunction.h"
#include "StSlotMethod.h"
#include "StSlotMethodUnsafe.h"
#include "StSlotProxy.h"

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
   mySender.onSignal = stSlot(this, &ReceiverClass::doCallback);
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

        public:

    /**
     * Empty constructor.
     */
    StSignal() : mySlot() {}
    virtual ~StSignal() {}

    /**
     * Disconnect all connected slots.
     */
    void disconnect() {
        mySlot.nullify();
    }

    /**
     * Connect class method to this signal (analog for connect() method).
     * It is supposed to use stSlot() template to create argument from class pointer + method pointer pair.
     */
    template<typename class_t>
    StSignal& operator=(const stSlotPair_t<class_t, typename StSlotMethod<class_t, slotMethod_t>::method_t>& theMethod) {
        mySlot.nullify();
        if(theMethod.ClassPtr  != NULL
        && theMethod.MethodPtr != NULL) {
            mySlot = new StSlotMethod<class_t, slotMethod_t>(theMethod.ClassPtr, theMethod.MethodPtr);
        }
        return *this;
    }

    /**
     * Connect function to this signal. Analog for connectStatic() method.
     */
    StSignal& operator=(slotMethod_t theFunction) {
        mySlot.nullify();
        if(theFunction != NULL) {
            mySlot = new StSlotFunction<slotMethod_t>(theFunction);
        }
        return *this;
    }

    /**
     * Connect more slots to this signal.
     */
    void connectExtra(const StHandle< StSlot<slotMethod_t> >& theSlot) {
        if(theSlot.isNull()) {
            return;
        } else if(mySlot.isNull()) {
            mySlot = theSlot;
            return;
        }

        // check there no duplicates
        StHandle< StSlotProxy<slotMethod_t> > aProxy;
        for(StHandle< StSlot<slotMethod_t> > aSlotIter = mySlot; aProxy.downcastFrom(aSlotIter); aSlotIter = aProxy->mySlot) {
            if(!aProxy->mySlot.isNull()
             && aProxy->mySlot->isEqual(*theSlot)) {
                return;
            } else if(!aProxy->myNext.isNull()
                    && aProxy->myNext->isEqual(*theSlot)) {
                return;
            }
        }

        StHandle< StSlot<slotMethod_t> > aNewSlot = new StSlotProxy<slotMethod_t>(mySlot, theSlot);
        mySlot = aNewSlot;
    }

    /**
     * Connect one more class method to this signal.
     */
    template<typename class_t>
    void operator+=(const stSlotPair_t<class_t, typename StSlotMethod<class_t, slotMethod_t>::method_t>& theMethod) {
        if(theMethod.ClassPtr  != NULL
        && theMethod.MethodPtr != NULL) {
            connectExtra(new StSlotMethod<class_t, slotMethod_t>(theMethod.ClassPtr, theMethod.MethodPtr));
        }
    }

    /**
     * Connect one more function to this signal.
     */
    void operator+=(slotMethod_t theFunction) {
        if(theFunction != NULL) {
            connectExtra(new StSlotFunction<slotMethod_t>(theFunction));
        }
    }

    /**
     * Disconnect slot from this signal.
     */
    void disconnect(const StSlot<slotMethod_t>& theSlot) {
        if(mySlot->isEqual(theSlot)) {
            mySlot.nullify();
            return;
        }

        StHandle< StSlotProxy<slotMethod_t> > aProxy;
        StHandle< StSlotProxy<slotMethod_t> > aProxyPrev;
        for(StHandle< StSlot<slotMethod_t> > aSlotIter = mySlot; aProxy.downcastFrom(aSlotIter); aSlotIter = aProxy->mySlot) {
            if(!aProxy->mySlot.isNull()
             && aProxy->mySlot->isEqual(theSlot)) {
                if(aProxyPrev.isNull()) {
                    mySlot = aProxy->myNext;
                } else {
                    aProxyPrev->mySlot = aProxy->myNext;
                }
                break;
            } else if(!aProxy->myNext.isNull()
                    && aProxy->myNext->isEqual(theSlot)) {
                if(aProxyPrev.isNull()) {
                    mySlot = aProxy->mySlot;
                } else {
                    aProxyPrev->mySlot = aProxy->mySlot;
                }
                break;
            }
            aProxyPrev = aProxy;
        }
    }

    /**
     * Disconnect class method from this signal.
     */
    template<typename class_t>
    void operator-=(const stSlotPair_t<class_t, typename StSlotMethod<class_t, slotMethod_t>::method_t>& theMethod) {
        if(theMethod.ClassPtr  == NULL
        || theMethod.MethodPtr == NULL
        || mySlot.isNull()) {
            return;
        }

        const StSlotMethod<class_t, slotMethod_t> aSlot(theMethod.ClassPtr, theMethod.MethodPtr);
        disconnect(aSlot);
    }

    /**
     * Disconnect function from this signal.
     */
    void operator-=(slotMethod_t theFunction) {
        if(theFunction == NULL
        || mySlot.isNull()) {
            return;
        }

        const StSlot<slotMethod_t> aSlot(theFunction);
        disconnect(aSlot);
    }

    /**
     * Connect Slot - the receiver of this Signal.
     * Previously connected slot(s) will be disconnected in result.
     * @param theClassPtr pointer to the class instance
     * @param theMethod   pointer to the class method
     * @return true if parameters are valid
     */
    template<typename class_t>
    bool connect(class_t* theClassPtr,
                 typename StSlotMethod<class_t, slotMethod_t>::method_t theMethod) {
        mySlot.nullify();
        if(theClassPtr == NULL
        || theMethod   == NULL) {
            return false;
        }

        mySlot = new StSlotMethod<class_t, slotMethod_t> (theClassPtr, theMethod);
        return mySlot->isValid();
    }

    /**
     * Method to connect Slot, doesn't provide compiler-time types check.
     * @param theClassPtr pointer to the class instance
     * @param theMethod   pointer to the static class method which retrieves the class pointer as first argument
     * @return true if parameters are valid
     */
    bool connectUnsafe(void* theClassPtr,
                       typename StSlotMethodUnsafe<slotMethod_t>::method_t theMethod) {
        mySlot = new StSlotMethodUnsafe<slotMethod_t> (theClassPtr, theMethod);
        return mySlot->isValid();
    }

    /**
     * Method to connect Slot.
     * @param theFunction pointer to the static function
     * @return true if parameters are valid
     */
    bool connectStatic(slotMethod_t theFunction) {
        mySlot.nullify();
        if(theFunction == NULL) {
            return false;
        }

        mySlot = new StSlotFunction<slotMethod_t>(theFunction);
        return mySlot->isValid();
    }

        public:

    /**
     * Emit callback Slot without arguments.
     */
    bool emit() const {
        return !mySlot.isNull() ? mySlot->call() : false;
    }

    /**
     * Emit callback Slot with 1 argument.
     */
    bool emit(typename types::arg1_t arg1) const {
        return !mySlot.isNull() ? mySlot->call(arg1) : false;
    }

    /**
     * Emit callback Slot with 2 arguments.
     */
    bool emit(typename types::arg1_t arg1,
              typename types::arg2_t arg2) const {
        return !mySlot.isNull() ? mySlot->call(arg1, arg2) : false;
    }

    /**
     * Emit callback Slot with 3 arguments.
     */
    bool emit(typename types::arg1_t arg1,
              typename types::arg2_t arg2,
              typename types::arg3_t arg3) const {
        return !mySlot.isNull() ? mySlot->call(arg1, arg2, arg3) : false;
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

        private:

    StHandle< StSlot<slotMethod_t> > mySlot; //!< handle to connected Slot

};

#endif //__StSignal_h_
