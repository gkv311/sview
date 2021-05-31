/**
 * Copyright © 2009, 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __stConsole_h__
#define __stConsole_h__

#include <stTypes.h>

#include <iostream>
#include <fstream>
#include <string>

namespace st {
#if(defined(_WIN32) || defined(__WIN32__))
    typedef std::wofstream ofstream;
    typedef std::wostream  ostream;
    static std::wostream& cout = std::wcout;
    static std::wostream& cerr = std::wcerr;
    #define stostream_text(theQuote) L##theQuote
#else
    typedef std::ofstream ofstream;
    typedef std::ostream  ostream;
    static std::ostream& cout = std::cout;
    static std::ostream& cerr = std::cerr;
    #define stostream_text(theQuote) theQuote
#endif

    ST_CPPEXPORT int getch();

    ST_CPPEXPORT st::ostream& SYS_PAUSE_EMPTY   (st::ostream& theOStream);
    ST_CPPEXPORT st::ostream& SYS_PAUSE         (st::ostream& theOStream);

    // console output text-color functions
    ST_CPPEXPORT st::ostream& COLOR_FOR_RED     (st::ostream& theOStream);
    ST_CPPEXPORT st::ostream& COLOR_FOR_GREEN   (st::ostream& theOStream);
    ST_CPPEXPORT st::ostream& COLOR_FOR_YELLOW_L(st::ostream& theOStream);
    ST_CPPEXPORT st::ostream& COLOR_FOR_YELLOW  (st::ostream& theOStream);
    ST_CPPEXPORT st::ostream& COLOR_FOR_BLUE    (st::ostream& theOStream);
    ST_CPPEXPORT st::ostream& COLOR_FOR_WHITE   (st::ostream& theOStream);

};

#endif //__stConsole_h__
