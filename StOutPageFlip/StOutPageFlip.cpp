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

#include "StOutPageFlipExt.h"
#include "StVuzixSDK.h"
#include "StQuadBufferCheck.h"
#include "StDXNVWindow.h"
#include "StGLDeviceControl.h"

#include <StGL/StGLEnums.h>
#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StGLStereo/StGLStereoFrameBuffer.h>
#include <StCore/StWindow.h>
#include <StSettings/StSettings.h>
#include <StSettings/StTranslations.h>
#include <StImage/StLibAVImage.h>
#include <StSys/StSys.h>

namespace {
    static bool HAS_LOGGER_ID = StLogger::IdentifyModule("StOutPageFlip");

    static const char ST_OUT_PLUGIN_NAME[] = "StOutPageFlip";

    static const char ST_SETTING_ADVANCED[]   = "advanced";
    static const char ST_SETTING_WINDOWPOS[]  = "windowPos";
    static const char ST_SETTING_DEVICE_ID[]  = "deviceId";
    static const char ST_SETTING_QUADBUFFER[] = "quadBufferType";
};

StOutPageFlip::StOutDirect3D::StOutDirect3D()
: myGLBuffer(),
#if(defined(_WIN32) || defined(__WIN32__))
  myDxWindow(),
  myDxThread(),
#endif
  myGLIoBuff(0),
  myActivateStep(0),
  myIsActive(false),
  myToUsePBO(true) {
    //
}

#if(defined(_WIN32) || defined(__WIN32__))
SV_THREAD_FUNCTION StOutPageFlip::StOutDirect3D::dxThreadFunction(void* theStOutD3d) {
    StOutPageFlip::StOutDirect3D* aStOutD3d = (StOutPageFlip::StOutDirect3D* )theStOutD3d;
    aStOutD3d->myDxWindow->dxLoop();
    return SV_THREAD_RETURN 0;
}
#endif // _WIN32

static StMonitor getHigestFreqMonitor() {
    size_t hfreqMon = 0;
    int hfreqMax = 0;
    StArrayList<StMonitor> aMonitors = StCore::getStMonitors();
    for(size_t aMonIter = 0; aMonIter < aMonitors.size(); ++aMonIter) {
        const StMonitor& aMon = aMonitors[aMonIter];
        if(aMon.getFreqMax() > hfreqMax) {
            hfreqMax = aMon.getFreqMax();
            hfreqMon = aMonIter;
        }
    }
    return !aMonitors.isEmpty() ? aMonitors[hfreqMon] : StMonitor();
}

void StOutPageFlip::optionsStructAlloc() {
    StTranslations aLangMap(ST_OUT_PLUGIN_NAME);

    // create device options structure
    myOptions = (StSDOptionsList_t* )StWindow::memAlloc(sizeof(StSDOptionsList_t)); stMemSet(myOptions, 0, sizeof(StSDOptionsList_t));
    myOptions->curRendererPath = StWindow::memAllocNCopy(myPluginPath);
    myOptions->curDeviceId = myDevice;

    myOptions->optionsCount = myDeviceOptionsNb;
    myOptions->options = (StSDOption_t** )StWindow::memAlloc(sizeof(StSDOption_t*) * myOptions->optionsCount);

    // Show FPS option
    myOptions->options[DEVICE_OPTION_SHOWFPS] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDOnOff_t));
    myOptions->options[DEVICE_OPTION_SHOWFPS]->optionType = ST_DEVICE_OPTION_ON_OFF;
    ((StSDOnOff_t* )myOptions->options[DEVICE_OPTION_SHOWFPS])->value = myToShowFPS;
    myOptions->options[DEVICE_OPTION_SHOWFPS]->title = StWindow::memAllocNCopy(aLangMap.changeValueId(STTR_PARAMETER_SHOW_FPS, "Show FPS"));

    // Show Extra option
    myOptions->options[DEVICE_OPTION_EXTRA] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDOnOff_t));
    myOptions->options[DEVICE_OPTION_EXTRA]->optionType = ST_DEVICE_OPTION_ON_OFF;
    ((StSDOnOff_t* )myOptions->options[DEVICE_OPTION_EXTRA])->value = false;
    myOptions->options[DEVICE_OPTION_EXTRA]->title = StWindow::memAllocNCopy("Show Extra Options");

    // Quad Buffer type option
    myOptions->options[DEVICE_OPTION_QUADBUFFER] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDSwitch_t));
    myOptions->options[DEVICE_OPTION_QUADBUFFER]->title = StWindow::memAllocNCopy(aLangMap.changeValueId(STTR_PARAMETER_QBUFFER_TYPE, "Quad Buffer type"));
    myOptions->options[DEVICE_OPTION_QUADBUFFER]->optionType = ST_DEVICE_OPTION_SWITCH;
    StSDSwitch_t* aSwitchQB = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_QUADBUFFER]);
    aSwitchQB->value = myQuadBuffer;
    aSwitchQB->valuesCount = myQuadBufferMax + 1;
    aSwitchQB->valuesTitles = (stUtf8_t** )StWindow::memAlloc(aSwitchQB->valuesCount * sizeof(stUtf8_t*));
    aSwitchQB->valuesTitles[QUADBUFFER_HARD_OPENGL] = StWindow::memAllocNCopy(aLangMap.changeValueId(STTR_PARAMETER_QB_HARDWARE, "OpenGL Hardware"));
    StString aDxDesc;
    if(myDxInfo.hasAqbsSupport && myDxInfo.hasNvStereoSupport) {
        aDxDesc = aLangMap.changeValueId(STTR_PARAMETER_QB_D3D_ANY,     "Direct3D (Fullscreen)");
    } else if(myDxInfo.hasAqbsSupport) {
        aDxDesc = aLangMap.changeValueId(STTR_PARAMETER_QB_D3D_AMD,     "Direct3D AMD (Fullscreen)");
    } else if(myDxInfo.hasNvStereoSupport) {
        aDxDesc = aLangMap.changeValueId(STTR_PARAMETER_QB_D3D_NV,      "Direct3D NVIDIA (Fullscreen)");
    } else if(myDxInfo.hasAmdAdapter) {
        aDxDesc = aLangMap.changeValueId(STTR_PARAMETER_QB_D3D_AMD_OFF, "Direct3D AMD (Unavailable)");
    } else if(myDxInfo.hasNvAdapter) {
        aDxDesc = aLangMap.changeValueId(STTR_PARAMETER_QB_D3D_NV_OFF,  "Direct3D NVIDIA (Disabled)");
    } else {
        aDxDesc = aLangMap.changeValueId(STTR_PARAMETER_QB_D3D_OFF,     "Direct3D (Unavailable)");
    }
    aSwitchQB->valuesTitles[QUADBUFFER_HARD_D3D_ANY] = StWindow::memAllocNCopy(aDxDesc);
}

StOutPageFlip::StOutPageFlip(const StHandle<StSettings>& theSettings)
: mySettings(theSettings),
  myWinAttribs(stDefaultWinAttributes()),
  myOptions(NULL),
  myDevice(DEVICE_AUTO),
  myDeviceOptionsNb(DEVICE_OPTION_QUADBUFFER + 1),
  myQuadBuffer(QUADBUFFER_AUTO),
  myQuadBufferMax(QUADBUFFER_HARD_D3D_ANY),
  myFPSControl(),
  myToSavePlacement(true),
  myToDrawStereo(false),
  myToShowFPS(false),
#if(defined(_WIN32) || defined(__WIN32__))
  myIsVistaPlus(StSys::isVistaPlus()),
#endif
  myOutD3d() {
    //
}

StOutPageFlip::~StOutPageFlip() {
    if(!myWarning.isNull()) {
        myWarning->release(*myContext);
        myWarning.nullify();
    }

    dxRelease();
    if(!myStCore.isNull() && !mySettings.isNull()) {
        stMemFree(myOptions, StWindow::memFree);

        // read windowed placement
        getStWindow()->hide(ST_WIN_MASTER);
        if(myToSavePlacement) {
            getStWindow()->setFullScreen(false);
            mySettings->saveInt32Rect(ST_SETTING_WINDOWPOS, getStWindow()->getPlacement());
        }
        mySettings->saveInt32(ST_SETTING_DEVICE_ID,  myDevice);
        mySettings->saveInt32(ST_SETTING_QUADBUFFER, myQuadBuffer);
    }
    myVuzixSDK.nullify();
    myStCore.nullify();
    StCore::FREE();
}

void StOutPageFlip::setupDevice() {
    switch(myDevice) {
        case DEVICE_VUZIX: {
            if(!StVuzixSDK::isConnected()) {
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
    if(myOptions != NULL) {
        myOptions->curDeviceId = myDevice;
    }
}

bool StOutPageFlip::dxInit() {
    dxRelease();
    myOutD3d.myIsActive = false;
    myOutD3d.myActivateStep = 0;
#if(defined(_WIN32) || defined(__WIN32__))
    StMonitor aNvMonitor = StCore::getMonitorFromPoint(getStWindow()->getPlacement().center());
    GLsizei aFrBufferSizeX = aNvMonitor.getVRect().width();
    GLsizei aFrBufferSizeY = aNvMonitor.getVRect().height();

    // INIT framebuffers
    if(!myContext->stglIsRectangularFboSupported()) {
        StGLFrameBuffer::convertToPowerOfTwo(*myContext, aFrBufferSizeX, aFrBufferSizeY);
        myOutD3d.myToUsePBO = false; // force compatible mode
        ST_DEBUG_LOG(ST_OUT_PLUGIN_NAME + " Plugin, old videocard detected (GLSL 1.1)!");
    }

    myOutD3d.myDxWindow = new StDXNVWindow(aFrBufferSizeX, aFrBufferSizeY, aNvMonitor, getStWindow());
    myOutD3d.myDxThread = new StThread(StOutDirect3D::dxThreadFunction, (void* )&myOutD3d);
    return true;
#else
    return false;
#endif // _WIN32
}

void StOutPageFlip::dxRelease() {
#if(defined(_WIN32) || defined(__WIN32__))
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
#if(defined(_WIN32) || defined(__WIN32__))
    // activate Direct3D Fullscreen window
    if(!myOutD3d.myIsActive
    && !myOutD3d.myDxWindow.isNull()) {
        if(myOutD3d.myActivateStep == 0) {
            // Direct3D device will fail to Reset if GL fullscreen device is active
            // so we switch out main GL window into windowed state temporarily
            // (we need to wait some time to ensure system perform needed movements)
            getStWindow()->setFullScreen(false);
            ++myOutD3d.myActivateStep;
            return;
        }
        myOutD3d.myDxWindow->waitReady();
        myOutD3d.myDxWindow->show();

        myOutD3d.myIsActive = true;
        if(myOutD3d.myActivateStep == 1) {
            // at second step switch out main GL window back to fullscreen
            myOutD3d.myActivateStep = 0;
            getStWindow()->setFullScreen(true);

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
#if(defined(_WIN32) || defined(__WIN32__))
    if(myOutD3d.myIsActive) {
        // set this flag first to make recursive calls
        myOutD3d.myIsActive = false;
        if(!myOutD3d.myDxWindow.isNull()) {
            myOutD3d.myDxWindow->hide();
            if(myOutD3d.myDxWindow->hasOwnWindow()) {
                getStWindow()->show(ST_WIN_MASTER);
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

bool StOutPageFlip::init(const StString&     inRendererPath,
                         const int&          theDeviceId,
                         const StNativeWin_t theNativeParent) {
    myToSavePlacement = (theNativeParent == (StNativeWin_t )NULL);
    myDevice = DeviceEnum(theDeviceId);
    myPluginPath = inRendererPath;
    if(!StVersionInfo::checkTimeBomb("sView - PageFlip Output plugin")) {
        return false;
    } else if(mySettings.isNull()) {
        return false;
    }
    ST_DEBUG_LOG_AT("INIT PageFlip output plugin");
    // Firstly INIT core library!
    if(StCore::INIT() != STERROR_LIBNOERROR) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, Core library not available!");
        return false;
    }
    myStCore = new StCore();

    // load window position
    StRect<int32_t> loadedRect(256, 768, 256, 1024);
    if(mySettings->loadInt32Rect(ST_SETTING_WINDOWPOS, loadedRect)) {
        StMonitor aMonitor = StCore::getMonitorFromPoint(loadedRect.center());
        if(!aMonitor.getVRect().isPointIn(loadedRect.center())) {
            ST_DEBUG_LOG("Warning, stored window position is out of the monitor(" + aMonitor.getId() + ")!" + loadedRect.toString());
            int w = loadedRect.width();
            int h = loadedRect.height();
            loadedRect.left()   = aMonitor.getVRect().left() + 256;
            loadedRect.right()  = loadedRect.left() + w;
            loadedRect.top()    = aMonitor.getVRect().top() + 256;
            loadedRect.bottom() = loadedRect.top() + h;
        }
    } else {
        // try to open window on display with highest frequency
        loadedRect = getHigestFreqMonitor().getVRect();
        loadedRect.left()   = loadedRect.left() + 256;
        loadedRect.right()  = loadedRect.left() + 1024;
        loadedRect.top()    = loadedRect.top()  + 256;
        loadedRect.bottom() = loadedRect.top()  + 512;
    }
    getStWindow()->setPlacement(loadedRect);

    // load device settings
    int deviceInt = int(myDevice);
    if(deviceInt == StRendererInfo::DEVICE_AUTO) {
        mySettings->loadInt32(ST_SETTING_DEVICE_ID, deviceInt);
        if(deviceInt == StRendererInfo::DEVICE_AUTO) {
            myDevice = DEVICE_SHUTTERS;
        } else {
            myDevice = DeviceEnum(deviceInt);
        }
    }

    // load Quad Buffer type
    int32_t quadBufferTypeInt = myQuadBuffer;
    mySettings->loadInt32(ST_SETTING_QUADBUFFER, quadBufferTypeInt);
    myQuadBuffer = QuadBufferEnum(quadBufferTypeInt);
    if(myQuadBuffer < 0 || myQuadBuffer > myQuadBufferMax) {
        myQuadBuffer = QUADBUFFER_AUTO;
    }

#if(defined(_WIN32) || defined(__WIN32__))
    StDXManager::getInfo(myDxInfo);
#endif

    if(myQuadBuffer == QUADBUFFER_AUTO) {
        // first call - try to detect best
        myQuadBuffer = testQuadBufferSupportThreaded() ? QUADBUFFER_HARD_OPENGL : QUADBUFFER_HARD_D3D_ANY;
    }

    // allocate and setup the structure pointer
    optionsStructAlloc();
    getStWindow()->setValue(ST_WIN_DATAKEYS_RENDERER, (size_t )myOptions);

    // create our window!
    myWinAttribs.isGlStereo = (myQuadBuffer == QUADBUFFER_HARD_OPENGL);
    getStWindow()->stglCreate(&myWinAttribs, theNativeParent);

    // initialize GL context
    myContext = new StGLContext();
    if(!myContext->stglInit()) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, OpenGL context is broken!\n(OpenGL library internal error?)");
        return false;
    } else if(!myContext->isGlGreaterEqual(2, 0)) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, OpenGL2.0+ not available!");
        return false;
    }
    if(!myContext->stglSetVSync(true)) {
        // TODO (Kirill Gavrilov#5#) could be optional for MONO output
        ST_DEBUG_LOG(ST_OUT_PLUGIN_NAME + " Plugin, VSync extension not available!");
    }

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
    if(myQuadBuffer == QUADBUFFER_HARD_D3D_ANY) {
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

void StOutPageFlip::stglResize(const StRectI_t& theWinRect) {
    getStWindow()->stglMakeCurrent(ST_WIN_MASTER);
    myContext->core20fwd->glViewport(0, 0, theWinRect.width(), theWinRect.height()); // reset master window Viewport
}

void StOutPageFlip::parseKeys(bool* theKeysMap) {
    if(theKeysMap[ST_VK_F11]) {
        getStWindow()->stglSwap(ST_WIN_MASTER);
        theKeysMap[ST_VK_F11] = false;
    }
    if(theKeysMap[ST_VK_F12]) {
        myToShowFPS = !myToShowFPS;
        theKeysMap[ST_VK_F12] = false;

        // send 'update' message to StDrawer
        StMessage_t aMsg; aMsg.uin = StMessageList::MSG_DEVICE_OPTION;
        StSDSwitch_t* anOption = ((StSDSwitch_t* )myOptions->options[DEVICE_OPTION_SHOWFPS]);
        anOption->value = myToShowFPS; aMsg.data = (void* )anOption;
        getStWindow()->appendMessage(aMsg);
    }
}

void StOutPageFlip::updateOptions(const StSDOptionsList_t* theOptions,
                                  StMessage_t&             theMsg) {
    myToShowFPS = ((StSDOnOff_t* )theOptions->options[DEVICE_OPTION_SHOWFPS])->value;
    QuadBufferEnum newQuadBuffer = QuadBufferEnum(((StSDSwitch_t* )theOptions->options[DEVICE_OPTION_QUADBUFFER])->value);

    bool toShowExtra = ((StSDOnOff_t* )myOptions->options[DEVICE_OPTION_EXTRA])->value;
    mySettings->saveBool(ST_SETTING_ADVANCED, toShowExtra);
    bool isExtra = (dynamic_cast<StOutPageFlipExt*>(this) != NULL);
    if(toShowExtra != isExtra) {
        // force to restart plugin
        theMsg.uin = StMessageList::MSG_DEVICE_INFO;
    }

    if((myQuadBuffer == QUADBUFFER_HARD_OPENGL && newQuadBuffer != QUADBUFFER_HARD_OPENGL) ||
       (myQuadBuffer != QUADBUFFER_HARD_OPENGL && newQuadBuffer == QUADBUFFER_HARD_OPENGL)) {
        // force to restart plugin
        theMsg.uin = StMessageList::MSG_DEVICE_INFO;
    } else if(myQuadBuffer == QUADBUFFER_HARD_D3D_ANY && newQuadBuffer != QUADBUFFER_HARD_D3D_ANY) {
        dxRelease();
    } else if(myQuadBuffer != QUADBUFFER_HARD_D3D_ANY && newQuadBuffer == QUADBUFFER_HARD_D3D_ANY) {
        dxInit();
    }
    myQuadBuffer = newQuadBuffer;
}

void StOutPageFlip::callback(StMessage_t* stMessages) {
    myStCore->callback(stMessages);
    for(size_t i = 0; stMessages[i].uin != StMessageList::MSG_NULL; ++i) {
        switch(stMessages[i].uin) {
            case StMessageList::MSG_RESIZE: {
                stglResize(getStWindow()->getPlacement());
                break;
            }
            case StMessageList::MSG_KEYS: {
                parseKeys((bool* )stMessages[i].data);
                break;
            }
            case StMessageList::MSG_DEVICE_INFO: {
                if(myOptions->curDeviceId != int(myDevice)) {
                    StString newPluginPath(myOptions->curRendererPath);
                    if(newPluginPath == myPluginPath) {
                        myDevice = DeviceEnum(myOptions->curDeviceId);
                        setupDevice();
                        stMessages[i].uin = StMessageList::MSG_NONE;
                    } // else - another plugin
                }
                break;
            }
            case StMessageList::MSG_DEVICE_OPTION: {
                updateOptions(myOptions, stMessages[i]);
                break;
            }
        #if(defined(_WIN32) || defined(__WIN32__))
            case StMessageList::MSG_WIN_ON_NEW_MONITOR: {
                if(myQuadBuffer != QUADBUFFER_HARD_D3D_ANY || myOutD3d.myIsActive) {
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
#if(defined(_WIN32) || defined(__WIN32__))
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

    const StRectI_t aRect = getStWindow()->getPlacement();
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

void StOutPageFlip::stglDraw(unsigned int ) {
    myFPSControl.setTargetFPS(getStWindow()->stglGetTargetFps());
    if(myToShowFPS && myFPSControl.isUpdated()) {
        getStWindow()->setTitle(StString("PageFlip Rendering FPS= ") + myFPSControl.getAverage());
    }

    getStWindow()->stglMakeCurrent(ST_WIN_MASTER);
    if(!getStWindow()->isStereoOutput()) {
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
        if(myQuadBuffer == QUADBUFFER_HARD_OPENGL) {
            myContext->core20fwd->glDrawBuffer(GL_BACK);
        }

        // draw new MONO frame
        myStCore->stglDraw(ST_DRAW_LEFT);
        if(myDevice != DEVICE_VUZIX) {
            stglDrawExtra(ST_DRAW_LEFT, StGLDeviceControl::OUT_MONO);
        }

        // decrease FPS to target by thread sleeps
        myFPSControl.sleepToTarget();
        getStWindow()->stglSwap(ST_WIN_MASTER);
        ++myFPSControl;
        return;
    } else if(!myToDrawStereo) {
        if(myDevice == DEVICE_VUZIX && !myVuzixSDK.isNull() && myQuadBuffer != QUADBUFFER_HARD_OPENGL) {
            myVuzixSDK->setStereoOut();
        }
        myToDrawStereo = true;
    }

    switch(myQuadBuffer) {
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
                myStCore->stglDraw(ST_DRAW_RIGHT);
                myStCore->stglDraw(ST_DRAW_LEFT);
                stglDrawWarning();
            } else {
                myContext->core20fwd->glDrawBuffer(GL_BACK_LEFT);
                myStCore->stglDraw(ST_DRAW_LEFT);
                stglDrawExtra(ST_DRAW_LEFT, StGLDeviceControl::OUT_STEREO);

                myContext->core20fwd->glDrawBuffer(GL_BACK_RIGHT);
                myStCore->stglDraw(ST_DRAW_RIGHT);
                stglDrawExtra(ST_DRAW_RIGHT, StGLDeviceControl::OUT_STEREO);
            }
            StThread::sleep(1);
            getStWindow()->stglSwap(ST_WIN_MASTER);
            ++myFPSControl;
            return;
        }
    #if(defined(_WIN32) || defined(__WIN32__))
        case QUADBUFFER_HARD_D3D_ANY: {
            if(myOutD3d.myActivateStep == 1 || getStWindow()->isFullScreen()) {
                dxActivate();

                // draw into virtual frame buffers (textures)
                if(!myOutD3d.myGLBuffer.isNull()) {
                    GLint aVPort[4]; // real window viewport
                    myContext->core20fwd->glGetIntegerv(GL_VIEWPORT, aVPort);
                        myOutD3d.myGLBuffer->setupViewPort(*myContext); // we set TEXTURE sizes here
                        myOutD3d.myGLBuffer->bindBuffer(*myContext);
                            myStCore->stglDraw(ST_DRAW_LEFT);
                            stglDrawExtra(ST_DRAW_LEFT, StGLDeviceControl::OUT_STEREO);
                            while(myOutD3d.myDxWindow->isInUpdate()) {
                                StThread::sleep(2);
                            }
                            // read OpenGL buffers and write into Direct3D surface
                            dxDraw(ST_DRAW_LEFT);
                            myStCore->stglDraw(ST_DRAW_RIGHT);
                            stglDrawExtra(ST_DRAW_RIGHT, StGLDeviceControl::OUT_STEREO);
                            dxDraw(ST_DRAW_RIGHT);
                        myOutD3d.myGLBuffer->unbindBuffer(*myContext);
                    myContext->core20fwd->glViewport(aVPort[0], aVPort[1], aVPort[2], aVPort[3]);
                }
                break;
            } else { //is windowed output - we doesn't break, just use aggressive behaviour!
                // disactivate Direct3D Fullscreen window
                dxDisactivate();

                getStWindow()->stglMakeCurrent(ST_WIN_MASTER);
                myStCore->stglDraw(ST_DRAW_RIGHT); // reverse order to avoid non-smooth mono->stereo transition
                myStCore->stglDraw(ST_DRAW_LEFT);

                stglDrawWarning();

                myFPSControl.sleepToTarget();
                getStWindow()->stglSwap(ST_WIN_MASTER);
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
    getStWindow()->stglMakeCurrent(ST_WIN_MASTER);
    myStCore->stglDraw(theView);

    if(myDevice == DEVICE_VUZIX) {
        if(!myVuzixSDK.isNull()) {
            if(theView == ST_DRAW_LEFT) { myVuzixSDK->waitAckLeft(); } else { myVuzixSDK->waitAckRight(); }
        }
    } else {
        stglDrawExtra(theView, StGLDeviceControl::OUT_STEREO);
    }

    getStWindow()->stglSwap(ST_WIN_MASTER);
    ++myFPSControl;

    // Inform VR920 to begin scanning on next vSync, a new right eye frame.
    if(myDevice == DEVICE_VUZIX && !myVuzixSDK.isNull()) {
        if(theView == ST_DRAW_LEFT) { myVuzixSDK->setLeft(); } else { myVuzixSDK->setRight(); }
    }
}

// SDK version was used
ST_EXPORT void getSDKVersion(StVersion* ver) {
    *ver = StVersionInfo::getSDKVersion();
}

// plugin version
ST_EXPORT void getPluginVersion(StVersion* ver) {
    *ver = StVersionInfo::getSDKVersion();
}

ST_EXPORT const StRendererInfo_t* getDevicesInfo(const stBool_t theToDetectPriority) {
    static StRendererInfo_t ST_SELF_INFO = { NULL, NULL, NULL, 0 };
    if(ST_SELF_INFO.devices != NULL) {
        return &ST_SELF_INFO;
    }

    StTranslations aLangMap(ST_OUT_PLUGIN_NAME);

    // detect connected displays
    int supportLevelShutters = ST_DEVICE_SUPPORT_NONE;
    int supportLevelVuzix    = ST_DEVICE_SUPPORT_NONE;
    if(theToDetectPriority) {
        if(StCore::INIT() != STERROR_LIBNOERROR) {
            ST_DEBUG_LOG(ST_OUT_PLUGIN_NAME + " Plugin, Core library not available!");
        } else {
            if(StVuzixSDK::isConnected()) {
                supportLevelVuzix = ST_DEVICE_SUPPORT_PREFER;
            }
            StMonitor aMon = getHigestFreqMonitor();
            if(aMon.getFreqMax() >= 110) {
                supportLevelShutters = ST_DEVICE_SUPPORT_HIGHT;
            }
        #if !(defined(__APPLE__))
            // actually almost always available on mac but... is it useful?
            if(testQuadBufferSupportThreaded()) {
                supportLevelShutters = ST_DEVICE_SUPPORT_FULL;
            }
        #endif
            StCore::FREE();
        }

    #if(defined(_WIN32) || defined(__WIN32__))
        StDXInfo aDxInfo;
        if(supportLevelShutters != ST_DEVICE_SUPPORT_FULL
        && StDXManager::getInfoThreaded(aDxInfo)
        && (aDxInfo.hasNvStereoSupport || aDxInfo.hasAqbsSupport)) {
            // if user enable NVIDIA stereo driver - we prefer use it for shutters
            supportLevelShutters = ST_DEVICE_SUPPORT_FULL;
        }
    #endif
    }

    static StString aPageFlipName = aLangMap.changeValueId(STTR_PAGEFLIP_NAME, "Shutter glasses");
    static StString aPageFlipDesc = aLangMap.changeValueId(STTR_PAGEFLIP_DESC, "Shutter glasses");
    static StString aVuzixName    = aLangMap.changeValueId(STTR_VUZIX_NAME,    "Vuzix HMD");
    static StString aVuzixDesc    = aLangMap.changeValueId(STTR_VUZIX_DESC,    "Vuzix HMD");

    static StStereoDeviceInfo_t aDevicesArray[2] = {
        { "Pageflip", aPageFlipName.toCString(), aPageFlipDesc.toCString(), supportLevelShutters },
        { "Vuzix",    aVuzixName.toCString(),    aVuzixDesc.toCString(),    supportLevelVuzix    }
    };
    ST_SELF_INFO.devices = &aDevicesArray[0];
    ST_SELF_INFO.count   = 2;

    // about string
    StString& aTitle     = aLangMap.changeValueId(STTR_PLUGIN_TITLE,   "sView - PageFlip Output module");
    StString& aVerString = aLangMap.changeValueId(STTR_VERSION_STRING, "version");
    StString& aDescr     = aLangMap.changeValueId(STTR_PLUGIN_DESCRIPTION,
        "(C) 2007-2013 Kirill Gavrilov <kirill@sview.ru>\nOfficial site: www.sview.ru\n\nThis library distributed under LGPL3.0");
    static StString anAboutString = aTitle + '\n' + aVerString + ": " + StVersionInfo::getSDKVersionString() + "\n \n" + aDescr;
    ST_SELF_INFO.aboutString = (stUtf8_t* )anAboutString.toCString();

    return &ST_SELF_INFO;
}

ST_EXPORT StRendererInterface* StRenderer_new() {
    // INIT settings library
    StHandle<StSettings> aSettings = new StSettings(ST_OUT_PLUGIN_NAME);
    bool isAdvanced = false;
    aSettings->loadBool(ST_SETTING_ADVANCED, isAdvanced);
    if(isAdvanced) {
        return new StOutPageFlipExt(aSettings);
    } else {
        return new StOutPageFlip(aSettings);
    }
}
ST_EXPORT void StRenderer_del(StRendererInterface* inst) {
    delete (StOutPageFlip* )inst;
}
ST_EXPORT StWindowInterface* StRenderer_getStWindow(StRendererInterface* inst) {
    // This is VERY important return libImpl pointer here!
    return ((StOutPageFlip* )inst)->getStWindow()->getLibImpl(); }
ST_EXPORT stBool_t StRenderer_init(StRendererInterface* inst,
                                   const stUtf8_t*      theRendererPath,
                                   const int&           theDeviceId,
                                   const StNativeWin_t  theNativeParent) {
    return ((StOutPageFlip* )inst)->init(StString(theRendererPath), theDeviceId, theNativeParent); }
ST_EXPORT stBool_t StRenderer_open(StRendererInterface* inst, const StOpenInfo_t* stOpenInfo) {
    return ((StOutPageFlip* )inst)->open(StOpenInfo(stOpenInfo)); }
ST_EXPORT void StRenderer_callback(StRendererInterface* inst, StMessage_t* stMessages) {
    ((StOutPageFlip* )inst)->callback(stMessages); }
ST_EXPORT void StRenderer_stglDraw(StRendererInterface* inst, unsigned int views) {
    ((StOutPageFlip* )inst)->stglDraw(views); }
