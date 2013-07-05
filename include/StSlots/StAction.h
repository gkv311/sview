/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StAction_h_
#define __StAction_h_

#include <StTemplates/StHandle.h>
#include <StSettings/StParam.h>
#include <StSlots/StSignal.h>

union StEvent;

/**
 * This class represent action - the single slot without arguments.
 */
class StAction {

        public:

    /**
     * Constructor.
     */
    ST_CPPEXPORT StAction(const StCString& theName);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StAction();

    /**
     * Execute action.
     * @param theEvent event description (optional)
     */
    virtual void doTrigger(const StEvent* theEvent) = 0;

    /**
     * @return hot key 1 to trigger action
     */
    ST_LOCAL unsigned int getHotKey1() const {
        return myHotKey1;
    }

    /**
     * @return hot key 1 to trigger action
     */
    ST_LOCAL unsigned int& changeHotKey1() {
        return myHotKey1;
    }

    /**
     * @param theKey hot key 1 to trigger action
     */
    ST_LOCAL void setHotKey1(unsigned int theKey) {
        myHotKey1 = theKey;
    }

    /**
     * @return hot key 2 to trigger action
     */
    ST_LOCAL unsigned int getHotKey2() const {
        return myHotKey2;
    }

    /**
     * @return hot key 2 to trigger action
     */
    ST_LOCAL unsigned int& changeHotKey2() {
        return myHotKey2;
    }

    /**
     * @param theKey hot key 2 to trigger action
     */
    ST_LOCAL void setHotKey2(unsigned int theKey) {
        myHotKey2 = theKey;
    }

    /**
     * @return true if this action process duration value
     */
    ST_LOCAL bool isHoldKey() const {
        return myToHoldKey;
    }

    /**
     * @param theValue if true this action process duration value
     */
    ST_LOCAL void setHoldKey(const bool theValue) {
        myToHoldKey = theValue;
    }

    /**
     * @return name of this action
     */
    ST_LOCAL const StString& getName() const {
        return myName;
    }

        protected:

    StString     myName;      //!< action name
    unsigned int myHotKey1;   //!< key combination to execute action
    unsigned int myHotKey2;   //!< key combination to execute action (extra)
    bool         myToHoldKey; //!< this action process key hold event

};

template<> inline void StArray< StHandle<StAction> >::sort() {}

/**
 * Reverse boolean parameter value when triggered.
 */
class StActionBool : public StAction {

        public:

    /**
     * Constructor.
     */
    ST_CPPEXPORT StActionBool(const StCString&             theName,
                              const StHandle<StBoolParam>& theParam);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StActionBool();

    /**
     * Execute action.
     */
    ST_CPPEXPORT virtual void doTrigger(const StEvent* );

        protected:

    StHandle<StBoolParam> myParam; //!< boolean parameter

};

// define StHandle template specialization
ST_DEFINE_HANDLE(StActionBool, StAction);

/**
 * Set new integer value when triggered.
 */
class StActionIntValue : public StAction {

        public:

    /**
     * Constructor.
     */
    ST_CPPEXPORT StActionIntValue(const StCString&              theName,
                                  const StHandle<StInt32Param>& theParam,
                                  const int32_t                 theValue);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StActionIntValue();

    /**
     * Execute action.
     */
    ST_CPPEXPORT virtual void doTrigger(const StEvent* );

        protected:

    StHandle<StInt32Param> myParam; //!< integer parameter
    int32_t                myValue; //!< integer value

};

// define StHandle template specialization
ST_DEFINE_HANDLE(StActionIntValue, StAction);

/**
 * Call slot with specified integer value when triggered.
 */
class StActionIntSlot : public StAction {

        public:

    /**
     * Constructor.
     */
    template<typename class_t>
    StActionIntSlot(const StCString& theName,
                    const stSlotPair_t<class_t, typename StSlotMethod<class_t, void (const size_t )>::method_t>& theMethod,
                    const size_t     theValue)
    : StAction(theName),
      myValue(theValue) {
        if(theMethod.ClassPtr  != NULL
        && theMethod.MethodPtr != NULL) {
            mySlot = new StSlotMethod<class_t, void (const size_t )>(theMethod.ClassPtr, theMethod.MethodPtr);
        }
    }

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StActionIntSlot();

    /**
     * Execute action.
     */
    ST_CPPEXPORT virtual void doTrigger(const StEvent* );

        protected:

    StHandle< StSlot<void (const size_t )> > mySlot; //!< slot
    size_t myValue;                                  //!< integer value

};

// define StHandle template specialization
ST_DEFINE_HANDLE(StActionIntSlot, StAction);


/**
 * Call slot with double value (event duration) when triggered.
 */
class StActionHoldSlot : public StAction {

        public:

    /**
     * Constructor.
     */
    template<typename class_t>
    StActionHoldSlot(const StCString& theName,
                     const stSlotPair_t<class_t, typename StSlotMethod<class_t, void (const double )>::method_t>& theMethod)
    : StAction(theName) {
        if(theMethod.ClassPtr  != NULL
        && theMethod.MethodPtr != NULL) {
            mySlot = new StSlotMethod<class_t, void (const double )>(theMethod.ClassPtr, theMethod.MethodPtr);
        }
        myToHoldKey = true;
    }

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StActionHoldSlot();

    /**
     * Execute action.
     */
    ST_CPPEXPORT virtual void doTrigger(const StEvent* );

        protected:

    StHandle< StSlot<void (const double )> > mySlot; //!< slot

};

// define StHandle template specialization
ST_DEFINE_HANDLE(StActionHoldSlot, StAction);


#endif // __StAction_h_
