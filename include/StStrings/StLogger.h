/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StLogger_h__
#define __StLogger_h__

#include "StString.h"
#include <StTemplates/StHandle.h>

// forward declarations
class StMutexSlim;

/**
 * This class provide logging (to console, to file) functionality.
 */
class ST_LOCAL StLogger {

        public:

    /**
     * This enumeration defines log levels.
     * Splitting messages into groups allows to apply filter (automatically ignore not important messages).
     */
    typedef enum tagLevel {
        ST_QUIET   = -1, //!< logging is blocked at all
        ST_PANIC   =  0, //!< message before crash
        ST_FATAL   =  1, //!< abnormal call - application may crash
        ST_ERROR   =  2, //!< normal handled error
        ST_WARNING =  3, //!< just warning, not an error
        ST_INFO    =  4, //!< message show just for info
        ST_VERBOSE =  5,
        ST_DEBUG   =  6, //!< message will be shown only when compiled with debugging info
    } Level;

    enum {
        ST_OPT_NONE = 0x00, //!< no options
        ST_OPT_COUT = 0x01, //!< (additionally) write into standard streams std::cerr and std::cout.
        ST_OPT_LOCK = 0x02, //!< use mutex to ensure thread-safety
    };

        private:

    StHandle<StMutexSlim> myMutex; //!< mutex lock for thread-safety
    StString           myModuleId; //!< module name
#if(defined(_WIN32) || defined(__WIN32__))
    StStringUtfWide    myFilePath; //!< file to write into
#else
    StString           myFilePath; //!< file to write into
#endif
    FILE*            myFileHandle; //!< file object
    StLogger::Level      myFilter; //!< define messages filter
    const bool        myToLogCout;

        public:

    /**
     * Default constructor.
     * @param theLogFile (const StString& ) - log file name (if empty string - no logging to file);
     * @param theFilter (const StLogger::Level ) - log level;
     */
    StLogger(const StString&       theLogFile,
             const StLogger::Level theFilter  = StLogger::ST_VERBOSE,
             const int             theOptions = StLogger::ST_OPT_COUT | StLogger::ST_OPT_LOCK);

    /**
     * Destructor.
     */
    virtual ~StLogger();

    StLogger::Level getFilter() const {
        return myFilter;
    }

    void setFilter(const StLogger::Level theFilter) {
        myFilter = theFilter;
    }

    /**
     * Main logging function.
     * @param theMessage (const StString& ) - message text;
     * @param theLevel (const StLogger::Level ) - message weight.
     */
    virtual void write(const StString&       theMessage,
                       const StLogger::Level theLevel);

        public:

    /**
     * Retrieve default (global!) instance.
     */
    static StLogger& GetDefault();

    /**
     * Special method to identify the logger within module name.
     */
    static bool IdentifyModule(const StString& theModuleName);

        private:

    StLogger(const StLogger& theCopy);
    const StLogger& operator=(const StLogger& theOther);

};

/**
 * Show INFO popup window.
 */
ST_LOCAL void stInfo(const StString& theMessage);
ST_LOCAL void stInfoConsole(const StString& theMessage);

/**
 * Show WARNING popup window.
 */
ST_LOCAL void stWarn(const StString& theMessage);
ST_LOCAL void stWarnConsole(const StString& theMessage);

/**
 * Show ERROR popup window.
 */
ST_LOCAL void stError(const StString& theMessage);
ST_LOCAL void stErrorConsole(const StString& theMessage);

/**
 * Show SUCCESS popup window.
 */
ST_LOCAL void stSuccess(const StString& theMessage);
ST_LOCAL void stSuccessConsole(const StString& theMessage);

ST_LOCAL bool stQuestion(const StString& theMessage);
ST_LOCAL bool stQuestionConsole(const StString& theMessage);

/**
 * Debugging info output.
 */
#define ST_ERROR_LOG(msg);        StLogger::GetDefault().write(StString() + msg, StLogger::ST_ERROR);
#ifndef __ST_DEBUG__
    #define ST_DEBUG_LOG(msg);
    #define ST_DEBUG_LOG_AT(msg);
    #define ST_ERROR_LOG_AT(msg); StLogger::GetDefault().write(StString() + msg, StLogger::ST_ERROR);
#else
    #define ST_DEBUG_LOG(msg);    StLogger::GetDefault().write(StString() + msg, StLogger::ST_DEBUG);
    #define STRINGIFY(x) #x
    #define TOSTRING(x) STRINGIFY(x)
    #define __AT __FILE__ ":" TOSTRING(__LINE__)
    #define ST_DEBUG_LOG_AT(msg); StLogger::GetDefault().write(StString() + __AT + " " + msg, StLogger::ST_DEBUG);
    #define ST_ERROR_LOG_AT(msg); StLogger::GetDefault().write(StString() + __AT + " " + msg, StLogger::ST_ERROR);
#endif

#endif //__StLogger_h__
