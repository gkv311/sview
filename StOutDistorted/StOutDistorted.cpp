/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
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
#include <StImage/StImageFile.h>

namespace {

    static const char ST_OUT_PLUGIN_NAME[]   = "StOutDistorted";

    static const char ST_SETTING_DEVICE_ID[] = "deviceId";
    static const char ST_SETTING_WINDOWPOS[] = "windowPos";
    static const char ST_SETTING_VSYNC[]     = "vsync";

    // translation resources
    enum {
        STTR_DISTORTED_NAME     = 1000,
        STTR_DISTORTED_DESC     = 1001,

        // parameters
        STTR_PARAMETER_VSYNC    = 1100,
        STTR_PARAMETER_SLAVE_ID = 1102,

        // about info
        STTR_PLUGIN_TITLE       = 2000,
        STTR_VERSION_STRING     = 2001,
        STTR_PLUGIN_DESCRIPTION = 2002,
    };

};

/**
 * Distortion GLSL program.
 */
class StProgramBarrel : public StGLProgram {

        private:

    StGLVarLocation atrVVertexLoc;
    StGLVarLocation atrVTexCoordLoc;

        public:

    StProgramBarrel()
    : StGLProgram("StProgramBarrel"),
      atrVVertexLoc(),
      atrVTexCoordLoc() {
        //
    }

    StGLVarLocation getVVertexLoc() const {
        return atrVVertexLoc;
    }

    StGLVarLocation getVTexCoordLoc() const {
        return atrVTexCoordLoc;
    }

    virtual bool init(StGLContext& theCtx) {
        const char VERTEX_SHADER[] =
           "attribute vec4 vVertex; \
            attribute vec2 vTexCoord; \
            varying vec2 fTexCoord; \
            void main(void) { \
                fTexCoord = vTexCoord; \
                gl_Position = vVertex; \
            }";

        const char FRAGMENT_SHADER[] =
           "uniform sampler2D texR, texL; \
            varying vec2 fTexCoord; \
            void main(void) { \
                gl_FragColor = texture2D(texR, fTexCoord); \
            }";

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

StAtomic<int32_t> StOutDistorted::myInstancesNb(0);

StString StOutDistorted::getRendererAbout() const {
    return myAbout;
}

const char* StOutDistorted::getRendererId() const {
    return ST_OUT_PLUGIN_NAME;
}

const char* StOutDistorted::getDeviceId() const {
    return "Distorted";
}

bool StOutDistorted::setDevice(const StString& theDevice) {
    return theDevice == "Distorted";
}

void StOutDistorted::getDevices(StOutDevicesList& theList) const {
    for(size_t anIter = 0; anIter < myDevices.size(); ++anIter) {
        theList.add(myDevices[anIter]);
    }
}

void StOutDistorted::getOptions(StParamsList& theList) const {
    theList.add(params.IsVSyncOn);
}

StOutDistorted::StOutDistorted(const StNativeWin_t theParentWindow)
: StWindow(theParentWindow),
  mySettings(new StSettings(ST_OUT_PLUGIN_NAME)),
  myFrBuffer(new StGLFrameBuffer()),
  myCursor(new StGLTexture(GL_RGBA8)),
  myProgram(new StProgramBarrel()),
  myToShowCursor(true),
  myToSavePlacement(theParentWindow == (StNativeWin_t )NULL),
  myToCompressMem(myInstancesNb.increment() > 1),
  myIsBroken(false) {
    const StSearchMonitors& aMonitors = StWindow::getMonitors();
    StTranslations aLangMap(ST_OUT_PLUGIN_NAME);

    // about string
    StString& aTitle     = aLangMap.changeValueId(STTR_PLUGIN_TITLE,   "sView - Distorted Output module");
    StString& aVerString = aLangMap.changeValueId(STTR_VERSION_STRING, "version");
    StString& aDescr     = aLangMap.changeValueId(STTR_PLUGIN_DESCRIPTION,
        "(C) 2013 Kirill Gavrilov <kirill@sview.ru>\nOfficial site: www.sview.ru\n\nThis library distributed under LGPL3.0");
    myAbout = aTitle + '\n' + aVerString + ": " + StVersionInfo::getSDKVersionString() + "\n \n" + aDescr;

    // detect connected displays
    int aSupportLevel = ST_DEVICE_SUPPORT_NONE;
    for(size_t aMonIter = 0; aMonIter < aMonitors.size(); ++aMonIter) {
        const StMonitor& aMon = aMonitors[aMonIter];
        if(aMon.getPnPId().isStartsWith("OVR")) {
            // Oculus Rift
            aSupportLevel = ST_DEVICE_SUPPORT_HIGHT;
            break;
        }
    }

    // devices list
    StHandle<StOutDevice> aDevDistorted = new StOutDevice();
    aDevDistorted->PluginId = ST_OUT_PLUGIN_NAME;
    aDevDistorted->DeviceId = "Distorted";
    aDevDistorted->Priority = aSupportLevel;
    aDevDistorted->Name     = aLangMap.changeValueId(STTR_DISTORTED_NAME, "Distorted Output");
    aDevDistorted->Desc     = aLangMap.changeValueId(STTR_DISTORTED_DESC, "Oculus Rift");
    myDevices.add(aDevDistorted);

    // VSync option
    params.IsVSyncOn = new StBoolParamNamed(true, aLangMap.changeValueId(STTR_PARAMETER_VSYNC, "VSync"));
    params.IsVSyncOn->signals.onChanged.connect(this, &StOutDistorted::doVSync);

    // load window position
    StRect<int32_t> aRect(256, 768, 256, 1024);
    mySettings->loadInt32Rect(ST_SETTING_WINDOWPOS, aRect);
    StWindow::setPlacement(aRect, true);
    StWindow::setTitle("sView - Distorted Renderer");

    // load VSync option
    mySettings->loadParam(ST_SETTING_VSYNC, params.IsVSyncOn);
}

void StOutDistorted::releaseResources() {
    if(!myContext.isNull()) {
        myProgram->release(*myContext);
        myFrVertsBuf  .release(*myContext);
        myFrTCoordBuf .release(*myContext);
        myCurVertsBuf .release(*myContext);
        myCurTCoordBuf.release(*myContext);
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
    mySettings->saveParam(ST_SETTING_VSYNC, params.IsVSyncOn);
}

StOutDistorted::~StOutDistorted() {
    myInstancesNb.decrement();
    releaseResources();
}

void StOutDistorted::close() {
    releaseResources();
    StWindow::close();
}

bool StOutDistorted::create() {
    StWindow::show();
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

    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglSetVSync(params.IsVSyncOn->getValue() ? StGLContext::VSync_ON : StGLContext::VSync_OFF);

    if(!myProgram->init(*myContext)) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, Failed to init Shader");
        return false;
    }
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

    myFrVertsBuf  .init(*myContext, 4, 4, QUAD_VERTICES);
    myFrTCoordBuf .init(*myContext, 2, 4, QUAD_TEXCOORD);
    myCurVertsBuf .init(*myContext, 4, 4, QUAD_VERTICES);
    myCurTCoordBuf.init(*myContext, 2, 4, QUAD_TEXCOORD);

    // cursor texture
    const StString aTexturesFolder = StProcess::getStCoreFolder() + "textures" + SYS_FS_SPLITTER;
    const StString aCursorPath     = aTexturesFolder + "cursor.std";
    StHandle<StImageFile> aCursorImg = StImageFile::create();
    if(!aCursorImg.isNull()
    && aCursorImg->load(aCursorPath, StImageFile::ST_TYPE_PNG)) {
        //myCursor->setMinMagFilter(*myContext, GL_NEAREST);
        myCursor->init(*myContext, aCursorImg->getPlane());
    }
    aCursorImg.nullify();
    return true;
}

void StOutDistorted::processEvents(StMessage_t* theMessages) {
    StWindow::processEvents(theMessages);
    for(size_t anIter = 0; theMessages[anIter].uin != StMessageList::MSG_NULL; ++anIter) {
        if(theMessages[anIter].uin != StMessageList::MSG_KEYS) {
            continue;
        }

        //bool* aKeys = ((bool* )theMessages[anIter].data);
    }
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
    myProgram->use(*myContext);
        myCurVertsBuf .bindVertexAttrib(*myContext, myProgram->getVVertexLoc());
        myCurTCoordBuf.bindVertexAttrib(*myContext, myProgram->getVTexCoordLoc());

        myContext->core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        myCurTCoordBuf.unBindVertexAttrib(*myContext, myProgram->getVTexCoordLoc());
        myCurVertsBuf .unBindVertexAttrib(*myContext, myProgram->getVVertexLoc());
    myProgram->unuse(*myContext);
    myCursor->unbind(*myContext);

    myContext->core20fwd->glDisable(GL_BLEND);
}

void StOutDistorted::stglDraw() {
    myFPSControl.setTargetFPS(StWindow::getTargetFps());

    const StGLBoxPx aViewPort = StWindow::stglViewport(ST_WIN_MASTER);
    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    if(!StWindow::isStereoOutput() || myIsBroken) {
        if(myToCompressMem) {
            myFrBuffer->release(*myContext);
        }

        myContext->stglResizeViewport(aViewPort);
        myContext->stglSetScissorRect(aViewPort, false);
        StWindow::showCursor(myToShowCursor);
        StWindow::signals.onRedraw(ST_DRAW_LEFT);
        myContext->stglResetScissorRect();

        myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
        StWindow::stglSwap(ST_WIN_ALL);
        ++myFPSControl;
        return;
    }

    const StPointD_t aCursorPos = StWindow::getMousePos();

    StGLBoxPx aViewPortL = aViewPort;
    aViewPortL.width() /= 2;
    StGLBoxPx aViewPortR = aViewPortL;
    aViewPortR.x() += aViewPortL.width();

    // resize FBO
    if(!myFrBuffer->initLazy(*myContext, aViewPortL.width(), aViewPortL.height(), StWindow::hasDepthBuffer())) {
        stError(StString(ST_OUT_PLUGIN_NAME) + " Plugin, Failed to init Frame Buffer");
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
    myFrTCoordBuf.init(*myContext, aTCoords);

    if(myCursor->isValid()) {
        // compute cursor position
        StArray<StGLVec4> aVerts(4);
        const GLfloat aCurLeft   = -1.0f + aCursorPos.x() * 2.0f;
        const GLfloat aCurTop    =  1.0f - aCursorPos.y() * 2.0f;
        const GLfloat aCurWidth  = GLfloat(myCursor->getSizeX()) / GLfloat(aViewPortL.width());
        const GLfloat aCurHeight = 2.0f * GLfloat(myCursor->getSizeY()) / GLfloat(aViewPortL.height());
        aVerts[0] = StGLVec4(aCurLeft + aCurWidth, aCurTop - aCurHeight, 0.0f, 1.0f);
        aVerts[1] = StGLVec4(aCurLeft + aCurWidth, aCurTop,              0.0f, 1.0f);
        aVerts[2] = StGLVec4(aCurLeft,             aCurTop - aCurHeight, 0.0f, 1.0f);
        aVerts[3] = StGLVec4(aCurLeft,             aCurTop,              0.0f, 1.0f);
        myCurVertsBuf.init(*myContext, aVerts);
    }

    // draw Left View into virtual frame buffer
    myFrBuffer->setupViewPort(*myContext); // we set TEXTURE sizes here
    myFrBuffer->bindBuffer(*myContext);
        StWindow::signals.onRedraw(ST_DRAW_LEFT);
        stglDrawCursor();
    myFrBuffer->unbindBuffer(*myContext);

    // now draw to real screen buffer
    // clear the screen and the depth buffer
    myContext->stglResizeViewport(aViewPort);
    myContext->stglSetScissorRect(aViewPort, false);
    myContext->core20fwd->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw Left view
    myContext->stglResizeViewport(aViewPortL);
    myContext->stglSetScissorRect(aViewPortL, false);

    myFrBuffer->bindTexture(*myContext);
    myProgram->use(*myContext);
        myFrVertsBuf .bindVertexAttrib(*myContext, myProgram->getVVertexLoc());
        myFrTCoordBuf.bindVertexAttrib(*myContext, myProgram->getVTexCoordLoc());

        myContext->core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        myFrTCoordBuf.unBindVertexAttrib(*myContext, myProgram->getVTexCoordLoc());
        myFrVertsBuf .unBindVertexAttrib(*myContext, myProgram->getVVertexLoc());
    myProgram->unuse(*myContext);
    myFrBuffer->unbindTexture(*myContext);
    myContext->stglResetScissorRect();

    // draw Right View into virtual frame buffer
    myFrBuffer->setupViewPort(*myContext); // we set TEXTURE sizes here
    myFrBuffer->bindBuffer(*myContext);
        StWindow::signals.onRedraw(ST_DRAW_RIGHT);
        stglDrawCursor();
    myFrBuffer->unbindBuffer(*myContext);

    // draw Right view
    myContext->stglResizeViewport(aViewPortR);
    myContext->stglSetScissorRect(aViewPortR, false);

    myFrBuffer->bindTexture(*myContext);
    myProgram->use(*myContext);
    myFrVertsBuf .bindVertexAttrib(*myContext, myProgram->getVVertexLoc());
    myFrTCoordBuf.bindVertexAttrib(*myContext, myProgram->getVTexCoordLoc());

    myContext->core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    myFrTCoordBuf.unBindVertexAttrib(*myContext, myProgram->getVTexCoordLoc());
    myFrVertsBuf .unBindVertexAttrib(*myContext, myProgram->getVVertexLoc());

    myProgram->unuse(*myContext);
    myFrBuffer->unbindTexture(*myContext);
    myContext->stglResetScissorRect();

    myFPSControl.sleepToTarget(); // decrease FPS to target by thread sleeps
    StWindow::stglSwap(ST_WIN_ALL);
    ++myFPSControl;
}

void StOutDistorted::doVSync(const bool theValue) {
    if(myContext.isNull()) {
        return;
    }

    StWindow::stglMakeCurrent(ST_WIN_MASTER);
    myContext->stglSetVSync(theValue ? StGLContext::VSync_ON : StGLContext::VSync_OFF);
}
