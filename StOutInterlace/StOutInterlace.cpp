/**
 * StOutInterlace, class providing stereoscopic output in row interlaced format using StCore toolkit.
 * Copyright © 2009-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#include "StOutInterlace.h"

#include <StGL/StGLContext.h>
#include <StGL/StGLEnums.h>
#include <StGL/StGLVec.h>
#include <StGLCore/StGLCore20.h>
#include <StSettings/StSettings.h>
#include <StSettings/StTranslations.h>
#include <StSettings/StEnumParam.h>
#include <StCore/StSearchMonitors.h>
#include <StImage/StImagePlane.h>
#include <StVersion.h>

#include <fstream>

namespace {

    static const char ST_OUT_PLUGIN_NAME[] = "StOutInterlace";

    // shaders data files
    static const char VSHADER_ED[]              = "vED.shv";
    static const char FSHADER_EDINTERLACE_ON[]  = "fEDinterlace.shf";
    static const char FSHADER_ED_OFF[]          = "fEDoff.shf";

    static const char ST_SETTING_DEVICE_ID[]    = "deviceId";
    static const char ST_SETTING_WINDOWPOS[]    = "windowPos";

    struct StMonInterlacedInfo_t {
        const stUtf8_t* pnpid;
        bool            isReversed;
        bool            isRowInterlaced;
    };

    /**
     * Database of known interlaced monitors.
     */
    static const StMonInterlacedInfo_t THE_KNOWN_MONITORS[] = {
        {"ZMT1900", false, true}, // Zalman Trimon M190S
        {"ZMT2200", false, true}, // Zalman Trimon M220W
        {"ENV2373", true , true}, // Envision
        {"HIT8002", false, true}, // Hyundai W220S D-Sub
        {"HIT8D02", false, true}, // Hyundai W220S DVID
        {"HIT7003", false, true}, // Hyundai W240S D-Sub
        {"HIT7D03", false, true}, // Hyundai W240S D-Sub
        {"ACI23D3", false, true}, // ASUS VG23AH
        {"ACI27C2", false, true}, // ASUS VG27AH
        //
        {"ST@COL0", false, false}, // Android devices with parallel barrier (column-interleaved)
        {       "", false, false}  // NULL-terminate array
    };

#if defined(__ANDROID__)
    // Truly / Freevi / Commander  activation
    namespace mi3d_tn {
        static const char* CTRL_FILE = "/dev/mi3d_tn_ctrl";
        enum {
            SET_TN_OFF           =  16,
            SET_VERTICAL_TN_ON   =  32,
            SET_HORIZONTAL_TN_ON =  64,
            GET_TN_STATUS        = 128,
        };
    }
#endif

    // translation resources
    enum {
        STTR_HINTERLACE_NAME    = 1000,
        STTR_HINTERLACE_DESC    = 1001,
        STTR_VINTERLACE_NAME    = 1002,
        STTR_VINTERLACE_DESC    = 1003,
        STTR_CHESSBOARD_NAME    = 1006,
        STTR_CHESSBOARD_DESC    = 1007,
        STTR_HINTERLACE_ED_NAME = 1008,
        STTR_HINTERLACE_ED_DESC = 1009,

        // parameters
        STTR_PARAMETER_REVERSE  = 1102,
        STTR_PARAMETER_BIND_MON = 1103,
        STTR_PARAMETER_USE_MASK = 1104,

        // about info
        STTR_PLUGIN_TITLE       = 2000,
        STTR_VERSION_STRING     = 2001,
        STTR_PLUGIN_DESCRIPTION = 2002,
    };

    static const char* ST_SHADER_TEMPLATE[3] = {
        "uniform sampler2D uTexture;\n"
        "varying vec2 fTexCoord;\n"
        "void main(void) {\n",
        "\n",
        "    gl_FragColor = texture2D(uTexture, fTexCoord);\n"
        "}\n"
    };

    static const StGLVarLocation ST_VATTRIB_VERTEX(0);
    static const StGLVarLocation ST_VATTRIB_TCOORD(1);

    inline bool isInterlacedMonitor(const StMonitor& theMon,
                                    bool&            theIsReversed,
                                    bool&            theIsRowInterlaced) {
        if(theMon.getPnPId().getSize() != 7) {
            return false;
        }
        for(size_t anIter = 0;; ++anIter) {
            const StMonInterlacedInfo_t& aMon = THE_KNOWN_MONITORS[anIter];
            if(aMon.pnpid[0] == '\0') {
                return false;
            } else if(stAreEqual(aMon.pnpid, theMon.getPnPId().toCString(), 7)) {
                theIsReversed      = aMon.isReversed;
                theIsRowInterlaced = aMon.isRowInterlaced;
                return true;
            }
        }
    }

}

StProgramFB::StProgramFB(const StString& theTitle)
: StGLProgram(theTitle) {
    //
}

bool StProgramFB::link(StGLContext& theCtx) {
    StGLProgram::bindAttribLocation(theCtx, "vVertex",   ST_VATTRIB_VERTEX);
    StGLProgram::bindAttribLocation(theCtx, "vTexCoord", ST_VATTRIB_TCOORD);

    if(!StGLProgram::link(theCtx)) {
        return false;
    }
    StGLVarLocation aTextureLoc  = StGLProgram::getUniformLocation(theCtx, "uTexture");
    StGLVarLocation aTexture2Loc = StGLProgram::getUniformLocation(theCtx, "uMaskTexture");
    if(aTextureLoc.isValid()) {
        use(theCtx);
        theCtx.core20fwd->glUniform1i(aTextureLoc, StGLProgram::TEXTURE_SAMPLE_0); // GL_TEXTURE0
        if(aTexture2Loc.isValid()) {
            theCtx.core20fwd->glUniform1i(aTexture2Loc, StGLProgram::TEXTURE_SAMPLE_1); // GL_TEXTURE1
        }
        unuse(theCtx);
    }
    return aTextureLoc.isValid();
}

StAtomic<int32_t> StOutInterlace::myInstancesNb(0);

StHandle<StMonitor> StOutInterlace::getInterlacedMonitor(const StArrayList<StMonitor>& theMonitors,
                                                         bool& theIsReversed,
                                                         bool& theIsRowInterlaced) {
    for(size_t aMonIter = 0; aMonIter < theMonitors.size(); ++aMonIter) {
        const StMonitor& aMon = theMonitors[aMonIter];
        if(isInterlacedMonitor(aMon, theIsReversed, theIsRowInterlaced)) {
            return new StMonitor(aMon);
        }
    }
    return NULL;
}

StString StOutInterlace::getRendererAbout() const {
    return myAbout;
}

const char* StOutInterlace::getRendererId() const {
    return ST_OUT_PLUGIN_NAME;
}

const char* StOutInterlace::getDeviceId() const {
    switch(myDevice) {
        case DEVICE_COL_INTERLACED:      return "Col";
        case DEVICE_COL_INTERLACED_MI3D: return "ColMI3D";
        case DEVICE_CHESSBOARD:          return "Chess";
        case DEVICE_ROW_INTERLACED_ED:   return "RowED";
        case DEVICE_ROW_INTERLACED:
        default:                         return "Row";
    }
}

bool StOutInterlace::setDevice(const StString& theDevice) {
    if(theDevice == "Row") {
        myDevice = DEVICE_ROW_INTERLACED;
    } else if(theDevice == "Col") {
        myDevice = DEVICE_COL_INTERLACED;
    } else if(theDevice == "ColMI3D") {
        myDevice = DEVICE_COL_INTERLACED_MI3D;
    } else if(theDevice == "Chess") {
        myDevice = DEVICE_CHESSBOARD;
    } else if(theDevice == "RowED") {
        myDevice = DEVICE_ROW_INTERLACED_ED;
    }
    return false;
}

void StOutInterlace::getDevices(StOutDevicesList& theList) const {
    for(size_t anIter = 0; anIter < myDevices.size(); ++anIter) {
        if(myDevices[anIter]->Priority == ST_DEVICE_SUPPORT_IGNORE) {
            continue;
        }
        theList.add(myDevices[anIter]);
    }
}

void StOutInterlace::getOptions(StParamsList& theList) const {
    theList.add(params.ToReverse);
#if !defined(__ANDROID__)
    theList.add(params.BindToMon);
#endif
    theList.add(params.ToUseMask);
}

void StOutInterlace::updateStrings() {
    StTranslations aLangMap(getResourceManager(), ST_OUT_PLUGIN_NAME);
    myDevices[DEVICE_ROW_INTERLACED]   ->Name = aLangMap.changeValueId(STTR_HINTERLACE_NAME,    "Row Interlaced");
    myDevices[DEVICE_ROW_INTERLACED]   ->Desc = aLangMap.changeValueId(STTR_HINTERLACE_DESC,    "Row interlaced displays: Zalman, Hyundai,...");
    myDevices[DEVICE_COL_INTERLACED]   ->Name = aLangMap.changeValueId(STTR_VINTERLACE_NAME,    "Column Interlaced");
    myDevices[DEVICE_COL_INTERLACED]   ->Desc = aLangMap.changeValueId(STTR_VINTERLACE_DESC,    "Column interlaced displays");
    myDevices[DEVICE_CHESSBOARD]       ->Name = aLangMap.changeValueId(STTR_CHESSBOARD_NAME,    "DLP TV (chessboard)");
    myDevices[DEVICE_CHESSBOARD]       ->Desc = aLangMap.changeValueId(STTR_CHESSBOARD_DESC,    "DLP TV (chessboard)");
    myDevices[DEVICE_ROW_INTERLACED_ED]->Name = aLangMap.changeValueId(STTR_HINTERLACE_ED_NAME, "Interlaced ED");
    myDevices[DEVICE_ROW_INTERLACED_ED]->Desc = aLangMap.changeValueId(STTR_HINTERLACE_ED_DESC, "EDimensional in interlaced mode");
    myDevices[DEVICE_COL_INTERLACED_MI3D]->Name = "Interlaced [MI3D]";
    myDevices[DEVICE_COL_INTERLACED_MI3D]->Desc = "Interlaced display with parallel barrier";

    params.ToReverse->setName(aLangMap.changeValueId(STTR_PARAMETER_REVERSE,  "Reverse Order"));
    params.BindToMon->setName(aLangMap.changeValueId(STTR_PARAMETER_BIND_MON, "Bind To Supported Monitor"));
    params.ToUseMask->setName(aLangMap.changeValueId(STTR_PARAMETER_USE_MASK, "Use texture mask (compatibility)"));

    // about string
    StString& aTitle     = aLangMap.changeValueId(STTR_PLUGIN_TITLE,   "sView - Interlaced Output library");
    StString& aVerString = aLangMap.changeValueId(STTR_VERSION_STRING, "version");
    StString& aDescr     = aLangMap.changeValueId(STTR_PLUGIN_DESCRIPTION,
        "(C) {0} Kirill Gavrilov <{1}>\nOfficial site: {2}\n\nThis library is distributed under LGPL3.0");
    myAbout = aTitle + '\n' + aVerString + " " + StVersionInfo::getSDKVersionString() + "\n \n"
            + aDescr.format("2009-2020", "kirill@sview.ru", "www.sview.ru");
}

StOutInterlace::StOutInterlace(const StHandle<StResourceManager>& theResMgr,
                               const StNativeWin_t                theParentWindow)
: StWindow(theResMgr, theParentWindow),
  mySettings(new StSettings(theResMgr, ST_OUT_PLUGIN_NAME)),
  myFrmBuffer(new StGLFrameBuffer()),
  myTextureMask(new StGLTexture(GL_ALPHA)),
  myTexMaskDevice(DEVICE_AUTO),
  myTexMaskReversed(false),
  myDevice(DEVICE_AUTO),
  myBarrierState(BarrierState_Unknown),
  myEDTimer(true),
  myEDIntelaceOn(new StGLProgram("ED Interlace On")),
  myEDOff(new StGLProgram("ED Interlace Off")),
  myVpSizeY(10),
  myIsMonReversed(false),
  myIsMonPortrait(false),
  myIsStereo(false),
  myIsEDactive(false),
  myIsEDCodeFinished(false),
  myToCompressMem(myInstancesNb.increment() > 1),
  myIsFirstDraw(true),
  myIsBroken(false) {
    myWinRect.left()   = 0;
    myWinRect.right()  = 0;
    myWinRect.top()    = 0;
    myWinRect.bottom() = 0;
    myEDRect.left()    = 0;
    myEDRect.right()   = 1;
    myEDRect.top()     = 0;
    myEDRect.bottom()  = 10;
    StWindow::signals.onAnotherMonitor = stSlot(this, &StOutInterlace::doNewMonitor);

    const StSearchMonitors& aMonitors = StWindow::getMonitors();
    myGlPrograms[DEVICE_ROW_INTERLACED]       = new StProgramFB("Row Interlace");
    myGlPrograms[DEVICE_COL_INTERLACED]       = new StProgramFB("Column Interlace");
    myGlPrograms[DEVICE_CHESSBOARD]           = new StProgramFB("Chessboard");
    myGlPrograms[DEVICE_ROW_INTERLACED_ED]    = myGlPrograms[DEVICE_ROW_INTERLACED];
    myGlPrograms[DEVICE_COL_INTERLACED_MI3D]  = myGlPrograms[DEVICE_COL_INTERLACED];

    myGlProgramsRev[DEVICE_ROW_INTERLACED]      = new StProgramFB("Row Interlace Inversed");
    myGlProgramsRev[DEVICE_COL_INTERLACED]      = new StProgramFB("Column Interlace Inversed");
    myGlProgramsRev[DEVICE_CHESSBOARD]          = new StProgramFB("Chessboard Inversed");
    myGlProgramsRev[DEVICE_ROW_INTERLACED_ED]   = myGlProgramsRev[DEVICE_ROW_INTERLACED];
    myGlProgramsRev[DEVICE_COL_INTERLACED_MI3D] = myGlProgramsRev[DEVICE_COL_INTERLACED];

    myGlProgramMask = new StProgramFB("Interlace Mask");

    // devices list
    StHandle<StOutDevice> aDevRow = new StOutDevice();
    aDevRow->PluginId = ST_OUT_PLUGIN_NAME;
    aDevRow->DeviceId = stCString("Row");
    aDevRow->Priority = ST_DEVICE_SUPPORT_NONE;
    aDevRow->Name     = stCString("Row Interlaced");
    myDevices.add(aDevRow);

    StHandle<StOutDevice> aDevCol = new StOutDevice();
    aDevCol->PluginId = ST_OUT_PLUGIN_NAME;
    aDevCol->DeviceId = stCString("Col");
    aDevCol->Priority = ST_DEVICE_SUPPORT_NONE;
    aDevCol->Name     = stCString("Column Interlaced");
    myDevices.add(aDevCol);

    StHandle<StOutDevice> aDevChess = new StOutDevice();
    aDevChess->PluginId = ST_OUT_PLUGIN_NAME;
    aDevChess->DeviceId = stCString("Chess");
#if defined(__ANDROID__)
    aDevChess->Priority = ST_DEVICE_SUPPORT_IGNORE;
#else
    aDevChess->Priority = ST_DEVICE_SUPPORT_NONE;
#endif
    aDevChess->Name     = stCString("DLP TV (chessboard)");
    myDevices.add(aDevChess);

    StHandle<StOutDevice> aDevED = new StOutDevice();
    aDevED->PluginId = ST_OUT_PLUGIN_NAME;
    aDevED->DeviceId = stCString("RowED");
#if defined(__ANDROID__)
    aDevED->Priority = ST_DEVICE_SUPPORT_IGNORE;
#else
    aDevED->Priority = ST_DEVICE_SUPPORT_NONE;
#endif
    aDevED->Name     = stCString("Interlaced ED");
    myDevices.add(aDevED);

    StHandle<StOutDevice> aDevMI3D = new StOutDevice();
    aDevMI3D->PluginId = ST_OUT_PLUGIN_NAME;
    aDevMI3D->DeviceId = stCString("ColMI3D");
    aDevMI3D->Priority = ST_DEVICE_SUPPORT_IGNORE;
#if defined(__ANDROID__)
    {
        std::fstream aCtrlFile;
        aCtrlFile.open(mi3d_tn::CTRL_FILE, std::ios_base::in);
        if(aCtrlFile.is_open()) {
            aCtrlFile.close();
            aDevMI3D->Priority = ST_DEVICE_SUPPORT_PREFER;
            setBarrierState(BarrierState_Off);
        }
    }
#endif
    aDevMI3D->Name = stCString("Interlaced [MI3D]");
    myDevices.add(aDevMI3D);

    // detect connected displays
    bool isDummyReversed = false;
    bool isRowInterlaced = false;
    StHandle<StMonitor> aMonInterlaced = getInterlacedMonitor(aMonitors, isDummyReversed, isRowInterlaced);
    if(!aMonInterlaced.isNull()) {
        if(isRowInterlaced) {
            aDevRow->Priority = ST_DEVICE_SUPPORT_PREFER;
        } else {
            aDevCol->Priority = ST_DEVICE_SUPPORT_PREFER;
        }
    }

    // options
    params.ToReverse = new StBoolParamNamed(false, stCString("reverse"),     stCString("reverse"));
    params.BindToMon = new StBoolParamNamed(true,  stCString("bindMonitor"), stCString("bindMonitor"));
    params.ToUseMask = new StBoolParamNamed(false, stCString("useMask"),     stCString("useMask"));
    updateStrings();

    mySettings->loadParam(params.ToReverse);
    mySettings->loadParam(params.BindToMon);
    myIsFirstDraw = !mySettings->loadParam(params.ToUseMask);
    params.BindToMon->signals.onChanged.connect(this, &StOutInterlace::doSetBindToMonitor);

    // load window position
    if(isMovable()) {
        const int32_t aDefWidth  = 768;
        const int32_t aDefHeight = 512;
        StRect<int32_t> aRect(256, 256 + aDefHeight,
                              256, 256 + aDefWidth);
        const bool isLoadedPosition = mySettings->loadInt32Rect(ST_SETTING_WINDOWPOS, aRect);
        StMonitor aMonitor = aMonitors[aRect.center()];
        if(params.BindToMon->getValue()
        && !aMonInterlaced.isNull()
        && !isInterlacedMonitor(aMonitor, isDummyReversed, isRowInterlaced)) {
            aMonitor = *aMonInterlaced;
        }
        if(isLoadedPosition) {
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
            // try to open window on correct display
            aRect = defaultRect(&aMonitor);
        }
        StWindow::setPlacement(aRect);
    }

    // setup myIsMonReversed value for actual placement
    if(!aMonInterlaced.isNull()) {
        myIsMonReversed = false;
        StMonitor aMonitor = aMonitors[StWindow::getPlacement().center()];
        isInterlacedMonitor(aMonitor, myIsMonReversed, isRowInterlaced);
    }

    // load device settings
    mySettings->loadInt32(ST_SETTING_DEVICE_ID, myDevice);
    if(myDevice == DEVICE_AUTO
    || myDevice >= (int )myDevices.size()
    || myDevices[myDevice]->Priority == ST_DEVICE_SUPPORT_IGNORE) {
        myDevice = DEVICE_ROW_INTERLACED;
    }

    // request slave window
    const StWinAttr anAttribs[] = {
        StWinAttr_SlaveCfg,    (StWinAttr )StWinSlave_slaveHLineTop,
        StWinAttr_ToAlignEven, (StWinAttr )true,
        StWinAttr_NULL
    };
    StWindow::setAttributes(anAttribs);
    StWindow::hide(ST_WIN_SLAVE); // slave is hidden by default
}

void StOutInterlace::releaseResources() {
    if(!myContext.isNull()) {
        for(size_t anIter = 0; anIter < DEVICE_NB; ++anIter) {
            myGlPrograms   [anIter]->release(*myContext);
            myGlProgramsRev[anIter]->release(*myContext);
        }
        myEDIntelaceOn->release(*myContext);
        myEDOff->release(*myContext);
        myQuadVertBuf.release(*myContext);
        myQuadTexCoordBuf.release(*myContext);
        myFrmBuffer->release(*myContext);
        myTextureMask->release(*myContext);
        myGlProgramMask->release(*myContext);
    }
    myContext.nullify();

    // read windowed placement
    StWindow::hide();
    if(isMovable()) {
        StWindow::setFullScreen(false);
    }
}

StOutInterlace::~StOutInterlace() {
    myInstancesNb.decrement();
    releaseResources();
}

void StOutInterlace::setBarrierState(BarrierState theBarrierState) {
    if(myBarrierState == theBarrierState) {
        return;
    }

    myBarrierState = theBarrierState;
#if defined(__ANDROID__)
    unsigned char aValue = mi3d_tn::SET_TN_OFF;
    switch(myBarrierState) {
        case BarrierState_Unknown:
        case BarrierState_Off:
            aValue = mi3d_tn::SET_TN_OFF;
            break;
        case BarrierState_Landscape:
            aValue = mi3d_tn::SET_VERTICAL_TN_ON;
            break;
        case BarrierState_Portrait:
            aValue = mi3d_tn::SET_HORIZONTAL_TN_ON;
            break;
    }
    std::fstream aCtrlFile;
    aCtrlFile.open(mi3d_tn::CTRL_FILE);
    if(aCtrlFile.is_open()) {
        aCtrlFile << aValue;
        aCtrlFile.close();
    }
#endif
}

void StOutInterlace::close() {
    StWindow::params.VSyncMode->signals.onChanged -= stSlot(this, &StOutInterlace::doSwitchVSync);
    beforeClose();
    releaseResources();
    StWindow::close();
}

void StOutInterlace::beforeClose() {
    if(isMovable() && myWasUsed) {
        mySettings->saveInt32Rect(ST_SETTING_WINDOWPOS, StWindow::getWindowedPlacement());
    }
    mySettings->saveParam(params.BindToMon);
    mySettings->saveParam(params.ToReverse);
    mySettings->saveParam(params.ToUseMask);
    mySettings->saveInt32(ST_SETTING_DEVICE_ID,    myDevice);
    mySettings->flush();

    // process exit from StApplication
    if((myDevice == DEVICE_ROW_INTERLACED_ED) && myIsEDactive) {
        // disactivate eDimensional shuttered glasses
        myEDTimer.restart();
        myIsEDactive = false;
        while(myEDTimer.getElapsedTime() <= 0.5) {
            stglDraw();
            StThread::sleep(10);
        }
    }
    setBarrierState(BarrierState_Off);
}

void StOutInterlace::show() {
    // slave window should be displayed only in special cases
    StWindow::show(ST_WIN_MASTER);
}

bool StOutInterlace::create() {
    show();
    if(!StWindow::create()) {
        return false;
    }

    // initialize GL context
    myContext = StWindow::getContext();
    myContext->setMessagesQueue(myMsgQueue);
    if(!myContext->isGlGreaterEqual(2, 0)) {
        myMsgQueue->pushError(stCString("OpenGL 2.0 is required by Interlace Output"));
        myIsBroken = true;
        return true;
    }
    myContext->stglSetVSync((StGLContext::VSync_Mode )StWindow::params.VSyncMode->getValue());
    StWindow::params.VSyncMode->signals.onChanged += stSlot(this, &StOutInterlace::doSwitchVSync);

    // INIT shaders
    StCString aShadersError = stCString("Interlace output - critical error:\nShaders initialization failed!");
    StGLVertexShader aVertexShader("Interlace"); // common vertex shader
    StGLAutoRelease aTmp1(*myContext, aVertexShader);
    if(!aVertexShader.init(*myContext,
                           "attribute vec4 vVertex;\n"
                           "attribute vec2 vTexCoord;\n"
                           "varying vec2 fTexCoord;\n"
                           "void main(void) {\n"
                           "  fTexCoord = vTexCoord;\n"
                           "  gl_Position = vVertex;\n"
                           "}\n")) {
        myMsgQueue->pushError(aShadersError);
        myIsBroken = true;
        return true;
    }

    // row interlaced
    StGLFragmentShader aShaderRow(myGlPrograms[DEVICE_ROW_INTERLACED]->getTitle());
    StGLFragmentShader aShaderRowRev(myGlProgramsRev[DEVICE_ROW_INTERLACED]->getTitle());
    StGLAutoRelease aTmp2(*myContext, aShaderRow);
    StGLAutoRelease aTmp3(*myContext, aShaderRowRev);
    if(!aShaderRow.init(*myContext,
                        ST_SHADER_TEMPLATE[0],
                        // drop odd horizontal line (starts from bottom)
                        "if(int(mod(gl_FragCoord.y - 1023.5, 2.0)) != 1) { discard; }\n",
                        ST_SHADER_TEMPLATE[2])) {
        myMsgQueue->pushError(aShadersError);
        myIsBroken = true;
        return true;
    } else if(!aShaderRowRev.init(*myContext,
                                  ST_SHADER_TEMPLATE[0],
              // drop even horizontal line (starts from bottom)
              "if(int(mod(gl_FragCoord.y - 1023.5, 2.0)) == 1) { discard; }\n",
              ST_SHADER_TEMPLATE[2])) {
        myMsgQueue->pushError(aShadersError);
        myIsBroken = true;
        return true;
    }
    myGlPrograms[DEVICE_ROW_INTERLACED]->create(*myContext)
                                       .attachShader(*myContext, aVertexShader)
                                       .attachShader(*myContext, aShaderRow)
                                       .link(*myContext);
    myGlProgramsRev[DEVICE_ROW_INTERLACED]->create(*myContext)
                                       .attachShader(*myContext, aVertexShader)
                                       .attachShader(*myContext, aShaderRowRev)
                                       .link(*myContext);

    // column interlaced
    StGLFragmentShader aShaderCol   (myGlPrograms   [DEVICE_COL_INTERLACED]->getTitle());
    StGLFragmentShader aShaderColRev(myGlProgramsRev[DEVICE_COL_INTERLACED]->getTitle());
    StGLAutoRelease aTmp4(*myContext, aShaderCol);
    StGLAutoRelease aTmp5(*myContext, aShaderColRev);
    if(!aShaderCol.init(*myContext,
                        ST_SHADER_TEMPLATE[0],
                        // drop odd column (starts from left)
                        "if(int(mod(gl_FragCoord.x - 1023.5, 2.0)) == 1) { discard; }\n",
                        ST_SHADER_TEMPLATE[2])) {
        myMsgQueue->pushError(aShadersError);
        myIsBroken = true;
        return true;
    } else if(!aShaderColRev.init(*myContext,
                                  ST_SHADER_TEMPLATE[0],
              // drop even column (starts from left)
              "if(int(mod(gl_FragCoord.x - 1023.5, 2.0)) != 1) { discard; }\n",
              ST_SHADER_TEMPLATE[2])) {
        myMsgQueue->pushError(aShadersError);
        myIsBroken = true;
        return true;
    }
    myGlPrograms[DEVICE_COL_INTERLACED]->create(*myContext)
                                       .attachShader(*myContext, aVertexShader)
                                       .attachShader(*myContext, aShaderCol)
                                       .link(*myContext);
    myGlProgramsRev[DEVICE_COL_INTERLACED]->create(*myContext)
                                       .attachShader(*myContext, aVertexShader)
                                       .attachShader(*myContext, aShaderColRev)
                                       .link(*myContext);

    // chessboard
    StGLFragmentShader aShaderChess(myGlPrograms[DEVICE_CHESSBOARD]->getTitle());
    StGLFragmentShader aShaderChessRev(myGlProgramsRev[DEVICE_CHESSBOARD]->getTitle());
    StGLAutoRelease aTmp6(*myContext, aShaderChess);
    StGLAutoRelease aTmp7(*myContext, aShaderChessRev);
    if(!aShaderChess.init(*myContext,
                          ST_SHADER_TEMPLATE[0],
                          "bool isEvenX = int(mod(floor(gl_FragCoord.x - 1023.5), 2.0)) != 1;\n"
                          "bool isEvenY = int(mod(floor(gl_FragCoord.y - 1023.5), 2.0)) == 1;\n"
                          "if((isEvenX && isEvenY) || (!isEvenX && !isEvenY)) { discard; }\n",
                           ST_SHADER_TEMPLATE[2])) {
        myMsgQueue->pushError(aShadersError);
        myIsBroken = true;
        return true;
    } else if(!aShaderChessRev.init(*myContext,
                                    ST_SHADER_TEMPLATE[0],
              "bool isEvenX = int(mod(floor(gl_FragCoord.x - 1023.5), 2.0)) != 1;\n"
              "bool isEvenY = int(mod(floor(gl_FragCoord.y - 1023.5), 2.0)) == 1;\n"
              "if(!((isEvenX && isEvenY) || (!isEvenX && !isEvenY))) { discard; }\n",
              ST_SHADER_TEMPLATE[2])) {
        myMsgQueue->pushError(aShadersError);
        myIsBroken = true;
        return true;
    }
    myGlPrograms   [DEVICE_CHESSBOARD]->create(*myContext)
                                       .attachShader(*myContext, aVertexShader)
                                       .attachShader(*myContext, aShaderChess)
                                       .link(*myContext);
    myGlProgramsRev[DEVICE_CHESSBOARD]->create(*myContext)
                                       .attachShader(*myContext, aVertexShader)
                                       .attachShader(*myContext, aShaderChessRev)
                                       .link(*myContext);

    // discard mask texture
    StGLFragmentShader aShaderMask(myGlProgramMask->getTitle());
    StGLAutoRelease aTmp8(*myContext, aShaderMask);

    const char* FRAGMENT_GET_ALPHA = myContext->arbTexRG
                                   ? "#define stTextureAlpha(theSampler, theCoords) texture2D(theSampler, theCoords).r\n"
                                   : "#define stTextureAlpha(theSampler, theCoords) texture2D(theSampler, theCoords).a\n";
    const StString aMaskProgram = StString() + FRAGMENT_GET_ALPHA
     + "uniform sampler2D uTexture;\n"
       "uniform sampler2D uMaskTexture;\n"
       "varying vec2 fTexCoord;\n"
       "void main(void) {\n"
       "  float aMask = stTextureAlpha(uMaskTexture, fTexCoord);\n"
       "  if(aMask < 0.5) { discard; }\n"
       "  gl_FragColor = texture2D(uTexture, fTexCoord);\n"
       "}\n";

    if(!aShaderMask.init(*myContext, aMaskProgram.toCString())) {
        myMsgQueue->pushError(aShadersError);
        myIsBroken = true;
        return true;
    }
    myGlProgramMask->create(*myContext)
                   .attachShader(*myContext, aVertexShader)
                   .attachShader(*myContext, aShaderMask)
                   .link(*myContext);

#if !defined(__ANDROID__)
    const StString aShadersRoot = StString("shaders" ST_FILE_SPLITTER) + ST_OUT_PLUGIN_NAME + SYS_FS_SPLITTER;
    StGLVertexShader stVShaderED("ED control");
    StGLAutoRelease aTmp9(*myContext, stVShaderED);
    if(!stVShaderED.initFile(*myContext, aShadersRoot + VSHADER_ED)) {
        myMsgQueue->pushError(aShadersError);
        myIsBroken = true;
        return true;
    }

    StGLFragmentShader stFInterlaceOn(myEDIntelaceOn->getTitle());
    StGLAutoRelease aTmp10(*myContext, stFInterlaceOn);
    if(!stFInterlaceOn.initFile(*myContext, aShadersRoot + FSHADER_EDINTERLACE_ON)) {
        myMsgQueue->pushError(aShadersError);
        myIsBroken = true;
        return true;
    }
    myEDIntelaceOn->create(*myContext)
                   .attachShader(*myContext, stVShaderED)
                   .attachShader(*myContext, stFInterlaceOn)
                   .link(*myContext);

    StGLFragmentShader stFShaderEDOff(myEDOff->getTitle());
    StGLAutoRelease aTmp11(*myContext, stFShaderEDOff);
    if(!stFShaderEDOff.initFile(*myContext, aShadersRoot + FSHADER_ED_OFF)) {
        myMsgQueue->pushError(aShadersError);
        myIsBroken = true;
        return true;
    }
    myEDOff->create(*myContext)
            .attachShader(*myContext, stVShaderED)
            .attachShader(*myContext, stFShaderEDOff)
            .link(*myContext);

    myVpSizeYOnLoc  = myEDIntelaceOn->getUniformLocation(*myContext, "vpSizeY");
    myVpSizeYOffLoc = myEDOff       ->getUniformLocation(*myContext, "vpSizeY");

    if(myDevice == DEVICE_ROW_INTERLACED_ED) {
        // could be eDimensional shuttered glasses
        myEDTimer.restart(2000000.0);
    }
#endif

    // create vertices buffers to draw simple textured quad
    const GLfloat QUAD_VERTICES[4 * 4] = {
         1.0f, -1.0f, 0.0f, 1.0f, // top-right
         1.0f,  1.0f, 0.0f, 1.0f, // bottom-right
        -1.0f, -1.0f, 0.0f, 1.0f, // top-left
        -1.0f,  1.0f, 0.0f, 1.0f  // bottom-left
    };

    const GLfloat QUAD_TEXCOORD[2 * 4] = {
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 1.0f
    };

    myQuadVertBuf    .init(*myContext, 4, 4, QUAD_VERTICES);
    myQuadTexCoordBuf.init(*myContext, 2, 4, QUAD_TEXCOORD);
    myIsBroken = false;

    return true;
}

void StOutInterlace::doNewMonitor(const StSizeEvent& ) {
    const StSearchMonitors& aMonitors = StWindow::getMonitors();
    const StRectI_t  aRect = StWindow::getPlacement();
    const StMonitor& aMon  = aMonitors[aRect.center()];
    myIsMonReversed = false;
    // note that this enumeration is not enough to handle rotation direction,
    // which is required to automatically swap lines/columns order)
    myIsMonPortrait = aMon.getOrientation() == StMonitor::Orientation_Portrait;
    bool isRowInterlaced;
    isInterlacedMonitor(aMon, myIsMonReversed, isRowInterlaced);
}

void StOutInterlace::processEvents() {
    StWindow::processEvents();

    const StKeysState& aKeys = StWindow::getKeysState();
    if(aKeys.isKeyDown(ST_VK_F1)) {
        myDevice = DEVICE_ROW_INTERLACED;
    } else if(aKeys.isKeyDown(ST_VK_F2)) {
        myDevice = DEVICE_COL_INTERLACED;
    } else if(aKeys.isKeyDown(ST_VK_F3)) {
        myDevice = DEVICE_CHESSBOARD;
    } else if(aKeys.isKeyDown(ST_VK_F4)) {
        myDevice = DEVICE_ROW_INTERLACED_ED;
    }

    // resize ED rectangle
    const StRectI_t aRect = StWindow::getPlacement();
    if(aRect != myWinRect) {
        myWinRect = aRect;
        myVpSizeY = aRect.height();
        if(!StWindow::isFullScreen()) {
            const StSearchMonitors& aMonitors = StWindow::getMonitors();
            if(myMonitor.isNull()) {
                myMonitor = new StMonitor(aMonitors[aRect.center()]);
            } else if(!myMonitor->getVRect().isPointIn(aRect.center())) {
                *myMonitor = aMonitors[aRect.center()];
            }
            myEDRect.left()   = 0;
            myEDRect.right()  = myMonitor->getVRect().width();
            myEDRect.top()    = 0;
            myEDRect.bottom() = 10;
            myVpSizeY = 10;
        }
    }
}

bool StOutInterlace::initTextureMask(int  theDevice,
                                     bool theToReverse,
                                     int  theSizeX,
                                     int  theSizeY) {
    if(myTextureMask->getSizeX() == theSizeX
    && myTextureMask->getSizeY() == theSizeY
    && myTexMaskDevice   == theDevice
    && myTexMaskReversed == theToReverse) {
        return true;
    }

    StImagePlane anImage;
    if(!anImage.initTrash(StImagePlane::ImgGray, theSizeX, theSizeY)) {
        myMsgQueue->pushError(stCString("Interlace output - critical error:\nNot enough memory for mask image!"));
        myIsBroken = true;
        return false;
    }

    for(int aRowIter = 0; aRowIter < theSizeY; ++aRowIter) {
        for(int aColIter = 0; aColIter < theSizeX; ++aColIter) {
            stUByte_t* aPixel = anImage.changeData(aRowIter, aColIter);
            switch(theDevice) {
                case DEVICE_ROW_INTERLACED:
                case DEVICE_ROW_INTERLACED_ED: {
                    if(theToReverse) {
                        *aPixel = (aRowIter % 2 != 0) ? 255 : 0;
                    } else {
                        *aPixel = (aRowIter % 2 == 0) ? 255 : 0;
                    }
                    break;
                }
                case DEVICE_COL_INTERLACED_MI3D:
                case DEVICE_COL_INTERLACED: {
                    if(theToReverse) {
                        *aPixel = (aColIter % 2 == 0) ? 255 : 0;
                    } else {
                        *aPixel = (aColIter % 2 != 0) ? 255 : 0;
                    }
                    break;
                }
                case DEVICE_CHESSBOARD: {
                    const bool isEvenX = (aRowIter % 2 == 0);
                    const bool isEvenY = (aColIter % 2 != 0);
                    if(theToReverse) {
                        *aPixel =  ((isEvenX && isEvenY) || (!isEvenX && !isEvenY)) ? 255 : 0;
                    } else {
                        *aPixel = !((isEvenX && isEvenY) || (!isEvenX && !isEvenY)) ? 255 : 0;
                    }
                    break;
                }
                case DEVICE_AUTO:
                case DEVICE_NB:
                    break;
            }
        }
    }

    const GLint aTexFormat = myContext->arbTexRG ? GL_R8 : GL_ALPHA;
    if(myTextureMask->getTextureFormat() != aTexFormat) {
        myTextureMask->setTextureFormat(aTexFormat);
    }

    if(!myTextureMask->init(*myContext, anImage)) {
        myMsgQueue->pushError(stCString("Interlace output - critical error:\nMask texture resize failed!"));
        myIsBroken = true;
        return false;
    }

    myTexMaskDevice   = theDevice;
    myTexMaskReversed = theToReverse;
    return true;
}

void StOutInterlace::stglDrawEDCodes() {
    if(myEDTimer.getElapsedTime() > 0.5) {
        StWindow::hide(ST_WIN_SLAVE);
        myIsEDCodeFinished = true;
        return;
    }
    if(!StWindow::isFullScreen()) {
        StWindow::show(ST_WIN_SLAVE);
        StWindow::stglMakeCurrent(ST_WIN_SLAVE);
        myContext->stglResize(myEDRect);
        // clear the screen and the depth buffer
        myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    myContext->core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    myContext->core20fwd->glEnable(GL_BLEND);
    if(myIsEDactive) {
        myEDIntelaceOn->use(*myContext);
        if(myVpSizeYOnLoc != -1) {
            // TODO (Kirill Gavrilov#9#) use glViewport instead
            myContext->core20fwd->glUniform1i(myVpSizeYOnLoc, myVpSizeY);
        }
    } else {
        myEDOff->use(*myContext);
        if(myVpSizeYOffLoc != -1) {
            // TODO (Kirill Gavrilov#9#) use glViewport instead
            myContext->core20fwd->glUniform1i(myVpSizeYOffLoc, myVpSizeY);
        }
    }

#if !defined(GL_ES_VERSION_2_0)
    // TODO (Kirill Gavrilov#5) use vertex buffer
    myContext->core11->glBegin(GL_QUADS);
        myContext->core11->glVertex2f(-1.0f, -1.0f);
        myContext->core11->glVertex2f( 1.0f, -1.0f);
        myContext->core11->glVertex2f( 1.0f,  1.0f);
        myContext->core11->glVertex2f(-1.0f,  1.0f);
    myContext->core11->glEnd();
#endif
    myEDIntelaceOn->unuse(*myContext); // this is global unuse
    myContext->core20fwd->glDisable(GL_BLEND);
    if(!StWindow::isFullScreen()) {
        StWindow::stglSwap(ST_WIN_SLAVE);
    }
}

void StOutInterlace::stglDraw() {
    if(!StWindow::stglMakeCurrent(ST_WIN_MASTER)) {
        StWindow::signals.onRedraw(ST_DRAW_MONO);
        StThread::sleep(10);
        return;
    }

    myFPSControl.setTargetFPS(StWindow::getTargetFps());

    // always draw LEFT view into real screen buffer
    const StGLBoxPx aVPort = StWindow::stglViewport(ST_WIN_MASTER);
    myContext->stglResizeViewport(aVPort);
    StWindow::signals.onRedraw(ST_DRAW_LEFT);

    if(myIsFirstDraw) {
        myIsFirstDraw = false;

        // fall-back on known problematic devices
        const StString aGlRenderer((const char* )myContext->core11fwd->glGetString(GL_RENDERER));
        if(aGlRenderer.isContains(stCString("PowerVR Rogue G6200"))) {
            params.ToUseMask->setValue(true);
        }
    }

    if(!StWindow::isStereoOutput() || myIsBroken) {
        if(myToCompressMem) {
            myFrmBuffer->release(*myContext);
            myTextureMask->release(*myContext);
        }

        if(myDevice == DEVICE_ROW_INTERLACED_ED) {
            // EDimensional deactivation
            if(myIsEDCodeFinished) {
                if(myIsStereo) {
                    if(myIsEDactive) {
                        myEDTimer.restart();
                        myIsEDactive = false;
                        myIsEDCodeFinished = false;
                    }
                    myIsStereo = false;
                }
            }
            stglDrawEDCodes();
        } else if(myDevice == DEVICE_COL_INTERLACED_MI3D) {
            setBarrierState(BarrierState_Off);
        }

        // decrease FPS to target by thread sleeps
        myFPSControl.sleepToTarget();
        StWindow::stglSwap(ST_WIN_MASTER);
        ++myFPSControl;
        return;
    }

    // reverse L/R according to window position
    bool isPixelReverse = false;
    const StRectI_t aWinRect = StWindow::getPlacement();
    StGLBoxPx aBackStore;
    aBackStore.x()      = aWinRect.left();
    aBackStore.y()      = aWinRect.top();
    aBackStore.width()  = aWinRect.width();
    aBackStore.height() = aWinRect.height();
    convertRectToBacking(aBackStore, ST_WIN_MASTER);

    // resize FBO
    if(!myFrmBuffer->initLazy(*myContext, GL_RGBA8, aVPort.width(), aVPort.height(), StWindow::hasDepthBuffer())) {
        myMsgQueue->pushError(stCString("Interlace output - critical error:\nFrame Buffer Object resize failed!"));
        myIsBroken = true;
        return;
    }

    int aDevice = myDevice;

    // handle portrait orientation
    if(myDevice == DEVICE_COL_INTERLACED_MI3D) {
        aDevice = DEVICE_COL_INTERLACED;
    } else if(myIsMonPortrait) {
        switch(myDevice) {
            case DEVICE_ROW_INTERLACED:
                aDevice = DEVICE_COL_INTERLACED;
                break;
            case DEVICE_COL_INTERLACED:
                aDevice = DEVICE_ROW_INTERLACED;
                break;
        }
    }

    // odd vertically?
    if(!StWindow::isFullScreen() && (aBackStore.y() + aBackStore.height()) % 2 == 1) {
        switch(aDevice) {
            case DEVICE_CHESSBOARD:
            case DEVICE_ROW_INTERLACED:
            case DEVICE_ROW_INTERLACED_ED:
                isPixelReverse = !isPixelReverse; break;
        }
    }

    // odd horizontally?
    if(!StWindow::isFullScreen() && aBackStore.x() % 2 == 1) {
        switch(aDevice) {
            case DEVICE_CHESSBOARD:
            case DEVICE_COL_INTERLACED:
            case DEVICE_COL_INTERLACED_MI3D:
                isPixelReverse = !isPixelReverse; break;
        }
    }

    // known monitor model with reversed rows order?
    if(myIsMonReversed) {
        isPixelReverse = !isPixelReverse;
    }

    // reversed by external event
    if(StWindow::toSwapEyesHW()) {
        isPixelReverse = !isPixelReverse;
    }

    // reversed by user?
    if(params.ToReverse->getValue()) {
        isPixelReverse = !isPixelReverse;
    }

    // initialize mask texture
    const bool toUseTexMask = params.ToUseMask->getValue();
    if(toUseTexMask) {
        if(!initTextureMask(aDevice, isPixelReverse, myFrmBuffer->getSizeX(), myFrmBuffer->getSizeY())) {
            return;
        }
    } else {
        myTextureMask->release(*myContext);
    }

    // reduce viewport to avoid additional aliasing of narrow lines
    GLfloat aDX = GLfloat(myFrmBuffer->getVPSizeX()) / GLfloat(myFrmBuffer->getSizeX());
    GLfloat aDY = GLfloat(myFrmBuffer->getVPSizeY()) / GLfloat(myFrmBuffer->getSizeY());
    StArray<StGLVec2> aTCoords(4);
    aTCoords[0] = StGLVec2(aDX,  0.0f);
    aTCoords[1] = StGLVec2(aDX,  aDY);
    aTCoords[2] = StGLVec2(0.0f, 0.0f);
    aTCoords[3] = StGLVec2(0.0f, aDY);
    myQuadTexCoordBuf.init(*myContext, aTCoords);

    // draw into virtual frame buffer
    myFrmBuffer->setupViewPort(*myContext); // we set TEXTURE sizes here
    myFrmBuffer->bindBuffer(*myContext);
        StWindow::signals.onRedraw(ST_DRAW_RIGHT);
    myFrmBuffer->unbindBuffer(*myContext);

    myContext->core20fwd->glDisable(GL_DEPTH_TEST);
    myContext->core20fwd->glDisable(GL_BLEND);

    myContext->stglResizeViewport(aVPort);
    myFrmBuffer->bindTexture(*myContext);
    if(toUseTexMask) {
        myTextureMask->bind(*myContext, GL_TEXTURE1);
    }
    const StHandle<StProgramFB>& aProgram = toUseTexMask
                                          ? myGlProgramMask
                                          : (isPixelReverse
                                            ? myGlProgramsRev[aDevice]
                                            : myGlPrograms[aDevice]);
    aProgram->use(*myContext);
    myQuadVertBuf.bindVertexAttrib(*myContext, ST_VATTRIB_VERTEX);
    myQuadTexCoordBuf.bindVertexAttrib(*myContext, ST_VATTRIB_TCOORD);

    myContext->core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    myQuadTexCoordBuf.unBindVertexAttrib(*myContext, ST_VATTRIB_TCOORD);
    myQuadVertBuf.unBindVertexAttrib(*myContext, ST_VATTRIB_VERTEX);
    aProgram->unuse(*myContext);
    if(toUseTexMask) {
        myTextureMask->unbind(*myContext);
    }
    myFrmBuffer->unbindTexture(*myContext);

    if(myDevice == DEVICE_ROW_INTERLACED_ED) {
        // EDimensional activation
        if(myIsEDCodeFinished) {
            if(!myIsStereo) {
                if(!myIsEDactive) {
                    myEDTimer.restart();
                    myIsEDactive = true;
                    myIsEDCodeFinished = false;
                }
                myIsStereo = true;
            }
        }
        stglDrawEDCodes();
    } else if(myDevice == DEVICE_COL_INTERLACED_MI3D) {
        setBarrierState(myIsMonPortrait ? BarrierState_Portrait : BarrierState_Landscape);
    }

    // decrease FPS to target by thread sleeps
    myFPSControl.sleepToTarget();
    StWindow::stglSwap(ST_WIN_MASTER);
    ++myFPSControl;
}

void StOutInterlace::doSwitchVSync(const int32_t theValue) {
    if(myContext.isNull()) {
        return;
    }

    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglSetVSync((StGLContext::VSync_Mode )theValue);
}

void StOutInterlace::doSetBindToMonitor(const bool theValue) {
    if(!theValue
    || StWindow::isFullScreen()) {
        return;
    }

    const StSearchMonitors& aMonitors = StWindow::getMonitors();
    StRectI_t aRect = StWindow::getPlacement();
    StMonitor aMon  = aMonitors[aRect.center()];
    bool isRowInterlaced = false;
    if(isInterlacedMonitor(aMon, myIsMonReversed, isRowInterlaced)
    || !isMovable()) {
        return;
    }

    StHandle<StMonitor> anInterlacedMon = getInterlacedMonitor(aMonitors, myIsMonReversed, isRowInterlaced);
    if(anInterlacedMon.isNull()) {
        return;
    }

    int aWidth  = aRect.width();
    int aHeight = aRect.height();
    aRect.left()   = anInterlacedMon->getVRect().left() + 256;
    aRect.right()  = aRect.left() + aWidth;
    aRect.top()    = anInterlacedMon->getVRect().top() + 256;
    aRect.bottom() = aRect.top() + aHeight;
    StWindow::setPlacement(aRect);
}
