/**
 * This is source code for sView
 *
 * Copyright © Kirill Gavrilov, 2011
 */

#include <StCore/StCore.h>
#include <StThreads/StFPSControl.h>
#include "../StCore/StSearchMonitors.h"

#if(defined(_WIN32) || defined(__WIN32__))
    #include <windows.h>
#elif(defined(__linux__) || defined(__linux))
    #include <X11/X.h>
    #include <X11/Xlib.h>
#endif

/**
 * Clone desktop main class.
 */
class ST_LOCAL StCloneDesktop {

        private:

#if(defined(_WIN32) || defined(__WIN32__))
    HWND             myWindowH;    //!< handle to the window
#elif(defined(__linux__) || defined(__linux))
    Window           myWindowH;    //!< X-window handle
#endif
    StFPSControl     myFPSControl; //!< FPS control
    StMonitor        myMonMaster;
    StMonitor        myMonSlave;
    bool             myIsFlipX;
    bool             myIsFlipY;

        public:

    /**
     * Default constructor.
     */
    StCloneDesktop();
    ~StCloneDesktop();

    /**
     * Creates the clone window.
     * @return true on success.
     */
    bool create();

    /**
     * Main message loop.
     */
    void mainLoop();

#if(defined(_WIN32) || defined(__WIN32__))
    /**
     * WinAPI callback method.
     */
    LRESULT stWndProc(HWND theWinH, UINT theMessage,
                      WPARAM wparam, LPARAM lparam);
#endif

};
