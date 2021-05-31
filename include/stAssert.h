/**
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __stAssert_h__
#define __stAssert_h__

#ifdef _WIN32
    #if defined(_MSC_VER)
        // VS-specific intrinsic
        #include <crtdbg.h>
        #define ST_DBGBREAK() __debugbreak()
    #else
        // WinAPI function
        #define ST_DBGBREAK() DebugBreak()
    #endif
    #include <windows.h>
#else
    // POSIX systems
    #include <signal.h>
    #define ST_DBGBREAK() raise(SIGTRAP)
#endif

#include <StStrings/StLogger.h>
#include <assert.h>

namespace st {

    /**
     * Answer enumeration for stAssertQuestionGUI() function.
     */
    typedef enum {
        ST_DBGASSERT_ABORT,
        ST_DBGASSERT_DEBUG,
        ST_DBGASSERT_IGNORE,
    } stAssertAns_t;

    /**
     * Function to ask developer interactively what to do (GUI part - do not call directly).
     * @param theMessage the message to show
     * @return user decision as enumeration
     */
#ifdef _WIN32
    inline stAssertAns_t stAssertQuestionGUI(const StString& theMessage) {
        int aResult = ::MessageBoxW(NULL, theMessage.toUtfWide().toCString(), L"Debug Assertion Failed",
                                    MB_ABORTRETRYIGNORE | MB_ICONSTOP | MB_SETFOREGROUND | MB_TOPMOST);
        return (aResult == IDABORT) ? ST_DBGASSERT_ABORT : ((aResult == IDIGNORE) ? ST_DBGASSERT_IGNORE : ST_DBGASSERT_DEBUG);
    }
#else
    inline stAssertAns_t stAssertQuestionGUI(const StString& ) {
        return ST_DBGASSERT_DEBUG;
    }
#endif

    /**
     * Function to ask developer interactively what to do with assertion.
     * Will prepare the description, display it in message box (if available) and log as error.
     * @param theFile (StString ) - file name;
     * @param theLine (StString ) - line number in file;
     * @param theExpr (StString ) - tested expression;
     * @param theDesc (StString ) - optional description to assertion;
     * @return true if user wants to debug this assertion.
     */
    inline bool stAssertQuestion(const StString& theFile,
                                 const StString& theLine,
                                 const StString& theExpr,
                                 const StString& theDesc) {
        StString aMsg = StString("Statement '") + theExpr + "' is not TRUE!\n"
            + "\nFile: '" + theFile +"'"
            + "\nLine: "  + theLine
            + (theDesc.isEmpty() ? theDesc : ("\nDescription: " + theDesc +"\n"));
        ST_ERROR_LOG(aMsg);
        stAssertAns_t aRes = stAssertQuestionGUI(aMsg);
        if(aRes == ST_DBGASSERT_ABORT) {
            exit(-1);
        }
        return aRes == ST_DBGASSERT_DEBUG;
    }

#ifdef ST_DEBUG
    /**
     * Classical assert. If expression is not TRUE will:
     *  - call debugger in DEBUG mode;
     *  - do nothing in RELEASE mode.
     * @param theTrueExpr - boolean expression that should be always TRUE;
     * @param theDesc     - optional description to the assertion.
     */
    #define ST_ASSERT(theTrueExpr, theDesc) \
        if(!(theTrueExpr) && st::stAssertQuestion(__FILE__, __LINE__, #theTrueExpr, theDesc)) { ST_DBGBREAK(); }

    /**
     * Advanced assert, same as ST_ASSERT() but will perform additional action
     * if expression is not TRUE even in RELEASE mode.
     * This could allow to process exceptional situation in safer way rather than just ignoring it.
     * @param theTrueExpr - boolean expression that should be always TRUE;
     * @param theDesc     - optional description to the assertion;
     * @param theAction   - action to perform if expression is FALSE.
     */
    #define ST_ASSERT_SLIP(theTrueExpr, theDesc, theAction) \
        if(!(theTrueExpr)) { if(st::stAssertQuestion(__FILE__, __LINE__, #theTrueExpr, theDesc)) { ST_DBGBREAK(); }; theAction; }
#else
    #define ST_ASSERT(theTrueExpr, theDesc)
    #define ST_ASSERT_SLIP(theTrueExpr, theDesc, theAction) \
        if(!(theTrueExpr)) { theAction; }
#endif

};

#endif // __stAssert_h__
