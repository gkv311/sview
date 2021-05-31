/**
 * Copyright © 2013-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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
     * @return hot key to trigger action
     */
    ST_LOCAL unsigned int getHotKey(int theIndex) const {
        ST_DEBUG_ASSERT(theIndex >= 0 && theIndex <= 1)
        return myHotKeys[theIndex];
    }

    /**
     * @return hot key 1 to trigger action
     */
    ST_LOCAL unsigned int getHotKey1() const { return myHotKeys[0]; }

    /**
     * @return hot key 1 to trigger action
     */
    ST_LOCAL unsigned int& changeHotKey1() { return myHotKeys[0]; }

    /**
     * @param theKey hot key to trigger action
     */
    ST_LOCAL void setHotKey(int theIndex, unsigned int theKey) {
        ST_DEBUG_ASSERT(theIndex >= 0 && theIndex <= 1)
        myHotKeys[theIndex] = theKey;
    }

    /**
     * @param theKey hot key 1 to trigger action
     */
    ST_LOCAL void setHotKey1(unsigned int theKey) { myHotKeys[0] = theKey; }

    /**
     * Default value of hot key 1.
     */
    ST_LOCAL unsigned int getDefaultHotKey1() const { return myHotKeysDef[0]; }

    /**
     * @param theKey default value of hot key 1
     */
    ST_LOCAL void setDefaultHotKey1(unsigned int theKey) {
        myHotKeysDef[0] = theKey;
        myHotKeys[0]    = theKey;
    }

    /**
     * @return hot key 2 to trigger action
     */
    ST_LOCAL unsigned int getHotKey2() const { return myHotKeys[1]; }

    /**
     * @return hot key 2 to trigger action
     */
    ST_LOCAL unsigned int& changeHotKey2() { return myHotKeys[1]; }

    /**
     * @param theKey hot key 2 to trigger action
     */
    ST_LOCAL void setHotKey2(unsigned int theKey) { myHotKeys[1] = theKey; }

    /**
     * Default value of hot key 2.
     */
    ST_LOCAL unsigned int getDefaultHotKey2() const { return myHotKeysDef[1]; }

    /**
     * @param theKey default value of hot key 2
     */
    ST_LOCAL void setDefaultHotKey2(unsigned int theKey) {
        myHotKeysDef[1] = theKey;
        myHotKeys[1]    = theKey;
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

    StString     myName;          //!< action name
    unsigned int myHotKeys[2];    //!< key combination to execute action
    unsigned int myHotKeysDef[2]; //!< default value of hot-key1
    bool         myToHoldKey;     //!< this action process key hold event

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
    ST_CPPEXPORT virtual void doTrigger(const StEvent* ) ST_ATTR_OVERRIDE;

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
    ST_CPPEXPORT virtual void doTrigger(const StEvent* ) ST_ATTR_OVERRIDE;

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
    ST_CPPEXPORT virtual void doTrigger(const StEvent* ) ST_ATTR_OVERRIDE;

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
    ST_CPPEXPORT virtual void doTrigger(const StEvent* ) ST_ATTR_OVERRIDE;

        protected:

    StHandle< StSlot<void (const double )> > mySlot; //!< slot

};

// define StHandle template specialization
ST_DEFINE_HANDLE(StActionHoldSlot, StAction);


#endif // __StAction_h_
