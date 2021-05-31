/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2007-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include <StGLCore/StGLCore11Fwd.h>
#include <StGL/StGLContext.h>

#include "StWindowImpl.h"

void StWindow::copySignals() {
    params.VSyncMode = new StEnumParam(0, stCString("vsyncMode"), stCString("VSync mode"));
    params.VSyncMode->changeValues().add("Off");
    params.VSyncMode->changeValues().add("On");
    params.VSyncMode->changeValues().add("Mixed");

    myWin->signals.onClose     = &signals.onClose;
    myWin->signals.onPause     = &signals.onPause;
    myWin->signals.onResize    = &signals.onResize;
    myWin->signals.onAnotherMonitor = &signals.onAnotherMonitor;
    myWin->signals.onKeyUp     = &signals.onKeyUp;
    myWin->signals.onKeyDown   = &signals.onKeyDown;
    myWin->signals.onKeyHold   = &signals.onKeyHold;
    myWin->signals.onMouseUp   = &signals.onMouseUp;
    myWin->signals.onMouseDown = &signals.onMouseDown;
    myWin->signals.onTouch     = &signals.onTouch;
    myWin->signals.onGesture   = &signals.onGesture;
    myWin->signals.onScroll    = &signals.onScroll;
    myWin->signals.onFileDrop  = &signals.onFileDrop;
    myWin->signals.onNavigate  = &signals.onNavigate;
    myWin->signals.onAction    = &signals.onAction;
}

StWindow::StWindow()
: myWin(new StWindowImpl(new StResourceManager(), (StNativeWin_t )NULL)),
  myTargetFps(0.0),
  myWasUsed(false),
  myIsForcedStereo(false) {
    copySignals();
}

StWindow::StWindow(const StHandle<StResourceManager>& theResMgr,
                   const StNativeWin_t                theParentWindow)
: myWin(new StWindowImpl(theResMgr, theParentWindow)),
  myTargetFps(0.0),
  myWasUsed(false),
  myIsForcedStereo(false) {
    copySignals();
}

StWindow::~StWindow() {
    delete myWin;
}

StNativeWin_t StWindow::getParentWindow() const {
    return myWin->myParentWin;
}

void StWindow::close() {
    myWin->close();
}

void StWindow::beforeClose() {
    //
}

void StWindow::setTitle(const StString& theTitle) {
    myWin->setTitle(theTitle);
}

bool StWindow::hasDepthBuffer() const {
    return myWin->hasDepthBuffer();
}

void StWindow::getAttributes(StWinAttr* theAttributes) const {
    myWin->getAttributes(theAttributes);
}

void StWindow::setAttributes(const StWinAttr* theAttributes) {
    myWin->setAttributes(theAttributes);
}

bool StWindow::isActive() const {
    return myWin->isActive();
}

bool StWindow::isPaused() const {
    return myWin->myIsPaused;
}

bool StWindow::isLostDevice() const {
    return myWin->myToResetDevice;
}

const char* StWindow::getRendererId() const {
    return "StWindow";
}

const char* StWindow::getDeviceId() const {
    return "NONE";
}

StString StWindow::getRendererAbout() const {
    return "StWindow";
}

bool StWindow::setDevice(const StString& ) {
    return false;
}

void StWindow::getDevices(StOutDevicesList& ) const {
    //
}

void StWindow::getOptions(StParamsList& ) const {
    //
}

bool StWindow::isStereoFullscreenOnly() const {
    return false;
}

bool StWindow::isStereoOutput() const {
    return myIsForcedStereo || myWin->isStereoOutput();
}

void StWindow::setStereoOutput(const bool theStereoState) {
    myWin->setStereoOutput(theStereoState);
}

bool StWindow::isStereoSource() const {
    return myWin->isStereoOutput();
}

float StWindow::getMaximumTargetFps() const {
    const StSearchMonitors& aMonitors = myWin->getMonitors();;
    const StRectI_t aRect = myWin->getPlacement();
    StMonitor aMon = aMonitors[aRect.center()];
    return aMon.getFreq() >= 24
         ? aMon.getFreq()
         : 60; // fallback for unknown values
}

double StWindow::getTargetFps() const {
    return myTargetFps;
}

void StWindow::setTargetFps(const double theFPS) {
    myTargetFps = theFPS;
}

void StWindow::doChangeLanguage() {
    //
}

void StWindow::show() {
    myWin->show(ST_WIN_ALL);
}

void StWindow::show(const int theWinEnum) {
    myWin->show(theWinEnum);
}

void StWindow::hide() {
    myWin->hide(ST_WIN_ALL);
}

void StWindow::hide(const int theWinEnum) {
    myWin->hide(theWinEnum);
}

void StWindow::showCursor(const bool theToShow) {
    myWin->showCursor(theToShow);
}

bool StWindow::isFullScreen() const {
    return myWin->isFullScreen();
}

void StWindow::setFullScreen(const bool theFullScreen) {
    myWin->setFullScreen(theFullScreen);
}

StRectI_t StWindow::getPlacement() const {
    return myWin->getPlacement();
}

StRectI_t StWindow::getWindowedPlacement() const {
    return myWin->myRectNorm;
}

bool StWindow::isMovable() const {
    return myWin->myParentWin == NULL;
}

bool StWindow::isMobile() {
#if defined(__ANDROID__)
    return true;
#else
    return false;
#endif
}

bool StWindow::hasFullscreenMode() const {
#if defined(__ANDROID__)
    return false;
#else
    return true;
#endif
}

void StWindow::setPlacement(const StRectI_t& theRect,
                            const bool       theMoveToScreen) {
    myWin->setPlacement(theRect, theMoveToScreen);
}

StMarginsI StWindow::getMargins() const {
    return StMarginsI();
}

StPointD_t StWindow::getMousePos() const {
    return myWin->getMousePos();
}

bool StWindow::isPreciseCursor() const {
    return myWin->isPreciseCursor();
}

bool StWindow::create() {
    myWasUsed = true;
    return myWin->create();
}

void StWindow::stglSwap() {
     myWin->stglSwap(ST_WIN_ALL);
}

void StWindow::stglSwap(const int theWinEnum) {
     myWin->stglSwap(theWinEnum);
}

bool StWindow::stglMakeCurrent() {
    const bool isBound = myWin->stglMakeCurrent(ST_WIN_MASTER);
    if(!myWin->myGlContext.isNull()) {
        myWin->myGlContext->setBound(isBound);
    }
    return isBound;
}

bool StWindow::stglMakeCurrent(const int theWinEnum) {
    const bool isBound = myWin->stglMakeCurrent(theWinEnum);
    if(!myWin->myGlContext.isNull()) {
        myWin->myGlContext->setBound(isBound);
    }
    return isBound;
}

void StWindow::stglDraw() {
    if(myWin->myGlContext.isNull()) {
        return;
    }

    const StGLBoxPx aVPMaster = StWindow::stglViewport(ST_WIN_MASTER);
    const GLsizei aHeight = (aVPMaster .height() == 0) ? 1 : aVPMaster .height();
    const StGLBoxPx aRect = {{ aVPMaster.x(), aVPMaster.y(), aVPMaster.width(), aHeight }};
    myWin->myGlContext->stglResizeViewport(aRect);

    signals.onRedraw(ST_DRAW_LEFT);
    stglSwap(ST_WIN_ALL);
}

StGLBoxPx StWindow::stglViewport(const int theWinEnum) const {
    return myWin->stglViewport(theWinEnum);
}

void StWindow::processEvents() {
    myWin->processEvents();
}

void StWindow::post(StEvent& theEvent) {
    myWin->post(theEvent);
}

bool StWindow::isMouseMoved() const {
    return myWin->myIsMouseMoved;
}

const StHandle<StResourceManager>& StWindow::getResourceManager() const {
    return myWin->myResMgr;
}

const StHandle<StMsgQueue>& StWindow::getMessagesQueue() const {
    return myMsgQueue;
}

void StWindow::setMessagesQueue(const StHandle<StMsgQueue>& theQueue) {
    myMsgQueue = theQueue;
}

const StKeysState& StWindow::getKeysState() const {
    return myWin->myKeysState;
}

StKeysState& StWindow::changeKeysState() {
    return myWin->myKeysState;
}

double StWindow::getEventTime(const uint32_t theTime) const {
    return myWin->getEventTime(theTime);
}

const StSearchMonitors& StWindow::getMonitors() const {
    return myWin->getMonitors();
}

bool StWindow::isParentOnScreen() const {
    return myWin->isParentOnScreen();
}

GLfloat StWindow::getScaleFactor() const {
    return myWin->getScaleFactor();
}

const StHandle<StGLContext>& StWindow::getContext() const {
    return myWin->myGlContext;
}

void StWindow::convertRectToBacking(StGLBoxPx& theRect,
                                    const int  theWinId) {
    return myWin->convertRectToBacking(theRect, theWinId);
}

GLfloat StWindow::getLensDist() const {
    return 0.0f;
}

void StWindow::toClipboard(const StString& theText) {
    myWin->toClipboard(theText);
}

bool StWindow::fromClipboard(StString& theText) {
    return myWin->fromClipboard(theText);
}

StRectI_t StWindow::defaultRect(const StMonitor* theMon) const {
    const StMonitor& aMon = theMon != NULL ? *theMon : myWin->getMonitors()[0];
    StRectI_t aRect;
    aRect.left()   = aMon.getVRect().left() + 256;
    aRect.right()  = aRect.left() + int32_t(aMon.getScale() * 768.0f);
    aRect.top()    = aMon.getVRect().top()  + 256;
    aRect.bottom() = aRect.top()  + int32_t(aMon.getScale() * 512.0f);
    return aRect;
}

double StWindow::stglAspectRatio() const {
    if(myWin->myForcedAspect > 0.0) {
        return myWin->myForcedAspect;
    }

    const StGLBoxPx aVPMaster = StWindow::stglViewport(ST_WIN_MASTER);
    return double(aVPMaster.width() != 0 ? aVPMaster.width() : 1) / double(aVPMaster.height() != 0 ? aVPMaster.height() : 1);
}

void StWindow::setForcedAspect(double theAspect) {
    myWin->myForcedAspect = theAspect;
}

void* StWindow::getNativeOglWin() const {
#ifdef _WIN32
    return (void* )myWin->myMaster.hWindowGl;
#elif defined(__APPLE__)
    return (void* )myWin->myMaster.hWindow;
#elif defined(__ANDROID__)
    return (void* )myWin->myMaster.hWindowGl;
#elif defined(__linux__)
    return (void* )myWin->myMaster.hWindowGl;
#else
    return NULL;
#endif
}

void* StWindow::getNativeOglDC() const {
#ifdef _WIN32
    return (void* )myWin->myMaster.hDC;
#else
    return NULL;
#endif
}

void* StWindow::getNativeOglRC() const {
#ifdef _WIN32
    if(myWin->myMaster.hRC.isNull()) {
        return NULL;
    }
    return (void* )myWin->myMaster.hRC->getRenderContext();
#else
    return NULL;
#endif
}

const StString& StWindow::getStatistics() const {
    return myWin->myStatistics;
}

void StWindow::setHardwareStereoOn(const bool theToEnable) {
    myWin->myToEnableStereoHW = theToEnable;
}

bool StWindow::hasOrientationSensor() const {
    return myWin->myHasOrientSensor;
}

bool StWindow::isPoorOrientationSensor() const {
    return myWin->myIsPoorOrient;
}

bool StWindow::toTrackOrientation() const {
    return myWin->myToTrackOrient;
}

void StWindow::setTrackOrientation(const bool theToTrack) {
    if(myWin->myHasOrientSensor) {
        myWin->myToTrackOrient = theToTrack;
    }
}

void StWindow::setHideSystemBars(bool theToHideStatusBar,
                                 bool theToHideNavBar) {
    myWin->myToHideStatusBar = theToHideStatusBar;
    myWin->myToHideNavBar    = theToHideNavBar;
}

StQuaternion<double> StWindow::getDeviceOrientation() const {
    if(myWin->myToTrackOrient
    && myWin->myHasOrientSensor) {
        return myWin->myQuaternion;
    }
    return StQuaternion<double>();
}

bool StWindow::getCustomProjection(StRectF_t& , StRectF_t& ) const {
    return false;
}

bool StWindow::toSwapEyesHW() const {
    return myWin->myToSwapEyesHW;
}
