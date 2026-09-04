/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StString_h__
#define __StString_h__

#include <StStrings/StStringUnicode.h>

#ifdef _WIN32
    constexpr stUtf8_t SYS_FS_SPLITTER = '\\';
    #define ST_FILE_SPLITTER "\\"
#else
    constexpr stUtf8_t SYS_FS_SPLITTER = '/';
    #define ST_FILE_SPLITTER "/"
#endif


#if defined(__APPLE__)
/**
 * Auxiliary function to convert from UTF8-Mac.
 */
extern StString stFromUtf8Mac(const char* theString);
extern StString stToUtf8Mac  (const char* theString);
#endif

#endif //__StString_h__
