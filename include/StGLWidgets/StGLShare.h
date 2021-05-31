/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLShare_h_
#define __StGLShare_h_

#include <StTemplates/StAtomic.h>
#include <StGL/StGLContext.h>

/**
 * Helper struct which contains object pointer and reference counter.
 * We use atomic variable for counter however this is redundant because
 * shared GL objects should be destructed in GL context and that means
 * in ONE thread in most cases.
 */
class StGLSharePointer {

        public:

    /**
     * Empty constructor
     */
    StGLSharePointer()
    : myPointer(NULL),
      myCounter(0) {
        //
    }

        public:

    void*                 myPointer;
    StAtomic<ptrdiff_t>   myCounter;
    StHandle<StGLContext> myCtx;

};

/**
 * This is template for shared object handle within one GL context (or shared GL contexts).
 * This is useful for similar objects rendered in scene that uses common
 * GLSL shader program, texture or font.
 * Sharing allows to save memory (and video memory!) and initialization time.
 * Notice that this template is similar to StHandle (but with some important differencies)
 * and actually should be StHandle when shared objects will inherit from one base class
 * (to provide common virtual destructor).
 */
template <class Type>
class StGLShare {

        public:

    /**
     * Main constructor
     */
    StGLShare(StGLSharePointer* theItem)
    : myEntity(theItem) {
        myEntity->myCounter.increment();
    }

    /**
     * Destructor
     */
    ~StGLShare() {
        if(myEntity->myCounter.decrement() == 0) {
            Type* aPtr = (Type* )myEntity->myPointer;
            if(aPtr != NULL) {
                if(!myEntity->myCtx.isNull()) {
                    aPtr->release(*myEntity->myCtx);
                }
                delete aPtr;
            }
            myEntity->myPointer = NULL;
            myEntity->myCtx.nullify();
        }
    }

    /**
     * Create (assign) new instance
     */
    void create(const StHandle<StGLContext>& theCtx,
                Type*                        theValue) {
        myEntity->myPointer = theValue;
        myEntity->myCtx     = theCtx;
    }

    void setContext(const StHandle<StGLContext>& theCtx) {
        myEntity->myCtx = theCtx;
    }

    /**
     * Check for being null
     */
    bool isNull() const
    {
        return myEntity->myPointer == NULL;
    }

    /**
     * Cast handle to contained type
     */
    Type* operator->() {
        return (Type* )myEntity->myPointer;
    }

    /**
     * Cast handle to contained type
     */
    const Type* operator->() const {
        return (Type* )myEntity->myPointer;
    }

    /**
     * Cast handle to contained type
     */
    Type& operator*() {
        return *((Type* )myEntity->myPointer);
    }

    /**
     * Cast handle to contained type
     */
    const Type& operator*() const {
        return *((Type* )myEntity->myPointer);
    }

        private:

    StGLSharePointer*     myEntity;

};

#endif //__StGLShare_h_
