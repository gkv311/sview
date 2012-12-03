/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StAtomicOp_h_
#define __StAtomicOp_h_

#include <stTypes.h>

#if (defined(_WIN32) || defined(__WIN32__))
    #include <windows.h>
#elif (defined(__APPLE__))
    #include <libkern/OSAtomic.h>
#endif

/**
 * Class provide atomic operations on integer types.
 * The target CPU architecture should support such operations on requested type!
 */
class StAtomicOp {

        public:

    /**
     * Increment the value with 1 and return result.
     * @param theValue (volatile int32_t& ) - input value;
     * @return incremented value.
     */
    static int32_t Increment(volatile int32_t& theValue) {
    #ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
        // g++ compiler
        return __sync_add_and_fetch(&theValue, 1);
    #elif (defined(_WIN32) || defined(__WIN32__))
        return InterlockedIncrement((volatile LONG* )&theValue);
    #elif (defined(__APPLE__))
        return OSAtomicIncrement32Barrier(&theValue);
    #elif defined(__GNUC__)
        #error "Set at least -march=i486 for gcc compiler"
        return ++theValue;
    #else
        #error "Atomic operation doesn't implemented for current platform!"
        return ++theValue;
    #endif
    }

    /**
     * Decrement the value with 1 and return result.
     * @param theValue (volatile int32_t& ) - input value;
     * @return decremented value.
     */
    static int32_t Decrement(volatile int32_t& theValue) {
    #ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
        // g++ compiler
        return __sync_sub_and_fetch(&theValue, 1);
    #elif (defined(_WIN32) || defined(__WIN32__))
        return InterlockedDecrement((volatile LONG* )&theValue);
    #elif (defined(__APPLE__))
        return OSAtomicDecrement32Barrier(&theValue);
    #elif defined(__GNUC__)
        #error "Set at least -march=i486 for gcc compiler"
        return --theValue;
    #else
        #error "Atomic operation doesn't implemented for current platform!"
        return --theValue;
    #endif
    }

    /**
     * Increment the value with 1 and return result.
     * @param theValue (volatile uint32_t& ) - input value;
     * @return incremented value.
     */
    static uint32_t Increment(volatile uint32_t& theValue) {
        return (uint32_t )Increment((volatile int32_t& )theValue);
    }

    /**
     * Decrement the value with 1 and return result.
     * @param theValue (volatile uint32_t& ) - input value;
     * @return decremented value.
     */
    static uint32_t Decrement(volatile uint32_t& theValue) {
        return (uint32_t )Decrement((volatile int32_t& )theValue);
    }

    // int64_t, actually available on win32 too, but since WinNT 5.2 (Windows XP x64)
#if (defined(_WIN64) || defined(__WIN64__))\
 || (defined(_LP64)  || defined(__LP64__))
    /**
     * Increment the value with 1 and return result.
     * @param theValue (volatile int64_t& ) - pointer to the value;
     * @return incremented value.
     */
    static int64_t Increment(volatile int64_t& theValue) {
    #ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8
        // g++ compiler
        return __sync_add_and_fetch(&theValue, 1);
    #elif (defined(_WIN32) || defined(__WIN32__))
        return InterlockedIncrement64(&theValue);
    #elif (defined(__APPLE__))
        return OSAtomicIncrement64Barrier(&theValue);
    #elif defined(__GNUC__)
        #error "Set at least -march=k8 for gcc compiler"
        return ++theValue;
    #else
        #error "Atomic operation doesn't implemented for current platform!"
        return ++theValue;
    #endif
    }

    /**
     * Decrement the value with 1 and return result.
     * @param theValue (volatile int64_t& ) - input value;
     * @return decremented value.
     */
    static int64_t Decrement(volatile int64_t& theValue) {
    #ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8
        // g++ compiler
        return __sync_sub_and_fetch(&theValue, 1);
    #elif (defined(_WIN32) || defined(__WIN32__))
        return InterlockedDecrement64(&theValue);
    #elif (defined(__APPLE__))
        return OSAtomicDecrement64Barrier(&theValue);
    #elif defined(__GNUC__)
        #error "Set at least -march=k8 for gcc compiler"
        return --theValue;
    #else
        #error "Atomic operation doesn't implemented for current platform!"
        return --theValue;
    #endif
    }

    /**
     * Increment the value with 1 and return result.
     * @param theValue (volatile uint64_t& ) - input value;
     * @return incremented value.
     */
    static uint64_t Increment(volatile uint64_t& theValue) {
        return (uint64_t )Increment((volatile int64_t& )theValue);
    }

    /**
     * Decrement the value with 1 and return result.
     * @param theValue (volatile uint64_t& ) - input value;
     * @return decremented value.
     */
    static uint64_t Decrement(volatile uint64_t& theValue) {
        return (uint64_t )Decrement((volatile int64_t& )theValue);
    }

#ifdef ST_HAS_INT64_EXT
    static stInt64ext_t Increment(volatile stInt64ext_t& theValue) {
        return (stInt64ext_t )Increment((volatile int64_t& )theValue);
    }

    static stInt64ext_t Decrement(volatile stInt64ext_t& theValue) {
        return (stInt64ext_t )Decrement((volatile int64_t& )theValue);
    }

    static stUInt64ext_t Increment(volatile stUInt64ext_t& theValue) {
        return (stUInt64ext_t )Increment((volatile int64_t& )theValue);
    }

    static stUInt64ext_t Decrement(volatile stUInt64ext_t& theValue) {
        return (stUInt64ext_t )Decrement((volatile int64_t& )theValue);
    }
#endif

#endif

};

#endif // __StAtomicOp_h_
