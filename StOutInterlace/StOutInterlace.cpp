/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutInterlace library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutInterlace library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StOutInterlace.h"

#include <StGL/StGLContext.h>
#include <StGL/StGLEnums.h>
#include <StGL/StGLVec.h>
#include <StGLCore/StGLCore20.h>
#include <StCore/StWindow.h>
#include <StSettings/StSettings.h>
#include <StSettings/StTranslations.h>

namespace {
    static bool HAS_LOGGER_ID = StLogger::IdentifyModule("StOutInterlace");

    static const char ST_OUT_PLUGIN_NAME[] = "StOutInterlace";

    // shaders data
    static const char VSHADER_ED[]             = "vED.shv";
    static const char FSHADER_EDINTERLACE_ON[] = "fEDinterlace.shf";
    static const char FSHADER_ED_OFF[]         = "fEDoff.shf";

    static const char ST_SETTING_DEVICE_ID[]      = "deviceId";
    static const char ST_SETTING_WINDOWPOS[]      = "windowPos";
    static const char ST_SETTING_BIND_MONITOR[]   = "bindMonitor";
    static const char ST_SETTING_VSYNC[]          = "vsync";
    static const char ST_SETTING_REVERSE[]        = "reverse";

    struct StMonInterlacedInfo_t {
        const stUtf8_t* pnpid;
        bool            isReversed;
    };

    /**
     * Database of known interlaced monitors.
     */
    static const StMonInterlacedInfo_t THE_KNOWN_MONITORS[] = {
        {"ZMT1900", false}, // Zalman Trimon M190S
        {"ZMT2200", false}, // Zalman Trimon M220W
        {"ENV2373", true }, // Envision
        {"HIT8002", false}, // Hyundai W220S D-Sub
        {"HIT8D02", false}, // Hyundai W220S DVID
        {"HIT7003", false}, // Hyundai W240S D-Sub
        {"HIT7D03", false}, // Hyundai W240S D-Sub
      //{"ACI23C2", false}, // ASUS VG23AH
        {"ACI27C2", false}, // ASUS VG27AH
        {       "", false}  // NULL-terminate array
    };

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
        STTR_PARAMETER_VSYNC    = 1100,
        STTR_PARAMETER_SHOW_FPS = 1101,
        STTR_PARAMETER_REVERSE  = 1102,
        STTR_PARAMETER_BIND_MON = 1103,

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

};

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
    StGLVarLocation aTextureLoc = StGLProgram::getUniformLocation(theCtx, "uTexture");
    if(aTextureLoc.isValid()) {
        use(theCtx);
        theCtx.core20fwd->glUniform1i(aTextureLoc, StGLProgram::TEXTURE_SAMPLE_0); // GL_TEXTURE0
        unuse(theCtx);
    }
    return aTextureLoc.isValid();
}

StAtomic<int32_t> StOutInterlace::myInstancesNb(0);

void StOutInterlace::setDevice(const int theDeviceId) {
    if(theDeviceId < 0 || theDeviceId >= DEVICE_NB) {
        ST_DEBUG_LOG("Incorrect device ID!");
        return;
    }
    myDeviceId = theDeviceId;
    if(myOptionsStruct != NULL) {
        myOptionsStruct->curDeviceId = myDeviceId;
    }
}

void StOutInterlace::optionsStructAlloc() {
    StTranslations stLangMap(ST_OUT_PLUGIN_NAME);

    // create device options structure
    myOptionsStruct = (StSDOptionsList_t* )StWindow::memAlloc(sizeof(StSDOptionsList_t)); stMemSet(myOptionsStruct, 0, sizeof(StSDOptionsList_t));
    myOptionsStruct->curRendererPath = StWindow::memAllocNCopy(myPluginPath);
    myOptionsStruct->curDeviceId = myDeviceId;

    myOptionsStruct->optionsCount = 4;
    myOptionsStruct->options = (StSDOption_t** )StWindow::memAlloc(sizeof(StSDOption_t*) * myOptionsStruct->optionsCount);

    // VSync option
    myOptionsStruct->options[DEVICE_OPTION_VSYNC] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDOnOff_t));
    myOptionsStruct->options[DEVICE_OPTION_VSYNC]->optionType = ST_DEVICE_OPTION_ON_OFF;
    ((StSDOnOff_t* )myOptionsStruct->options[DEVICE_OPTION_VSYNC])->value = myIsVSync;
    myOptionsStruct->options[DEVICE_OPTION_VSYNC]->title = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_PARAMETER_VSYNC, "VSync"));

    // Show FPS option
    myOptionsStruct->options[DEVICE_OPTION_SHOWFPS] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDOnOff_t));
    myOptionsStruct->options[DEVICE_OPTION_SHOWFPS]->optionType = ST_DEVICE_OPTION_ON_OFF;
    ((StSDOnOff_t* )myOptionsStruct->options[DEVICE_OPTION_SHOWFPS])->value = myToShowFPS;
    myOptionsStruct->options[DEVICE_OPTION_SHOWFPS]->title = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_PARAMETER_SHOW_FPS, "Show FPS"));

    // Reverse Order option
    myOptionsStruct->options[DEVICE_OPTION_REVERSE] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDOnOff_t));
    myOptionsStruct->options[DEVICE_OPTION_REVERSE]->optionType = ST_DEVICE_OPTION_ON_OFF;
    ((StSDOnOff_t* )myOptionsStruct->options[DEVICE_OPTION_REVERSE])->value = myIsReversed;
    myOptionsStruct->options[DEVICE_OPTION_REVERSE]->title = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_PARAMETER_REVERSE, "Reverse Order"));

    // Bind Monitor option
    myOptionsStruct->options[DEVICE_OPTION_BINDMON] = (StSDOption_t* )StWindow::memAlloc(sizeof(StSDOnOff_t));
    myOptionsStruct->options[DEVICE_OPTION_BINDMON]->optionType = ST_DEVICE_OPTION_ON_OFF;
    ((StSDOnOff_t* )myOptionsStruct->options[DEVICE_OPTION_BINDMON])->value = myToBindToMonitor;
    myOptionsStruct->options[DEVICE_OPTION_BINDMON]->title = StWindow::memAllocNCopy(stLangMap.changeValueId(STTR_PARAMETER_BIND_MON, "Bind To Supported Monitor"));
}

inline bool isInterlacedMonitor(const StMonitor& theMon,
                                bool&            theIsReversed) {
    if(theMon.getPnPId().getSize() != 7) {
        return false;
    }
    for(size_t anIter = 0;; ++anIter) {
        const StMonInterlacedInfo_t& aMon = THE_KNOWN_MONITORS[anIter];
        if(aMon.pnpid[0] == '\0') {
            return false;
        } else if(stAreEqual(aMon.pnpid, theMon.getPnPId().toCString(), 7)) {
            theIsReversed = aMon.isReversed;
            return true;
        }
    }
}

StHandle<StMonitor> StOutInterlace::getHInterlaceMonitor(bool& theIsReversed) {
    StArrayList<StMonitor> aMonitors = StCore::getStMonitors();
    for(size_t aMonIter = 0; aMonIter < aMonitors.size(); ++aMonIter) {
        const StMonitor& aMon = aMonitors[aMonIter];
        if(isInterlacedMonitor(aMon, theIsReversed)) {
            return new StMonitor(aMon);
        }
    }
    return NULL;
}

StOutInterlace::StOutInterlace()
: myDeviceId(DEVICE_AUTO),
  myEDTimer(true),
  myEDIntelaceOn(new StGLProgram("ED Interlace On")),
  myEDOff(new StGLProgram("ED Interlace Off")),
  myVpSizeY(10),
  myVpSizeYOnLoc(),
  myVpSizeYOffLoc(),
  myOptionsStruct(NULL),
  myFPSControl(),
  myToSavePlacement(true),
  myToBindToMonitor(true),
  myIsVSync(true),
  myToShowFPS(false),
  myIsReversed(false),
  myIsMonReversed(false),
  myIsStereo(false),
  myIsEDactive(false),
  myIsEDCodeFinished(false),
  myToCompressMem(myInstancesNb.increment() > 1),
  myIsBroken(false) {
    myFrmBuffer = new StGLFrameBuffer();
    myGlPrograms[DEVICE_HINTERLACE] = new StProgramFB("Row Interlace");
    myGlPrograms[DEVICE_VINTERLACE] = new StProgramFB("Column Interlace");
    myGlPrograms[DEVICE_CHESSBOARD] = new StProgramFB("Chessboard");
    myGlPrograms[DEVICE_HINTERLACE_ED] = myGlPrograms[DEVICE_HINTERLACE];

    myGlProgramsRev[DEVICE_HINTERLACE] = new StProgramFB("Row Interlace Inversed");
    myGlProgramsRev[DEVICE_VINTERLACE] = new StProgramFB("Column Interlace Inversed");
    myGlProgramsRev[DEVICE_CHESSBOARD] = new StProgramFB("Chessboard Inversed");
    myGlProgramsRev[DEVICE_HINTERLACE_ED] = myGlProgramsRev[DEVICE_HINTERLACE];
}

StOutInterlace::~StOutInterlace() {
    myInstancesNb.decrement();
    if(!myStCore.isNull() && !mySettings.isNull()) {
        if(!myContext.isNull()) {
            for(size_t anIter = 0; anIter < DEVICE_NB; ++anIter) {
                myGlPrograms[anIter]->release(*myContext);
                myGlProgramsRev[anIter]->release(*myContext);
            }
            myEDIntelaceOn->release(*myContext);
            myEDOff->release(*myContext);
            myQuadVertBuf.release(*myContext);
            myQuadTexCoordBuf.release(*myContext);
            myFrmBuffer->release(*myContext);
        }
        stMemFree(myOptionsStruct, StWindow::memFree);

        // read windowed placement
        getStWindow()->hide(ST_WIN_MASTER);
        getStWindow()->hide(ST_WIN_SLAVE);
        if(myToSavePlacement) {
            getStWindow()->setFullScreen(false);
            mySettings->saveInt32Rect(ST_SETTING_WINDOWPOS, getStWindow()->getPlacement());
        }
        mySettings->saveBool (ST_SETTING_BIND_MONITOR, myToBindToMonitor);
        mySettings->saveBool (ST_SETTING_VSYNC,        myIsVSync);
        mySettings->saveBool (ST_SETTING_REVERSE,      myIsReversed);
        mySettings->saveInt32(ST_SETTING_DEVICE_ID,    myDeviceId);
    }
    mySettings.nullify();
    myStCore.nullify();
    StCore::FREE();
}

bool StOutInterlace::init(const StString&     inRendererPath,
                          const int&          theDeviceId,
                          const StNativeWin_t theNativeParent) {
    myToSavePlacement = (theNativeParent == (StNativeWin_t )NULL);
    myDeviceId   = theDeviceId;
    myPluginPath = inRendererPath;
    if(!StVersionInfo::checkTimeBomb("sView - Interlace Output plugin")) {
        return false;
    }
    ST_DEBUG_LOG_AT("INIT Interlace output plugin");
    // Firstly INIT core library!
    if(StCore::INIT() != STERROR_LIBNOERROR) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, Core library not available!");
        return false;
    }

    // INIT settings library
    mySettings = new StSettings(ST_OUT_PLUGIN_NAME);
    myStCore   = new StCore();

    // load window position
    StHandle<StMonitor> anInterlacedMon = StOutInterlace::getHInterlaceMonitor(myIsMonReversed);
    StRect<int32_t> loadedRect(256, 768, 256, 1024);
    bool isLoadedPosition = mySettings->loadInt32Rect(ST_SETTING_WINDOWPOS, loadedRect);
    mySettings->loadBool(ST_SETTING_BIND_MONITOR, myToBindToMonitor);
    StMonitor aMonitor = StCore::getMonitorFromPoint(loadedRect.center());
    if(myToBindToMonitor && !anInterlacedMon.isNull() && !isInterlacedMonitor(aMonitor, myIsMonReversed)) {
        aMonitor = *anInterlacedMon;
    }
    if(isLoadedPosition) {
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
        // try to open window on correct display
        loadedRect = aMonitor.getVRect();
        loadedRect.left()   = loadedRect.left() + 256;
        loadedRect.right()  = loadedRect.left() + 1024;
        loadedRect.top()    = loadedRect.top()  + 256;
        loadedRect.bottom() = loadedRect.top()  + 512;
    }
    getStWindow()->setPlacement(loadedRect);

    mySettings->loadBool(ST_SETTING_VSYNC,   myIsVSync);
    mySettings->loadBool(ST_SETTING_REVERSE, myIsReversed);

    // load device settings
    if(myDeviceId == StRendererInfo::DEVICE_AUTO) {
        mySettings->loadInt32(ST_SETTING_DEVICE_ID, myDeviceId);
        if(myDeviceId == StRendererInfo::DEVICE_AUTO) {
            myDeviceId = DEVICE_HINTERLACE;
        }
    }

    // allocate and setup the structure pointer
    optionsStructAlloc();
    getStWindow()->setValue(ST_WIN_DATAKEYS_RENDERER, (size_t )myOptionsStruct);

    setDevice(myDeviceId);

    // create our window!
    StWinAttributes_t attribs = stDefaultWinAttributes();
    attribs.isSlave = true;
    attribs.isSlaveHLineTop = true;
    attribs.isSlaveHide = true;
    getStWindow()->stglCreate(&attribs, theNativeParent);

    // initialize GL context
    myContext = new StGLContext();
    if(!myContext->stglInit()) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, OpenGL context is broken!\n(OpenGL library internal error?)");
        return false;
    } else if(!myContext->isGlGreaterEqual(2, 0)) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, OpenGL2.0+ not available!");
        return false;
    }
    if(!myContext->stglSetVSync(myIsVSync ? StGLContext::VSync_ON : StGLContext::VSync_OFF)) {
        // enable/disable VSync by config
        ST_DEBUG_LOG(ST_OUT_PLUGIN_NAME + " Plugin, VSync extension not available!");
    }

    // INIT shaders
    StString aShadersError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, Failed to init Shaders");
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
        stError(aShadersError);
        return false;
    }

    // row interlaced
    StGLFragmentShader aShaderRow(myGlPrograms[DEVICE_HINTERLACE]->getTitle());
    StGLFragmentShader aShaderRowRev(myGlProgramsRev[DEVICE_HINTERLACE]->getTitle());
    StGLAutoRelease aTmp2(*myContext, aShaderRow);
    StGLAutoRelease aTmp3(*myContext, aShaderRowRev);
    if(!aShaderRow.init(*myContext,
                        ST_SHADER_TEMPLATE[0],
                        // drop odd horizontal line (starts from bottom)
                        "if(int(mod(gl_FragCoord.y + 1.5, 2.0)) == 1) { discard; }\n",
                        ST_SHADER_TEMPLATE[2])) {
        stError(aShadersError);
        return false;
    } else if(!aShaderRowRev.init(*myContext,
                                  ST_SHADER_TEMPLATE[0],
              // drop even horizontal line (starts from bottom)
              "if(int(mod(gl_FragCoord.y + 1.5, 2.0)) != 1) { discard; }\n",
              ST_SHADER_TEMPLATE[2])) {
        stError(aShadersError);
        return false;
    }
    myGlPrograms   [DEVICE_HINTERLACE]->create(*myContext)
                                       .attachShader(*myContext, aVertexShader)
                                       .attachShader(*myContext, aShaderRow)
                                       .link(*myContext);
    myGlProgramsRev[DEVICE_HINTERLACE]->create(*myContext)
                                       .attachShader(*myContext, aVertexShader)
                                       .attachShader(*myContext, aShaderRowRev)
                                       .link(*myContext);

    // column interlaced
    StGLFragmentShader aShaderCol(myGlPrograms[DEVICE_VINTERLACE]->getTitle());
    StGLFragmentShader aShaderColRev(myGlProgramsRev[DEVICE_VINTERLACE]->getTitle());
    StGLAutoRelease aTmp4(*myContext, aShaderCol);
    StGLAutoRelease aTmp5(*myContext, aShaderColRev);
    if(!aShaderCol.init(*myContext,
                        ST_SHADER_TEMPLATE[0],
                        // drop odd column (starts from left)
                        "if(int(mod(gl_FragCoord.x + 1.5, 2.0)) != 1) { discard; }\n",
                        ST_SHADER_TEMPLATE[2])) {
        stError(aShadersError);
        return false;
    } else if(!aShaderColRev.init(*myContext,
                                  ST_SHADER_TEMPLATE[0],
              // drop even column (starts from left)
              "if(int(mod(gl_FragCoord.x + 1.5, 2.0)) == 1) { discard; }\n",
              ST_SHADER_TEMPLATE[2])) {
        stError(aShadersError);
        return false;
    }
    myGlPrograms   [DEVICE_VINTERLACE]->create(*myContext)
                                       .attachShader(*myContext, aVertexShader)
                                       .attachShader(*myContext, aShaderCol)
                                       .link(*myContext);
    myGlProgramsRev[DEVICE_VINTERLACE]->create(*myContext)
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
                          "bool isEvenX = int(mod(floor(gl_FragCoord.x + 1.5), 2.0)) == 1;\n"
                          "bool isEvenY = int(mod(floor(gl_FragCoord.y + 1.5), 2.0)) != 1;\n"
                          "if((isEvenX && isEvenY) || (!isEvenX && !isEvenY)) { discard; }\n",
                           ST_SHADER_TEMPLATE[2])) {
        stError(aShadersError);
        return false;
    } else if(!aShaderChessRev.init(*myContext,
                                    ST_SHADER_TEMPLATE[0],
              "bool isEvenX = int(mod(floor(gl_FragCoord.x + 1.5), 2.0)) == 1;\n"
              "bool isEvenY = int(mod(floor(gl_FragCoord.y + 1.5), 2.0)) != 1;\n"
              "if(!((isEvenX && isEvenY) || (!isEvenX && !isEvenY))) { discard; }\n",
              ST_SHADER_TEMPLATE[2])) {
        stError(aShadersError);
        return false;
    }
    myGlPrograms   [DEVICE_CHESSBOARD]->create(*myContext)
                                       .attachShader(*myContext, aVertexShader)
                                       .attachShader(*myContext, aShaderChess)
                                       .link(*myContext);
    myGlProgramsRev[DEVICE_CHESSBOARD]->create(*myContext)
                                       .attachShader(*myContext, aVertexShader)
                                       .attachShader(*myContext, aShaderChessRev)
                                       .link(*myContext);

    /// TODO (Kirill Gavrilov#3) fix shaders
    const StString aShadersRoot = StProcess::getStCoreFolder() + "shaders" + SYS_FS_SPLITTER
                                + ST_OUT_PLUGIN_NAME + SYS_FS_SPLITTER;
    StGLVertexShader stVShaderED("ED control");
    StGLAutoRelease aTmp8(*myContext, stVShaderED);
    if(!stVShaderED.initFile(*myContext, aShadersRoot + VSHADER_ED)) {
        stError(aShadersError);
        return false;
    }

    StGLFragmentShader stFInterlaceOn(myEDIntelaceOn->getTitle());
    StGLAutoRelease aTmp9(*myContext, stFInterlaceOn);
    if(!stFInterlaceOn.initFile(*myContext, aShadersRoot + FSHADER_EDINTERLACE_ON)) {
        stError(aShadersError);
        return false;
    }
    myEDIntelaceOn->create(*myContext)
                   .attachShader(*myContext, stVShaderED)
                   .attachShader(*myContext, stFInterlaceOn)
                   .link(*myContext);

    StGLFragmentShader stFShaderEDOff(myEDOff->getTitle());
    StGLAutoRelease aTmp10(*myContext, stFShaderEDOff);
    if(!stFShaderEDOff.initFile(*myContext, aShadersRoot + FSHADER_ED_OFF)) {
        stError(aShadersError);
        return false;
    }
    myEDOff->create(*myContext)
            .attachShader(*myContext, stVShaderED)
            .attachShader(*myContext, stFShaderEDOff)
            .link(*myContext);

    myVpSizeYOnLoc  = myEDIntelaceOn->getUniformLocation(*myContext, "vpSizeY");
    myVpSizeYOffLoc = myEDOff       ->getUniformLocation(*myContext, "vpSizeY");

    if(myDeviceId == DEVICE_HINTERLACE_ED) {
        // could be eDimensional shuttered glasses
        myEDTimer.restart(2000000.0);
    }

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

    return true;
}

void StOutInterlace::callback(StMessage_t* stMessages) {
    myStCore->callback(stMessages);
    for(size_t i = 0; stMessages[i].uin != StMessageList::MSG_NULL; ++i) {
        switch(stMessages[i].uin) {
            case StMessageList::MSG_RESIZE: {
                const StRectI_t aRect = getStWindow()->getPlacement();
                myVpSizeY = aRect.height();
                if(!getStWindow()->isFullScreen()) {
                    if(myMonitor.isNull()) {
                        myMonitor = new StMonitor(StCore::getMonitorFromPoint(aRect.center()));
                    } else if(!myMonitor->getVRect().isPointIn(aRect.center())) {
                        *myMonitor = StCore::getMonitorFromPoint(aRect.center());
                    }
                    myEDRect.left()   = 0;
                    myEDRect.right()  = myMonitor->getVRect().width();
                    myEDRect.top()    = 0;
                    myEDRect.bottom() = 10;
                    myVpSizeY = 10;
                }
                break;
            }
            case StMessageList::MSG_KEYS: {
                bool* keysMap = ((bool* )stMessages[i].data);
                if(keysMap[ST_VK_F1]) {
                    setDevice(DEVICE_HINTERLACE);    keysMap[ST_VK_F1] = false;
                } else if(keysMap[ST_VK_F2]) {
                    setDevice(DEVICE_VINTERLACE);    keysMap[ST_VK_F2] = false;
                } else if(keysMap[ST_VK_F3]) {
                    setDevice(DEVICE_CHESSBOARD);    keysMap[ST_VK_F3] = false;
                } else if(keysMap[ST_VK_F4]) {
                    setDevice(DEVICE_HINTERLACE_ED); keysMap[ST_VK_F4] = false;
                }
                if(keysMap[ST_VK_F12]) {
                    myToShowFPS = !myToShowFPS;
                    keysMap[ST_VK_F12] = false;

                    // send 'update' message to StDrawer
                    StMessage_t msg; msg.uin = StMessageList::MSG_DEVICE_OPTION;
                    StSDSwitch_t* option = ((StSDSwitch_t* )myOptionsStruct->options[DEVICE_OPTION_SHOWFPS]);
                    option->value = myToShowFPS; msg.data = (void* )option;
                    getStWindow()->appendMessage(msg);
                }
                break;
            }
            case StMessageList::MSG_DEVICE_INFO: {
                if(myOptionsStruct->curDeviceId != myDeviceId) {
                    StString newPluginPath(myOptionsStruct->curRendererPath);
                    if(newPluginPath == myPluginPath) {
                        setDevice(myOptionsStruct->curDeviceId);
                        stMessages[i].uin = StMessageList::MSG_NONE;
                    } // else - another plugin
                }
                break;
            }
            case StMessageList::MSG_DEVICE_OPTION: {
                bool newVSync = ((StSDOnOff_t* )myOptionsStruct->options[DEVICE_OPTION_VSYNC])->value;
                if(newVSync != myIsVSync) {
                    myIsVSync = newVSync;
                    getStWindow()->stglMakeCurrent(ST_WIN_MASTER);
                    myContext->stglSetVSync(myIsVSync ? StGLContext::VSync_ON : StGLContext::VSync_OFF);
                }
                myToShowFPS          = ((StSDOnOff_t* )myOptionsStruct->options[DEVICE_OPTION_SHOWFPS])->value;
                myIsReversed         = ((StSDOnOff_t* )myOptionsStruct->options[DEVICE_OPTION_REVERSE])->value;
                bool toBindToMonitor = ((StSDOnOff_t* )myOptionsStruct->options[DEVICE_OPTION_BINDMON])->value;

                bool toMovePos = !myToBindToMonitor && toBindToMonitor;
                myToBindToMonitor = toBindToMonitor;
                if(toMovePos && !getStWindow()->isFullScreen()) {
                    StRectI_t aRect = getStWindow()->getPlacement();
                    StMonitor aMon  = StCore::getMonitorFromPoint(aRect.center());
                    StHandle<StMonitor> anInterlacedMon = StOutInterlace::getHInterlaceMonitor(myIsMonReversed);
                    if(!anInterlacedMon.isNull() && !isInterlacedMonitor(aMon, myIsMonReversed)) {
                        int aWidth  = aRect.width();
                        int aHeight = aRect.height();
                        aRect.left()   = anInterlacedMon->getVRect().left() + 256;
                        aRect.right()  = aRect.left() + aWidth;
                        aRect.top()    = anInterlacedMon->getVRect().top() + 256;
                        aRect.bottom() = aRect.top() + aHeight;
                        getStWindow()->setPlacement(aRect);
                    }
                }

                break;
            }
            case StMessageList::MSG_WIN_ON_NEW_MONITOR: {
                const StRectI_t aRect = getStWindow()->getPlacement();
                const StMonitor aMon  = StCore::getMonitorFromPoint(aRect.center());
                myIsMonReversed = false;
                isInterlacedMonitor(aMon, myIsMonReversed);
                break;
            }
            case StMessageList::MSG_EXIT: {
                if((myDeviceId == DEVICE_HINTERLACE_ED) && myIsEDactive) {
                    // disactivate eDimensional shuttered glasses
                    myEDTimer.restart();
                    myIsEDactive = false;
                    while(myEDTimer.getElapsedTime() <= 0.5) {
                        stglDraw(ST_DRAW_BOTH);
                        StThread::sleep(10);
                    }
                }
                break;
            }
        }
    }
}

void StOutInterlace::stglDrawEDCodes() {
    if(myEDTimer.getElapsedTime() > 0.5) {
        getStWindow()->hide(ST_WIN_SLAVE);
        myIsEDCodeFinished = true;
        return;
    }
    if(!getStWindow()->isFullScreen()) {
        getStWindow()->show(ST_WIN_SLAVE);
        getStWindow()->stglMakeCurrent(ST_WIN_SLAVE);
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
    // TODO (Kirill Gavrilov#5) use vertex buffer
    myContext->core11->glBegin(GL_QUADS);
        myContext->core11->glVertex2f(-1.0f, -1.0f);
        myContext->core11->glVertex2f( 1.0f, -1.0f);
        myContext->core11->glVertex2f( 1.0f,  1.0f);
        myContext->core11->glVertex2f(-1.0f,  1.0f);
    myContext->core11->glEnd();
    myEDIntelaceOn->unuse(*myContext); // this is global unuse
    myContext->core20fwd->glDisable(GL_BLEND);
    if(!getStWindow()->isFullScreen()) {
        getStWindow()->stglSwap(ST_WIN_SLAVE);
    }
}

void StOutInterlace::stglDraw(unsigned int ) {
    myFPSControl.setTargetFPS(getStWindow()->stglGetTargetFps());
    if(myToShowFPS && myFPSControl.isUpdated()) {
        getStWindow()->setTitle(StString("Interlace Rendering FPS= ") + myFPSControl.getAverage());
    }

    // always draw LEFT view into real screen buffer
    getStWindow()->stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglResize(getStWindow()->getPlacement());
    myStCore->stglDraw(ST_DRAW_LEFT);

    if(!getStWindow()->isStereoOutput() || myIsBroken) {
        if(myToCompressMem) {
            myFrmBuffer->release(*myContext);
        }

        if(myDeviceId == DEVICE_HINTERLACE_ED) {
            // EDimensional disactivation
            // TODO (Kirill Gavrilov#4#) implement logic to sync multiple sView instances
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
        }

        // decrease FPS to target by thread sleeps
        myFPSControl.sleepToTarget();
        getStWindow()->stglSwap(ST_WIN_MASTER);
        ++myFPSControl;
        return;
    }

    // reverse L/R according to window position
    bool isPixelReverse = false;
    const StRectI_t aWinRect = getStWindow()->getPlacement();

    // resize FBO
    if(!myFrmBuffer->initLazy(*myContext, aWinRect.width(), aWinRect.height())) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, Failed to init Frame Buffer");
        myIsBroken = true;
        return;
    }

    // odd vertically?
    if(!getStWindow()->isFullScreen() && aWinRect.bottom() % 2 == 1) {
        switch(myDeviceId) {
            case DEVICE_CHESSBOARD:
            case DEVICE_HINTERLACE:
            case DEVICE_HINTERLACE_ED:
                isPixelReverse = !isPixelReverse; break;
        }
    }

    // odd horizontally?
    if(!getStWindow()->isFullScreen() && aWinRect.left() % 2 == 1) {
        switch(myDeviceId) {
            case DEVICE_CHESSBOARD:
            case DEVICE_VINTERLACE:
                isPixelReverse = !isPixelReverse; break;
        }
    }

    // known monitor model with reversed rows order?
    if(myIsMonReversed) {
        isPixelReverse = !isPixelReverse;
    }

    // reversed by user?
    if(myIsReversed) {
        isPixelReverse = !isPixelReverse;
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
    GLint aVPort[4]; // real window viewport
    myContext->core20fwd->glGetIntegerv(GL_VIEWPORT, aVPort);
    myFrmBuffer->setupViewPort(*myContext); // we set TEXTURE sizes here
    myFrmBuffer->bindBuffer(*myContext);
        myStCore->stglDraw(ST_DRAW_RIGHT);
    myFrmBuffer->unbindBuffer(*myContext);
    myContext->core20fwd->glViewport(aVPort[0], aVPort[1], aVPort[2], aVPort[3]);

    myContext->core20fwd->glDisable(GL_DEPTH_TEST);
    myContext->core20fwd->glDisable(GL_BLEND);

    myFrmBuffer->bindTexture(*myContext);
    const StHandle<StProgramFB>& aProgram = isPixelReverse ? myGlProgramsRev[myDeviceId] : myGlPrograms[myDeviceId];
    aProgram->use(*myContext);
    myQuadVertBuf.bindVertexAttrib(*myContext, ST_VATTRIB_VERTEX);
    myQuadTexCoordBuf.bindVertexAttrib(*myContext, ST_VATTRIB_TCOORD);

    myContext->core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    myQuadTexCoordBuf.unBindVertexAttrib(*myContext, ST_VATTRIB_TCOORD);
    myQuadVertBuf.unBindVertexAttrib(*myContext, ST_VATTRIB_VERTEX);
    aProgram->unuse(*myContext);
    myFrmBuffer->unbindTexture(*myContext);

    if(myDeviceId == DEVICE_HINTERLACE_ED) {
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
    }

    // decrease FPS to target by thread sleeps
    myFPSControl.sleepToTarget();
    getStWindow()->stglSwap(ST_WIN_MASTER);
    ++myFPSControl;
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
    int aRowSupportLevel = ST_DEVICE_SUPPORT_NONE;
    if(theToDetectPriority) {
        if(StCore::INIT() != STERROR_LIBNOERROR) {
            ST_DEBUG_LOG(ST_OUT_PLUGIN_NAME + " Plugin, Core library not available!");
        } else {
            bool dummy = false;
            StHandle<StMonitor> aMon = StOutInterlace::getHInterlaceMonitor(dummy);
            if(!aMon.isNull()) {
                aRowSupportLevel = ST_DEVICE_SUPPORT_HIGHT;
                aMon.nullify();
            }
            StCore::FREE();
        }
    }

    // devices list
    static StString aRowInterName = aLangMap.changeValueId(STTR_HINTERLACE_NAME,    "Row Interlaced");
    static StString aRowInterDesc = aLangMap.changeValueId(STTR_HINTERLACE_DESC,    "Row interlaced displays: Zalman, Hyundai,...");
    static StString aColInterName = aLangMap.changeValueId(STTR_VINTERLACE_NAME,    "Column Interlaced");
    static StString aColInterDesc = aLangMap.changeValueId(STTR_VINTERLACE_DESC,    "Column interlaced displays");
    static StString aChessName    = aLangMap.changeValueId(STTR_CHESSBOARD_NAME,    "DLP TV (chessboard)");
    static StString aChessDesc    = aLangMap.changeValueId(STTR_CHESSBOARD_DESC,    "DLP TV (chessboard)");
    static StString aRowEdName    = aLangMap.changeValueId(STTR_HINTERLACE_ED_NAME, "Interlaced ED");
    static StString aRowEdDesc    = aLangMap.changeValueId(STTR_HINTERLACE_ED_DESC, "EDimensional in interlaced mode");
    static StStereoDeviceInfo_t aDevicesArray[4] = {
        { "StOutHInterlace",   aRowInterName.toCString(), aRowInterDesc.toCString(), aRowSupportLevel },
        { "StOutVInterlace",   aColInterName.toCString(), aColInterDesc.toCString(), ST_DEVICE_SUPPORT_NONE },
        { "StOutChessboard",   aChessName.toCString(),    aChessDesc.toCString(),    ST_DEVICE_SUPPORT_NONE },
        { "StOutHInterlaceED", aRowEdName.toCString(),    aRowEdDesc.toCString(),    ST_DEVICE_SUPPORT_NONE }
    };

    ST_SELF_INFO.devices = &aDevicesArray[0];
    ST_SELF_INFO.count   = 4;

    // about string
    StString& aTitle     = aLangMap.changeValueId(STTR_PLUGIN_TITLE,   "sView - Interlaced Output library");
    StString& aVerString = aLangMap.changeValueId(STTR_VERSION_STRING, "version");
    StString& aDescr     = aLangMap.changeValueId(STTR_PLUGIN_DESCRIPTION,
        "(C) 2009-2013 Kirill Gavrilov <kirill@sview.ru>\nOfficial site: www.sview.ru\n\nThis library distributed under LGPL3.0");
    static StString anAboutString = aTitle + '\n' + aVerString + ": " + StVersionInfo::getSDKVersionString() + "\n \n" + aDescr;
    ST_SELF_INFO.aboutString = (stUtf8_t* )anAboutString.toCString();

    return &ST_SELF_INFO;
}

ST_EXPORT StRendererInterface* StRenderer_new() {
    return new StOutInterlace(); }
ST_EXPORT void StRenderer_del(StRendererInterface* inst) {
    delete (StOutInterlace* )inst; }
ST_EXPORT StWindowInterface* StRenderer_getStWindow(StRendererInterface* inst) {
    // This is VERY important return libImpl pointer here!
    return ((StOutInterlace* )inst)->getStWindow()->getLibImpl(); }
ST_EXPORT stBool_t StRenderer_init(StRendererInterface* inst,
                                   const stUtf8_t*      theRendererPath,
                                   const int&           theDeviceId,
                                   const StNativeWin_t  theNativeParent) {
    return ((StOutInterlace* )inst)->init(StString(theRendererPath), theDeviceId, theNativeParent); }
ST_EXPORT stBool_t StRenderer_open(StRendererInterface* inst, const StOpenInfo_t* stOpenInfo) {
    return ((StOutInterlace* )inst)->open(StOpenInfo(stOpenInfo)); }
ST_EXPORT void StRenderer_callback(StRendererInterface* inst, StMessage_t* stMessages) {
    ((StOutInterlace* )inst)->callback(stMessages); }
ST_EXPORT void StRenderer_stglDraw(StRendererInterface* inst, unsigned int views) {
    ((StOutInterlace* )inst)->stglDraw(views); }
