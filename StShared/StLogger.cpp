/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StStrings/StLogger.h>

#include <StStrings/stConsole.h>
#include <StThreads/StMutexSlim.h>
#include <StThreads/StProcess.h>

// we do not use st::cerr here to avoid
// global static variables initialization ambiguity
#if(defined(_WIN32) || defined(__WIN32__))
    // Unicode version
    #define ST_LOG_CERR std::wcerr
#else
    #define ST_LOG_CERR std::cerr
#endif

StLogger& StLogger::GetDefault() {
    // global instance
    static StLogger THE_DEFAULT_LOGGER(
    #if(defined(__ST_DEBUG_LOG_TO_FILE__) && defined(__ST_DEBUG__))
        StString(__ST_DEBUG_LOG_TO_FILE__),
    #else
        StString(),
    #endif
    #ifdef __ST_DEBUG__
        StLogger::ST_DEBUG,
    #else
        StLogger::ST_VERBOSE,
    #endif
        StLogger::ST_OPT_COUT | StLogger::ST_OPT_LOCK
    );
    return THE_DEFAULT_LOGGER;
}

bool StLogger::IdentifyModule(const StString& theModuleName) {
    GetDefault().myModuleId = theModuleName;
    ST_DEBUG_LOG("  ==  Process " + StProcess::getProcessName()
               + " (" + StProcess::getPID() + "), module " + theModuleName
               + " Loaded  ==");
    return true;
}

StLogger::StLogger(const StString&       theLogFile,
                   const StLogger::Level theFilter,
                   const int             theOptions)
: myMutex((theOptions & StLogger::ST_OPT_LOCK) ? new StMutexSlim() : (StMutexSlim* )NULL),
  myModuleId(),
#if(defined(_WIN32) || defined(__WIN32__))
  myFilePath(theLogFile.toUtfWide()),
#else
  myFilePath(theLogFile),
#endif
  myFileHandle(NULL),
  myFilter(theFilter),
  myToLogCout(theOptions & StLogger::ST_OPT_COUT) {
    //
}

StLogger::~StLogger() {
    if(!myModuleId.isEmpty()) {
        ST_DEBUG_LOG("  ==  Process " + StProcess::getProcessName()
                   + " (" + StProcess::getPID() + "), module " + myModuleId
                   + " Unloaded  ==");
    }
}

void StLogger::write(const StString&       theMessage,
                     const StLogger::Level theLevel) {
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
    #if(defined(_WIN32) || defined(__WIN32__))
        myFileHandle = _wfopen(myFilePath.toCString(), L"ab");
    #elif(defined(__linux__) || defined(__linux))
        myFileHandle =   fopen(myFilePath.toCString(),  "ab");
    #endif
        if(myFileHandle != NULL) {
            switch(theLevel) {
                case ST_PANIC:
                    fwrite("PANIC !! ", 1, 9, myFileHandle);
                    fwrite(theMessage.toCString(), 1, theMessage.getSize(), myFileHandle);
                    break;
                case ST_FATAL:
                    fwrite("FATAL !! ", 1, 9, myFileHandle);
                    fwrite(theMessage.toCString(), 1, theMessage.getSize(), myFileHandle);
                    break;
                case ST_ERROR:
                    fwrite("ERROR !! ", 1, 9, myFileHandle);
                    fwrite(theMessage.toCString(), 1, theMessage.getSize(), myFileHandle);
                    break;
                case ST_WARNING:
                    fwrite("WARN  -- ", 1, 9, myFileHandle);
                    fwrite(theMessage.toCString(), 1, theMessage.getSize(), myFileHandle);
                    break;
                case ST_INFO:
                case ST_VERBOSE:
                    fwrite("INFO  -- ", 1, 9, myFileHandle);
                    fwrite(theMessage.toCString(), 1, theMessage.getSize(), myFileHandle);
                    break;
                case ST_DEBUG:
                    fwrite("DEBUG -- ", 1, 9, myFileHandle);
                    fwrite(theMessage.toCString(), 1, theMessage.getSize(), myFileHandle);
                    break;
                default:
                    fwrite(theMessage.toCString(), 1, theMessage.getSize(), myFileHandle);
                    break;
            }
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
            case ST_DEBUG:
                ST_LOG_CERR << st::COLOR_FOR_YELLOW_L << stostream_text("DEBUG -- ") << st::COLOR_FOR_WHITE << theMessage << stostream_text('\n');
                break;
            default:
                ST_LOG_CERR << theMessage << stostream_text('\n');
                break;
        }
    }

    // log to the system journal(s)
/*#if(defined(_WIN32) || defined(__WIN32__))
    // get a handle to the event log
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
            case ST_DEBUG:
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
    }
#endif*/

    // unlock mutex
    if(!myMutex.isNull()) {
        myMutex->unlock();
    }
}

#if(defined(_WIN32) || defined(__WIN32__))
    #include <windows.h>
#elif(defined(__linux__) || defined(__linux))
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

    static bool stGtkInit() {
        static const bool isInitOK = stGtkInitForce();
        return isInitOK;
    }
};
#endif

// GUI dialogs are in Object-C for MacOS
#ifndef __APPLE__

void stInfo(const StString& theMessage) {
    StLogger::GetDefault().write(theMessage, StLogger::ST_INFO);
#if(defined(_WIN32) || defined(__WIN32__))
    MessageBoxW(NULL, theMessage.toUtfWide().toCString(), L"Info", MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST);
#elif(defined(__linux__) || defined(__linux))
    if(stGtkInit()) {
        gdk_threads_enter();
        GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", theMessage.toCString());
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        gdk_flush(); // we need this call!
        gdk_threads_leave();
    }
#endif
}

void stWarn(const StString& theMessage) {
    StLogger::GetDefault().write(theMessage, StLogger::ST_WARNING);
#if(defined(_WIN32) || defined(__WIN32__))
    MessageBoxW(NULL, theMessage.toUtfWide().toCString(), L"Warning", MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_TOPMOST);
#elif(defined(__linux__) || defined(__linux))
    if(stGtkInit()) {
        gdk_threads_enter();
        GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", theMessage.toCString());
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        gdk_flush(); // we need this call!
        gdk_threads_leave();
    }
#endif
}

void stError(const StString& theMessage) {
    StLogger::GetDefault().write(theMessage, StLogger::ST_ERROR);
#if(defined(_WIN32) || defined(__WIN32__))
    MessageBoxW(NULL, theMessage.toUtfWide().toCString(), L"Error", MB_OK | MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
#elif(defined(__linux__) || defined(__linux))
    if(stGtkInit()) {
        gdk_threads_enter();
        GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", theMessage.toCString());
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        gdk_flush(); // we need this call!
        gdk_threads_leave();
    }
#endif
}

void stSuccess(const StString& theMessage) {
    StLogger::GetDefault().write(theMessage, StLogger::ST_INFO);
#if(defined(_WIN32) || defined(__WIN32__))
    MessageBoxW(NULL, theMessage.toUtfWide().toCString(), L"Success", MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_TOPMOST);
#elif(defined(__linux__) || defined(__linux))
    if(stGtkInit()) {
        gdk_threads_enter();
        GtkWidget* dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", theMessage.toCString());
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        gdk_flush(); // we need this call!
        gdk_threads_leave();
    }
#endif
}

bool stQuestion(const StString& theMessage) {
#if(defined(_WIN32) || defined(__WIN32__))
    return MessageBoxW(NULL, theMessage.toUtfWide().toCString(), L"Question", MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND | MB_TOPMOST) == IDYES;
#elif(defined(__linux__) || defined(__linux))
    if(stGtkInit()) {
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

void stInfoConsole(const StString& theMessage) {
    StLogger::GetDefault().write(theMessage, StLogger::ST_INFO);
    st::cout << stostream_text("(Info) ") << theMessage << stostream_text('\n');
}

void stWarnConsole(const StString& theMessage) {
    StLogger::GetDefault().write(theMessage, StLogger::ST_WARNING);
    st::cout << stostream_text("(Warning) ") << theMessage << stostream_text('\n');
}

void stErrorConsole(const StString& theMessage) {
    StLogger::GetDefault().write(theMessage, StLogger::ST_ERROR);
    st::cout << stostream_text("(Error) ") << theMessage << stostream_text('\n');
}

void stSuccessConsole(const StString& theMessage) {
    StLogger::GetDefault().write(theMessage, StLogger::ST_INFO);
    st::cout << stostream_text("(Success) ") << theMessage << stostream_text('\n');
}

bool stQuestionConsole(const StString& theMessage) {
    st::cout << theMessage << stostream_text('\n');
    st::cout << stostream_text("Enter 'y' (yes) or 'n' (no)... ");
    st::cout << stostream_text('\n');
    int aKey = st::getch();
    return aKey == 'y';
}
