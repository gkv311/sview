/**
 * Copyright © 2009-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __stTypes_h_
#define __stTypes_h_

#ifdef ST_HAVE_STCONFIG
    #include <stconfig.conf>
#endif

/**
 * Below are compiler-dependent useful macros to warn
 * when some parts of code are used in wrong way.
 * Note that to enable annotations using MSVC you should add compiler option /analyze.
 *
 * Available attributes:
 *  - ST_ATTR_CHECK_RETURN. To warn if a caller of the function does not use its return value.
 *    Notice that this works only with primitive types (useless for returned structure).
 *    Usage:
 *      ST_ATTR_CHECK_RETURN int doInit() { return 0; }
 *      doInit(); // warning will appear
 */
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    //#define _USE_ATTRIBUTES_FOR_SAL 1 // doesn't work without /analyze option!
    #if (_MSC_VER < 1900)
      #include <CodeAnalysis/SourceAnnotations.h>
    #endif
    //#define ST_ATTR_CHECK_RETURN [returnvalue:vc_attributes::Post(MustCheck=vc_attributes::Yes)]
    //#include <sal.h>
    #define ST_ATTR_CHECK_RETURN _Check_return_
#elif defined(__GNUC__)
    #define ST_ATTR_CHECK_RETURN __attribute__((warn_unused_result))
#else
    #define ST_ATTR_CHECK_RETURN
#endif

#if defined(_MSC_VER)
    #define ST_ATTR_DEPRECATED __declspec(deprecated)
#elif defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
    #define ST_ATTR_DEPRECATED __attribute__((deprecated))
#else
    #define ST_ATTR_DEPRECATED
#endif

// Disable deprecation warnings.
#if defined(__ICL) || defined (__INTEL_COMPILER)
    #define ST_DISABLE_DEPRECATION_WARNINGS __pragma(warning(push)) __pragma(warning(disable:1478))
    #define ST_ENABLE_DEPRECATION_WARNINGS  __pragma(warning(pop))
#elif defined(_MSC_VER)
    #define ST_DISABLE_DEPRECATION_WARNINGS __pragma(warning(push)) __pragma(warning(disable:4996))
    #define ST_ENABLE_DEPRECATION_WARNINGS  __pragma(warning(pop))
#elif (defined(__GNUC__) && __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) || defined(__clang__)
    // available since at least gcc 4.2 (maybe earlier), however only gcc 4.6+ supports this pragma inside the function body
    // CLang also supports this gcc syntax (in addition to "clang diagnostic ignored")
    #define ST_DISABLE_DEPRECATION_WARNINGS _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
    #define ST_ENABLE_DEPRECATION_WARNINGS  _Pragma("GCC diagnostic warning \"-Wdeprecated-declarations\"")
#else
    #define ST_DISABLE_DEPRECATION_WARNINGS
    #define ST_ENABLE_DEPRECATION_WARNINGS
#endif

#if defined(__cplusplus) && (__cplusplus >= 201100L)
    // part of C++11 standard
    #define ST_ATTR_OVERRIDE override
#elif defined(_MSC_VER) && (_MSC_VER >= 1700)
    // versions before VS2012 emits warning as MSVC-specific extension
    #define ST_ATTR_OVERRIDE override
#else
    #define ST_ATTR_OVERRIDE
#endif

#if defined(_MSC_VER)
    // M_PI on old MSVC
    #define _USE_MATH_DEFINES
#endif

#include <cmath>       // fabs
#include <cfloat>
#include <cstddef>     // size_t, NULL
#include <cstdlib>
#include <cstring>     // for memcpy

#if(defined(_MSC_VER) && (_MSC_VER < 1800))
    // only Visual Studio 2013+ (vc12) provides <cinttypes> header
    #define PRId64 "I64d"
    #define PRIu64 "I64u"
    #define SCNd64 "I64d"
    #define SCNu64 "I64u"
    #ifdef _WIN64
        #define PRIdPTR "I64d"
        #define PRIuPTR "I64u"
        #define SCNdPTR "I64d"
        #define SCNuPTR "I64u"
    #else
        #define PRIdPTR "d"
        #define PRIuPTR "u"
        #define SCNdPTR "d"
        #define SCNuPTR "u"
    #endif
#else
    // use <inttypes.h< (C99) instead of <cinttypes> (C++11) for compatibility
    #ifndef __STDC_FORMAT_MACROS
        #define __STDC_FORMAT_MACROS
    #endif
    #include <inttypes.h>
#endif

#if defined(__i386) || defined(__x86_64) || defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64)
    #include <xmmintrin.h> // for memory alignment
#endif
#include <stdio.h>     // for _snprintf on MinGW

/**
 * Define the default alignment boundary.
 * We define it to 16 bytes (needed for SSE instructions).
 */
#ifndef ST_ALIGNMENT
    #define ST_ALIGNMENT 16
#endif

/**
 * Macro for function arguments not used in function body.
 */
#define ST_UNUSED(theParam)

// activate some C99 macros like UINT64_C in "stdint.h" which used by FFmpeg
#ifndef __STDC_CONSTANT_MACROS
    #define __STDC_CONSTANT_MACROS
#endif

// int16_t/int32_t and so on
#if defined(_MSC_VER) && (_MSC_VER > 0 && _MSC_VER < 1600)
    // old MSVC - hasn't stdint header
    #include <sysForVC/stdint.h>
#else
    #include <stdint.h>
#endif
//#include <cstdint> // C++0x

#ifdef __cplusplus
    typedef bool stBool_t;
    static const stBool_t ST_TRUE  = true;
    static const stBool_t ST_FALSE = false;
#else
    // C-consistent bool type (C doesn't have true 'bool' type)
    typedef unsigned char stBool_t;
    static const stBool_t ST_TRUE  = (stBool_t )1;
    static const stBool_t ST_FALSE = (stBool_t )0;
#endif

// some primitives
typedef signed   int  stInt_t;
typedef unsigned int  stUInt_t;
typedef signed   char stByte_t;
typedef unsigned char stUByte_t;

// fixed-size primitives
typedef int32_t  stInt16_t;
typedef uint32_t stUInt16_t;
typedef int32_t  stInt32_t;
typedef uint32_t stUInt32_t;
typedef int64_t  stInt64_t;
typedef uint64_t stUInt64_t;

#if defined(__cplusplus) && (__cplusplus >= 201100L)
    //
#elif defined(__GNUC__) && (__GNUC__ < 4 || (__GNUC__ == 4 && __GNUC_MINOR__ < 6))
    // compatibility with old compilers
    #ifndef nullptr
        #define nullptr (void*)0
    #endif
#endif

/**
 * Modern compilers provide fixed-size primitives like int32_t.
 * Some developers can use these types in function overloads
 * to logically cover all variations depending on primitive size.
 * However some language primitives can recall to the same
 * absolute type but distinct by compiler.
 * For example int and long int both are 32-bit integers on ILP32.
 * Defines below could be used to extend overloaded functions list
 * to support both reincarnations available in compiler when
 * only bitness distinctness is needed.
 * Notice that language can provide more similar types (like char32_t).
 */
#if (defined(_LP64) || defined(__LP64__))
    #define ST_HAS_INT64_EXT
    #if (defined(__APPLE__))
        typedef signed   long stInt64ext_t;
        typedef unsigned long stUInt64ext_t;
    #else
        typedef signed   long long stInt64ext_t;
        typedef unsigned long long stUInt64ext_t;
    #endif
#else
    #define ST_HAS_INT32_EXT
    typedef signed   long stInt32ext_t;
    typedef unsigned long stUInt32ext_t;
#endif

/**
 * In the Windows API, the maximum length for a path is MAX_PATH, which is defined as 260 characters.
 * A local path is structured in the following order: drive letter, colon, backslash, components separated by backslashes,
 * and a terminating null character. For example, the maximum path on drive D is "D:\<some 256 character path string><NUL>"
 * where "<NUL>" represents the invisible terminating null character for the current system codepage.
 * (The characters < > are used here for visual clarity and cannot be part of a valid path string.)
 * The Windows API has many functions that also have Unicode versions to permit an extended-length path for a maximum total path length
 * of 32,767 characters. This type of path is composed of components separated by backslashes, each up to the value returned
 * in the lpMaximumComponentLength parameter of the GetVolumeInformation function (this value is commonly 255 characters).
 * To specify an extended-length path, use the "\\?\" prefix. For example, "\\?\D:\<very long path>". (The characters < > are used here
 * for visual clarity and cannot be part of a valid path string.)
 * Note: The maximum path of 32,767 characters is approximate, because the "\\?\" prefix may be expanded to a longer string by the system
 * at run time, and this expansion applies to the total length.
 **/
#define ST_MAX_PATH 4096

/**
 * Unicode primitives.
 */
typedef char          stUtf8_t;     //!< signed   UTF-8 char is just a byte
typedef unsigned char stUtf8u_t;    //!< unsigned UTF-8 char is just a byte
#if (defined(__cplusplus) && (__cplusplus >= 201100L)) \
 || (defined(_MSC_VER) && (_MSC_VER >= 1600)) \
 || (!defined(__GNUC__) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 2)) || (__GNUC__ > 4))
    typedef char16_t  stUtf16_t;    //!< UTF-16 char (always unsigned)
    typedef char32_t  stUtf32_t;    //!< UTF-32 char (always unsigned)
#else // obsolete compilers compatibility
    typedef uint16_t  stUtf16_t;    //!< UTF-16 char (always unsigned)
    typedef uint32_t  stUtf32_t;    //!< UTF-32 char (always unsigned)
#endif
typedef wchar_t       stUtfWide_t;  //!< wide char (unsigned UTF-16 on Windows platform and signed UTF-32 on Linux)

/**
 * Let len be the length of the formatted data string (not including the terminating null). len and count are in bytes for _snprintf, wide characters for _snwprintf.
 * If len < count, then len characters are stored in buffer, a null-terminator is appended, and len is returned.
 * If len = count, then len characters are stored in buffer, no null-terminator is appended, and len is returned.
 * If len > count, then count characters are stored in buffer, no null-terminator is appended, and a negative value is returned.
 * @param argBuffer (char* ) - string buffer;
 * @param argCount (size_t ) - maximum number of characters to store;
 * @param argFormat (const char* ) - string format;
 * @param ... - arguments to format;
 * @return len (int ).
 */
#ifdef _WIN32
    #define stsprintf(argBuffer, argCount, argFormat, ...); _snprintf(argBuffer, argCount, argFormat, __VA_ARGS__);
#else
    #define stsprintf(argBuffer, argCount, argFormat, ...);  snprintf(argBuffer, argCount, argFormat, __VA_ARGS__);
#endif

/**
 * Define the big-endian flag. If your compiler doesn't provide identification defines
 * - use stconfig.conf to override this. This macro is really important for some functions
 * to work correctly on big-endian CPUs (or bi-endian CPUs with big-endian enabled in OS)!
 */
#ifndef ST_HAVE_BIGENDIAN
    #if (defined(BYTE_ORDER) && BYTE_ORDER==BIG_ENDIAN) || \
        (defined(__BYTE_ORDER) && __BYTE_ORDER==__BIG_ENDIAN) || \
        defined(__BIG_ENDIAN__)
        #define ST_HAVE_BIGENDIAN
    #endif
#endif

/**
 * OpenGL types, just typedef them here (be sure them always in sync - check GLEW headers after update!)
 */
typedef unsigned int  GLenum;
typedef unsigned int  GLuint;
typedef int           GLint;
typedef int           GLsizei;
typedef unsigned char GLboolean;
typedef signed char   GLbyte;
typedef unsigned char GLubyte;
typedef float         GLfloat;
typedef double        GLdouble;
typedef double        GLclampd;
#if defined(__arm__) && defined(__ANDROID__)
typedef signed long int GLintptr;
typedef signed long int GLsizeiptr;
#else
typedef ptrdiff_t     GLintptr;
typedef ptrdiff_t     GLsizeiptr;
#endif

/**
 * Dummy structure definition.
 */
typedef struct stNoType {} stNoType;

/**
 * @param value1 (const GLfloat ) - first value for equal test;
 * @param value2 (const GLfloat ) - second value for equal test;
 * @param tolerance (const GLfloat ) - precision for equal test;
 * @return true if test values equal with given precision.
 */
inline bool stAreEqual(const GLfloat value1, const GLfloat value2,
                       const GLfloat tolerance) {
    return (std::abs(value1 - value2) <= tolerance);
}

inline bool stAreEqual(const void* theBuffer1, const void* theBuffer2,
                       size_t theBytes) {
    if(theBytes % 4 == 0) {
        theBytes /= 4;
        int32_t* aBuffer1 = (int32_t* )theBuffer1;
        int32_t* aBuffer2 = (int32_t* )theBuffer2;
        for(; theBytes != 0; --theBytes, ++aBuffer1, ++aBuffer2) {
            if(*aBuffer1 != *aBuffer2) {
                return false;
            }
        }
    } else {
        stUByte_t* aBuffer1 = (stUByte_t* )theBuffer1;
        stUByte_t* aBuffer2 = (stUByte_t* )theBuffer2;
        for(; theBytes != 0; --theBytes, ++aBuffer1, ++aBuffer2) {
            if(*aBuffer1 != *aBuffer2) {
                return false;
            }
        }
    }
    return true;
}

/**
 * Copies the values from the location pointed by source directly to the memory block pointed by destination.
 * @param theDestinationPtr - pointer to the destination array where the content is to be copied, type-casted to a pointer of type void*;
 * @param theSourcePtr      - pointer to the source of data to be copied, type-casted to a pointer of type void*;
 * @param theBytesCount     - number of bytes to copy;
 * @return theDestinationPtr.
 */
inline void* stMemCpy(void*        theDestinationPtr,
                      const void*  theSourcePtr,
                      const size_t theBytesCount) {
    return std::memcpy(theDestinationPtr, theSourcePtr, theBytesCount);
}

/**
 * Sets the first bytes of the block of memory pointed by thePtr to the specified value (interpreted as an unsigned char).
 * @param thePtr        - pointer to the block of memory to fill;
 * @param theValue      - value to be set, the value is passed as an int, but the function fills the block of memory using the unsigned char conversion of this value;
 * @param theBytesCount - number of bytes to be set to the value;
 * @return thePtr.
 */
inline void* stMemSet(void*        thePtr,
                      int          theValue,
                      const size_t theBytesCount) {
    return std::memset(thePtr, theValue, theBytesCount);
}

/**
 * Sets the first bytes of the block of memory pointed by thePtr to 0.
 * @param thePtr        - pointer to the block of memory to fill with zeros;
 * @param theBytesCount - number of bytes to be set to the value;
 * @return thePtr.
 */
inline void* stMemZero(void*        thePtr,
                       const size_t theBytesCount) {
    return stMemSet(thePtr, 0, theBytesCount);
}

/**
 * Allocates a block of 'theBytesCount' bytes of memory, returning a pointer to the beginning of the block.
 * You should deallocate memory with stMemFree().
 * @param theBytesCount - size of the memory block, in bytes;
 * @return on success, a pointer to the memory block allocated by the function; null pointer otherwise.
 */
inline void* stMemAlloc(const size_t theBytesCount) {
    return std::malloc(theBytesCount);
}

inline void* stMemRealloc(void*        thePtr,
                          const size_t theBytesCount) {
    return std::realloc(thePtr, theBytesCount);
}

/**
 * Deallocate space in memory.
 * @param ptr (void* ) - pointer to a memory block previously allocated with stMemAlloc() to be deallocated.
 */
typedef void (*stMemFree_t)(void* );
inline void stMemFree(void* ptr) {
    return std::free(ptr);
}

/**
 * Allocates an ALIGNED block in memory.
 * For SSE commands alignment should be 16.
 * You should use stMemFreeAligned() to deallocate memory.
 * @param theNbBytes size of the memory block, in bytes
 * @param theAlign   alignment constraint, must be a power of two
 * @return on success, a pointer to the memory block allocated by the function; null pointer otherwise
 */
inline void* stMemAllocAligned(const size_t& theNbBytes,
                               const size_t& theAlign = ST_ALIGNMENT) {
#if defined(_MSC_VER)
    return _aligned_malloc(theNbBytes, theAlign);
#elif defined(__ANDROID__) || defined(__QNX__)
    return memalign(theAlign, theNbBytes);
#elif (defined(__GNUC__) && __GNUC__ >= 4 && __GNUC_MINOR__ >= 1 && (defined(__i386) || defined(__x86_64)))
    return _mm_malloc(theNbBytes, theAlign);
#else
    void* aPtr;
    if(posix_memalign(&aPtr, theAlign, theNbBytes)) {
        return NULL;
    }
    return aPtr;
#endif
}

/**
 * Same as stMemAllocAligned() but fill the allocated block with zeros;
 */
inline void* stMemAllocZeroAligned(const size_t& theNbBytes,
                                   const size_t& theAlign = ST_ALIGNMENT) {
    void* aPtr = stMemAllocAligned(theNbBytes, theAlign);
    if(aPtr != NULL) {
        stMemSet(aPtr, 0, theNbBytes);
    }
    return aPtr;
}

/**
 * Deallocate space in memory.
 * @param ptr (void* ) - pointer to a memory block previously allocated with stMemAllocAligned() to be deallocated.
 */
inline void stMemFreeAligned(void* thePtrAligned) {
#if defined(_MSC_VER)
    _aligned_free(thePtrAligned);
#elif defined(__ANDROID__) || defined(__QNX__)
    free(thePtrAligned);
#elif (defined(__GNUC__) && __GNUC__ >= 4 && __GNUC_MINOR__ >= 1 && (defined(__i386) || defined(__x86_64)))
    _mm_free(thePtrAligned);
#else
    free(thePtrAligned);
#endif
}

/**
 * Auxiliary method to compute nearest integer number.
 */
inline int stRound(const float theNumber) {
    return theNumber >= 0.0f
         ? (int )std::floor(double(theNumber) + 0.5)
         : (int )std::ceil (double(theNumber) + 0.5);
}

// namespaces in export-headers
#ifdef __cplusplus
    #define ST_NAMESPACE_START(name) namespace name {
    #define ST_NAMESPACE_END(name) }
#else
    #define ST_NAMESPACE_START(name)
    #define ST_NAMESPACE_END(name)
#endif

// Add EXPORT or IMPORT prefix before every exported function
// C-style used
#ifdef __cplusplus
    #define EXTERN_C_PREFIX extern "C"
#else
    #define EXTERN_C_PREFIX
#endif

/**
 * Set of export macro definitions:
 *  - ST_LOCAL     Do not export symbol
 *  - ST_CEXPORT   Export symbol, C-style (prevent name decorations)
 *  - ST_CIMPORT   Import symbol, C-style
 *  - ST_CPPEXPORT Export symbol, C++-style (with name decorations)
 *  - ST_CPPIMPORT Import symbol, C++-style
 *  - ST_EXPORT    Deprecated macro, alias to ST_CEXPORT
 *  - ST_IMPORT    Deprecated macro, alias to ST_CIXPORT
 *
 * Notice that function/class definition without extra specifiers
 * leads to different behaviour on different platforms!
 *  - On Windows, symbol considered for local usage (not exported)
 *  - On Linux (and others), symbol considered for export
 *
 * On Windows exported and imported symbols should be marked with different specifiers
 * which leads to extra complications in library headers
 * (library should be build with export specifier whilst
 * application based on it should see import specifier).
 * However C++ method could be marked as "exported" in both cases.
 */
#ifdef _WIN32
    #define ST_LOCAL
    #define ST_CPPEXPORT __declspec(dllexport)
    #define ST_CPPIMPORT __declspec(dllimport)
    #define ST_CEXPORT   EXTERN_C_PREFIX __declspec(dllexport)
    #define ST_CIMPORT   EXTERN_C_PREFIX __declspec(dllimport)
#elif(__GNUC__ >= 4)
    #define ST_LOCAL     __attribute__ ((visibility("hidden")))
    #define ST_CPPEXPORT __attribute__ ((visibility("default")))
    #define ST_CPPIMPORT __attribute__ ((visibility("default")))
    #define ST_CEXPORT   EXTERN_C_PREFIX __attribute__ ((visibility("default")))
    #define ST_CIMPORT   EXTERN_C_PREFIX __attribute__ ((visibility("default")))
#else
    #define ST_CPPEXPORT
    #define ST_CPPIMPORT
    #define ST_CEXPORT EXTERN_C_PREFIX
    #define ST_CIMPORT EXTERN_C_PREFIX
    #define ST_LOCAL
#endif
#define ST_EXPORT ST_CEXPORT
#define ST_IMPORT ST_CIMPORT

/**
 * Returns nearest (greater or equal) aligned number.
 */
inline size_t getAligned(const size_t theNumber, const size_t theAlignment = ST_ALIGNMENT) {
    return theNumber + theAlignment - 1 - (theNumber - 1) % theAlignment;
}

/**
 * Check number for alignment.
 */
inline bool isAligned(const size_t theNumber, const size_t theAlignment = ST_ALIGNMENT) {
    return (theNumber % theAlignment) == 0;
}

/**
 * Simple function for getting power of to number
 * larger or equal to input number.
 * @param inNumber (const size_t& ) - number to 'power of two';
 * @param threshold (const size_t& ) - upper threshold;
 * @return (size_t ) power of two number.
 */
inline size_t getPowerOfTwo(const size_t inNumber, const size_t threshold) {
    for(size_t p2 = 2; p2 <= threshold; p2 <<= 1) {
        if(inNumber <= p2) {
            return p2;
        }
    }
    return threshold;
}

inline GLsizei getPowerOfTwo(const GLsizei inNumber, const GLsizei threshold) {
    for(GLsizei p2 = 2; p2 <= threshold; p2 <<= 1) {
        if(inNumber <= p2) {
            return p2;
        }
    }
    return threshold;
}

inline bool isOddNumber(const int64_t theNumber) {
    return theNumber & 0x01;
}

inline bool isEvenNumber(const int64_t theNumber) {
    return !(isOddNumber(theNumber));
}

inline bool isOddNumber(const size_t number) {
    return number & 0x01;
}

inline bool isEvenNumber(const size_t number) {
    return !(isOddNumber(number));
}

/**
 * Simple function for getting even number larger then input number.
 * @param number (const size_t ) - number to even;
 * @return (size_t ) even number.
 */
inline size_t getEvenNumber(const size_t number) {
    return isOddNumber(number) ? (number + 1) : number;
}

inline bool isOddNumber(const int number) {
    return number & 0x01;
}

inline bool isEvenNumber(const int number) {
    return !(isOddNumber(number));
}

inline int getEvenNumber(const int number) {
    return isOddNumber(number) ? (number + 1) : number;
}

namespace st {
    /**
     * Return true for NaN.
     */
    inline bool isNaN(double theValue) {
    #if defined(_MSC_VER)
        return ::_isnan(theValue) != 0;
    #else
        return std::isnan(theValue);
    #endif
    }
}

#include <StTemplates/StTemplates.h> // include commonly-used templates

/**
 * Convert degrees to radians.
 */
template<typename Type>
inline Type stToRadians(const Type theDegrees) {
    return Type(3.14159265358979323846) * theDegrees / Type(180.0);
}

/**
 * Convert radians to degrees.
 */
template<typename Type>
inline Type stToDegrees(const Type theRadians) {
    return Type(180.0) * theRadians / Type(3.14159265358979323846);
}

#endif //__stTypes_h_
