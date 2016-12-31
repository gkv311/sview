/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2016
 */

#include "StCADViewer.h"

#include "StCADViewerGUI.h"
#include "StCADViewerStrings.h"
#include "StCADLoader.h"
#include "StCADWindow.h"
#include "StCADFrameBuffer.h"
#include "StCADMsgPrinter.h"

#include <AIS_ConnectedInteractive.hxx>
#include <AIS_Shape.hxx>
#include <Message_Messenger.hxx>
#include <Message_PrinterOStream.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_DirectionalLight.hxx>
#include <V3d_AmbientLight.hxx>

#include <StGLCore/StGLCore20.h>
#include <StGLWidgets/StGLMsgStack.h>

#include <StSettings/StSettings.h>

#include "../StOutAnaglyph/StOutAnaglyph.h"
#include "../StOutDual/StOutDual.h"
#include "../StOutIZ3D/StOutIZ3D.h"
#include "../StOutInterlace/StOutInterlace.h"
#include "../StOutPageFlip/StOutPageFlipExt.h"
#include "../StOutDistorted/StOutDistorted.h"

#ifdef __ANDROID__
    #include <EGL/egl.h>
#endif

namespace {
    static const char ST_SETTING_LAST_FOLDER[] = "lastFolder";
    static const char ST_SETTING_FPSTARGET[] = "fpsTarget";
}

const StString StCADViewer::ST_DRAWER_PLUGIN_NAME = "StCADViewer";

void StCADViewer::updateStrings() {
    using namespace StCADViewerStrings;
    params.IsFullscreen->setName(tr(MENU_VIEW_FULLSCREEN));
    params.ToShowPlayList->setName(stCString("Show Playlist"));
    params.ToShowFps->setName(tr(MENU_SHOW_FPS));
    params.ToShowTrihedron->setName(tr(MENU_VIEW_TRIHEDRON));
    params.ProjectMode->setName(tr(MENU_VIEW_PROJECTION));
    params.ProjectMode->defineOption(ST_PROJ_ORTHO,  tr(MENU_VIEW_PROJ_ORTHO));
    params.ProjectMode->defineOption(ST_PROJ_PERSP,  tr(MENU_VIEW_PROJ_PERSP));
    params.ProjectMode->defineOption(ST_PROJ_STEREO, tr(MENU_VIEW_PROJ_STEREO));
    myLangMap->params.language->setName(tr(MENU_HELP_LANGS));
}

StCADViewer::StCADViewer(const StHandle<StResourceManager>& theResMgr,
                         const StNativeWin_t                theParentWin,
                         const StHandle<StOpenInfo>&        theOpenInfo)
: StApplication(theResMgr, theParentWin, theOpenInfo),
  myPlayList(new StPlayList(1, false)),
  myIsLeftHold(false),
  myIsRightHold(false),
  myIsMiddleHold(false),
  myIsCtrlPressed(false) {
    mySettings = new StSettings(myResMgr, ST_DRAWER_PLUGIN_NAME);
    myLangMap  = new StTranslations(myResMgr, ST_DRAWER_PLUGIN_NAME);
    StCADViewerStrings::loadDefaults(*myLangMap);
    myLangMap->params.language->signals.onChanged += stSlot(this, &StCADViewer::doChangeLanguage);

    myTitle = "sView - CAD Viewer";
    //
    params.IsFullscreen    = new StBoolParamNamed(false, stCString("isFullscreen"));
    params.IsFullscreen->signals.onChanged.connect(this, &StCADViewer::doFullscreen);
    params.ToShowPlayList  = new StBoolParamNamed(false, stCString("showPlaylist"));
    params.ToShowFps       = new StBoolParamNamed(false, stCString("toShowFps"));
    params.ToShowTrihedron = new StBoolParamNamed(true,  stCString("showTrihedron"));
    params.ProjectMode = new StEnumParam(ST_PROJ_STEREO, stCString("projMode"));
    params.ProjectMode->signals.onChanged.connect(this, &StCADViewer::doChangeProjection);

    params.ZFocus = new StFloat32Param(1.0f,       // current value
                                       0.2f, 2.0f, // min/max
                                       1.0f,       // default
                                       0.025f,     // step
                                       0.001f);
    params.StereoIOD = new StFloat32Param(0.05f,       // current value
                                          0.01f, 0.3f, // min/max
                                          0.05f,       // default
                                          0.005f,      // step
                                          0.0001f);

    params.TargetFps = 0;
    updateStrings();

    mySettings->loadString(ST_SETTING_LAST_FOLDER, params.LastFolder);
    mySettings->loadInt32 (ST_SETTING_FPSTARGET,   params.TargetFps);
    mySettings->loadParam (params.ToShowFps);
    mySettings->loadParam (params.ToShowPlayList);

    // workaround current limitations of OCCT - no support of viewport with offset
    const bool toForceFboUsage = true;
#if defined(__ANDROID__)
    addRenderer(new StOutInterlace  (myResMgr, theParentWin));
    addRenderer(new StOutAnaglyph   (myResMgr, theParentWin));
    StOutDistorted* aDistOut = new StOutDistorted  (myResMgr, theParentWin);
    aDistOut->setForcedFboUsage(toForceFboUsage);
    addRenderer(aDistOut);
#else
    addRenderer(new StOutAnaglyph   (myResMgr, theParentWin));
    addRenderer(new StOutDual       (myResMgr, theParentWin));
    addRenderer(new StOutIZ3D       (myResMgr, theParentWin));
    addRenderer(new StOutInterlace  (myResMgr, theParentWin));
    StOutDistorted* aDistOut = new StOutDistorted  (myResMgr, theParentWin);
    aDistOut->setForcedFboUsage(toForceFboUsage);
    addRenderer(aDistOut);
    addRenderer(new StOutPageFlipExt(myResMgr, theParentWin));
#endif

    // need Depth buffer
    const StWinAttr anAttribs[] = {
        StWinAttr_GlDepthSize,   (StWinAttr )24,
        StWinAttr_GlStencilSize, (StWinAttr )8,
        StWinAttr_NULL
    };
    for(size_t aRendIter = 0; aRendIter < myRenderers.size(); ++aRendIter) {
        StHandle<StWindow>& aRend = myRenderers[aRendIter];
        aRend->setAttributes(anAttribs);
    }

    // create actions
    StHandle<StAction> anAction;
    anAction = new StActionBool(stCString("DoFullscreen"), params.IsFullscreen);
    addAction(Action_Fullscreen, anAction, ST_VK_RETURN);

    anAction = new StActionBool(stCString("DoShowFPS"), params.ToShowFps);
    addAction(Action_ShowFps, anAction, ST_VK_F12);

    anAction = new StActionIntSlot(stCString("DoListFirst"), stSlot(this, &StCADViewer::doListFirst), 0);
    addAction(Action_ListFirst, anAction, ST_VK_HOME);

    anAction = new StActionIntSlot(stCString("DoListLast"), stSlot(this, &StCADViewer::doListLast), 0);
    addAction(Action_ListLast, anAction, ST_VK_END);

    anAction = new StActionIntSlot(stCString("DoListPrev"), stSlot(this, &StCADViewer::doListPrev), 0);
    addAction(Action_ListPrev, anAction, ST_VK_PRIOR);

    anAction = new StActionIntSlot(stCString("DoListNext"), stSlot(this, &StCADViewer::doListNext), 0);
    addAction(Action_ListNext, anAction, ST_VK_NEXT);

    anAction = new StActionIntSlot(stCString("DoFitAll"), stSlot(this, &StCADViewer::doFitAll), 0);
    addAction(Action_FitAll, anAction, ST_VK_F);

    anAction = new StActionIntValue(stCString("DoProjOrthogonal"),  params.ProjectMode, ST_PROJ_ORTHO);
    addAction(Action_ProjOrthogonal, anAction, ST_VK_O);

    anAction = new StActionIntValue(stCString("DoProjPerspective"), params.ProjectMode, ST_PROJ_PERSP);
    addAction(Action_ProjPerspective, anAction, ST_VK_M, ST_VK_P);

    anAction = new StActionIntValue(stCString("DoProjStereo"),      params.ProjectMode, ST_PROJ_STEREO);
    addAction(Action_ProjStereo, anAction, ST_VK_S);

    anAction = new StActionHoldSlot(stCString("DoZoomIn"),  stSlot(this, &StCADViewer::doZoomIn));
    addAction(Action_ZoomIn,  anAction, ST_VK_OEM_PLUS,  ST_VK_ADD);

    anAction = new StActionHoldSlot(stCString("DoZoomOut"), stSlot(this, &StCADViewer::doZoomOut));
    addAction(Action_ZoomOut, anAction, ST_VK_OEM_MINUS, ST_VK_SUBTRACT);

    anAction = new StActionHoldSlot(stCString("DoStereoZFocusCloser"),  stSlot(this, &StCADViewer::doStereoZFocusCloser));
    addAction(Action_StereoZFocusCloser, anAction, ST_VK_DIVIDE | ST_VF_CONTROL);

    anAction = new StActionHoldSlot(stCString("DoStereoZFocusFarther"), stSlot(this, &StCADViewer::doStereoZFocusFarther));
    addAction(Action_StereoZFocusFarther, anAction, ST_VK_MULTIPLY | ST_VF_CONTROL);

    anAction = new StActionHoldSlot(stCString("DoStereoIODDec"),  stSlot(this, &StCADViewer::doStereoIODDec));
    addAction(Action_StereoIODDec, anAction, ST_VK_DIVIDE);

    anAction = new StActionHoldSlot(stCString("DoStereoIODInc"), stSlot(this, &StCADViewer::doStereoIODInc));
    addAction(Action_StereoIODInc, anAction, ST_VK_MULTIPLY);
}

bool StCADViewer::resetDevice() {
    if(myGUI.isNull()
    || myCADLoader.isNull()) {
        return init();
    }

    // be sure Render plugin process quit correctly
    myWindow->beforeClose();

    releaseDevice();
    myWindow->close();
    myWindow.nullify();
    return open();
}

void StCADViewer::saveGuiParams() {
    if(myGUI.isNull()) {
        return;
    }

    mySettings->saveString(ST_SETTING_LAST_FOLDER, params.LastFolder);
    mySettings->saveParam(params.ToShowTrihedron);
    mySettings->saveParam(params.ProjectMode);
    mySettings->saveInt32(ST_SETTING_FPSTARGET, params.TargetFps);
    mySettings->saveParam(params.ToShowFps);
    mySettings->saveParam(params.ToShowPlayList);
}

void StCADViewer::saveAllParams() {
    saveGuiParams();
    if(!myGUI.isNull()) {
        // store hot-keys
        for(std::map< int, StHandle<StAction> >::iterator anIter = myActions.begin();
            anIter != myActions.end(); ++anIter) {
            mySettings->saveHotKey(anIter->second);
        }
    }

    mySettings->flush();
}

void StCADViewer::releaseDevice() {
    saveAllParams();

    // release GUI data and GL resources before closing the window
    myGUI.nullify();
    myContext.nullify();
    myAisContext.Nullify();
    myView.Nullify();
    myViewer.Nullify();
}

StCADViewer::~StCADViewer() {
    releaseDevice();
    // wait working threads to quit and release resources
    myCADLoader.nullify();
}

bool StCADViewer::initOcctViewer() {
    Message::DefaultMessenger()->RemovePrinters(STANDARD_TYPE(StCADMsgPrinter));
    Message::DefaultMessenger()->RemovePrinters(STANDARD_TYPE(Message_PrinterOStream));
    Handle(StCADMsgPrinter) aPrinter = new StCADMsgPrinter(myMsgQueue);
    Message::DefaultMessenger()->AddPrinter(aPrinter);

#ifdef __ANDROID__
    int aWidth = 2, aHeight = 2;
    EGLint aCfgId = 0;
    EGLDisplay anEglDisplay = eglGetCurrentDisplay();
    EGLContext anEglContext = eglGetCurrentContext();
    EGLSurface anEglSurf    = eglGetCurrentSurface(EGL_DRAW);
    if(anEglDisplay == EGL_NO_DISPLAY
    || anEglContext == EGL_NO_CONTEXT
    || anEglSurf    == EGL_NO_SURFACE) {
        myMsgQueue->pushError(stCString("Critical error:\nNo active EGL context!"));
        return false;
    }

    eglQuerySurface(anEglDisplay, anEglSurf, EGL_WIDTH,     &aWidth);
    eglQuerySurface(anEglDisplay, anEglSurf, EGL_HEIGHT,    &aHeight);
    eglQuerySurface(anEglDisplay, anEglSurf, EGL_CONFIG_ID, &aCfgId);

    const EGLint aConfigAttribs[] = { EGL_CONFIG_ID, aCfgId, EGL_NONE };
    EGLint       aNbConfigs = 0;
    void*        anEglConfig = NULL;

    if(eglChooseConfig(anEglDisplay, aConfigAttribs, &anEglConfig, 1, &aNbConfigs) != EGL_TRUE) {
        myMsgQueue->pushError(stCString("Critical error:\nEGL does not provide compatible configurations!"));
        return false;
    }

    if(!myViewer.IsNull()) {
        Handle(OpenGl_GraphicDriver) aDriver = Handle(OpenGl_GraphicDriver)::DownCast(myViewer->Driver());
        Handle(StCADWindow)          aWindow = Handle(StCADWindow)::DownCast(myView->Window());
        if(!aDriver->InitEglContext(anEglDisplay, anEglContext, anEglConfig)) {
            myMsgQueue->pushError(stCString("Critical error:\nOpenGl_GraphicDriver can not be initialized!"));
            return false;
        }

        aWindow->SetSize(aWidth, aHeight);
        myView->SetWindow(aWindow, (Aspect_RenderingContext )anEglContext);
        return true;
    }

#elif defined(_WIN32)
    HWND  aWinHandle = (HWND  )myWindow->getNativeOglWin();
    HDC   aWindowDC  = (HDC   )myWindow->getNativeOglDC();
    HGLRC aRendCtx   = (HGLRC )myWindow->getNativeOglRC();
    if(aWinHandle == NULL
    || aWindowDC  == NULL
    || aRendCtx   == NULL) {
        myMsgQueue->pushError(stCString("Critical error:\nNo active WGL context!"));
        return false;
    }

    if(!myViewer.IsNull()) {
        Handle(StCADWindow) aWindow = new StCADWindow(aWinHandle);
        myView->SetWindow(aWindow, (Aspect_RenderingContext )aRendCtx);
        return true;
    }
#endif

    Handle(OpenGl_GraphicDriver) aDriver = new OpenGl_GraphicDriver(NULL, Standard_False);
    aDriver->ChangeOptions().ffpEnable     = Standard_False;
    aDriver->ChangeOptions().buffersNoSwap = Standard_True;
#ifdef ST_DEBUG_GL
    aDriver->ChangeOptions().contextDebug  = Standard_True;
#else
    aDriver->ChangeOptions().contextDebug  = Standard_False;
#endif
#ifdef ST_DEBUG_SHADERS
    aDriver->ChangeOptions().glslWarnings  = Standard_True;
#else
    aDriver->ChangeOptions().glslWarnings  = Standard_False;
#endif

#ifdef __ANDROID__
    if(!aDriver->InitEglContext(anEglDisplay, anEglContext, anEglConfig)) {
        myMsgQueue->pushError(stCString("Critical error:\nOpenGl_GraphicDriver can not be initialized!!"));
        return false;
    }
#endif

    myViewer = new V3d_Viewer(aDriver);
    myViewer->SetDefaultBackgroundColor(Quantity_NOC_BLACK);
    Handle(V3d_DirectionalLight) aLightDir = new V3d_DirectionalLight(myViewer, V3d_Zneg, Quantity_NOC_WHITE, Standard_True);
    Handle(V3d_AmbientLight)     aLightAmb = new V3d_AmbientLight(myViewer);
    aLightDir->SetDirection ( 1.0, -2.0, -10.0);
    myViewer->SetLightOn (aLightDir);
    myViewer->SetLightOn (aLightAmb);

    myAisContext = new AIS_InteractiveContext(myViewer);
    myAisContext->SetDisplayMode(0);
    myAisContext->SetAutoActivateSelection(Standard_False);
    const Handle(Prs3d_Drawer)& aDrawer = myAisContext->DefaultDrawer();
    aDrawer->SetAutoTriangulation (Standard_False);
#ifdef __ANDROID__
    Handle(StCADWindow) aWindow = new StCADWindow();
    aWindow->SetSize(aWidth, aHeight);
#elif defined(_WIN32)
    Handle(StCADWindow) aWindow = new StCADWindow(aWinHandle);
#endif

    myView = myViewer->CreateView();
    myView->Camera()->SetProjectionType(myProjection.isPerspective()
                                      ? Graphic3d_Camera::Projection_Perspective
                                      : Graphic3d_Camera::Projection_Orthographic);
    myView->SetImmediateUpdate(Standard_False);
    //
    myView->View()->SetImmediateModeDrawToFront(Standard_False);

#ifdef __ANDROID__
    myView->SetWindow(aWindow, (Aspect_RenderingContext )anEglContext);
#else
    myView->SetWindow(aWindow, (Aspect_RenderingContext )aRendCtx);
#endif

    return true;
}

bool StCADViewer::createGui() {
    if(!myGUI.isNull()) {
        saveGuiParams();
        myGUI.nullify();
        myKeyActions.clear();
    }

    // create the GUI with default values
    //params.ScaleHiDPI->setValue(myWindow->getScaleFactor());
    myGUI = new StCADViewerGUI(this, myLangMap.access(), myPlayList);
    myGUI->setContext(myContext);

    // load settings
    myWindow->setTargetFps(double(params.TargetFps));
    mySettings->loadParam(params.ToShowTrihedron);
    mySettings->loadParam(params.ProjectMode);

    myGUI->stglInit();
    myGUI->stglResize(myWindow->stglViewport(ST_WIN_MASTER));

    registerHotKeys();
    return true;
}

void StCADViewer::doChangeLanguage(const int32_t theNewLang) {
    StApplication::doChangeLanguage(theNewLang);
    StCADViewerStrings::loadDefaults(*myLangMap);
    updateStrings();
}

bool StCADViewer::init() {
    const bool isReset = !myCADLoader.isNull();
    if(!myContext.isNull()
    && !myGUI.isNull()) {
        return true;
    }

    // initialize GL context
    myContext = myWindow->getContext();
    myContext->setMessagesQueue(myMsgQueue);
    if(!myContext->isGlGreaterEqual(2, 0)) {
        myMsgQueue->pushError(stCString("OpenGL 2.0 is required by CAD Viewer!"));
        myMsgQueue->popAll();
        return false;
    }

    myWindow->setTargetFps(double(params.TargetFps));
    myWindow->setStereoOutput(params.ProjectMode->getValue() == ST_PROJ_STEREO);

    if(!initOcctViewer()) {
        //
    }

    // load hot-keys
    if(!isReset) {
        for(std::map< int, StHandle<StAction> >::iterator anIter = myActions.begin();
            anIter != myActions.end(); ++anIter) {
            mySettings->loadHotKey(anIter->second);
        }
    }

    // create the GUI with default values
    if(!createGui()) {
        myMsgQueue->pushError(stCString("CAD Viewer - GUI initialization failed!"));
        myMsgQueue->popAll();
        myGUI.nullify();
        return false;
    }

    myGUI->stglResize(myWindow->stglViewport(ST_WIN_MASTER));

    // create working threads
    if(!isReset) {
        myCADLoader = new StCADLoader(myLangMap, myPlayList);
        myCADLoader->signals.onError = stSlot(myMsgQueue.access(), &StMsgQueue::doPushError);
    }

    if(isReset) {
        if(params.IsFullscreen->getValue()) {
            myWindow->setFullScreen(true);
        }
    }

    return true;
}

bool StCADViewer::open() {
    const bool isReset = !mySwitchTo.isNull();
    if(!StApplication::open()
    || !init()) {
        myMsgQueue->popAll();
        return false;
    }

    if(isReset) {
        myCADLoader->doLoadNext();
        return true;
    }

    //parseArguments(myOpenFileInfo.getArgumentsMap());
    const StMIME anOpenMIME = myOpenFileInfo->getMIME();
    if(myOpenFileInfo->getPath().isEmpty()) {
        // open drawer without files
        return true;
    }

    // clear playlist first
    myPlayList->clear();

    if(!anOpenMIME.isEmpty()) {
        // create just one-file playlist
        myPlayList->addOneFile(myOpenFileInfo->getPath(), anOpenMIME);
    } else {
        // create playlist from file's folder
        myPlayList->open(myOpenFileInfo->getPath());
    }

    if(!myPlayList->isEmpty()) {
        doUpdateStateLoading();
        myCADLoader->doLoadNext();
    }

    return true;
}

void StCADViewer::doPause(const StPauseEvent& theEvent) {
    StApplication::doPause(theEvent);
    saveAllParams();
}

void StCADViewer::doResize(const StSizeEvent& ) {
    if(myGUI.isNull()) {
        return;
    }

    const StGLBoxPx aWinRect = myWindow->stglViewport(ST_WIN_MASTER);
    myGUI->stglResize(aWinRect);
    myProjection.resize(aWinRect.width(), aWinRect.height());
}

void StCADViewer::doMouseDown(const StClickEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    bool isItemClicked = false;
    myGUI->tryClick(theEvent, isItemClicked);
    if(isItemClicked) {
        return;
    }

    if(theEvent.Button == ST_MOUSE_LEFT) {
        myIsLeftHold = true;
        myPrevMouse.x() = theEvent.PointX;
        myPrevMouse.y() = theEvent.PointY;
        if(!myIsCtrlPressed && !myView.IsNull()) {
            StRectI_t aWinRect = myWindow->getPlacement();
            myView->StartRotation(int(double(aWinRect.width())  * theEvent.PointX),
                                  int(double(aWinRect.height()) * theEvent.PointY));
        }
    } else if(theEvent.Button == ST_MOUSE_RIGHT) {
        myIsRightHold = true;
        myPrevMouse.x() = theEvent.PointX;
        myPrevMouse.y() = theEvent.PointY;
        if(myIsCtrlPressed && !myView.IsNull()) {
            StRectI_t aWinRect = myWindow->getPlacement();
            myView->StartRotation(int(double(aWinRect.width())  * theEvent.PointX),
                                  int(double(aWinRect.height()) * theEvent.PointY));
        }
    } else if(theEvent.Button == ST_MOUSE_MIDDLE) {
        myIsMiddleHold = true;
        myPrevMouse.x() = theEvent.PointX;
        myPrevMouse.y() = theEvent.PointY;
    }
}

void StCADViewer::doMouseUp(const StClickEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    bool isItemUnclicked = false;
    myGUI->tryUnClick(theEvent, isItemUnclicked);
    switch(theEvent.Button) {
        case ST_MOUSE_LEFT: {
            myIsLeftHold = false;
            break;
        }
        case ST_MOUSE_RIGHT: {
            myIsRightHold = false;
            break;
        }
        case ST_MOUSE_MIDDLE: {
            if(!myIsCtrlPressed && !isItemUnclicked) {
                //params.IsFullscreen->reverse();
            }
            myIsMiddleHold = false;
            break;
        }
        default: break;
    }
}

void StCADViewer::doGesture(const StGestureEvent& theEvent) {
    if(!myGUI.isNull()
    &&  myGUI->getFocus() != NULL) {
        return;
    }

    StRectI_t aWinRect = myWindow->getPlacement();
    switch(theEvent.Type) {
        case stEvent_GestureCancel: {
            return;
        }
        case stEvent_Gesture1DoubleTap: {
            doFitAll();
            return;
        }
        case stEvent_Gesture2Rotate: {
            return;
        }
        case stEvent_Gesture2Move: {
            if(!theEvent.OnScreen) {
                // this gesture conflicts with scrolling on OS X
                return;
            }
            StPointD_t aPntFrom(theEvent.Point1X, theEvent.Point1Y);
            StPointD_t aPntTo  (theEvent.Point2X, theEvent.Point2Y);
            int aDeltaX =  int(double(aWinRect.width())  * (theEvent.Point2X - theEvent.Point1X));
            int aDeltaY = -int(double(aWinRect.height()) * (theEvent.Point2Y - theEvent.Point1Y));
            if(!myView.IsNull()) {
                myView->Pan(aDeltaX, aDeltaY);
            }
            return;
        }
        case stEvent_Gesture2Pinch: {
            StPointD_t aCursor((theEvent.Point1X + theEvent.Point2X) * 0.5,
                               (theEvent.Point1Y + theEvent.Point2Y) * 0.5);
            if(!theEvent.OnScreen) {
                aCursor = myGUI->getCursorZo();
            }
            if(!myView.IsNull()) {
                int aDeltaX = int(theEvent.Value);
                //myView->StartZoomAtPoint(int(aCursor.x() * double(aWinRect.width())),
                //                         int(aCursor.y() * double(aWinRect.height())));
                //myView->ZoomAtPoint(0, 0, aDeltaX, aDeltaX);
                myView->Zoom(0, 0, aDeltaX, aDeltaX);
            }
            return;
        }
        default: {
            return;
        }
    }
}

void StCADViewer::doScroll(const StScrollEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }
    if(myGUI->doScroll(theEvent)) {
        return;
    }

    const StPointD_t aCursor(theEvent.PointX, theEvent.PointY);
    if(theEvent.StepsY >= 1) {
        if(myIsCtrlPressed) {
            doStereoZFocusCloser(0.05);
        } else {
            scaleAt(aCursor, 10);
        }
    } else if(theEvent.StepsY <= -1) {
        if(myIsCtrlPressed) {
            doStereoZFocusFarther(0.05);
        } else {
            scaleAt(aCursor, -10);
        }
    }
}

void StCADViewer::doKeyDown(const StKeyEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    if(myGUI->getFocus() != NULL) {
        myGUI->doKeyDown(theEvent);
        return;
    }

    StApplication::doKeyDown(theEvent);
    switch(theEvent.VKey) {
        case ST_VK_ESCAPE:
            StApplication::exit(0);
            return;

        case ST_VK_LEFT:
            ///myCam.rotateX(-1.0f);
            return;
        case ST_VK_RIGHT:
            ///myCam.rotateX(1.0f);
            return;
        case ST_VK_UP:
            ///myCam.rotateY(-1.0f);
            return;
        case ST_VK_DOWN:
            ///myCam.rotateY(1.0f);
            return;
        case ST_VK_Q:
            ///myCam.rotateZ(-1.0f);
            return;
        case ST_VK_W:
            ///myCam.rotateZ(1.0f);
            return;

        default:
            break;
    }
}

void StCADViewer::doKeyHold(const StKeyEvent& theEvent) {
    if(myGUI.isNull()) {
        return;
    }

    if(myGUI->getFocus() != NULL) {
        myGUI->doKeyHold(theEvent);
    } else {
        StApplication::doKeyHold(theEvent);
    }
}

void StCADViewer::doKeyUp(const StKeyEvent& theEvent) {
    if(!myGUI.isNull()
    && myGUI->getFocus() != NULL) {
        myGUI->doKeyUp(theEvent);
    }
}

void StCADViewer::doOpen1FileFromGui(StHandle<StString> thePath) {
    if(thePath.isNull()) {
        return;
    }

    StDNDropEvent anEvent;
    anEvent.Type = stEvent_FileDrop;
    anEvent.Time = 0.0;
    const char* aFiles[1] = { thePath->toCString() };
    anEvent.NbFiles = 1;
    anEvent.Files   = aFiles;
    doFileDrop(anEvent);

    StString aDummy;
    StFileNode::getFolderAndFile(*thePath, params.LastFolder, aDummy);
}

void StCADViewer::doFileDrop(const StDNDropEvent& theEvent) {
    if(theEvent.NbFiles == 0) {
        return;
    }

    const StString aFilePath = theEvent.Files[0];
    //if(myPlayList->checkExtension(aFilePath)) {
        myPlayList->open(aFilePath);
        doUpdateStateLoading();
        myCADLoader->doLoadNext();
    //}
}

void StCADViewer::doNavigate(const StNavigEvent& theEvent) {
    switch(theEvent.Target) {
        case stNavigate_Backward: doListPrev(); break;
        case stNavigate_Forward:  doListNext(); break;
        default: break;
    }
}

void StCADViewer::beforeDraw() {
    if(myGUI.isNull()) {
        return;
    }

    myIsCtrlPressed = myWindow->getKeysState().isKeyDown(ST_VK_CONTROL);
    Handle(Graphic3d_Camera) aCam = !myView.IsNull()
                                  ?  myView->Camera()
                                  : Handle(Graphic3d_Camera)();
    //if(myIsMiddleHold && myIsCtrlPressed && !aCam.IsNull()) {
    if(myIsMiddleHold && !aCam.IsNull()) {
        // move
        StPointD_t aPt = myWindow->getMousePos();
        gp_Vec2d aFlatMove( 2.0 * (aPt.x() - myPrevMouse.x()),
                           -2.0 * (aPt.y() - myPrevMouse.y()));

        const gp_Dir aSide    = aCam->Direction().Crossed(aCam->Up());
        const gp_Pnt aViewDim = aCam->ViewDimensions();
        const gp_Vec aMoveSide = gp_Vec(aSide)      * 0.5 * aFlatMove.X() * aViewDim.X();
        const gp_Vec aMoveUp   = gp_Vec(aCam->Up()) * 0.5 * aFlatMove.Y() * aViewDim.Y();

        gp_Pnt aCenter = aCam->Center();
        gp_Pnt anEye   = aCam->Eye();
        aCenter.Translate(-aMoveSide);
        anEye  .Translate(-aMoveSide);
        aCenter.Translate(-aMoveUp);
        anEye  .Translate(-aMoveUp);

        aCam->SetCenter(aCenter);
        aCam->SetEye(anEye);

        myPrevMouse = aPt;
    }
    if((myIsRightHold &&  myIsCtrlPressed)
    || (myIsLeftHold  && !myIsCtrlPressed)) {
        const StPointD_t aPt = myWindow->getMousePos();
        StRectI_t aWinRect = myWindow->getPlacement();
        myView->Rotation(int(double(aWinRect.width())  * aPt.x()),
                         int(double(aWinRect.height()) * aPt.y()));
    }

    if(!myAisContext.IsNull()) {
        NCollection_Sequence<Handle(AIS_InteractiveObject)> aNewPrsList;
        if(myCADLoader->getNextDoc(aNewPrsList, myDoc)) {
            myAisContext->RemoveAll(false);
            for(NCollection_Sequence<Handle(AIS_InteractiveObject)>::Iterator aPrsIter(aNewPrsList); aPrsIter.More(); aPrsIter.Next()) {
                myAisContext->Display(aPrsIter.Value(), aPrsIter.Value()->DisplayMode(), -1, false);
            }

            doFitAll();
            doUpdateStateLoaded(!aNewPrsList.IsEmpty());
        }
    }

    myGUI->setVisibility(myWindow->getMousePos(), true);
    myGUI->stglUpdate(myWindow->getMousePos());

    // recreate menu event
    if(myToRecreateMenu) {
        createGui();
        myToRecreateMenu = false;
    }
}

void StCADViewer::stglDraw(unsigned int theView) {
    if(myGUI.isNull()) {
        return;
    }

    if(!myView.IsNull()) {
        Graphic3d_Camera::Projection aProj = Graphic3d_Camera::Projection_Orthographic;
        if(myProjection.isPerspective()) {
            switch(theView) {
                case ST_DRAW_LEFT:  aProj = Graphic3d_Camera::Projection_MonoLeftEye;  break;
                case ST_DRAW_RIGHT: aProj = Graphic3d_Camera::Projection_MonoRightEye; break;
                case ST_DRAW_MONO:  aProj = Graphic3d_Camera::Projection_Perspective;  break;
            }
        }

        if(params.ToShowTrihedron->getValue()) {
          myView->TriedronDisplay(Aspect_TOTP_RIGHT_LOWER, Quantity_NOC_WHITE, 0.1, V3d_ZBUFFER);
        } else {
          myView->TriedronErase();
        }

        // Do the magic:
        // - define default FBO for OCCT from StGLContext
        // - resize virtual window without OCCT viewer redraw
        // - copy viewport restore it back
        // What does not handled:
        // - Dual Output, OCCT makes OpenGL context always on master window
        // - scissor test is likely incorrectly applied
        // - MSAA blitting might use incorrect viewport
        Handle(OpenGl_GraphicDriver) aDriver = Handle(OpenGl_GraphicDriver)::DownCast(myViewer->Driver());
        Handle(OpenGl_Context) aCtx = aDriver->GetSharedContext();
        Handle(StCADFrameBuffer) anFboWrapper = Handle(StCADFrameBuffer)::DownCast(aCtx->DefaultFrameBuffer());
        if(anFboWrapper.IsNull()) {
            anFboWrapper = new StCADFrameBuffer();
        }
        anFboWrapper->wrapFbo(*myContext);
        aCtx->SetDefaultFrameBuffer(anFboWrapper);
        myContext->stglBindFramebuffer(0);

        Handle(StCADWindow) aWindow = Handle(StCADWindow)::DownCast(myView->Window());
        if(anFboWrapper->GetVPSizeX() > 0
        && anFboWrapper->GetVPSizeY() > 0
        && aWindow->SetSize(anFboWrapper->GetVPSizeX(), anFboWrapper->GetVPSizeY())) {
            StRectI_t aWinRect = myWindow->getPlacement();
            Standard_Real aRatio = double(aWinRect.width()) / double(aWinRect.height());
            myView->Camera()->SetAspect(aRatio);
            myView->View()->Resized();
        }

        StGLBoxPx aVPort = myContext->stglViewport();
        StGLBoxPx aScissRect;
        bool toSetScissorRect = myContext->stglScissorRect(aScissRect);

        myView->Camera()->SetProjectionType(aProj);
        myView->Camera()->SetZFocus(Graphic3d_Camera::FocusType_Relative, params.ZFocus->getValue());
        myView->Camera()->SetIOD   (Graphic3d_Camera::IODType_Relative,   params.StereoIOD->getValue());

        myView->Redraw();

        myContext->stglResizeViewport(aVPort);
        if(toSetScissorRect) {
            myContext->stglSetScissorRect(aScissRect, false);
        }
    } else if(!myContext.isNull()
            && myContext->core20fwd != NULL) {
        // clear the screen and the depth buffer
        myContext->core11fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    myGUI->getCamera()->setView(theView);
    myProjection.setView(theView);

    // draw GUI
    myContext->core11fwd->glDisable(GL_DEPTH_TEST);
    myGUI->stglDraw(theView);
}

void StCADViewer::doUpdateStateLoading() {
    const StString aFileToLoad = myPlayList->getCurrentTitle();
    if(aFileToLoad.isEmpty()) {
        myWindow->setTitle("sView - CAD Viewer");
    } else {
        myWindow->setTitle(aFileToLoad + " Loading... - sView");
    }
}

void StCADViewer::doUpdateStateLoaded(bool isSuccess) {
    const StString aFileLoaded = myPlayList->getCurrentTitle();
    if(aFileLoaded.isEmpty()) {
        myWindow->setTitle("sView - CAD Viewer");
    } else {
        myWindow->setTitle(aFileLoaded + (isSuccess ? StString() : StString(" FAIL to open")) + " - sView");
    }
}

void StCADViewer::doFullscreen(const bool theIsFullscreen) {
    if(!myWindow.isNull()) {
        myWindow->setFullScreen(theIsFullscreen);
    }
}

void StCADViewer::doListFirst(const size_t ) {
    if(myPlayList->walkToFirst()) {
        myCADLoader->doLoadNext();
        doUpdateStateLoading();
    }
}

void StCADViewer::doListPrev(const size_t ) {
    if(myPlayList->walkToPrev()) {
        myCADLoader->doLoadNext();
        doUpdateStateLoading();
    }
}

void StCADViewer::doListNext(const size_t ) {
    if(myPlayList->walkToNext()) {
        myCADLoader->doLoadNext();
        doUpdateStateLoading();
    }
}

void StCADViewer::doListLast(const size_t ) {
    if(myPlayList->walkToLast()) {
        myCADLoader->doLoadNext();
        doUpdateStateLoading();
    }
}

void StCADViewer::doFileNext() {
    if(myCADLoader.isNull()) {
        return;
    }

    myCADLoader->doLoadNext();
    doUpdateStateLoading();
}

void StCADViewer::doFitAll(const size_t ) {
    if(!myView.IsNull()) {
        myView->FitAll(0.01, Standard_False);
    }
}

void StCADViewer::doChangeProjection(const int32_t theProj) {
    if(myWindow.isNull()) {
        return;
    }

    switch(theProj) {
        case ST_PROJ_ORTHO: {
            myWindow->setStereoOutput(false);
            myProjection.setPerspective(false);
            if(!myView.IsNull()) {
                myView->Camera()->SetProjectionType(Graphic3d_Camera::Projection_Orthographic);
            }
            break;
        }
        case ST_PROJ_PERSP: {
            myWindow->setStereoOutput(false);
            myProjection.setPerspective(true);
            if(!myView.IsNull()) {
                myView->Camera()->SetProjectionType(Graphic3d_Camera::Projection_Perspective);
            }
            break;
        }
        case ST_PROJ_STEREO: {
            myWindow->setStereoOutput(true);
            myProjection.setPerspective(true);
            if(!myView.IsNull()) {
                myView->Camera()->SetProjectionType(Graphic3d_Camera::Projection_Perspective);
            }
            break;
        }
    }
}

void StCADViewer::scaleAt(const StPointD_t& thePoint,
                          const float       theStep) {
    if(myView.IsNull()) {
        return;
    }

    int aWinSizeX = 0;
    int aWinSizeY = 0;
    myView->Window()->Size(aWinSizeX, aWinSizeY);

    myView->StartZoomAtPoint(int(thePoint.x() * aWinSizeX),
                             int(thePoint.y() * aWinSizeY));
    myView->ZoomAtPoint(0, 0, (int )theStep, (int )theStep);
}

void StCADViewer::doZoomIn(const double theValue) {
    if(myView.IsNull()) {
        return;
    }

    myView->SetZoom(1.0 + theValue, Standard_True);
    //myProjection.setZoom(myProjection.getZoom() * 1.1f);
}

void StCADViewer::doZoomOut(const double theValue) {
    if(myView.IsNull()) {
        return;
    }

    myView->SetZoom(1.0 - theValue, Standard_True);
    //myProjection.setZoom(myProjection.getZoom() * 0.9f);
}

void StCADViewer::doStereoZFocusCloser(const double theValue) {
    if(myView.IsNull()
    || params.ProjectMode->getValue() != ST_PROJ_STEREO) {
        return;
    }

    float aFocus = params.ZFocus->getValue() - float(theValue * 0.5);
    params.ZFocus->setValue(aFocus);
}

void StCADViewer::doStereoZFocusFarther(const double theValue) {
    if(myView.IsNull()
    || params.ProjectMode->getValue() != ST_PROJ_STEREO) {
        return;
    }

    float aFocus = params.ZFocus->getValue() + float(theValue * 0.5);
    params.ZFocus->setValue(aFocus);
}

void StCADViewer::doStereoIODDec(const double theValue) {
    if(myView.IsNull()
    || params.ProjectMode->getValue() != ST_PROJ_STEREO) {
        return;
    }

    float aDist = params.StereoIOD->getValue() - float(theValue * 0.1);
    params.StereoIOD->setValue(aDist);
}

void StCADViewer::doStereoIODInc(const double theValue) {
    if(myView.IsNull()
    || params.ProjectMode->getValue() != ST_PROJ_STEREO) {
        return;
    }

    float aDist = params.StereoIOD->getValue() + float(theValue * 0.1);
    params.StereoIOD->setValue(aDist);
}
