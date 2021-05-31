/**
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StAtomicOp_h_
#define __StAtomicOp_h_

#include <stTypes.h>

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__APPLE__)
    #include <libkern/OSAtomic.h>
#elif defined(__ANDROID__)
    //#include <sys/atomics.h>
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
    static inline int32_t Increment(volatile int32_t& theValue) {
    #ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
        // g++ compiler
        return __sync_add_and_fetch(&theValue, 1);
    #elif defined(_WIN32)
        return InterlockedIncrement((volatile LONG* )&theValue);
    #elif defined(__APPLE__)
        return OSAtomicIncrement32Barrier(&theValue);
    //#elif defined(__ANDROID__)
    //    return __atomic_inc(&theValue) + 1; // analog of __sync_fetch_and_add
    #elif defined(__GNUC__)
        #error "Set -march=i486 or -march=armv7-a for gcc compiler"
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
    static inline int32_t Decrement(volatile int32_t& theValue) {
    #ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
        // g++ compiler
        return __sync_sub_and_fetch(&theValue, 1);
    #elif defined(_WIN32)
        return InterlockedDecrement((volatile LONG* )&theValue);
    #elif defined(__APPLE__)
        return OSAtomicDecrement32Barrier(&theValue);
    //#elif defined(__ANDROID__)
    //    return __atomic_dec(&theValue) - 1; // analog of __sync_fetch_and_sub
    #elif defined(__GNUC__)
        #error "Set -march=i486 or -march=armv7-a for gcc compiler"
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
    static inline uint32_t Increment(volatile uint32_t& theValue) {
        return (uint32_t )Increment((volatile int32_t& )theValue);
    }

    /**
     * Decrement the value with 1 and return result.
     * @param theValue (volatile uint32_t& ) - input value;
     * @return decremented value.
     */
    static inline uint32_t Decrement(volatile uint32_t& theValue) {
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
    static inline int64_t Increment(volatile int64_t& theValue) {
    #ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8
        // g++ compiler
        return __sync_add_and_fetch(&theValue, 1);
    #elif defined(_WIN32)
        return InterlockedIncrement64(&theValue);
    #elif defined(__APPLE__)
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
    static inline int64_t Decrement(volatile int64_t& theValue) {
    #ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8
        // g++ compiler
        return __sync_sub_and_fetch(&theValue, 1);
    #elif defined(_WIN32)
        return InterlockedDecrement64(&theValue);
    #elif defined(__APPLE__)
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
    static inline uint64_t Increment(volatile uint64_t& theValue) {
        return (uint64_t )Increment((volatile int64_t& )theValue);
    }

    /**
     * Decrement the value with 1 and return result.
     * @param theValue (volatile uint64_t& ) - input value;
     * @return decremented value.
     */
    static inline uint64_t Decrement(volatile uint64_t& theValue) {
        return (uint64_t )Decrement((volatile int64_t& )theValue);
    }

#ifdef ST_HAS_INT64_EXT
    static inline stInt64ext_t Increment(volatile stInt64ext_t& theValue) {
        return (stInt64ext_t )Increment((volatile int64_t& )theValue);
    }

    static inline stInt64ext_t Decrement(volatile stInt64ext_t& theValue) {
        return (stInt64ext_t )Decrement((volatile int64_t& )theValue);
    }

    static inline stUInt64ext_t Increment(volatile stUInt64ext_t& theValue) {
        return (stUInt64ext_t )Increment((volatile int64_t& )theValue);
    }

    static inline stUInt64ext_t Decrement(volatile stUInt64ext_t& theValue) {
        return (stUInt64ext_t )Decrement((volatile int64_t& )theValue);
    }
#endif

#endif

};

#endif // __StAtomicOp_h_
