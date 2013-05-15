/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StString_h__
#define __StString_h__

#include <stTypes.h>

#ifdef _WIN32
    static const stUtf8_t SYS_FS_SPLITTER = '\\';
#else
    static const stUtf8_t SYS_FS_SPLITTER = '/';
#endif

#ifndef __ST_DEBUG__
    #define ST_DEBUG_ASSERT(expression);
#else
    #include <assert.h>
    #define ST_DEBUG_ASSERT(expression); assert(expression);
#endif

// template used in StString class method, so - we declare it
// before template implementation #include
template<typename Element_Type>
class StArrayList;

#include <StStrings/StStringUnicode.h>
typedef StStringUtf8  StString;  // dynamically allocated string class
typedef StCStringUtf8 StCString; // POD structure for constant string

#if (defined(__APPLE__))
/**
 * Auxiliary function to convert from UTF8-Mac.
 */
extern StString stFromUtf8Mac(const char* theString);
extern StString stToUtf8Mac  (const char* theString);
#endif

#endif //__StString_h__
