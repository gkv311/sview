/**
 * Copyright © 2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * StCore library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StCore library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#if defined(__ANDROID__)

#include "StWindowImpl.h"

#include <StCore/StAndroidGlue.h>
#include <StStrings/StLogger.h>
#include <StGL/StGLContext.h>

#include <cmath>

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

    myParentWin->signals.onInputEvent += stSlot(this, &StWindowImpl::onAndroidInput);
    myParentWin->signals.onAppCmd     += stSlot(this, &StWindowImpl::onAndroidCommand);

    // Prepare to monitor accelerometer
    //mySensorManager       = ASensorManager_getInstance();
    //myAccelerometerSensor = ASensorManager_getDefaultSensor(mySensorManager, ASENSOR_TYPE_ACCELEROMETER);
    //mySensorEventQueue    = ASensorManager_createEventQueue(mySensorManager, myParentWin->getLooper(), LooperId_USER, NULL, NULL);

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
    if(!attribs.IsFullScreen && (ANativeWindow* )myParentWin != NULL) {
        //myRectNorm.right()  = myRectNorm.left() + widthReturn;
        //myRectNorm.bottom() = myRectNorm.top() + heightReturn;

        // update only when changes
        if(myRectNorm != myRectNormPrev) {
            myRectNormPrev = myRectNorm;
            myIsUpdated    = true;

            const StRectI_t& aRect = attribs.IsFullScreen ? myRectFull : myRectNorm;
            myStEvent.Type       = stEvent_Size;
            myStEvent.Size.Time  = getEventTime();
            myStEvent.Size.SizeX = aRect.width();
            myStEvent.Size.SizeY = aRect.height();
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

    myStEvent.Type       = stEvent_Size;
    myStEvent.Size.Time  = getEventTime();
    myStEvent.Size.SizeX = myRectNorm.width();
    myStEvent.Size.SizeY = myRectNorm.height();
    signals.onResize->emit(myStEvent.Size);
}

void StWindowImpl::onAndroidInput(const AInputEvent* theEvent,
                                  bool&              theIsProcessed) {
    const int anEventType = AInputEvent_getType(theEvent);
    switch(anEventType) {
        case AINPUT_EVENT_TYPE_KEY: {
            return;
        }
        case AINPUT_EVENT_TYPE_MOTION: {
            theIsProcessed = true;

            //int32_t AInputEvent_getSource(theEvent);
            //AINPUT_SOURCE_TOUCHSCREEN
            //AINPUT_SOURCE_TRACKBALL

            float aPosX = AMotionEvent_getX(theEvent, 0);
            float aPosY = AMotionEvent_getY(theEvent, 0);

            const StRectI_t& aRect = myRectNorm;
            myMousePt.x() = double(aPosX) / double(aRect.width());
            myMousePt.y() = double(aPosY) / double(aRect.height());

            StVirtButton aMouseBtn = ST_MOUSE_LEFT;
            myStEvent.Button.Time    = getEventTime(); /// int64_t AMotionEvent_getEventTime(theEvent);
            myStEvent.Button.Button  = aMouseBtn;
            myStEvent.Button.Buttons = 0;
            myStEvent.Button.PointX  = myMousePt.x();
            myStEvent.Button.PointY  = myMousePt.y();

            int32_t anAction = AMotionEvent_getAction(theEvent);
            if((anAction & AMOTION_EVENT_ACTION_MASK) == AMOTION_EVENT_ACTION_DOWN) {
                myStEvent.Type = stEvent_MouseDown;
                signals.onMouseDown->emit(myStEvent.Button);
            } else if((anAction & AMOTION_EVENT_ACTION_MASK) == AMOTION_EVENT_ACTION_UP) {
                myStEvent.Type = stEvent_MouseUp;
                signals.onMouseUp->emit(myStEvent.Button);
            }

            return;
        }
    }
}

bool StWindowImpl::onAndroidInitWindow() {
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

        myRectNorm.left()   = 0;
        myRectNorm.top()    = 0;
        myRectNorm.right()  = myRectNorm.left() + aWidth;
        myRectNorm.bottom() = myRectNorm.top()  + aHeight;
        myRectFull = myRectNorm;

        myInitState = STWIN_INIT_SUCCESS;
        return true;
    }

    myMaster.hRC = new StWinGlrc(eglGetDisplay(EGL_DEFAULT_DISPLAY), attribs.IsGlDebug, attribs.GlDepthSize);
    if(!myMaster.hRC->isValid()) {
        myMaster.close();
        mySlave.close();
        myInitState = STWIN_ERROR_X_GLRC_CREATE;
        return false;
    }

    myMaster.hWindowGl = myParentWin->getWindow();
    myInitState = myMaster.glCreateContext(NULL, myRectNorm, attribs.GlDepthSize, attribs.IsGlStereo, attribs.IsGlDebug);
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
        case StAndroidGlue::CommandId_Stop: {
            if(myParentWin->getMemoryClass() < 50) {
                myStEvent.Type       = stEvent_Close;
                myStEvent.Close.Time = getEventTime();
                signals.onClose->emit(myStEvent.Close);
            }
            break;
        }
        case StAndroidGlue::CommandId_WindowTerm: {
            if(!myMaster.hRC.isNull()) {
                myMaster.hRC->makeCurrent(EGL_NO_SURFACE);
                if(myMaster.eglSurface != EGL_NO_SURFACE) {
                    eglDestroySurface(myMaster.hRC->getDisplay(), myMaster.eglSurface);
                    myMaster.eglSurface = EGL_NO_SURFACE;
                }
            }
            myMaster.hWindowGl = NULL;
            return;
        }
        case StAndroidGlue::CommandId_FocusGained: {
            /*if(myAccelerometerSensor != NULL) {
                ASensorEventQueue_enableSensor(mySensorEventQueue, myAccelerometerSensor);
                ASensorEventQueue_setEventRate(mySensorEventQueue, myAccelerometerSensor, (1000L / 60) * 1000);
            }*/
            return;
        }
        case StAndroidGlue::CommandId_FocusLost: {
            /*if(myAccelerometerSensor != NULL) {
                ASensorEventQueue_disableSensor(mySensorEventQueue, myAccelerometerSensor);
            }*/
            return;
        }
        case StAndroidGlue::CommandId_ConfigChanged: {
            // do not handle resize event here - screen might be not yet resized
            updateMonitors();

            myStEvent.Type  = stEvent_NewMonitor;
            myStEvent.Size.Time  = getEventTime();
            myStEvent.Size.SizeX = myRectNorm.width();
            myStEvent.Size.SizeY = myRectNorm.height();
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
    if(myParentWin->popOpenNewFile(aDndFile)) {
        myStEvent.Type = stEvent_FileDrop;
        myStEvent.DNDrop.Time = getEventTime();
        myStEvent.DNDrop.File = aDndFile.toCString();
        myEventsBuffer.append(myStEvent);
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

        /*if(aPollRes == LooperId_USER) {
            if(anEngine.accelerometerSensor != NULL) {
                ASensorEvent anEvent;
                while(ASensorEventQueue_getEvents(anEngine.sensorEventQueue, &anEvent, 1) > 0) {
                    anEvent.acceleration.x, anEvent.acceleration.y, anEvent.acceleration.z);
                }
            }
        }*/

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
    if(myMousePt.x() >= 0.0 && myMousePt.x() <= 1.0 && myMousePt.y() >= 0.0 && myMousePt.y() <= 1.0) {
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

#endif
