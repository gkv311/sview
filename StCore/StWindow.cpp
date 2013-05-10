/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
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

#include <StGLCore/StGLCore11Fwd.h>

#include "StWindowImpl.h"

void StWindow::copySignals() {
    myWin->signals.onResize    = &signals.onResize;
    myWin->signals.onKeyUp     = &signals.onKeyUp;
    myWin->signals.onKeyDown   = &signals.onKeyDown;
    myWin->signals.onKeyHold   = &signals.onKeyHold;
    myWin->signals.onMouseUp   = &signals.onMouseUp;
    myWin->signals.onMouseDown = &signals.onMouseDown;
    myWin->signals.onFileDrop  = &signals.onFileDrop;
}

StWindow::StWindow()
: myWin(new StWindowImpl((StNativeWin_t )NULL)),
  myTargetFps(0.0) {
    copySignals();
}

StWindow::StWindow(const StNativeWin_t theParentWindow)
: myWin(new StWindowImpl(theParentWindow)),
  myTargetFps(0.0) {
    copySignals();
}

StWindow::~StWindow() {
    delete myWin;
}

void StWindow::close() {
    myWin->close();
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

bool StWindow::isLostDevice() const {
    return false;
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

bool StWindow::isStereoOutput() const {
    return myWin->isStereoOutput();
}

void StWindow::setStereoOutput(const bool theStereoState) {
    myWin->setStereoOutput(theStereoState);
}

double StWindow::getTargetFps() const {
    return myTargetFps;
}

void StWindow::setTargetFps(const double theFPS) {
    myTargetFps = theFPS;
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

void StWindow::setPlacement(const StRectI_t& theRect,
                            const bool       theMoveToScreen) {
    myWin->setPlacement(theRect, theMoveToScreen);
}

StPointD_t StWindow::getMousePos() const {
    return myWin->getMousePos();
}

bool StWindow::create() {
    return myWin->create();
}

void StWindow::stglSwap() {
     myWin->stglSwap(ST_WIN_ALL);
}

void StWindow::stglSwap(const int theWinEnum) {
     myWin->stglSwap(theWinEnum);
}

void StWindow::stglMakeCurrent() {
    myWin->stglMakeCurrent(ST_WIN_MASTER);
}

void StWindow::stglMakeCurrent(const int theWinEnum) {
    myWin->stglMakeCurrent(theWinEnum);
}

void StWindow::stglDraw() {
    const StGLBoxPx aVPMaster = StWindow::stglViewport(ST_WIN_MASTER);
    const GLsizei aHeight = (aVPMaster .height() == 0) ? 1 : aVPMaster .height();
    glViewport(aVPMaster .x(), aVPMaster .y(), aVPMaster .width(), aHeight);
    signals.onRedraw(ST_DRAW_LEFT);
    stglSwap(ST_WIN_ALL);
}

StGLBoxPx StWindow::stglViewport(const int theWinEnum) const {
    return myWin->stglViewport(theWinEnum);
}

void StWindow::processEvents(StMessage_t* theMessages) {
    myWin->processEvents(theMessages);
}

void StWindow::post(StEvent& theEvent) {
    myWin->post(theEvent);
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
