/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StStrings/StLogger.h>

#include <StStrings/stConsole.h>
#include <StThreads/StMutexSlim.h>
#include <StThreads/StProcess.h>
#include <StThreads/StThread.h>

#if defined(__ANDROID__)
    #include <android/log.h>
#endif

// we do not use st::cerr here to avoid
// global static variables initialization ambiguity
#ifdef _WIN32
    // Unicode version
    #define ST_LOG_CERR std::wcerr
#else
    #define ST_LOG_CERR std::cerr
#endif

StLogger& StLogger::GetDefault() {
    // global instance
    static StLogger THE_DEFAULT_LOGGER(
    #if defined(ST_DEBUG_LOG_TO_FILE) && defined(ST_DEBUG)
        StString(ST_DEBUG_LOG_TO_FILE),
    #else
        StString(),
    #endif
    #ifdef ST_DEBUG
        StLogger::ST_TRACE,
    #else
        StLogger::ST_VERBOSE,
    #endif
        StLogger::ST_OPT_COUT | StLogger::ST_OPT_LOCK
    );
    return THE_DEFAULT_LOGGER;
}

StLogContext::StLogContext(const char* theName)
: myName(theName) {
    ST_DEBUG_LOG("  ==  Process " + StProcess::getProcessName()
               + " (" + StProcess::getPID() + "), module " + myName
               + " Loaded  ==");
}

StLogContext::~StLogContext() {
    ST_DEBUG_LOG("  ==  Process " + StProcess::getProcessName()
                   + " (" + StProcess::getPID() + "), module " + myName
                   + " Unloaded  ==");
}

StLogger::StLogger(const StString&       theLogFile,
                   const StLogger::Level theFilter,
                   const int             theOptions)
: myMutex((theOptions & StLogger::ST_OPT_LOCK) ? new StMutexSlim() : (StMutexSlim* )NULL),
#ifdef _WIN32
  myFilePath(theLogFile.toUtfWide()),
#else
  myFilePath(theLogFile),
#endif
  myFileHandle(NULL),
  myFilter(theFilter),
  myToLogCout(theOptions & StLogger::ST_OPT_COUT),
#ifdef ST_DEBUG_SYSLOG
  myToLogToSystem(true),
#else
  myToLogToSystem(false),
#endif
#ifdef ST_DEBUG_THREADID
  myToLogThreadId(true)
#else
  myToLogThreadId(false)
#endif
{
    //
}

StLogger::~StLogger() {
    //
}

void StLogger::write(const StString&       theMessage,
                     const StLogger::Level theLevel,
                     const StLogContext*   ) {
    if(theLevel > myFilter || theMessage.isEmpty()) {
        // just ignore
        return;
    }

    // lock for safety
    if(!myMutex.isNull()) {
        myMutex->lock();
    }

    // log to the file
    if(!myFilePath.isEmpty()) {
    #ifdef _WIN32
        myFileHandle = _wfopen(myFilePath.toCString(), L"ab");
    #else
        myFileHandle =   fopen(myFilePath.toCString(),  "ab");
    #endif
        if(myFileHandle != NULL) {
            switch(theLevel) {
                case ST_PANIC:   fwrite("PANIC !! ", 1, 9, myFileHandle); break;
                case ST_FATAL:   fwrite("FATAL !! ", 1, 9, myFileHandle); break;
                case ST_ERROR:   fwrite("ERROR !! ", 1, 9, myFileHandle); break;
                case ST_WARNING: fwrite("WARN  -- ", 1, 9, myFileHandle); break;
                case ST_INFO:
                case ST_VERBOSE: fwrite("INFO  -- ", 1, 9, myFileHandle); break;
                case ST_TRACE:   fwrite("TRACE -- ", 1, 9, myFileHandle); break;
                case ST_QUIET: break;
            }
            if(myToLogThreadId) {
                const size_t   aThreadId  = StThread::getCurrentThreadId();
                const StString aThreadStr = StString("[") + aThreadId + "]";
                fwrite(aThreadStr.toCString(), 1, aThreadStr.getSize(), myFileHandle);
            }
            fwrite(theMessage.toCString(), 1, theMessage.getSize(), myFileHandle);
            fwrite("\n", 1, 1, myFileHandle);
            fclose(myFileHandle);
            myFileHandle = NULL;
        }
    }

    // log to standard output (with colored prefix)
    if(myToLogCout) {
        switch(theLevel) {
            case ST_PANIC:
                ST_LOG_CERR << st::COLOR_FOR_RED      << stostream_text("PANIC !! ") << st::COLOR_FOR_WHITE << theMessage << stostream_text('\n');
                break;
            case ST_FATAL:
                ST_LOG_CERR << st::COLOR_FOR_RED      << stostream_text("FATAL !! ") << st::COLOR_FOR_WHITE << theMessage << stostream_text('\n');
                break;
            case ST_ERROR:
                ST_LOG_CERR << st::COLOR_FOR_RED      << stostream_text("ERROR !! ") << st::COLOR_FOR_WHITE << theMessage << stostream_text('\n');
                break;
            case ST_WARNING:
                ST_LOG_CERR << st::COLOR_FOR_YELLOW_L << stostream_text("WARN  -- ") << st::COLOR_FOR_WHITE << theMessage << stostream_text('\n');
                break;
            case ST_INFO:
            case ST_VERBOSE:
                ST_LOG_CERR << st::COLOR_FOR_YELLOW_L << stostream_text("INFO  -- ") << st::COLOR_FOR_WHITE << theMessage << stostream_text('\n');
                break;
            case ST_TRACE:
                ST_LOG_CERR << st::COLOR_FOR_YELLOW_L << stostream_text("TRACE -- ") << st::COLOR_FOR_WHITE << theMessage << stostream_text('\n');
                break;
            default:
                ST_LOG_CERR << theMessage << stostream_text('\n');
                break;
        }
    }

    // log to the system journal(s)
#if defined(_WIN32)
/*  // get a handle to the event log
    HANDLE anEventLog = RegisterEventSource(NULL,      // local computer
                                            L"sView"); // event source name
    if(anEventLog != NULL) {
        WORD aLogType = 0;
        switch(theLevel) {
            case ST_PANIC:
            case ST_FATAL:
            case ST_ERROR:
                aLogType = EVENTLOG_ERROR_TYPE;
                break;
            case ST_WARNING:
                aLogType = EVENTLOG_WARNING_TYPE;
                break;
            case ST_INFO:
            case ST_VERBOSE:
            case ST_TRACE:
            default:
                aLogType = EVENTLOG_INFORMATION_TYPE;
                break;
        }
        ReportEvent(anEventLog, aLogType,
                    0,               // event category
                    0,               // event identifier
                    NULL,            // no user security identifier
                    1,               // number of substitution strings
                    0,               // no data
                    (LPCWSTR* )&theMessage.utfText(), // pointer to strings
                    NULL))           // no binary data
        DeregisterEventSource(anEventLog);
    }*/

#elif defined(__ANDROID__)
    if(myToLogToSystem) {
        android_LogPriority anAPrior = ANDROID_LOG_INFO;
        switch(theLevel) {
            case ST_PANIC:   anAPrior = ANDROID_LOG_FATAL; break;
            case ST_FATAL:   anAPrior = ANDROID_LOG_FATAL; break;
            case ST_ERROR:   anAPrior = ANDROID_LOG_ERROR; break;
            case ST_WARNING: anAPrior = ANDROID_LOG_WARN;  break;
            case ST_INFO:    anAPrior = ANDROID_LOG_INFO;  break;
            case ST_VERBOSE: anAPrior = ANDROID_LOG_INFO;  break;
            case ST_TRACE:   anAPrior = ANDROID_LOG_DEBUG; break;
            case ST_QUIET:   break;
        }
        __android_log_write(anAPrior, "StLogger", theMessage.toCString());
    }
#endif

    // unlock mutex
    if(!myMutex.isNull()) {
        myMutex->unlock();
    }
}

#ifdef _WIN32
    #include <windows.h>
#elif defined(__ANDROID__)
    //
#elif defined(__linux__)
    #include <gtk/gtk.h>
    #include <X11/Xlib.h>
namespace {
    static int dummyXErrorHandler(Display* , XErrorEvent* ) { return 0; }
    static bool stGtkInitForce() {
        // remember current error handler
        typedef int (*xErrHandler_t)(Display* , XErrorEvent* );
        xErrHandler_t xErrHandlerOrig = XSetErrorHandler(dummyXErrorHandler);

    #ifndef GLIB_VERSION_2_32
        if(!g_thread_get_initialized()) {
            g_thread_init(NULL); // Initialize GLIB thread support
            gdk_threads_init();  // Initialize GDK locks
        }
    #endif
        int argc = 0;
        bool isOK = gtk_init_check(&argc, NULL);

        // turn back original error handler
        XSetErrorHandler(xErrHandlerOrig);
        return isOK;
    }

}
#endif

// GUI dialogs are in Object-C for MacOS
#ifndef __APPLE__

#if defined(__ANDROID__)
namespace {
    StMessageBox::msgBoxFunc_t THE_MSGBOX = NULL;
}

void StMessageBox::setCallback(msgBoxFunc_t theFunc) {
    THE_MSGBOX = theFunc;
}

#elif defined(__linux__)
bool StMessageBox::initGlobals() {
    static const bool isInitOK = stGtkInitForce();
    return isInitOK;
}
#endif

void StMessageBox::Info(const StString& theMessage) {
    StLogger::GetDefault().write(theMessage, StLogger::ST_INFO);
#ifdef _WIN32
    MessageBoxW(NULL, theMessage.toUtfWide().toCString(), L"Info", MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST);
#elif defined(__ANDROID__)
    if(THE_MSGBOX != NULL) {
        THE_MSGBOX(StMessageBox::MsgType_Info, theMessage.toCString());
    } else {
        StMessageBox::InfoConsole(theMessage);
    }
#elif defined(__linux__)
    if(initGlobals()) {
        gdk_threads_enter();
        GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", theMessage.toCString());
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        gdk_flush(); // we need this call!
        gdk_threads_leave();
    }
#endif
}

void StMessageBox::Warn(const StString& theMessage) {
    StLogger::GetDefault().write(theMessage, StLogger::ST_WARNING);
#ifdef _WIN32
    MessageBoxW(NULL, theMessage.toUtfWide().toCString(), L"Warning", MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_TOPMOST);
#elif defined(__ANDROID__)
    if(THE_MSGBOX != NULL) {
        THE_MSGBOX(StMessageBox::MsgType_Warning, theMessage.toCString());
    } else {
        StMessageBox::WarnConsole(theMessage);
    }
#elif defined(__linux__)
    if(initGlobals()) {
        gdk_threads_enter();
        GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", theMessage.toCString());
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        gdk_flush(); // we need this call!
        gdk_threads_leave();
    }
#endif
}

void StMessageBox::Error(const StString& theMessage) {
    StLogger::GetDefault().write(theMessage, StLogger::ST_ERROR);
#ifdef _WIN32
    MessageBoxW(NULL, theMessage.toUtfWide().toCString(), L"Error", MB_OK | MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
#elif defined(__ANDROID__)
    if(THE_MSGBOX != NULL) {
        THE_MSGBOX(StMessageBox::MsgType_Error, theMessage.toCString());
    } else {
        StMessageBox::ErrorConsole(theMessage);
    }
#elif defined(__linux__)
    if(initGlobals()) {
        gdk_threads_enter();
        GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", theMessage.toCString());
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        gdk_flush(); // we need this call!
        gdk_threads_leave();
    }
#endif
}

bool StMessageBox::Question(const StString& theMessage) {
#ifdef _WIN32
    return MessageBoxW(NULL, theMessage.toUtfWide().toCString(), L"Question", MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND | MB_TOPMOST) == IDYES;
#elif defined(__ANDROID__)
    if(THE_MSGBOX != NULL) {
        return THE_MSGBOX(StMessageBox::MsgType_Question, theMessage.toCString());
    } else {
        return StMessageBox::QuestionConsole(theMessage);
    }
#elif defined(__linux__)
    if(initGlobals()) {
        gdk_threads_enter();
        GtkWidget* aDialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "%s", theMessage.toCString());
        gint anAnswer = gtk_dialog_run(GTK_DIALOG(aDialog));
        gtk_widget_destroy(aDialog);
        gdk_flush(); // we need this call!
        gdk_threads_leave();
        return anAnswer == GTK_RESPONSE_YES;
    }
    return false;
#endif
}

#endif // __APPLE__

void StMessageBox::InfoConsole(const StString& theMessage) {
    StLogger::GetDefault().write(theMessage, StLogger::ST_INFO);
    st::cout << stostream_text("(Info) ") << theMessage << stostream_text('\n');
}

void StMessageBox::WarnConsole(const StString& theMessage) {
    StLogger::GetDefault().write(theMessage, StLogger::ST_WARNING);
    st::cout << stostream_text("(Warning) ") << theMessage << stostream_text('\n');
}

void StMessageBox::ErrorConsole(const StString& theMessage) {
    StLogger::GetDefault().write(theMessage, StLogger::ST_ERROR);
    st::cout << stostream_text("(Error) ") << theMessage << stostream_text('\n');
}

bool StMessageBox::QuestionConsole(const StString& theMessage) {
    st::cout << theMessage << stostream_text('\n');
    st::cout << stostream_text("Enter 'y' (yes) or 'n' (no)... ");
    st::cout << stostream_text('\n');
    int aKey = st::getch();
    return aKey == 'y';
}
