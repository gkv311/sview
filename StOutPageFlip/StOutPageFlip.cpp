/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutPageFlip library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutPageFlip library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StOutPageFlip.h"
#include "StVuzixSDK.h"
#include "StQuadBufferCheck.h"
#include "StDXNVWindow.h"
#include "StGLDeviceControl.h"

#include <StGL/StGLEnums.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StGLStereo/StGLStereoFrameBuffer.h>
#include <StCore/StSearchMonitors.h>
#include <StSettings/StSettings.h>
#include <StSettings/StTranslations.h>
#include <StImage/StLibAVImage.h>
#include <StSys/StSys.h>

namespace {

    static const char ST_OUT_PLUGIN_NAME[] = "StOutPageFlip";

    static const char ST_SETTING_ADVANCED[]   = "advanced";
    static const char ST_SETTING_WINDOWPOS[]  = "windowPos";
    static const char ST_SETTING_DEVICE_ID[]  = "deviceId";
    static const char ST_SETTING_QUADBUFFER[] = "quadBufferType";
};

StOutPageFlip::StOutDirect3D::StOutDirect3D()
: myGLBuffer(),
#ifdef _WIN32
  myDxWindow(),
  myDxThread(),
#endif
  myGLIoBuff(0),
  myActivateStep(0),
  myIsActive(false),
  myToUsePBO(true) {
    //
}

#ifdef _WIN32
SV_THREAD_FUNCTION StOutPageFlip::StOutDirect3D::dxThreadFunction(void* theStOutD3d) {
    StOutPageFlip::StOutDirect3D* aStOutD3d = (StOutPageFlip::StOutDirect3D* )theStOutD3d;
    aStOutD3d->myDxWindow->dxLoop();
    return SV_THREAD_RETURN 0;
}
#endif // _WIN32

static StMonitor getHigestFreqMonitor(const StSearchMonitors& theMonitors) {
    size_t hfreqMon = 0;
    int hfreqMax = 0;
    for(size_t aMonIter = 0; aMonIter < theMonitors.size(); ++aMonIter) {
        const StMonitor& aMon = theMonitors[aMonIter];
        if(aMon.getFreqMax() > hfreqMax) {
            hfreqMax = aMon.getFreqMax();
            hfreqMon = aMonIter;
        }
    }
    return !theMonitors.isEmpty() ? theMonitors[hfreqMon] : StMonitor();
}

StString StOutPageFlip::getRendererAbout() const {
    return myAbout;
}

const char* StOutPageFlip::getRendererId() const {
    return ST_OUT_PLUGIN_NAME;
}

const char* StOutPageFlip::getDeviceId() const {
    switch(myDevice) {
        case DEVICE_VUZIX:    return "Vuzix";
        case DEVICE_SHUTTERS:
        default:              return "Shutters";
    }
}

bool StOutPageFlip::isLostDevice() const {
    return myToResetDevice;
}

bool StOutPageFlip::setDevice(const StString& theDevice) {
    const int aPrevValue = myDevice;
    if(theDevice == "Shutters") {
        myDevice = DEVICE_SHUTTERS;
    } else if(theDevice == "Vuzix") {
        myDevice = DEVICE_VUZIX;
    }
    return myDevice != aPrevValue;
}

void StOutPageFlip::getDevices(StOutDevicesList& theList) const {
    for(size_t anIter = 0; anIter < myDevices.size(); ++anIter) {
        theList.add(myDevices[anIter]);
    }
}

void StOutPageFlip::getOptions(StParamsList& theList) const {
    theList.add(params.QuadBuffer);
    theList.add(params.ToShowExtra);
}

StOutPageFlip::StOutPageFlip(const StNativeWin_t theParentWindow)
: StWindow(theParentWindow),
  mySettings(new StSettings(ST_OUT_PLUGIN_NAME)),
  myLangMap(ST_OUT_PLUGIN_NAME),
  myDevice(DEVICE_AUTO),
  myToSavePlacement(theParentWindow == (StNativeWin_t )NULL),
  myToDrawStereo(false),
#ifdef _WIN32
  myIsVistaPlus(StSys::isVistaPlus()),
#endif
  myToResetDevice(false) {
    const StSearchMonitors& aMonitors = StWindow::getMonitors();

    // about string
    StString& aTitle     = myLangMap.changeValueId(STTR_PLUGIN_TITLE,   "sView - PageFlip Output module");
    StString& aVerString = myLangMap.changeValueId(STTR_VERSION_STRING, "version");
    StString& aDescr     = myLangMap.changeValueId(STTR_PLUGIN_DESCRIPTION,
        "(C) 2007-2013 Kirill Gavrilov <kirill@sview.ru>\nOfficial site: www.sview.ru\n\nThis library distributed under LGPL3.0");
    myAbout = aTitle + '\n' + aVerString + ": " + StVersionInfo::getSDKVersionString() + "\n \n" + aDescr;

    // detect connected displays
    int aSupportLevelShutters = ST_DEVICE_SUPPORT_NONE;
    int aSupportLevelVuzix    = StVuzixSDK::isConnected(aMonitors) ? ST_DEVICE_SUPPORT_PREFER : ST_DEVICE_SUPPORT_NONE;
    const StMonitor aMon = getHigestFreqMonitor(aMonitors);
    if(aMon.getFreqMax() >= 110) {
        aSupportLevelShutters = ST_DEVICE_SUPPORT_HIGHT;
    }
#ifndef __APPLE__
    // actually almost always available on mac but... is it useful?
    if(testQuadBufferSupportThreaded()) {
        aSupportLevelShutters = ST_DEVICE_SUPPORT_FULL;
    }
#endif

#ifdef _WIN32
    StDXInfo aDxInfo;
    if(aSupportLevelShutters != ST_DEVICE_SUPPORT_FULL
    && StDXManager::getInfoThreaded(aDxInfo)
    && (aDxInfo.hasNvStereoSupport || aDxInfo.hasAqbsSupport)) {
        // if user enable NVIDIA stereo driver - we prefer use it for shutters
        aSupportLevelShutters = ST_DEVICE_SUPPORT_FULL;
    }
#endif

    // devices list
    StHandle<StOutDevice> aDevShutters = new StOutDevice();
    aDevShutters->PluginId = ST_OUT_PLUGIN_NAME;
    aDevShutters->DeviceId = "Shutters";
    aDevShutters->Priority = aSupportLevelShutters;
    aDevShutters->Name     = myLangMap.changeValueId(STTR_PAGEFLIP_NAME, "Shutter glasses");
    aDevShutters->Desc     = myLangMap.changeValueId(STTR_PAGEFLIP_DESC, "Shutter glasses");
    myDevices.add(aDevShutters);

    StHandle<StOutDevice> aDevVuzix = new StOutDevice();
    aDevVuzix->PluginId = ST_OUT_PLUGIN_NAME;
    aDevVuzix->DeviceId = "Vuzix";
    aDevVuzix->Priority = aSupportLevelVuzix;
    aDevVuzix->Name     = myLangMap.changeValueId(STTR_VUZIX_NAME, "Vuzix HMD");
    aDevVuzix->Desc     = myLangMap.changeValueId(STTR_VUZIX_DESC, "Vuzix HMD");
    myDevices.add(aDevVuzix);

    // load window position
    StRect<int32_t> aRect(256, 768, 256, 1024);
    if(mySettings->loadInt32Rect(ST_SETTING_WINDOWPOS, aRect)) {
        StMonitor aMonitor = aMonitors[aRect.center()];
        if(!aMonitor.getVRect().isPointIn(aRect.center())) {
            ST_DEBUG_LOG("Warning, stored window position is out of the monitor(" + aMonitor.getId() + ")!" + aRect.toString());
            const int aWidth  = aRect.width();
            const int aHeight = aRect.height();
            aRect.left()   = aMonitor.getVRect().left() + 256;
            aRect.right()  = aRect.left() + aWidth;
            aRect.top()    = aMonitor.getVRect().top() + 256;
            aRect.bottom() = aRect.top() + aHeight;
        }
    } else {
        // try to open window on display with highest frequency
        aRect = aMon.getVRect();
        aRect.left()   = aRect.left() + 256;
        aRect.right()  = aRect.left() + 1024;
        aRect.top()    = aRect.top()  + 256;
        aRect.bottom() = aRect.top()  + 512;
    }
    StWindow::setPlacement(aRect);

    // load device settings
    int aDeviceInt = int(myDevice);
    mySettings->loadInt32(ST_SETTING_DEVICE_ID, aDeviceInt);
    if(aDeviceInt == DEVICE_AUTO) {
        myDevice = DEVICE_SHUTTERS;
    } else {
        myDevice = DeviceEnum(aDeviceInt);
    }

#ifdef _WIN32
    StDXManager::getInfo(myDxInfo);
#endif

    // Quad Buffer type option
    params.QuadBuffer = new StEnumParam(0, myLangMap.changeValueId(STTR_PARAMETER_QBUFFER_TYPE, "Quad Buffer type"));
    params.QuadBuffer->signals.onChanged.connect(this, &StOutPageFlip::doSetQuadBuffer);
    params.QuadBuffer->changeValues().add(myLangMap.changeValueId(STTR_PARAMETER_QB_HARDWARE, "OpenGL Hardware"));
    StString aDxDesc;
    if(myDxInfo.hasAqbsSupport && myDxInfo.hasNvStereoSupport) {
        aDxDesc = myLangMap.changeValueId(STTR_PARAMETER_QB_D3D_ANY,     "Direct3D (Fullscreen)");
    } else if(myDxInfo.hasAqbsSupport) {
        aDxDesc = myLangMap.changeValueId(STTR_PARAMETER_QB_D3D_AMD,     "Direct3D AMD (Fullscreen)");
    } else if(myDxInfo.hasNvStereoSupport) {
        aDxDesc = myLangMap.changeValueId(STTR_PARAMETER_QB_D3D_NV,      "Direct3D NVIDIA (Fullscreen)");
    } else if(myDxInfo.hasAmdAdapter) {
        aDxDesc = myLangMap.changeValueId(STTR_PARAMETER_QB_D3D_AMD_OFF, "Direct3D AMD (Unavailable)");
    } else if(myDxInfo.hasNvAdapter) {
        aDxDesc = myLangMap.changeValueId(STTR_PARAMETER_QB_D3D_NV_OFF,  "Direct3D NVIDIA (Disabled)");
    } else {
        aDxDesc = myLangMap.changeValueId(STTR_PARAMETER_QB_D3D_OFF,     "Direct3D (Unavailable)");
    }
    params.QuadBuffer->changeValues().add(aDxDesc);

    // Show Extra option
    params.ToShowExtra = new StBoolParamNamed(false, "Show Extra Options");
    params.ToShowExtra->signals.onChanged.connect(this, &StOutPageFlip::doShowExtra);
    mySettings->loadParam(ST_SETTING_ADVANCED, params.ToShowExtra);

    // load Quad Buffer type
    if(!mySettings->loadParam(ST_SETTING_QUADBUFFER, params.QuadBuffer)) {
        params.QuadBuffer->setValue((aSupportLevelShutters == ST_DEVICE_SUPPORT_FULL) ? QUADBUFFER_HARD_OPENGL : QUADBUFFER_HARD_D3D_ANY);
    }
    myToResetDevice = false;
}

void StOutPageFlip::releaseResources() {
    if(!myWarning.isNull()) {
        myWarning->release(*myContext);
        myWarning.nullify();
    }

    dxRelease();
    myContext.nullify();
    myVuzixSDK.nullify();

    // read windowed placement
    StWindow::hide();
    if(myToSavePlacement) {
        StWindow::setFullScreen(false);
        mySettings->saveInt32Rect(ST_SETTING_WINDOWPOS, StWindow::getPlacement());
    }
    mySettings->saveInt32(ST_SETTING_DEVICE_ID,  myDevice);
    mySettings->saveParam(ST_SETTING_QUADBUFFER, params.QuadBuffer);
    mySettings->saveParam(ST_SETTING_ADVANCED,   params.ToShowExtra);
}

StOutPageFlip::~StOutPageFlip() {
    releaseResources();
}

void StOutPageFlip::close() {
    myToResetDevice = false;
    releaseResources();
    StWindow::close();
}

void StOutPageFlip::setupDevice() {
    switch(myDevice) {
        case DEVICE_VUZIX: {
            if(!StVuzixSDK::isConnected(StWindow::getMonitors())) {
                stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, Vuzix HMD Not Found!");
                break;
            } else if(myVuzixSDK.isNull()) {
                stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, Failed to Load Vuzix VR920 Driver!");
                break;
            }
            myVuzixSDK->open();
            break;
        }
        default: {
            if(!myVuzixSDK.isNull()) {
                myVuzixSDK->setMonoOut();
                myVuzixSDK->close();
            }
            break;
        }
    }
}

bool StOutPageFlip::dxInit() {
    dxRelease();
    myOutD3d.myIsActive = false;
    myOutD3d.myActivateStep = 0;
#ifdef _WIN32
    StMonitor aNvMonitor = StCore::getMonitorFromPoint(StWindow::getPlacement().center());
    GLsizei aFrBufferSizeX = aNvMonitor.getVRect().width();
    GLsizei aFrBufferSizeY = aNvMonitor.getVRect().height();

    // INIT framebuffers
    if(!myContext->stglIsRectangularFboSupported()) {
        StGLFrameBuffer::convertToPowerOfTwo(*myContext, aFrBufferSizeX, aFrBufferSizeY);
        myOutD3d.myToUsePBO = false; // force compatible mode
        ST_DEBUG_LOG(ST_OUT_PLUGIN_NAME + " Plugin, old videocard detected (GLSL 1.1)!");
    }

    myOutD3d.myDxWindow = new StDXNVWindow(aFrBufferSizeX, aFrBufferSizeY, aNvMonitor, this);
    myOutD3d.myDxThread = new StThread(StOutDirect3D::dxThreadFunction, (void* )&myOutD3d);
    return true;
#else
    return false;
#endif // _WIN32
}

void StOutPageFlip::dxRelease() {
#ifdef _WIN32
    dxDisactivate();
    if(!myOutD3d.myDxThread.isNull()) {
        myOutD3d.myDxWindow->quit();
        myOutD3d.myDxThread->wait();
        myOutD3d.myDxThread.nullify();
    }
    myOutD3d.myDxWindow.nullify();
    if(!myOutD3d.myGLBuffer.isNull()) {
        myOutD3d.myGLBuffer->release(*myContext);
        myOutD3d.myGLBuffer.nullify();
    }

    if(myOutD3d.myGLIoBuff != 0) {
        myContext->core20fwd->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        myContext->core20fwd->glDeleteBuffers(1, &myOutD3d.myGLIoBuff);
        myOutD3d.myGLIoBuff = 0;
    }
#endif
}

void StOutPageFlip::dxActivate() {
#ifdef _WIN32
    // activate Direct3D Fullscreen window
    if(!myOutD3d.myIsActive
    && !myOutD3d.myDxWindow.isNull()) {
        if(myOutD3d.myActivateStep == 0) {
            // Direct3D device will fail to Reset if GL fullscreen device is active
            // so we switch out main GL window into windowed state temporarily
            // (we need to wait some time to ensure system perform needed movements)
            StWindow::setFullScreen(false);
            ++myOutD3d.myActivateStep;
            return;
        }
        myOutD3d.myDxWindow->waitReady();
        myOutD3d.myDxWindow->show();

        myOutD3d.myIsActive = true;
        if(myOutD3d.myActivateStep == 1) {
            // at second step switch out main GL window back to fullscreen
            myOutD3d.myActivateStep = 0;
            StWindow::setFullScreen(true);

            if(myOutD3d.myGLBuffer.isNull()) {
                myOutD3d.myGLBuffer = new StGLFrameBuffer();
                if(!myOutD3d.myGLBuffer->init(*myContext, (GLsizei )myOutD3d.myDxWindow->getFboSizeX(), (GLsizei )myOutD3d.myDxWindow->getFboSizeY())) {
                    stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, Failed to init OpenGL Frame Buffer");
                    myOutD3d.myGLBuffer->release(*myContext);
                    myOutD3d.myGLBuffer.nullify();
                }
            }
        }
    }
#endif
}

void StOutPageFlip::dxDisactivate() {
#ifdef _WIN32
    if(myOutD3d.myIsActive) {
        // set this flag first to make recursive calls
        myOutD3d.myIsActive = false;
        if(!myOutD3d.myDxWindow.isNull()) {
            myOutD3d.myDxWindow->hide();
            if(myOutD3d.myDxWindow->hasOwnWindow()) {
                StWindow::show(ST_WIN_MASTER);
            }
        }

        // release unused resources
        if(!myOutD3d.myGLBuffer.isNull()) {
            myOutD3d.myGLBuffer->release(*myContext);
            myOutD3d.myGLBuffer.nullify();
        }
        if(myOutD3d.myGLIoBuff != 0) {
            myContext->core20fwd->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            myContext->core20fwd->glDeleteBuffers(1, &myOutD3d.myGLIoBuff);
            myOutD3d.myGLIoBuff = 0;
        }
    }
#endif
}

bool StOutPageFlip::create() {
    StWindow::show();

    // request Quad Buffer
    StWinAttributes_t anAttribs = stDefaultWinAttributes();
    StWindow::getAttributes(anAttribs);
    anAttribs.isGlStereo = (params.QuadBuffer->getValue() == QUADBUFFER_HARD_OPENGL);
    StWindow::setAttributes(anAttribs);
    if(!StWindow::create()) {
        return false;
    }

    // initialize GL context
    myContext = new StGLContext();
    if(!myContext->stglInit()) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, OpenGL context is broken!\n(OpenGL library internal error?)");
        return false;
    } else if(!myContext->isGlGreaterEqual(2, 0)) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, OpenGL2.0+ not available!");
        return false;
    }
    myContext->stglSetVSync(StGLContext::VSync_ON);

    // load fullscreen-only warning
    StLibAVImage anImage;
    const StString aTexturesFolder  = StProcess::getStCoreFolder() + "textures" + SYS_FS_SPLITTER;
    const StString aWarnTexturePath = aTexturesFolder + "pageflip_fullscreen.std";
    if(anImage.load(aWarnTexturePath, StImageFile::ST_TYPE_PNG)) {
        myWarning = new StGLTexture(GL_RGBA8);
        if(!myWarning->init(*myContext, anImage.getPlane())) {
            ST_ERROR_LOG(ST_OUT_PLUGIN_NAME + " Plugin, Texture can not be initialized!");
            myWarning->release(*myContext);
            myWarning.nullify();
        }
    } else {
        ST_ERROR_LOG(ST_OUT_PLUGIN_NAME + " Plugin, Texture missed: " + anImage.getState());
    }

    // initialize Direct3D output
    if(params.QuadBuffer->getValue() == QUADBUFFER_HARD_D3D_ANY) {
        dxInit();
    }

    // initialize Vuzix library
    myVuzixSDK = new StVuzixSDK();
    if(myVuzixSDK->init() != STERROR_LIBNOERROR) {
        myVuzixSDK.nullify();
        //ST_DEBUG_LOG(ST_OUT_PLUGIN_NAME + ST_STRING(" Plugin, Failed to Load Vuzix VR920 Driver!"));
    }

    setupDevice();
    return true;
}

void StOutPageFlip::stglResize(const StRectI_t& ) {
    //
}

void StOutPageFlip::processEvents(StMessage_t* theMessages) {
    StWindow::processEvents(theMessages);
    for(size_t anIter = 0; theMessages[anIter].uin != StMessageList::MSG_NULL; ++anIter) {
        switch(theMessages[anIter].uin) {
            case StMessageList::MSG_RESIZE: {
                stglResize(StWindow::getPlacement());
                break;
            }
            case StMessageList::MSG_KEYS: {
                bool* aKeys = (bool* )theMessages[anIter].data;
                if(aKeys[ST_VK_F11]) {
                    StWindow::stglSwap(ST_WIN_MASTER);
                    aKeys[ST_VK_F11] = false;
                }
                break;
            }
        #ifdef _WIN32
            case StMessageList::MSG_WIN_ON_NEW_MONITOR: {
                if(params.QuadBuffer->getValue() != QUADBUFFER_HARD_D3D_ANY
                || myOutD3d.myIsActive) {
                    break;
                }
                dxRelease();
                dxInit();
                break;
            }
        #endif
        }
    }
}

void StOutPageFlip::dxDraw(unsigned int view) {
#ifdef _WIN32
    if(myOutD3d.myToUsePBO) {
        // use PBO to get some speed up
        // PBO macro (see spec for details)
        #define BUFFER_OFFSET(i) ((char *)NULL + (i))
        if(myOutD3d.myGLIoBuff == 0) {
            myContext->core20fwd->glGenBuffers(1, &myOutD3d.myGLIoBuff);
            myContext->core20fwd->glBindBuffer(GL_PIXEL_PACK_BUFFER, myOutD3d.myGLIoBuff);
            myContext->core20fwd->glBufferData(GL_PIXEL_PACK_BUFFER, myOutD3d.myGLBuffer->getSizeX() * myOutD3d.myGLBuffer->getSizeY() * 4, NULL, GL_STREAM_READ);
        }

        myContext->core20fwd->glReadBuffer(GL_COLOR_ATTACHMENT0);
        myContext->core20fwd->glBindBuffer(GL_PIXEL_PACK_BUFFER, myOutD3d.myGLIoBuff);
        myContext->core20fwd->glReadPixels(0, 0, myOutD3d.myGLBuffer->getSizeX(), myOutD3d.myGLBuffer->getSizeY(), GL_BGRA, GL_UNSIGNED_BYTE, BUFFER_OFFSET(0));
        GLubyte* ioMemBRGA = (GLubyte* )myContext->core20fwd->glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(ioMemBRGA == NULL) {
            ST_DEBUG_LOG_AT("PBO buffer is sucks!");
        }

        if(view == ST_DRAW_LEFT) {
            myOutD3d.myDxWindow->lockLRBuffers();
            myOutD3d.myDxWindow->allocateBuffers();
            if(myOutD3d.myDxWindow->getBuffLeft() != NULL) {
                stMemCpy(myOutD3d.myDxWindow->getBuffLeft(),  ioMemBRGA, myOutD3d.myGLBuffer->getSizeX() * myOutD3d.myGLBuffer->getSizeY() * 4);
            }
        } else {
            if(myOutD3d.myDxWindow->getBuffRight() != NULL) {
                stMemCpy(myOutD3d.myDxWindow->getBuffRight(), ioMemBRGA, myOutD3d.myGLBuffer->getSizeX() * myOutD3d.myGLBuffer->getSizeY() * 4);
            }
            myOutD3d.myDxWindow->unlockLRBuffers();
            myOutD3d.myDxWindow->update();
        }
        myContext->core20fwd->glUnmapBuffer(GL_PIXEL_PACK_BUFFER); // release memory, i.e. give control back to the driver
        myContext->core20fwd->glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    } else {
        // simple read
        if(view == ST_DRAW_LEFT) {
            myOutD3d.myDxWindow->lockLRBuffers();
            myOutD3d.myDxWindow->allocateBuffers();
            if(myOutD3d.myDxWindow->getBuffLeft() != NULL) {
                myContext->core20fwd->glReadPixels(0, 0, myOutD3d.myGLBuffer->getSizeX(), myOutD3d.myGLBuffer->getSizeY(), GL_BGRA, GL_UNSIGNED_BYTE,
                                                   (GLubyte* )myOutD3d.myDxWindow->getBuffLeft());
            }
        } else {
            if(myOutD3d.myDxWindow->getBuffRight() != NULL) {
                myContext->core20fwd->glReadPixels(0, 0, myOutD3d.myGLBuffer->getSizeX(), myOutD3d.myGLBuffer->getSizeY(), GL_BGRA, GL_UNSIGNED_BYTE,
                                                   (GLubyte* )myOutD3d.myDxWindow->getBuffRight());
            }
            myOutD3d.myDxWindow->unlockLRBuffers();
            myOutD3d.myDxWindow->update();
        }
    }
#endif
}

void StOutPageFlip::stglDrawWarning() {
    if(myWarning.isNull()) {
        return;
    }

    myContext->core20fwd->glDisable(GL_DEPTH_TEST);
    myContext->core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    myContext->core20fwd->glEnable(GL_BLEND);
    myContext->core11->glEnable(GL_TEXTURE_2D);

    myWarning->bind(*myContext);

    const StRectI_t aRect = StWindow::getPlacement();
    const int aWinSizeX = aRect.width();
    const int aWinSizeY = aRect.height();
    const GLfloat aWidth  = (aWinSizeX > 0) ?        GLfloat(myWarning->getSizeX()) / GLfloat(aWinSizeX) : 1.0f;
    const GLfloat aBottom = (aWinSizeY > 0) ? 100.0f / GLfloat(aWinSizeY) : 0.0f;
    const GLfloat aHeight = (aWinSizeY > 0) ? 2.0f * GLfloat(myWarning->getSizeY()) / GLfloat(aWinSizeY) : 1.0f;

    const GLfloat aVerts[] = {
         aWidth, -1.0f + aBottom + aHeight,
         aWidth, -1.0f + aBottom,
        -aWidth, -1.0f + aBottom + aHeight,
        -aWidth, -1.0f + aBottom,
    };

    const GLfloat aTCrds[] = {
        1.0f, 0.0f, // top-right
        1.0f, 1.0f, // bottom-right
        0.0f, 0.0f, // top-left
        0.0f, 1.0f  // bottom-left
    };

    myContext->core11->glLoadIdentity();

    myContext->core11->glEnableClientState(GL_VERTEX_ARRAY);
    myContext->core11->glVertexPointer(2, GL_FLOAT, 0, aVerts);
    myContext->core11->glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    myContext->core11->glTexCoordPointer(2, GL_FLOAT, 0, aTCrds);

    myContext->core11fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    myContext->core11->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    myContext->core11->glDisableClientState(GL_VERTEX_ARRAY);

    myWarning->unbind(*myContext);
    myContext->core11->glDisable(GL_TEXTURE_2D);
    myContext->core20fwd->glDisable(GL_BLEND);
}

void StOutPageFlip::stglDraw() {
    myFPSControl.setTargetFPS(StWindow::getTargetFps());

    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    const StRectI_t aRect = StWindow::getPlacement();
    myContext->stglResize(aRect);

    if(!StWindow::isStereoOutput()) {
        // Vuzix driver control
        if(myToDrawStereo) {
            if(myDevice == DEVICE_VUZIX && !myVuzixSDK.isNull()) {
                myVuzixSDK->setMonoOut();
            }
            myToDrawStereo = false;
        }

        // disactivate Direct3D Fullscreen window
        dxDisactivate();

        // setup whole back buffer (left+right) for hardware GL Quad Buffer
        if(params.QuadBuffer->getValue() == QUADBUFFER_HARD_OPENGL) {
            myContext->core20fwd->glDrawBuffer(GL_BACK);
        }

        // draw new MONO frame
        StWindow::signals.onRedraw(ST_DRAW_LEFT);
        if(myDevice != DEVICE_VUZIX) {
            stglDrawExtra(ST_DRAW_LEFT, StGLDeviceControl::OUT_MONO);
        }

        // decrease FPS to target by thread sleeps
        myFPSControl.sleepToTarget();
        StWindow::stglSwap(ST_WIN_MASTER);
        ++myFPSControl;
        return;
    } else if(!myToDrawStereo) {
        if(myDevice == DEVICE_VUZIX && !myVuzixSDK.isNull()
        && params.QuadBuffer->getValue() != QUADBUFFER_HARD_OPENGL) {
            myVuzixSDK->setStereoOut();
        }
        myToDrawStereo = true;
    }

    switch(params.QuadBuffer->getValue()) {
        case QUADBUFFER_HARD_OPENGL: {
            // We check capabilities at runtime to ensure that OpenGL context was created with Quad-Buffer.
            // Also we requires fullscreen for RadeOn cards on Windows 7.
            myContext->stglResetErrors(); // reset errors stack
            GLboolean isStereoOn = GL_FALSE;
            myContext->core20fwd->glGetBooleanv(GL_STEREO, &isStereoOn);
            if(isStereoOn) {
                myContext->core20fwd->glDrawBuffer(GL_BACK_RIGHT);
                isStereoOn = (myContext->core20fwd->glGetError() == GL_NO_ERROR);
            }

            if(!isStereoOn) {
                myContext->core20fwd->glDrawBuffer(GL_BACK);
                StWindow::signals.onRedraw(ST_DRAW_RIGHT);
                StWindow::signals.onRedraw(ST_DRAW_LEFT);
                stglDrawWarning();
            } else {
                myContext->core20fwd->glDrawBuffer(GL_BACK_LEFT);
                StWindow::signals.onRedraw(ST_DRAW_LEFT);
                stglDrawExtra(ST_DRAW_LEFT, StGLDeviceControl::OUT_STEREO);

                myContext->stglResize(aRect);
                myContext->core20fwd->glDrawBuffer(GL_BACK_RIGHT);
                StWindow::signals.onRedraw(ST_DRAW_RIGHT);
                stglDrawExtra(ST_DRAW_RIGHT, StGLDeviceControl::OUT_STEREO);
            }
            StThread::sleep(1);
            StWindow::stglSwap(ST_WIN_MASTER);
            ++myFPSControl;
            return;
        }
    #ifdef _WIN32
        case QUADBUFFER_HARD_D3D_ANY: {
            if(myOutD3d.myActivateStep == 1 || StWindow::isFullScreen()) {
                dxActivate();

                // draw into virtual frame buffers (textures)
                if(!myOutD3d.myGLBuffer.isNull()) {
                    GLint aVPort[4]; // real window viewport
                    myContext->core20fwd->glGetIntegerv(GL_VIEWPORT, aVPort);
                        myOutD3d.myGLBuffer->setupViewPort(*myContext); // we set TEXTURE sizes here
                        myOutD3d.myGLBuffer->bindBuffer(*myContext);
                            StWindow::signals.onRedraw(ST_DRAW_LEFT);
                            stglDrawExtra(ST_DRAW_LEFT, StGLDeviceControl::OUT_STEREO);
                            while(myOutD3d.myDxWindow->isInUpdate()) {
                                StThread::sleep(2);
                            }
                            // read OpenGL buffers and write into Direct3D surface
                            dxDraw(ST_DRAW_LEFT);
                            StWindow::signals.onRedraw(ST_DRAW_RIGHT);
                            stglDrawExtra(ST_DRAW_RIGHT, StGLDeviceControl::OUT_STEREO);
                            dxDraw(ST_DRAW_RIGHT);
                        myOutD3d.myGLBuffer->unbindBuffer(*myContext);
                    myContext->core20fwd->glViewport(aVPort[0], aVPort[1], aVPort[2], aVPort[3]);
                }
                break;
            } else { //is windowed output - we doesn't break, just use aggressive behaviour!
                // disactivate Direct3D Fullscreen window
                dxDisactivate();

                StWindow::stglMakeCurrent(ST_WIN_MASTER);
                StWindow::signals.onRedraw(ST_DRAW_RIGHT); // reverse order to avoid non-smooth mono->stereo transition
                StWindow::signals.onRedraw(ST_DRAW_LEFT);

                stglDrawWarning();

                myFPSControl.sleepToTarget();
                StWindow::stglSwap(ST_WIN_MASTER);
                ++myFPSControl;
                break;
            }
        }
    #endif
        default: {
            stglDrawAggressive(ST_DRAW_LEFT);
            stglDrawAggressive(ST_DRAW_RIGHT);
        }
    }
}

void StOutPageFlip::stglDrawExtra(unsigned int , int ) {
    //
}

void StOutPageFlip::stglDrawAggressive(unsigned int theView) {
    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglResize(StWindow::getPlacement());
    StWindow::signals.onRedraw(theView);

    if(myDevice == DEVICE_VUZIX) {
        if(!myVuzixSDK.isNull()) {
            if(theView == ST_DRAW_LEFT) { myVuzixSDK->waitAckLeft(); } else { myVuzixSDK->waitAckRight(); }
        }
    } else {
        stglDrawExtra(theView, StGLDeviceControl::OUT_STEREO);
    }

    StWindow::stglSwap(ST_WIN_MASTER);
    ++myFPSControl;

    // Inform VR920 to begin scanning on next vSync, a new right eye frame.
    if(myDevice == DEVICE_VUZIX && !myVuzixSDK.isNull()) {
        if(theView == ST_DRAW_LEFT) { myVuzixSDK->setLeft(); } else { myVuzixSDK->setRight(); }
    }
}

void StOutPageFlip::doSetQuadBuffer(const int32_t ) {
    myToResetDevice = true;
}

void StOutPageFlip::doShowExtra(const bool theValue) {
    myToResetDevice = true;
    if(theValue) {
        params.QuadBuffer->changeValues().add(myLangMap.changeValueId(STTR_PARAMETER_QB_EMULATED, "OpenGL Emulated"));
    } else {
        params.QuadBuffer->changeValues().remove(params.QuadBuffer->getValues().size() - 1);
        if(params.QuadBuffer->getValue() == QUADBUFFER_SOFT) {
            params.QuadBuffer->setValue(QUADBUFFER_HARD_OPENGL);
        }
    }
}
