/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StLogger_h__
#define __StLogger_h__

#include "StString.h"
#include <StTemplates/StHandle.h>

#include <typeinfo>

// forward declarations
class StMutexSlim;

/**
 * Logging context identifier.
 */
class StLogContext {

        public:

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StLogContext(const char* theName);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StLogContext();

    /**
     * @return context identifier
     */
    inline const StString& getName() const {
        return myName;
    }

        private:

    StString myName; //!< context identifier

};

/**
 * This class provide logging (to console, to file) functionality.
 */
class StLogger {

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
        ST_TRACE   =  6, //!< message will be shown only when compiled with debugging info
    } Level;

    enum {
        ST_OPT_NONE = 0x00, //!< no options
        ST_OPT_COUT = 0x01, //!< (additionally) write into standard streams std::cerr and std::cout.
        ST_OPT_LOCK = 0x02, //!< use mutex to ensure thread-safety
    };

        public:

    /**
     * Default constructor.
     * @param theLogFile log file name (if empty string - no logging to file)
     * @param theFilter  log level
     */
    ST_CPPEXPORT StLogger(const StString&       theLogFile,
                          const StLogger::Level theFilter  = StLogger::ST_VERBOSE,
                          const int             theOptions = StLogger::ST_OPT_COUT | StLogger::ST_OPT_LOCK);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StLogger();

    inline StLogger::Level getFilter() const {
        return myFilter;
    }

    inline void setFilter(const StLogger::Level theFilter) {
        myFilter = theFilter;
    }

    /**
     * Main logging function.
     * @param theMessage message text
     * @param theLevel   message weight
     * @param theCtx     logging context
     */
    ST_CPPEXPORT virtual void write(const StString&       theMessage,
                                    const StLogger::Level theLevel,
                                    const StLogContext*   theCtx = NULL);

        public:

    /**
     * Retrieve default (global!) instance.
     */
    ST_CPPEXPORT static StLogger& GetDefault();

        private:

    StHandle<StMutexSlim> myMutex;         //!< mutex lock for thread-safety
#ifdef _WIN32
    StStringUtfWide       myFilePath;      //!< file to write into
#else
    StString              myFilePath;      //!< file to write into
#endif
    FILE*                 myFileHandle;    //!< file object
    StLogger::Level       myFilter;        //!< define messages filter
    const bool            myToLogCout;
    const bool            myToLogToSystem; //!< log into system journal, false by default
    const bool            myToLogThreadId; //!< option to prepend thread id to each message, false by default

        private:

    StLogger(const StLogger& theCopy);
    const StLogger& operator=(const StLogger& theOther);

};

class StMessageBox {

        public:

    enum MsgType {
        MsgType_Info,
        MsgType_Warning,
        MsgType_Error,
        MsgType_Question,
    };

        public:

    /**
     * Show INFO popup window.
     */
    ST_CPPEXPORT static void Info(const StString& theMessage);
    ST_CPPEXPORT static void InfoConsole(const StString& theMessage);

    /**
     * Show WARNING popup window.
     */
    ST_CPPEXPORT static void Warn(const StString& theMessage);
    ST_CPPEXPORT static void WarnConsole(const StString& theMessage);

    /**
     * Show ERROR popup window.
     */
    ST_CPPEXPORT static void Error(const StString& theMessage);
    ST_CPPEXPORT static void ErrorConsole(const StString& theMessage);

    ST_CPPEXPORT static bool Question(const StString& theMessage);
    ST_CPPEXPORT static bool QuestionConsole(const StString& theMessage);

        public:

    typedef bool (*msgBoxFunc_t)(MsgType theType, const char* theMessage);

#if defined(__ANDROID__)
    ST_CPPEXPORT static void setCallback(msgBoxFunc_t theFunc);
#elif defined(__linux__)
    ST_LOCAL static bool initGlobals();
#endif

};

inline void stInfo           (const StString& theMsg) {        StMessageBox::Info           (theMsg); }
inline void stInfoConsole    (const StString& theMsg) {        StMessageBox::InfoConsole    (theMsg); }
inline void stWarn           (const StString& theMsg) {        StMessageBox::Warn           (theMsg); }
inline void stWarnConsole    (const StString& theMsg) {        StMessageBox::WarnConsole    (theMsg); }
inline void stError          (const StString& theMsg) {        StMessageBox::Error          (theMsg); }
inline void stErrorConsole   (const StString& theMsg) {        StMessageBox::ErrorConsole   (theMsg); }
inline bool stQuestion       (const StString& theMsg) { return StMessageBox::Question       (theMsg); }
inline bool stQuestionConsole(const StString& theMsg) { return StMessageBox::QuestionConsole(theMsg); }

/**
 * Debugging info output.
 */
#define ST_ERROR_LOG(msg);        StLogger::GetDefault().write(StString() + msg, StLogger::ST_ERROR);

#ifndef ST_DEBUG
    #define ST_DEBUG_VAR(theVariable)
    #define ST_DEBUG_LOG(msg);
    #define ST_DEBUG_LOG_CLASS(theMsg)
    #define ST_DEBUG_LOG_AT(msg);
    #define ST_ERROR_LOG_AT(msg); StLogger::GetDefault().write(StString() + msg, StLogger::ST_ERROR);
#else
    #define ST_DEBUG_VAR(theVariable)  theVariable
    #define ST_DEBUG_LOG(msg);         StLogger::GetDefault().write(StString() + msg, StLogger::ST_TRACE);
    #define ST_DEBUG_LOG_CLASS(theMsg) StLogger::GetDefault().write(StString() + "[" + typeid(*this).name() + "]" + theMsg, StLogger::ST_TRACE);
    #define STRINGIFY(x) #x
    #define TOSTRING(x) STRINGIFY(x)
    #define __AT __FILE__ ":" TOSTRING(__LINE__)
    #define ST_DEBUG_LOG_AT(msg); StLogger::GetDefault().write(StString() + __AT + " " + msg, StLogger::ST_TRACE);
    #define ST_ERROR_LOG_AT(msg); StLogger::GetDefault().write(StString() + __AT + " " + msg, StLogger::ST_ERROR);
#endif

#endif //__StLogger_h__
