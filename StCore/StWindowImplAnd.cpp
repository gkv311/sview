/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2014-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#if defined(__ANDROID__)

#include "StWindowImpl.h"
#include "stvkeysandroid.h" // Android NDK keys to VKEYs lookup array

#include <StCore/StAndroidGlue.h>
#include <StStrings/StLogger.h>
#include <StGL/StGLContext.h>

#include <cmath>
#include <vector>

void StWindowImpl::convertRectToBacking(StGLBoxPx& ,
                                        const int  ) const {
    //
}

// function create GUI window
bool StWindowImpl::create() {
    myKeysState.reset();
    myInitState     = STWIN_INITNOTSTART;
    myToResetDevice = false;
    if(myParentWin == NULL) {
        return false;
    }

    // retrieve fixed information
    myHasOrientSensor = myParentWin->hasOrientationSensor();
    myIsPoorOrient    = myParentWin->isPoorOrientationSensor();

    myParentWin->signals.onInputEvent += stSlot(this, &StWindowImpl::onAndroidInput);
    myParentWin->signals.onAppCmd     += stSlot(this, &StWindowImpl::onAndroidCommand);

    /*if(myParentWin->getSavedState() != NULL) {
        // we are starting with a previous saved state; restore from it
        myState = *(StSavedState* )myParentWin->getSavedState();
    }*/

    myIsUpdated = true;
    if(myParentWin->getWindow() != NULL) {
        // re-starting output for existing window
        return onAndroidInitWindow();
    }

    // first start - wait for CommandId_WindowInit...
    int aPollRes  = 0;
    int aNbEvents = 0;
    StAndroidPollSource* aSource = NULL;
    while((aPollRes = ALooper_pollAll(-1, NULL, &aNbEvents, (void** )&aSource)) >= 0) {
        if(aSource != NULL) {
            aSource->process(myParentWin, aSource);
        }

        if(myToResetDevice || myParentWin->ToDestroy()) {
            //myToResetDevice = true;
            myStEvent.Type       = stEvent_Close;
            myStEvent.Close.Time = getEventTime();
            signals.onClose->emit(myStEvent.Close);
            return false;
        } else if(myInitState != STWIN_INITNOTSTART) {
            break;
        }
    }

    return myInitState == STWIN_INIT_SUCCESS;
}

/**
 * Update StWindow position according to native parent position.
 */
void StWindowImpl::updateChildRect() {
    if(!attribs.IsFullScreen && myParentWin != NULL) {
        //myRectNorm.right()  = myRectNorm.left() + widthReturn;
        //myRectNorm.bottom() = myRectNorm.top() + heightReturn;

        // update only when changes
        if(myRectNorm != myRectNormPrev) {
            myRectNormPrev = myRectNorm;
            myIsUpdated    = true;

            const StRectI_t& aRect = attribs.IsFullScreen ? myRectFull : myRectNorm;
            myStEvent.Size.init(getEventTime(), aRect.width(), aRect.height(), myForcedAspect);
            signals.onResize->emit(myStEvent.Size);
        }
    }
}

void StWindowImpl::setFullScreen(bool theFullscreen) {
    //
}

void StWindowImpl::updateWindowPos() {
    if(myMaster.hRC.isNull()) {
        return;
    }

    EGLint aWidth  = 0, aHeight = 0;
    if(myMaster.hWindowGl != NULL) {
        aWidth  = ANativeWindow_getWidth (myMaster.hWindowGl);
        aHeight = ANativeWindow_getHeight(myMaster.hWindowGl);
    } else if(myMaster.eglSurface != EGL_NO_SURFACE) {
        eglQuerySurface(myMaster.hRC->getDisplay(), myMaster.eglSurface, EGL_WIDTH,  &aWidth);
        eglQuerySurface(myMaster.hRC->getDisplay(), myMaster.eglSurface, EGL_HEIGHT, &aHeight);
    } else {
        return;
    }

    if(myRectNorm.width()  == aWidth
    && myRectNorm.height() == aHeight) {
        return;
    }

    myRectNorm.left()   = 0;
    myRectNorm.top()    = 0;
    myRectNorm.right()  = myRectNorm.left() + aWidth;
    myRectNorm.bottom() = myRectNorm.top()  + aHeight;
    myRectFull = myRectNorm;

    myStEvent.Size.init(getEventTime(), myRectNorm.width(), myRectNorm.height(), myForcedAspect);
    signals.onResize->emit(myStEvent.Size);
}

void StWindowImpl::onAndroidInput(const AInputEvent* theEvent,
                                  bool&              theIsProcessed) {
    const int anEventType = AInputEvent_getType(theEvent);
    switch(anEventType) {
        case AINPUT_EVENT_TYPE_KEY: {
            StVirtKey aVKeySt = ST_VK_NULL;
            int32_t   aKeySym = AKeyEvent_getKeyCode(theEvent);
            if(aKeySym < ST_ANDROID2ST_VK_SIZE) {
                aVKeySt = (StVirtKey )ST_ANDROID2ST_VK[aKeySym];
            }
            if(aVKeySt == ST_VK_NULL) {
                return;
            }

            myStEvent.Key.Time = getEventTime(); //AKeyEvent_getEventTime(theEvent);
            myStEvent.Key.VKey = aVKeySt;
            myStEvent.Key.Char = 0;

            const int32_t aKeyAction = AKeyEvent_getAction(theEvent);
            if(aKeyAction == AKEY_EVENT_ACTION_DOWN) {
                postKeyDown(myStEvent);
            } else if(aKeyAction == AKEY_EVENT_ACTION_UP) {
                postKeyUp(myStEvent);
            }// else if(aKeyAction == AKEY_EVENT_ACTION_MULTIPLE) {}
            return;
        }
        case AINPUT_EVENT_TYPE_MOTION: {
            theIsProcessed = true;

            const int32_t aSource          = AInputEvent_getSource(theEvent);
            const int32_t anActionPak      = AMotionEvent_getAction(theEvent);
            const int32_t anAction         = anActionPak & AMOTION_EVENT_ACTION_MASK;
            const size_t  anActionPntIndex = anActionPak & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK;

            //const StRectI_t& aWinRect = attribs.IsFullScreen ? myRectFull : myRectNorm;
            const StRectI_t& aWinRect  = myRectNorm;
            const float      aWinSizeX = aWinRect.width();
            const float      aWinSizeY = aWinRect.height();

            // at least single point is defined
            StPointD_t aPos0Px(AMotionEvent_getX(theEvent, 0), AMotionEvent_getY(theEvent, 0));
            StPointD_t aPos0(double(aPos0Px.x()) / double(aWinSizeX),
                             double(aPos0Px.y()) / double(aWinSizeY));

            myStEvent.Type      = stEvent_None;
            myStEvent.Base.Time = getEventTime(); /// int64_t AMotionEvent_getEventTime(theEvent);
            switch(aSource) {
                case AINPUT_SOURCE_TOUCHSCREEN:
                case AINPUT_SOURCE_TOUCHPAD: {
                    const size_t aNbTouches = AMotionEvent_getPointerCount(theEvent);
                    myStEvent.Touch.NbTouches = std::min(aNbTouches, (size_t )ST_MAX_TOUCHES);
                    bool isUp = anAction == AMOTION_EVENT_ACTION_UP
                             || anAction == AMOTION_EVENT_ACTION_POINTER_UP;
                    if(isUp) {
                        myStEvent.Touch.NbTouches = std::max(myStEvent.Touch.NbTouches - 1, 0);
                    }
                    for(size_t aTouchIter = 0; aTouchIter < ST_MAX_TOUCHES; ++aTouchIter) {
                        StTouch& aTouch = myStEvent.Touch.Touches[aTouchIter];
                        aTouch = StTouch::Empty();
                        if(aTouchIter == anActionPntIndex
                        && isUp) {
                            continue;
                        }
                        if(aTouchIter >= aNbTouches) {
                            continue;
                        }

                        aTouch.Id       = AMotionEvent_getPointerId(theEvent, aTouchIter);
                        aTouch.DeviceId = AInputEvent_getDeviceId(theEvent);
                        aTouch.OnScreen = aSource == AINPUT_SOURCE_TOUCHSCREEN;

                        const float aPosX = AMotionEvent_getX(theEvent, aTouchIter);
                        const float aPosY = AMotionEvent_getY(theEvent, aTouchIter);
                        aTouch.PointX = (aPosX - float(aWinRect.left())) / aWinSizeX;
                        aTouch.PointY = (aPosY - float(aWinRect.top()))  / aWinSizeY;
                    }

                    switch(anAction) {
                        case AMOTION_EVENT_ACTION_DOWN:
                        case AMOTION_EVENT_ACTION_POINTER_DOWN: {
                            myStEvent.Type = stEvent_TouchDown;
                            doTouch(myStEvent.Touch);
                            if(aNbTouches == 1) {
                                // simulate mouse click
                                myMousePt = aPos0;
                                myIsPreciseCursor = false;
                                myStEvent.Type = stEvent_MouseDown;
                                myStEvent.Button.Button  = ST_MOUSE_LEFT;
                                myStEvent.Button.Buttons = 0;
                                myStEvent.Button.PointX  = myMousePt.x();
                                myStEvent.Button.PointY  = myMousePt.y();
                                signals.onMouseDown->emit(myStEvent.Button);
                            } else if(aNbTouches == 2) {
                                // emit special event to cancel previously simulated click
                                myStEvent.Type = stEvent_MouseCancel;
                                myStEvent.Button.Button  = ST_MOUSE_LEFT;
                                myStEvent.Button.Buttons = 0;
                                myStEvent.Button.PointX  = myMousePt.x();
                                myStEvent.Button.PointY  = myMousePt.y();
                                signals.onMouseUp->emit(myStEvent.Button);
                            }
                            break;
                        }
                        case AMOTION_EVENT_ACTION_MOVE: {
                            myStEvent.Type = stEvent_TouchMove;
                            if(aNbTouches == 1) {
                                // simulate mouse move
                                myMousePt = aPos0;
                                myIsPreciseCursor = false;
                            }
                            doTouch(myStEvent.Touch);
                            break;
                        }
                        case AMOTION_EVENT_ACTION_UP:
                        case AMOTION_EVENT_ACTION_POINTER_UP: {
                            myStEvent.Type = stEvent_TouchUp;
                            doTouch(myStEvent.Touch);
                            if(aNbTouches == 1) {
                                // simulate mouse unclick
                                myMousePt = aPos0;
                                myIsPreciseCursor = false;
                                myStEvent.Type = stEvent_MouseUp;
                                myStEvent.Button.Button  = ST_MOUSE_LEFT;
                                myStEvent.Button.Buttons = 0;
                                myStEvent.Button.PointX  = myMousePt.x();
                                myStEvent.Button.PointY  = myMousePt.y();
                                signals.onMouseUp->emit(myStEvent.Button);
                            }
                            break;
                        }
                        case AMOTION_EVENT_ACTION_CANCEL: {
                            myStEvent.Type = stEvent_TouchCancel;
                            doTouch(myStEvent.Touch);
                            break;
                        }
                    }
                    return;
                }
            }

            myMousePt = aPos0;
            myIsPreciseCursor = aSource == AINPUT_SOURCE_MOUSE; // || AINPUT_SOURCE_STYLUS

            StVirtButton aMouseBtn = ST_MOUSE_LEFT;
            myStEvent.Button.Button  = aMouseBtn;
            myStEvent.Button.Buttons = 0;
            myStEvent.Button.PointX  = myMousePt.x();
            myStEvent.Button.PointY  = myMousePt.y();

            if(anAction == AMOTION_EVENT_ACTION_DOWN) {
                myStEvent.Type = stEvent_MouseDown;
                signals.onMouseDown->emit(myStEvent.Button);
            } else if(anAction == AMOTION_EVENT_ACTION_UP) {
                myStEvent.Type = stEvent_MouseUp;
                signals.onMouseUp->emit(myStEvent.Button);
            } else if(anAction == AMOTION_EVENT_ACTION_SCROLL) {
                float aScrollX = AMotionEvent_getAxisValue(theEvent, AMOTION_EVENT_AXIS_HSCROLL, 0);
                float aScrollY = AMotionEvent_getAxisValue(theEvent, AMOTION_EVENT_AXIS_VSCROLL, 0);
                myStEvent.Type = stEvent_Scroll;
                myStEvent.Scroll.init(myStEvent.Base.Time, myMousePt.x(), myMousePt.y(),
                                      aScrollX, aScrollY, false);
                if((myStEvent.Scroll.Time - myScrollAcc.Time) > 0.1
                 || std::abs(aPos0Px.x() - myScrollAcc.PointX) > 10
                 || std::abs(aPos0Px.y() - myScrollAcc.PointY) > 10) {
                    myScrollAcc.reset();
                }
                myScrollAcc.Time = myStEvent.Scroll.Time;
                myScrollAcc.PointX = aPos0Px.x();
                myScrollAcc.PointY = aPos0Px.y();
                myStEvent.Scroll.StepsX = myScrollAcc.accumulateStepsX(int(aScrollX * 1000.0), 1000);
                myStEvent.Scroll.StepsY = myScrollAcc.accumulateStepsY(int(aScrollY * 1000.0), 1000);
                signals.onScroll->emit(myStEvent.Scroll);
            }
            return;
        }
    }
}

bool StWindowImpl::onAndroidInitWindow() {
    myIsPaused = false;
    if(myParentWin->getWindow() == NULL) {
        return false;
    }

    if(!myMaster.hRC.isNull()) {
        myMaster.hWindowGl = myParentWin->getWindow();

        EGLint anEglErr = eglGetError();
        (void )anEglErr;

        EGLint aFormat = 0;
        if(eglGetConfigAttrib(myMaster.hRC->getDisplay(), myMaster.hRC->getConfig(), EGL_NATIVE_VISUAL_ID, &aFormat) == EGL_FALSE) {
            anEglErr = eglGetError();
        }
        ANativeWindow_setBuffersGeometry(myMaster.hWindowGl, 0, 0, aFormat);

        myMaster.eglSurface = eglCreateWindowSurface(myMaster.hRC->getDisplay(), myMaster.hRC->getConfig(), myMaster.hWindowGl, NULL);
        if(myMaster.eglSurface == NULL) {
            anEglErr = eglGetError();
        }

        // bind the rendering context to the window
        if(!myMaster.hRC->makeCurrent(myMaster.eglSurface)) {
            myMaster.close();
            mySlave.close();
            myInitState = STWIN_ERROR_X_GLRC_CREATE;
            stError("Critical error - broken EGL context!");
            return false;
        }

        const EGLint aWidth  = ANativeWindow_getWidth (myMaster.hWindowGl);
        const EGLint aHeight = ANativeWindow_getHeight(myMaster.hWindowGl);

        const bool isResized = myRectNorm.width()  != aWidth
                            || myRectNorm.height() != aHeight;
        myRectNorm.left()   = 0;
        myRectNorm.top()    = 0;
        myRectNorm.right()  = myRectNorm.left() + aWidth;
        myRectNorm.bottom() = myRectNorm.top()  + aHeight;
        myRectFull = myRectNorm;

        myInitState = STWIN_INIT_SUCCESS;
        if(isResized) {
            myStEvent.Size.init(getEventTime(), myRectNorm.width(), myRectNorm.height(), myForcedAspect);
            signals.onResize->emit(myStEvent.Size);
        }
        return true;
    }

    myMaster.hRC = new StWinGlrc(eglGetDisplay(EGL_DEFAULT_DISPLAY), attribs.IsGlDebug, attribs.GlDepthSize, attribs.GlStencilSize);
    if(!myMaster.hRC->isValid()) {
        myMaster.close();
        mySlave.close();
        myInitState = STWIN_ERROR_X_GLRC_CREATE;
        return false;
    }

    myMaster.hWindowGl = myParentWin->getWindow();
    myInitState = myMaster.glCreateContext(NULL, myRectNorm, attribs.GlDepthSize, attribs.GlStencilSize, attribs.IsGlStereo, attribs.IsGlDebug);
    if(myInitState != STWIN_INIT_SUCCESS) {
        return false;
    }

    EGLint aWidth = 0, aHeight = 0;
    if(myMaster.hWindowGl != NULL) {
        aWidth  = ANativeWindow_getWidth (myMaster.hWindowGl);
        aHeight = ANativeWindow_getHeight(myMaster.hWindowGl);
    } else if(myMaster.eglSurface != EGL_NO_SURFACE) {
        eglQuerySurface(myMaster.hRC->getDisplay(), myMaster.eglSurface, EGL_WIDTH,  &aWidth);
        eglQuerySurface(myMaster.hRC->getDisplay(), myMaster.eglSurface, EGL_HEIGHT, &aHeight);
    }

    myRectNorm.left()   = 0;
    myRectNorm.top()    = 0;
    myRectNorm.right()  = myRectNorm.left() + aWidth;
    myRectNorm.bottom() = myRectNorm.top()  + aHeight;
    myRectFull = myRectNorm;

    myGlContext = new StGLContext(myResMgr);
    if(!myGlContext->stglInit()) {
        myMaster.close();
        mySlave.close();
        stError("Critical error - broken GL context!\nInvalid OpenGL driver?");
        myInitState = STWIN_ERROR_X_GLRC_CREATE;
        return false;
    }
    myInitState = STWIN_INIT_SUCCESS;
    return true;
}

void StWindowImpl::onAndroidCommand(int32_t theCommand) {
    switch(theCommand) {
        case StAndroidGlue::CommandId_SaveState: {
            // the system has asked us to save our current state
            /*StSavedState* aState = (StSavedState* )malloc(sizeof(StSavedState));
            *aState = state;
            myParentWin->setSavedState(aState, sizeof(StSavedState));*/
            return;
        }
        case StAndroidGlue::CommandId_WindowInit: {
            // the window is being shown, get it ready
            onAndroidInitWindow();
            return;
        }
        case StAndroidGlue::CommandId_BackPressed: {
            myStEvent.Key.Time = getEventTime();
            myStEvent.Key.VKey = ST_VK_ESCAPE;
            myStEvent.Key.Char = 0;
            postKeyDown(myStEvent);
            postKeyUp  (myStEvent);
            return;
        }
        case StAndroidGlue::CommandId_Resume: {
            myIsPaused = false;
            return;
        }
        case StAndroidGlue::CommandId_Pause: {
            myIsPaused = true;
            myStEvent.Type       = stEvent_Pause;
            myStEvent.Pause.Time = getEventTime();
            signals.onPause->emit(myStEvent.Pause);
            return;
        }
        case StAndroidGlue::CommandId_Stop: {
            if(myParentWin->getMemoryClass() < 50) {
                myStEvent.Type       = stEvent_Close;
                myStEvent.Close.Time = getEventTime();
                signals.onClose->emit(myStEvent.Close);
            }
            break;
        }
        case StAndroidGlue::CommandId_WindowChanged:
        case StAndroidGlue::CommandId_WindowTerm: {
            if(!myMaster.hRC.isNull()) {
                myMaster.hRC->makeCurrent(EGL_NO_SURFACE);
                if(myMaster.eglSurface != EGL_NO_SURFACE) {
                    eglDestroySurface(myMaster.hRC->getDisplay(), myMaster.eglSurface);
                    myMaster.eglSurface = EGL_NO_SURFACE;
                }
            }
            myMaster.hWindowGl = NULL;
            if(theCommand == StAndroidGlue::CommandId_WindowChanged) {
                onAndroidInitWindow();
            }
            return;
        }
        case StAndroidGlue::CommandId_FocusGained: {
            //
            return;
        }
        case StAndroidGlue::CommandId_FocusLost: {
            myKeysState.reset();
            return;
        }
        case StAndroidGlue::CommandId_ConfigChanged: {
            // do not handle resize event here - screen might be not yet resized
            updateMonitors();

            myStEvent.Size.init(getEventTime(), myRectNorm.width(), myRectNorm.height(), myForcedAspect);
            myStEvent.Type  = stEvent_NewMonitor;
            //myWinOnMonitorId = 0;
            signals.onAnotherMonitor->emit(myStEvent.Size);
            return;
        }
    }
}

void StWindowImpl::processEvents() {
    if(myParentWin == NULL
    || myToResetDevice) {
        // window is closed!
        return;
    }

    // check if we are exiting
    if(myParentWin->ToDestroy()) {
        myStEvent.Type       = stEvent_Close;
        myStEvent.Close.Time = getEventTime();
        signals.onClose->emit(myStEvent.Close);
        return;
    }

    // check onNewIntent event
    StString aDndFile;
    myParentWin->setWindowTitle(myWindowTitle);
    myParentWin->setHardwareStereoOn(myToEnableStereoHW);
    myParentWin->setTrackOrientation(myToTrackOrient);
    myParentWin->setHideSystemBars(myToHideStatusBar, myToHideNavBar);
    myParentWin->fetchState(aDndFile, myQuaternion, myToSwapEyesHW, myKeysState);
    if(!aDndFile.isEmpty()) {
        // notice - unpress event will NOT be generated!
        myStEvent.Type      = stEvent_KeyDown;
        myStEvent.Key.Time  = getEventTime();
        myStEvent.Key.Flags = ST_VF_NONE;
        if(aDndFile == "ACTION_PLAY_PREV") {
            myStEvent.Key.VKey  = ST_VK_MEDIA_PREV_TRACK;
            myEventsBuffer.append(myStEvent);
        } else if(aDndFile == "ACTION_PLAY_NEXT") {
            myStEvent.Key.VKey  = ST_VK_MEDIA_NEXT_TRACK;
            myEventsBuffer.append(myStEvent);
        } else if(aDndFile == "ACTION_PLAY_PAUSE") {
            myStEvent.Key.VKey  = ST_VK_MEDIA_PLAY_PAUSE;
            myEventsBuffer.append(myStEvent);
        } else {
            std::vector<const char*> aDndList;
            aDndList.push_back(aDndFile.toCString());
            myStEvent.Type = stEvent_FileDrop;
            myStEvent.DNDrop.Time = getEventTime();
            myStEvent.DNDrop.NbFiles = aDndList.size();
            myStEvent.DNDrop.Files   = &aDndList[0];
            myEventsBuffer.append(myStEvent);
        }
    }

    updateActiveState();

    StPointD_t anOldMousePt = myMousePt;
    int aPollRes  = 0;
    int aNbEvents = 0;
    StAndroidPollSource* aSource = NULL;
    bool toWaitEvents = false;
    while((aPollRes = ALooper_pollAll(toWaitEvents ? -1 : 0, NULL, &aNbEvents, (void** )&aSource)) >= 0) {
        if(aSource != NULL) {
            aSource->process(myParentWin, aSource);
        }
        if(myToResetDevice) {
            break;
        }

        // check if we are exiting
        if(myParentWin->ToDestroy()) {
            break;
        }
    }

    // check if we are exiting
    if(myParentWin->ToDestroy()) {
        myStEvent.Type       = stEvent_Close;
        myStEvent.Close.Time = getEventTime();
        signals.onClose->emit(myStEvent.Close);
        return;
    }

    myIsMouseMoved = false;
    if(myIsPreciseCursor
    && myMousePt.x() >= 0.0 && myMousePt.x() <= 1.0 && myMousePt.y() >= 0.0 && myMousePt.y() <= 1.0) {
        StPointD_t aDspl = myMousePt - anOldMousePt;
        if(std::abs(aDspl.x()) >= 0.0008 || std::abs(aDspl.y()) >= 0.0008) {
            myIsMouseMoved = true;
        }
    }

    // update position only when all messages are parsed
    updateWindowPos();
    myIsUpdated = false;

    // StWindow XLib implementation process events in the same thread
    // thus this double buffer is not in use
    // however user events may be posted to it
    swapEventsBuffers();
}

bool StWindowImpl::toClipboard(const StString& theText) {
    return false;
}

bool StWindowImpl::fromClipboard(StString& theText) {
    return false;
}

#endif
