/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StAtomic_h_
#define __StAtomic_h_

#include <StThreads/StAtomicOp.h>

/**
 * Template class implements atomic operations on integers.
 * The target CPU architecture should support such operations
 * on requested type!
 */
template <typename Type>
class StAtomic {

        private: // fields

    volatile Type myValue;

        private: // disabled defaults

    StAtomic(const StAtomic& );
    StAtomic& operator=(const StAtomic& );

        public: // the class API

    StAtomic(Type theValue = 0)
    : myValue(theValue) {}

    Type getValue() const {
        return myValue;
    }

    /**
     * Increment the value with 1.
     * @return incrementation result.
     */
    Type increment() {
        return StAtomicOp::Increment(myValue);
    }

    /**
     * Decrement the value with 1.
     * @return decrementation result.
     */
    Type decrement() {
        return StAtomicOp::Decrement(myValue);
    }

};

#endif // __StAtomic_h_
