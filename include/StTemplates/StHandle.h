/**
 * Copyright © 2010-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StHandle_h_
#define __StHandle_h_

#include <stTypes.h>
#include "StAtomic.h"

/**
 * This template implements Handle class (known as 'smart pointer').
 * Handle contains the pointer to object instance and usage counter.
 * When counter become 0 object will be automatically deleted.
 * Typical usage:
 *   StHandle<SomeClass> aVar = new SomeClass();
 *   aVar->callMethod();
 */
template <class Type>
class StHandle {

        protected:

    /**
     * Helper class which contains object pointer and counter.
     */
    class StPointer {

            public:

        Type*  myPointer;
        StAtomic<ptrdiff_t> myCounter;

            public:

        /**
         * Empty constructor
         */
        inline StPointer()
        : myPointer(NULL),
          myCounter(0) {
            //
        }

        /**
         * Copy constructor does nothing
         */
        inline StPointer(const StPointer& )
        : myPointer(NULL),
          myCounter(0) {
            //
        }

        /**
         * Constructor stores pointer to the object
         */
        inline StPointer(Type* theObj)
        : myPointer(theObj),
          myCounter(0) {
            //
        }

        /**
         * Assignment operator, needed to avoid copying reference counter
         */
        inline StPointer& operator=(const StPointer& ) {
            return *this;
        }

        /**
         * Destructor
         */
        inline ~StPointer() {
            if(myPointer != NULL) {
                delete myPointer;
                myPointer = NULL;
            }
        }

    };

        protected:

    StPointer* myEntity;

        private:

    /**
     * Assignment
     */
    inline void assign(const StPointer* theItem) {
        if(theItem == myEntity) {
            return;
        }
        endScope();
        myEntity = (StPointer* )theItem;
        beginScope();
    }

    /**
     * Increment reference counter of referred object
     */
    inline void beginScope() {
        if(myEntity != NULL) {
            myEntity->myCounter.increment();
        }
    }

    /**
     * Decrement reference counter and if it become 0, destroy referred object
     */
    inline void endScope() {
        if(myEntity == NULL) {
            return;
        }
        if(myEntity->myCounter.decrement() == 0) {
            delete myEntity;
        }
        myEntity = NULL;
    }

        public:

    /**
     * Create null handle
     */
    inline StHandle()
    : myEntity(NULL) {
        //
    }

    /**
     * Constructor of handle from pointer on newly allocated object
     */
    inline StHandle(Type* theObject)
    : myEntity((theObject != NULL) ? new StPointer(theObject) : NULL) {
        beginScope();
    }

    /**
     * Copy constructor
     */
    inline StHandle(const StHandle& theHandle)
    : myEntity(theHandle.myEntity) {
        beginScope();
    }

    /**
     * Destructor
     */
    inline ~StHandle() {
        endScope();
    }

    /**
     * Cast handle to contained type
     */
    inline Type* access() {
        return myEntity->myPointer;
    }

    /**
     * Cast handle to contained type
     */
    inline const Type* access() const {
        return myEntity->myPointer;
    }

    /**
     * Cast handle to contained type
     */
    inline Type* operator->() const {
        return myEntity->myPointer;
    }

    /**
     * Cast handle to contained type
     */
    //inline const Type* operator->() const {
    //    return myEntity->myPointer;
    //}

    /**
     * Cast handle to contained type
     */
    inline Type& operator*() {
        return *myEntity->myPointer;
    }

    /**
     * Cast handle to contained type
     */
    inline const Type& operator*() const {
        return *myEntity->myPointer;
    }

    /**
     * Assignment operator
     */
    inline StHandle& operator=(const StHandle& theHandle) {
        assign(theHandle.myEntity);
        return *this;
    }

    /**
     * Assignment operator
     */
    inline StHandle& operator=(const StPointer* theItem) {
        assign(theItem);
        return *this;
    }

    /**
     * Nullify the handle
     */
    inline void nullify() {
        endScope();
    }

    /**
     * Check for being null
     */
    inline bool isNull() const {
        return myEntity == NULL;
    }

    /**
     * Check for equality
     */
    inline bool operator==(const StHandle& theRight) const {
        return myEntity == theRight.myEntity;
    }

    /**
     * Check for equality
     */
    inline bool operator==(const StPointer* theRightItem) const {
        return myEntity == theRightItem;
    }

    /**
     * Check for equality
     */
    friend inline bool operator==(const StPointer* theLeftItem,
                                  const StHandle&  theRight) {
        return theLeftItem == theRight.myEntity;
    }

    /**
     * Check for inequality
     */
    inline bool operator!=(const StHandle& theRight) const {
        return myEntity != theRight.myEntity;
    }

    /**
     * Check for inequality
     */
    inline bool operator!=(const StPointer* theRightItem) const {
        return myEntity != theRightItem;
    }

    /**
     * Check for inequality
     */
    friend inline bool operator!=(const StPointer* theLeftItem,
                                  const StHandle&  theRight) {
        return theLeftItem != theRight.myEntity;
    }

    /**
     * Set all template concretizations as friends to access private methods.
     */
    template <class OtherType> friend class StHandle;

    /**
     * Downcast operation for handle. Uses standard C++ dynamic cast for stored pointer.
     * You can concretize this method in other way if have own RTTI implementation for your classes.
     * @param theHandle (StHandle<UpperType>& ) - the handle to perform downcast;
     * @return true if downcast success.
     */
    template <class UpperType>
    static inline StHandle downcast(const StHandle<UpperType>& theHandle) {
        StHandle aDownHandle;
        if(theHandle.isNull()) {
            return aDownHandle;
        }
        Type* aTest = dynamic_cast<Type*>(theHandle.myEntity->myPointer);
        if(aTest == NULL) {
            return aDownHandle;
        }
        // this code is hack enough but should work safely
        aDownHandle.assign((StPointer* )theHandle.myEntity);
        return aDownHandle;
    }

    /**
     * Shorter wrapper over static downcast() method.
     * @param theHandle (StHandle<UpperType>& ) - the handle to perform downcast;
     * @return true if downcast success.
     */
    template <class UpperType>
    inline bool downcastFrom(const StHandle<UpperType>& theHandle) {
        *this = downcast(theHandle);
        return !isNull();
    }

};

/**
 * Macros declares StHandle template <i><b>specialization</b></i> with inheritence information.
 * This allows to use same naming convention for both declarations - without inheritence and within.
 * However this definition should be included <i><b>before</b></i> first instantiation.
 * @param TheChildClass - class name for this template;
 * @param TheBaseClass  - class name for the parent (thus should be class TheChildClass : public TheBaseClass).
 */
#define ST_DEFINE_HANDLE(TheChildClass, TheBaseClass) \
template<> class StHandle<TheChildClass> : public StHandle<TheBaseClass> { \
    typedef StHandle<TheBaseClass> TheBaseHandle; \
        public: \
    StHandle()                          : TheBaseHandle() {} \
    StHandle(TheChildClass* theObject)  : TheBaseHandle((TheBaseClass* )theObject) {} \
    StHandle(const StHandle& theHandle) : TheBaseHandle(theHandle) {} \
          TheChildClass* access()           { return  (TheChildClass* )TheBaseHandle::access(); } \
    const TheChildClass* access()     const { return  (TheChildClass* )TheBaseHandle::access(); } \
          TheChildClass* operator->()       { return  (TheChildClass* )TheBaseHandle::access(); } \
    const TheChildClass* operator->() const { return  (TheChildClass* )TheBaseHandle::access(); } \
          TheChildClass& operator*()        { return *(TheChildClass* )TheBaseHandle::access(); } \
    const TheChildClass& operator*()  const { return *(TheChildClass* )TheBaseHandle::access(); } \
    template <class UpperType> \
    static inline StHandle<TheChildClass> downcast(const StHandle<UpperType>& theHandle) { \
        StHandle<TheChildClass> aDownHandle; \
        if(theHandle.isNull()) { \
            return aDownHandle; \
        } \
        TheChildClass* aTest = dynamic_cast<TheChildClass*>(theHandle.myEntity->myPointer); \
        if(aTest == NULL) { \
            return aDownHandle; \
        } \
        aDownHandle.assign((StPointer* )theHandle.myEntity); \
        return aDownHandle; \
    } \
    template <class UpperType> \
    inline bool downcastFrom(const StHandle<UpperType>& theHandle) { \
        *this = downcast(theHandle); \
        return !isNull(); \
    } \
};

/**
 * Macros declares Handle class with inheritence information.
 * This macro should be used <i><b>inside</b></i> class definition
 * Thus result handle should be used as TheChildClass::Handle.
 * In the topmost base class 'typedef StHandle<TheBaseBaseClass> Handle;' should be declared.
 * @param TheChildClass - class name for this class;
 * @param TheBaseClass  - class name for the parent (thus should be class TheChildClass : public TheBaseClass).
 */
#define ST_DEFINE_HANDLE_CLASS(TheChildClass, TheBaseClass) \
class Handle : public TheBaseClass::Handle { \
    typedef TheBaseClass::Handle TheBaseHandle; \
        public: \
    Handle()                         : TheBaseHandle() {} \
    Handle(TheChildClass* theObject) : TheBaseHandle(theObject) {} \
    Handle(const Handle& theHandle)  : TheBaseHandle(theHandle) {} \
          TheChildClass* access()           { return  (TheChildClass* )TheBaseHandle::access(); } \
    const TheChildClass* access()     const { return  (TheChildClass* )TheBaseHandle::access(); } \
          TheChildClass* operator->()       { return  (TheChildClass* )TheBaseHandle::access(); } \
    const TheChildClass* operator->() const { return  (TheChildClass* )TheBaseHandle::access(); } \
          TheChildClass& operator*()        { return *(TheChildClass* )TheBaseHandle::access(); } \
    const TheChildClass& operator*()  const { return *(TheChildClass* )TheBaseHandle::access(); } \
};

#endif // __StHandle_h_
