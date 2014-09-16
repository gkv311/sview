/**
 * Copyright © 2013-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutDual library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutDual library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StOutDistorted.h"

#include <StGL/StGLContext.h>
#include <StGL/StGLEnums.h>
#include <StGL/StGLProgram.h>
#include <StGL/StGLFrameBuffer.h>
#include <StGLCore/StGLCore20.h>
#include <StSettings/StSettings.h>
#include <StSettings/StTranslations.h>
#include <StSettings/StEnumParam.h>
#include <StCore/StSearchMonitors.h>
#include <StVersion.h>
#include <StAV/StAVImage.h>

namespace {

    static const char ST_OUT_PLUGIN_NAME[]   = "StOutDistorted";

    static const char ST_SETTING_DEVICE_ID[] = "deviceId";
    static const char ST_SETTING_WINDOWPOS[] = "windowPos";
    static const char ST_SETTING_LAYOUT[]    = "layout";
    static const char ST_SETTING_ANAMORPH[]  = "anamorph";
    static const char ST_SETTING_MONOCLONE[] = "monoClone";
    static const char ST_SETTING_MARGINS[]   = "margins";
    static const char ST_SETTING_WARP_COEF[] = "warpCoef";
    static const char ST_SETTING_CHROME_AB[] = "chromeAb";

    // translation resources
    enum {
        STTR_DISTORTED_NAME     = 1000,
        STTR_DISTORTED_DESC     = 1001,
        STTR_OCULUS_NAME        = 1002,
        STTR_OCULUS_DESC        = 1003,

        // parameters
        STTR_PARAMETER_LAYOUT     = 1110,
        STTR_PARAMETER_LAYOUT_SBS        = 1111,
        STTR_PARAMETER_LAYOUT_OVERUNDER  = 1112,
        STTR_PARAMETER_DISTORTION = 1120,
        STTR_PARAMETER_DISTORTION_OFF    = 1121,
        STTR_PARAMETER_MONOCLONE         = 1123,

        // about info
        STTR_PLUGIN_TITLE       = 2000,
        STTR_VERSION_STRING     = 2001,
        STTR_PLUGIN_DESCRIPTION = 2002,
    };

    static const char VERTEX_SHADER[] =
       "attribute vec4 vVertex;"
       "attribute vec2 vTexCoord;"
       "varying vec2 fTexCoord;"
       "void main(void) {"
       "    fTexCoord = vTexCoord;"
       "    gl_Position = vVertex;"
       "}";

}

/**
 * Flat GLSL program.
 */
class StProgramFlat : public StGLProgram {

        private:

    StGLVarLocation atrVVertexLoc;
    StGLVarLocation atrVTexCoordLoc;

        public:

    ST_LOCAL StProgramFlat() : StGLProgram("StProgramFlat") {}
    ST_LOCAL StGLVarLocation getVVertexLoc()   const { return atrVVertexLoc; }
    ST_LOCAL StGLVarLocation getVTexCoordLoc() const { return atrVTexCoordLoc; }

    ST_LOCAL virtual bool init(StGLContext& theCtx) {
        const char FRAGMENT_SHADER[] =
           "uniform sampler2D texR, texL;"
           "varying vec2 fTexCoord;"
           "void main(void) {"
           "    gl_FragColor = texture2D(texR, fTexCoord);"
           "}";

        StGLVertexShader aVertexShader(StGLProgram::getTitle());
        StGLAutoRelease aTmp1(theCtx, aVertexShader);
        aVertexShader.init(theCtx, VERTEX_SHADER);

        StGLFragmentShader aFragmentShader(StGLProgram::getTitle());
        StGLAutoRelease aTmp2(theCtx, aFragmentShader);
        aFragmentShader.init(theCtx, FRAGMENT_SHADER);
        if(!StGLProgram::create(theCtx)
           .attachShader(theCtx, aVertexShader)
           .attachShader(theCtx, aFragmentShader)
           .link(theCtx)) {
            return false;
        }

        atrVVertexLoc   = StGLProgram::getAttribLocation(theCtx, "vVertex");
        atrVTexCoordLoc = StGLProgram::getAttribLocation(theCtx, "vTexCoord");
        return atrVVertexLoc.isValid() && atrVTexCoordLoc.isValid();
    }

};

/**
 * Distortion GLSL program.
 */
class StProgramBarrel : public StGLProgram {

        public:

    ST_LOCAL StProgramBarrel() : StGLProgram("StProgramBarrel") {}
    ST_LOCAL StGLVarLocation getVVertexLoc()   const { return atrVVertexLoc; }
    ST_LOCAL StGLVarLocation getVTexCoordLoc() const { return atrVTexCoordLoc; }

    ST_LOCAL virtual bool init(StGLContext& theCtx) {
        const char FRAGMENT_SHADER[] =
           "uniform sampler2D texR, texL;"
           "varying vec2 fTexCoord;"

           "uniform vec4 uChromAb;"
           "uniform vec4 uWarpCoef;"
           "uniform vec2 uLensCenter;"
           "uniform vec2 uScale;"
           "uniform vec2 uScaleIn;"

           "void main(void) {"
           "    vec2 aTheta = (fTexCoord - uLensCenter) * uScaleIn;" // scales to [-1, 1]
           "    float rSq = aTheta.x * aTheta.x + aTheta.y * aTheta.y;"
           "    vec2 aTheta1 = aTheta * (uWarpCoef.x + uWarpCoef.y * rSq +"
           "                             uWarpCoef.z * rSq * rSq +"
           "                             uWarpCoef.w * rSq * rSq * rSq);"
           "    vec2 aThetaBlue = aTheta1 * (uChromAb.z + uChromAb.w * rSq);"
           "    vec2 aTCrdsBlue = uLensCenter + uScale * aThetaBlue;"
           "    if(any(bvec2(clamp(aTCrdsBlue, vec2(0.0, 0.0), vec2(1.0, 1.0)) - aTCrdsBlue))) {"
           "        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);"
           "        return;"
           "    }"

           "    vec2 aTCrdsGreen = uLensCenter + uScale * aTheta1;"
           "    vec2 aThetaRed = aTheta1 * (uChromAb.x + uChromAb.y * rSq);"
           "    vec2 aTCrdsRed = uLensCenter + uScale * aThetaRed;"
           "    gl_FragColor = vec4(texture2D(texR, aTCrdsRed  ).r,"
           "                        texture2D(texR, aTCrdsGreen).g,"
           "                        texture2D(texR, aTCrdsBlue ).b, 1.0);"
           "}";

        StGLVertexShader aVertexShader(StGLProgram::getTitle());
        StGLAutoRelease aTmp1(theCtx, aVertexShader);
        aVertexShader.init(theCtx, VERTEX_SHADER);

        StGLFragmentShader aFragmentShader(StGLProgram::getTitle());
        StGLAutoRelease aTmp2(theCtx, aFragmentShader);
        aFragmentShader.init(theCtx, FRAGMENT_SHADER);
        if(!StGLProgram::create(theCtx)
           .attachShader(theCtx, aVertexShader)
           .attachShader(theCtx, aFragmentShader)
           .link(theCtx)) {
            return false;
        }

        atrVVertexLoc    = StGLProgram::getAttribLocation (theCtx, "vVertex");
        atrVTexCoordLoc  = StGLProgram::getAttribLocation (theCtx, "vTexCoord");
        uniChromAbLoc    = StGLProgram::getUniformLocation(theCtx, "uChromAb");
        uniWarpCoeffLoc  = StGLProgram::getUniformLocation(theCtx, "uWarpCoef");
        uniLensCenterLoc = StGLProgram::getUniformLocation(theCtx, "uLensCenter");
        uniScaleLoc      = StGLProgram::getUniformLocation(theCtx, "uScale");
        uniScaleInLoc    = StGLProgram::getUniformLocation(theCtx, "uScaleIn");
        return atrVVertexLoc.isValid() && atrVTexCoordLoc.isValid();
    }

    /**
     * Setup distortion coefficients.
     */
    ST_LOCAL void setupCoeff(StGLContext&    theCtx,
                             const StGLVec4& theVec) {
        use(theCtx);
        theCtx.core20fwd->glUniform4fv(uniWarpCoeffLoc, 1, theVec);
        unuse(theCtx);
    }

    /**
     * Setup Chrome coefficients.
     */
    ST_LOCAL void setupChrome(StGLContext&    theCtx,
                              const StGLVec4& theVec) {
        use(theCtx);
        theCtx.core20fwd->glUniform4fv(uniChromAbLoc, 1, theVec);
        unuse(theCtx);
    }

    ST_LOCAL void setLensCenter(StGLContext&    theCtx,
                                const StGLVec2& theVec) {
        use(theCtx);
        theCtx.core20fwd->glUniform2fv(uniLensCenterLoc, 1, theVec);
        unuse(theCtx);
    }

    ST_LOCAL void setScale(StGLContext&    theCtx,
                           const StGLVec2& theVec) {
        use(theCtx);
        theCtx.core20fwd->glUniform2fv(uniScaleLoc, 1, theVec);
        unuse(theCtx);
    }

    ST_LOCAL void setScaleIn(StGLContext&    theCtx,
                             const StGLVec2& theVec) {
        use(theCtx);
        theCtx.core20fwd->glUniform2fv(uniScaleInLoc, 1, theVec);
        unuse(theCtx);
    }

        private:

    StGLVarLocation atrVVertexLoc;
    StGLVarLocation atrVTexCoordLoc;
    StGLVarLocation uniChromAbLoc;
    StGLVarLocation uniWarpCoeffLoc;
    StGLVarLocation uniLensCenterLoc;
    StGLVarLocation uniScaleLoc;
    StGLVarLocation uniScaleInLoc;

};

StAtomic<int32_t> StOutDistorted::myInstancesNb(0);

StString StOutDistorted::getRendererAbout() const {
    return myAbout;
}

const char* StOutDistorted::getRendererId() const {
    return ST_OUT_PLUGIN_NAME;
}

const char* StOutDistorted::getDeviceId() const {
    switch(myDevice) {
        case DEVICE_OCULUS:    return "Oculus";
        case DEVICE_DISTORTED:
        default:               return "Distorted";
    }
}

bool StOutDistorted::isLostDevice() const {
    return myToResetDevice || StWindow::isLostDevice();
}

bool StOutDistorted::setDevice(const StString& theDevice) {
    if(theDevice == "Oculus") {
        if(myDevice != DEVICE_OCULUS) {
            myToResetDevice = true;
        }
        myDevice = DEVICE_OCULUS;
    } else if(theDevice == "Distorted") {
        if(myDevice != DEVICE_DISTORTED) {
            myToResetDevice = true;
        }
        myDevice = DEVICE_DISTORTED;
    }
    return false;
}

void StOutDistorted::getDevices(StOutDevicesList& theList) const {
    for(size_t anIter = 0; anIter < myDevices.size(); ++anIter) {
        theList.add(myDevices[anIter]);
    }
}

void StOutDistorted::getOptions(StParamsList& theList) const {
    if(myDevice != DEVICE_OCULUS) {
        theList.add(params.Layout);
        theList.add(params.Anamorph);
    }
    theList.add(params.MonoClone);
}

StOutDistorted::StOutDistorted(const StHandle<StResourceManager>& theResMgr,
                               const StNativeWin_t                theParentWindow)
: StWindow(theResMgr, theParentWindow),
  mySettings(new StSettings(ST_OUT_PLUGIN_NAME)),
  myDevice(DEVICE_AUTO),
  myToResetDevice(false),
  myFrBuffer(new StGLFrameBuffer()),
  myCursor(new StGLTexture(GL_RGBA8)),
  myProgramFlat(new StProgramFlat()),
  myProgramBarrel(new StProgramBarrel()),
  myBarrelCoef(1.0f, 0.22f, 0.24f, 0.041f), // 7 inches
  //myBarrelCoef(1.0f, 0.18f, 0.115f, 0.0387f),
  myChromAb(0.996f, -0.004f, 1.014f, 0.0f),
  //myChromAb(1.0f, 0.0f, 1.0f, 0.0f),
  myToReduceGui(false),
  myToShowCursor(true),
  myToSavePlacement(theParentWindow == (StNativeWin_t )NULL),
  myToCompressMem(myInstancesNb.increment() > 1),
  myIsBroken(false),
  myIsStereoOn(false) {
    const StSearchMonitors& aMonitors = StWindow::getMonitors();
    StTranslations aLangMap(getResourceManager(), ST_OUT_PLUGIN_NAME);

    // about string
    StString& aTitle     = aLangMap.changeValueId(STTR_PLUGIN_TITLE,   "sView - Distorted Output module");
    StString& aVerString = aLangMap.changeValueId(STTR_VERSION_STRING, "version");
    StString& aDescr     = aLangMap.changeValueId(STTR_PLUGIN_DESCRIPTION,
        "(C) 2013-2014 Kirill Gavrilov <kirill@sview.ru>\nOfficial site: www.sview.ru\n\nThis library distributed under LGPL3.0");
    myAbout = aTitle + '\n' + aVerString + ": " + StVersionInfo::getSDKVersionString() + "\n \n" + aDescr;

    // detect connected displays
    int aSupportLevel = ST_DEVICE_SUPPORT_NONE;
    for(size_t aMonIter = 0; aMonIter < aMonitors.size(); ++aMonIter) {
        const StMonitor& aMon = aMonitors[aMonIter];
        if(aMon.getPnPId().isStartsWith(stCString("OVR"))) {
            // Oculus Rift
            aSupportLevel = ST_DEVICE_SUPPORT_HIGHT;
            break;
        }
    }

    // devices list
    StHandle<StOutDevice> aDevDistorted = new StOutDevice();
    aDevDistorted->PluginId = ST_OUT_PLUGIN_NAME;
    aDevDistorted->DeviceId = "Distorted";
    aDevDistorted->Priority = ST_DEVICE_SUPPORT_NONE;
    aDevDistorted->Name     = aLangMap.changeValueId(STTR_DISTORTED_NAME, "Parallel Pair");
    aDevDistorted->Desc     = aLangMap.changeValueId(STTR_DISTORTED_DESC, "Distorted Output");
    myDevices.add(aDevDistorted);

    StHandle<StOutDevice> aDevOculus = new StOutDevice();
    aDevOculus->PluginId = ST_OUT_PLUGIN_NAME;
    aDevOculus->DeviceId = "Oculus";
    aDevOculus->Priority = aSupportLevel;
    aDevOculus->Name     = aLangMap.changeValueId(STTR_OCULUS_NAME, "Oculus Rift");
    aDevOculus->Desc     = aLangMap.changeValueId(STTR_OCULUS_DESC, "Distorted Output");
    myDevices.add(aDevOculus);

    // load device settings
    mySettings->loadInt32(ST_SETTING_DEVICE_ID, myDevice);
    if(myDevice == DEVICE_AUTO) {
        myDevice = DEVICE_DISTORTED;
    }

    // Distortion parameters
    params.Anamorph = new StBoolParamNamed(false, "Anamorph");
    mySettings->loadParam(ST_SETTING_ANAMORPH, params.Anamorph);
    params.MonoClone = new StBoolParamNamed(false, aLangMap.changeValueId(STTR_PARAMETER_MONOCLONE, "Show Mono in Stereo"));
    mySettings->loadParam(ST_SETTING_MONOCLONE, params.MonoClone);

    // Layout option
    StHandle<StEnumParam> aLayoutParam = new StEnumParam(LAYOUT_SIDE_BY_SIDE,
                                                         aLangMap.changeValueId(STTR_PARAMETER_LAYOUT, "Layout"));
    aLayoutParam->changeValues().add(aLangMap.changeValueId(STTR_PARAMETER_LAYOUT_SBS,       "Side-by-Side"));
    aLayoutParam->changeValues().add(aLangMap.changeValueId(STTR_PARAMETER_LAYOUT_OVERUNDER, "Top-and-Bottom"));
    params.Layout = aLayoutParam;
    mySettings->loadParam(ST_SETTING_LAYOUT, params.Layout);

    // load window position
    StRect<int32_t> aRect(256, 768, 256, 1024);
    mySettings->loadInt32Rect(ST_SETTING_WINDOWPOS, aRect);
    StWindow::setPlacement(aRect, true);
    StWindow::setTitle("sView - Distorted Renderer");

    myBarMargins.left()   = 64;
    myBarMargins.right()  = 64;
    myBarMargins.top()    = 160;
    myBarMargins.bottom() = 160;
    mySettings->loadInt32Rect(ST_SETTING_MARGINS,   myBarMargins);
    mySettings->loadFloatVec4(ST_SETTING_WARP_COEF, myBarrelCoef);
    mySettings->loadFloatVec4(ST_SETTING_CHROME_AB, myChromAb);
}

void StOutDistorted::releaseResources() {
    if(!myContext.isNull()) {
        myProgramFlat->release(*myContext);
        myProgramBarrel->release(*myContext);
        myFrVertsBuf .release(*myContext);
        myFrTCrdsBuf .release(*myContext);
        myCurVertsBuf.release(*myContext);
        myCurTCrdsBuf.release(*myContext);
        myFrBuffer->release(*myContext);
        myCursor->release(*myContext);
    }
    myContext.nullify();

    // read windowed placement
    StWindow::hide();
    if(myToSavePlacement) {
        StWindow::setFullScreen(false);
        mySettings->saveInt32Rect(ST_SETTING_WINDOWPOS, StWindow::getPlacement());
    }
    mySettings->saveParam(ST_SETTING_LAYOUT,     params.Layout);
    mySettings->saveParam(ST_SETTING_ANAMORPH,   params.Anamorph);
    mySettings->saveParam(ST_SETTING_MONOCLONE,  params.MonoClone);
    mySettings->saveInt32Rect(ST_SETTING_MARGINS,   myBarMargins);
    mySettings->saveFloatVec4(ST_SETTING_WARP_COEF, myBarrelCoef);
    mySettings->saveFloatVec4(ST_SETTING_CHROME_AB, myChromAb);
    if(myWasUsed) {
        mySettings->saveInt32(ST_SETTING_DEVICE_ID, myDevice);
    }
}

StOutDistorted::~StOutDistorted() {
    myInstancesNb.decrement();
    releaseResources();
}

void StOutDistorted::close() {
    StWindow::params.VSyncMode->signals.onChanged -= stSlot(this, &StOutDistorted::doSwitchVSync);
    myToResetDevice = false;
    releaseResources();
    StWindow::close();
}

bool StOutDistorted::create() {
    StWindow::show();
    if(!StWindow::create()) {
        return false;
    }

    // initialize GL context
    myContext = StWindow::getContext();
    myContext->setMessagesQueue(myMsgQueue);
    if(!myContext->isGlGreaterEqual(2, 0)) {
        myMsgQueue->pushError(stCString("OpenGL 2.0 is required by Distorted Output"));
        myIsBroken = true;
        return true;
    }

    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglSetVSync((StGLContext::VSync_Mode )StWindow::params.VSyncMode->getValue());
    StWindow::params.VSyncMode->signals.onChanged += stSlot(this, &StOutDistorted::doSwitchVSync);

    if(!myProgramFlat  ->init(*myContext)
    || !myProgramBarrel->init(*myContext)) {
        myMsgQueue->pushError(stCString("Distorted output - critical error:\nShaders initialization failed!"));
        myIsBroken = true;
        return true;
    }
    myProgramBarrel->setupCoeff (*myContext, myBarrelCoef);
    myProgramBarrel->setupChrome(*myContext, myChromAb);

    // create vertices buffers to draw simple textured quad
    const GLfloat QUAD_VERTICES[4 * 4] = {
         1.0f, -1.0f, 0.0f, 1.0f, // top-right
         1.0f,  1.0f, 0.0f, 1.0f, // bottom-right
        -1.0f, -1.0f, 0.0f, 1.0f, // top-left
        -1.0f,  1.0f, 0.0f, 1.0f  // bottom-left
    };

    const GLfloat QUAD_TEXCOORD[2 * 4] = {
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        0.0f, 0.0f
    };

    myFrVertsBuf .init(*myContext, 4, 4, QUAD_VERTICES);
    myFrTCrdsBuf .init(*myContext, 2, 4, QUAD_TEXCOORD);
    myCurVertsBuf.init(*myContext, 4, 4, QUAD_VERTICES);
    myCurTCrdsBuf.init(*myContext, 2, 4, QUAD_TEXCOORD);

    // cursor texture
    StAVImage aCursorImg;
    StHandle<StResource> aCursorRes = getResourceManager()->getResource(StString("textures") + SYS_FS_SPLITTER + "cursor.png");
    uint8_t* aData     = NULL;
    int      aDataSize = 0;
    if(!aCursorRes.isNull()
    && !aCursorRes->isFile()
    &&  aCursorRes->read()) {
        aData     = (uint8_t* )aCursorRes->getData();
        aDataSize = aCursorRes->getSize();
    }
    if(aCursorImg.load(!aCursorRes.isNull() ? aCursorRes->getPath() : StString(), StImageFile::ST_TYPE_PNG, aData, aDataSize)) {
        //myCursor->setMinMagFilter(*myContext, GL_NEAREST);
        myCursor->init(*myContext, aCursorImg.getPlane());
    }
    myIsBroken = false;
    return true;
}

void StOutDistorted::processEvents() {
    StWindow::processEvents();
}

void StOutDistorted::showCursor(const bool theToShow) {
    myToShowCursor = theToShow;
}

void StOutDistorted::stglDrawCursor() {
    StWindow::showCursor(false);
    if(!myToShowCursor
    || !myCursor->isValid()) {
        return;
    }

    myContext->core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    myContext->core20fwd->glEnable(GL_BLEND);

    myCursor->bind(*myContext);
    myProgramFlat->use(*myContext);
        myCurVertsBuf.bindVertexAttrib(*myContext, myProgramFlat->getVVertexLoc());
        myCurTCrdsBuf.bindVertexAttrib(*myContext, myProgramFlat->getVTexCoordLoc());

        myContext->core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        myCurTCrdsBuf.unBindVertexAttrib(*myContext, myProgramFlat->getVTexCoordLoc());
        myCurVertsBuf.unBindVertexAttrib(*myContext, myProgramFlat->getVVertexLoc());
    myProgramFlat->unuse(*myContext);
    myCursor->unbind(*myContext);

    myContext->core20fwd->glDisable(GL_BLEND);
}

GLfloat StOutDistorted::getLensDist() const {
    return (myIsStereoOn && myDevice == DEVICE_OCULUS) ? 0.1453f : 0.0f;
}

GLfloat StOutDistorted::getScaleFactor() const {
    if(!myToReduceGui
    || !myIsStereoOn
    ||  myDevice != DEVICE_OCULUS) {
        return StWindow::getScaleFactor();
    }

    return 0.8f;
}

void StOutDistorted::setFullScreen(const bool theFullScreen) {
    if(!theFullScreen) {
        myMargins = StRectI_t();
    }
    StWindow::setFullScreen(theFullScreen);
}

void StOutDistorted::stglDraw() {
    myFPSControl.setTargetFPS(StWindow::getTargetFps());

    myIsStereoOn = (StWindow::isStereoSource() || params.MonoClone->getValue())
                && StWindow::isFullScreen()
                && !myIsBroken;
    myIsForcedStereo = myIsStereoOn && params.MonoClone->getValue();
    if(myIsStereoOn
    && myDevice == DEVICE_OCULUS) {
        myMargins = myBarMargins;
    } else {
        myMargins = StRectI_t();
    }

    if(myIsStereoOn
    && (myDevice == DEVICE_OCULUS || !params.Anamorph->getValue())) {
        StWindow::setAttribute(StWinAttr_SplitCfg, (params.Layout->getValue() == LAYOUT_OVER_UNDER && myDevice != DEVICE_OCULUS)
                                                 ? StWinSlave_splitVertical : StWinSlave_splitHorizontal);
    } else {
        StWindow::setAttribute(StWinAttr_SplitCfg, StWinSlave_splitOff);
    }

    const StGLBoxPx aVPMaster = StWindow::stglViewport(ST_WIN_MASTER);
    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    if(!myIsStereoOn) {
        if(myToCompressMem) {
            myFrBuffer->release(*myContext);
        }

        myContext->stglResizeViewport(aVPMaster);
        myContext->stglSetScissorRect(aVPMaster, false);
        StWindow::showCursor(myToShowCursor);
        StWindow::signals.onRedraw(ST_DRAW_LEFT);
        myContext->stglResetScissorRect();

        myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
        StWindow::stglSwap(ST_WIN_ALL);
        ++myFPSControl;
        return;
    }

    const StGLBoxPx  aVPSlave   = StWindow::stglViewport(ST_WIN_SLAVE);
    const StGLBoxPx  aVPBoth    = StWindow::stglViewport(ST_WIN_ALL);
    const StPointD_t aCursorPos = StWindow::getMousePos();

    StGLBoxPx aViewPortL = aVPMaster;
    StGLBoxPx aViewPortR = aVPSlave;
    if(params.Anamorph->getValue()
    && myDevice != DEVICE_OCULUS) {
        switch(params.Layout->getValue()) {
            case LAYOUT_OVER_UNDER: {
                aViewPortL.height() /= 2;
                aViewPortR.height() = aViewPortL.height();
                aViewPortR.y() += aViewPortL.height();
                break;
            }
            default:
            case LAYOUT_SIDE_BY_SIDE: {
                aViewPortL.width() /= 2;
                aViewPortR.width() = aViewPortL.width();
                aViewPortR.x() += aViewPortL.width();
                break;
            }
        }
    }

    // resize FBO
    GLint aFrSizeX = aViewPortL.width();
    GLint aFrSizeY = aViewPortL.height();
    if(myDevice == DEVICE_OCULUS) {
        myToReduceGui = aFrSizeX <= 640;
        aFrSizeX = int(std::ceil(double(aFrSizeX) * 1.25) + 0.5);
        aFrSizeY = int(std::ceil(double(aFrSizeY) * 1.25) + 0.5);
    }
    if(!myFrBuffer->initLazy(*myContext, GL_RGBA8, aFrSizeX, aFrSizeY, StWindow::hasDepthBuffer())) {
        myMsgQueue->pushError(stCString("Distorted output - critical error:\nFrame Buffer Object resize failed!"));
        myIsBroken = true;
        return;
    }

    // reduce viewport to avoid additional aliasing of narrow lines
    const GLfloat aDX = GLfloat(myFrBuffer->getVPSizeX()) / GLfloat(myFrBuffer->getSizeX());
    const GLfloat aDY = GLfloat(myFrBuffer->getVPSizeY()) / GLfloat(myFrBuffer->getSizeY());
    StArray<StGLVec2> aTCoords(4);
    aTCoords[0] = StGLVec2(aDX,  0.0f);
    aTCoords[1] = StGLVec2(aDX,  aDY);
    aTCoords[2] = StGLVec2(0.0f, 0.0f);
    aTCoords[3] = StGLVec2(0.0f, aDY);
    myFrTCrdsBuf.init(*myContext, aTCoords);

    const GLfloat aLensDisp = getLensDist() * 0.5f;

    // compute cursor position
    StArray<StGLVec4> aVerts(4);
    const GLfloat aCurLeft = GLfloat(-1.0 + aCursorPos.x() * 2.0);
    const GLfloat aCurTop  = GLfloat( 1.0 - aCursorPos.y() * 2.0);
    GLfloat aCurWidth  = 2.0f * GLfloat(myCursor->getSizeX()) / GLfloat(myFrBuffer->getVPSizeX());
    GLfloat aCurHeight = 2.0f * GLfloat(myCursor->getSizeY()) / GLfloat(myFrBuffer->getVPSizeY());
    if(params.Anamorph->getValue()
    && myDevice != DEVICE_OCULUS) {
        if(params.Layout->getValue() == LAYOUT_OVER_UNDER) {
            aCurHeight *= 0.5;
        } else {
            aCurWidth  *= 0.5;
        }
    }
    aVerts[0] = StGLVec4(2.0f * aLensDisp + aCurLeft + aCurWidth, aCurTop - aCurHeight, 0.0f, 1.0f);
    aVerts[1] = StGLVec4(2.0f * aLensDisp + aCurLeft + aCurWidth, aCurTop,              0.0f, 1.0f);
    aVerts[2] = StGLVec4(2.0f * aLensDisp + aCurLeft,             aCurTop - aCurHeight, 0.0f, 1.0f);
    aVerts[3] = StGLVec4(2.0f * aLensDisp + aCurLeft,             aCurTop,              0.0f, 1.0f);
    myCurVertsBuf.init(*myContext, aVerts);

    // draw Left View into virtual frame buffer
    myFrBuffer->setupViewPort(*myContext); // we set TEXTURE sizes here
    myFrBuffer->bindBuffer(*myContext);
        StWindow::signals.onRedraw(ST_DRAW_LEFT);
        stglDrawCursor();
    myFrBuffer->unbindBuffer(*myContext);

    // now draw to real screen buffer
    // clear the screen and the depth buffer
    myContext->stglResizeViewport(aVPBoth);
    myContext->stglSetScissorRect(aVPBoth, false);
    myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw Left view
    myContext->stglResizeViewport(aViewPortL);
    myContext->stglSetScissorRect(aViewPortL, false);

    StGLProgram*    aProgram   = myProgramFlat.access();
    StGLVarLocation aVertexLoc = myProgramFlat->getVVertexLoc();
    StGLVarLocation aTexCrdLoc = myProgramFlat->getVTexCoordLoc();
    if(myDevice == DEVICE_OCULUS) {
        aProgram   = myProgramBarrel.access();
        aVertexLoc = myProgramBarrel->getVVertexLoc();
        aTexCrdLoc = myProgramBarrel->getVTexCoordLoc();
        myProgramBarrel->setScaleIn(*myContext, StGLVec2(2.0f / aDX, 2.0f / aDY));
        myProgramBarrel->setScale  (*myContext, StGLVec2(0.4f * aDX, 0.4f * aDY));
    }

    myFrBuffer->bindTexture(*myContext);
    if(aProgram == myProgramBarrel.access()) {
        myProgramBarrel->setLensCenter(*myContext, StGLVec2((0.5f + aLensDisp) * aDX, 0.5f * aDY));
    }
    aProgram->use(*myContext);
        myFrVertsBuf.bindVertexAttrib(*myContext, aVertexLoc);
        myFrTCrdsBuf.bindVertexAttrib(*myContext, aTexCrdLoc);

        myContext->core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        myFrTCrdsBuf.unBindVertexAttrib(*myContext, aTexCrdLoc);
        myFrVertsBuf.unBindVertexAttrib(*myContext, aVertexLoc);
    aProgram->unuse(*myContext);
    myFrBuffer->unbindTexture(*myContext);
    myContext->stglResetScissorRect();

    // draw Right View into virtual frame buffer
    aVerts[0] = StGLVec4(-2.0f * aLensDisp + aCurLeft + aCurWidth, aCurTop - aCurHeight, 0.0f, 1.0f);
    aVerts[1] = StGLVec4(-2.0f * aLensDisp + aCurLeft + aCurWidth, aCurTop,              0.0f, 1.0f);
    aVerts[2] = StGLVec4(-2.0f * aLensDisp + aCurLeft,             aCurTop - aCurHeight, 0.0f, 1.0f);
    aVerts[3] = StGLVec4(-2.0f * aLensDisp + aCurLeft,             aCurTop,              0.0f, 1.0f);
    myCurVertsBuf.init(*myContext, aVerts);

    myFrBuffer->setupViewPort(*myContext); // we set TEXTURE sizes here
    myFrBuffer->bindBuffer(*myContext);
        StWindow::signals.onRedraw(ST_DRAW_RIGHT);
        stglDrawCursor();
    myFrBuffer->unbindBuffer(*myContext);

    // draw Right view
    myContext->stglResizeViewport(aViewPortR);
    myContext->stglSetScissorRect(aViewPortR, false);

    myFrBuffer->bindTexture(*myContext);
    if(aProgram == myProgramBarrel.access()) {
        myProgramBarrel->setLensCenter(*myContext, StGLVec2((0.5f - aLensDisp) * aDX, 0.5f * aDY));
    }
    aProgram->use(*myContext);
    myFrVertsBuf.bindVertexAttrib(*myContext, aVertexLoc);
    myFrTCrdsBuf.bindVertexAttrib(*myContext, aTexCrdLoc);

    myContext->core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    myFrTCrdsBuf.unBindVertexAttrib(*myContext, aTexCrdLoc);
    myFrVertsBuf.unBindVertexAttrib(*myContext, aVertexLoc);

    aProgram->unuse(*myContext);
    myFrBuffer->unbindTexture(*myContext);
    myContext->stglResetScissorRect();

    myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
    StWindow::stglSwap(ST_WIN_ALL);
    ++myFPSControl;
}

void StOutDistorted::doSwitchVSync(const int32_t theValue) {
    if(myContext.isNull()) {
        return;
    }

    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglSetVSync((StGLContext::VSync_Mode )theValue);
}
