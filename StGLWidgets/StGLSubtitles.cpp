/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2010-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLSubtitles.h>

#include <StGLCore/StGLCore20.h>
#include <StGL/StGLProgram.h>
#include <StGLWidgets/StGLImageRegion.h>
#include <StGLWidgets/StGLRootWidget.h>

namespace {
    static const size_t SHARE_IMAGE_PROGRAM_ID = StGLRootWidget::generateShareId();
    static StGLVCorner parseCorner(int theVal) { return (StGLVCorner )theVal; }
}

class StGLSubtitles::StImgProgram : public StGLProgram {

        public:

    StImgProgram() : StGLProgram("StGLSubtitles") {}

    StGLVarLocation getVVertexLoc()   const { return StGLVarLocation(0); }
    StGLVarLocation getVTexCoordLoc() const { return StGLVarLocation(1); }

    void setProjMat(StGLContext&      theCtx,
                    const StGLMatrix& theProjMat) {
        theCtx.core20fwd->glUniformMatrix4fv(uniProjMatLoc, 1, GL_FALSE, theProjMat);
    }

    virtual bool init(StGLContext& theCtx) {
        const char VERTEX_SHADER[] =
           "uniform mat4 uProjMat;\n"
           "uniform vec4 uDisp;\n"
           "attribute vec4 vVertex;\n"
           "attribute vec2 vTexCoord;\n"
           "varying   vec2 fTexCoord;\n"
           "void main(void) {\n"
           "    fTexCoord = vTexCoord;\n"
           "    gl_Position = uProjMat * (vVertex + uDisp);\n"
           "}\n";

        const char FRAGMENT_SHADER[] =
           "uniform sampler2D uTexture;\n"
           "varying vec2      fTexCoord;\n"
           "void main(void) {\n"
           "    gl_FragColor = texture2D(uTexture, fTexCoord);\n"
           "}\n";

        StGLVertexShader aVertexShader(StGLProgram::getTitle());
        aVertexShader.init(theCtx, VERTEX_SHADER);
        StGLAutoRelease aTmp1(theCtx, aVertexShader);

        StGLFragmentShader aFragmentShader(StGLProgram::getTitle());
        aFragmentShader.init(theCtx, FRAGMENT_SHADER);
        StGLAutoRelease aTmp2(theCtx, aFragmentShader);
        if(!StGLProgram::create(theCtx)
           .attachShader(theCtx, aVertexShader)
           .attachShader(theCtx, aFragmentShader)
           .bindAttribLocation(theCtx, "vVertex",   getVVertexLoc())
           .bindAttribLocation(theCtx, "vTexCoord", getVTexCoordLoc())
           .link(theCtx)) {
            return false;
        }

        StGLVarLocation uniTextureLoc = StGLProgram::getUniformLocation(theCtx, "uTexture");
        if(uniTextureLoc.isValid()) {
            StGLProgram::use(theCtx);
            theCtx.core20fwd->glUniform1i(uniTextureLoc, StGLProgram::TEXTURE_SAMPLE_0);
            StGLProgram::unuse(theCtx);
        }

        uniProjMatLoc = StGLProgram::getUniformLocation(theCtx, "uProjMat");
        uniDispLoc    = StGLProgram::getUniformLocation(theCtx, "uDisp");
        return uniProjMatLoc.isValid()
            && uniTextureLoc.isValid();
    }

        private:

    StGLVarLocation uniProjMatLoc;
    StGLVarLocation uniDispLoc;

};

StGLSubtitles::StSubShowItems::StSubShowItems()
: StArrayList<StHandle <StSubItem> >(8), Scale (1.0f) {
    //
}

bool StGLSubtitles::StSubShowItems::pop(const double thePTS) {
    bool isChanged = false;
    for(size_t anId = size() - 1; anId < size_t(-1); --anId) {
        // filter outdated and forward items
        const StHandle<StSubItem>& anItem = getValue(anId);
        if(anItem->TimeEnd < thePTS || anItem->TimeStart > thePTS) {
            remove(anId);
            isChanged = true;
        }
    }
    if(!isChanged) {
        return false;
    } else if(isEmpty()) {
        Text.clear();
        Image.nullify();
        return true;
    }

    // update active text
    Text = getFirst()->Text;
    for(size_t anId = 1; anId < size(); ++anId) {
        const StHandle<StSubItem>& anItem = getValue(anId);
        Text += StString('\n');
        Text += anItem->Text;
    }

    // update active image
    const StImagePlane& anImage = getFirst()->Image;
    if(!anImage.isNull()) {
        Image.initCopy(anImage, false);
        Scale = getFirst()->Scale;
    } else {
        Image.nullify();
        Scale = 1.0f;
    }

    return isChanged;
}

void StGLSubtitles::StSubShowItems::add(const StHandle<StSubItem>& theItem) {
    if(!Text.isEmpty()) {
        Text += StString('\n');
    }
    Text += theItem->Text;

    const StImagePlane& anImage = theItem->Image;
    if(!anImage.isNull()) {
        Image.initCopy(anImage, false);
        Scale = theItem->Scale;
    }

    StArrayList<StHandle <StSubItem> >::add(theItem);
}

StGLSubtitles::StGLSubtitles(StGLImageRegion* theParent,
                             const StHandle<StSubQueue>&     theSubQueue,
                             const StHandle<StInt32Param>&   thePlace,
                             const StHandle<StFloat32Param>& theFontSize)
: StGLTextArea(theParent,
               0, 0,
               StGLCorner(parseCorner(thePlace->getValue()), ST_HCORNER_CENTER),
               theParent->getRoot()->scale(800), theParent->getRoot()->scale(160)),
  myQueue(theSubQueue),
  myPTS(0.0),
  myImgProgram(getRoot()->getShare(SHARE_IMAGE_PROGRAM_ID)) {
    params.Place    = thePlace;
    params.FontSize = theFontSize;
    params.TopDY    = new StFloat32Param(100.0f);
    params.BottomDY = new StFloat32Param(100.0f);
    params.Parallax = new StFloat32Param(0.0f);
    params.Parser   = new StEnumParam(1, stCString("subsParser"));
    params.ToApplyStereo = new StBoolParamNamed(true, stCString("subsApplyStereo"));

    if(myQueue.isNull()) {
        myQueue = new StSubQueue();
    }

    myToDrawShadow = true;
    setTextColor(StGLVec3(1.0f, 1.0f, 1.0f));
    setBorder(false);

    myFormatter.setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                               StGLTextFormatter::ST_ALIGN_Y_BOTTOM);

    StHandle<StGLFont> aFontNew = new StGLFont();
    StHandle<StFTLibrary> aLib = getRoot()->getFontManager()->getLibraty();
    const FontSize     aSize       = (FontSize )(int )params.FontSize->getValue();
    const unsigned int aResolution = getRoot()->getFontManager()->getResolution();
    for(size_t anIter = 0; anIter < StFTFont::SubsetsNB; ++anIter) {
        StHandle<StGLFontEntry>& aFontGlSrc = myFont->changeFont((StFTFont::Subset )anIter);
        if(aFontGlSrc.isNull()) {
            continue;
        }

        StHandle<StFTFont> aFontFt = new StFTFont(aLib);
        for(int aStyleIt = 0; aStyleIt < StFTFont::StylesNB; ++aStyleIt) {
            aFontFt->load(aFontGlSrc->getFont()->getFilePath((StFTFont::Style )aStyleIt),
                          aFontGlSrc->getFont()->getFaceIndex((StFTFont::Style )aStyleIt),
                          (StFTFont::Style )aStyleIt);
        }
        aFontFt->init(aSize, aResolution);
        aFontNew->changeFont((StFTFont::Subset )anIter) = new StGLFontEntry(aFontFt);
    }
    mySize = aSize;
    myFont = aFontNew;
}

StGLSubtitles::~StGLSubtitles() {
    StGLContext& aCtx = getContext();
    myFont->release(aCtx);
    myFont.nullify();
    myTexture.release(aCtx);
    myVertBuf.release(aCtx);
    myTCrdBuf.release(aCtx);
}

bool StGLSubtitles::stglInit() {
    if(!myVertBuf.isValid()) {
        StArray<StGLVec2> aDummyVert(4);

        StGLContext& aCtx = getContext();
        myVertBuf.init(aCtx, aDummyVert);
        myTCrdBuf.init(aCtx, aDummyVert);

        if(myImgProgram.isNull()) {
            myImgProgram.create(getRoot()->getContextHandle(), new StImgProgram());
            myImgProgram->init(aCtx);
        }
    }
    return StGLTextArea::stglInit();
}

void StGLSubtitles::stglUpdate(const StPointD_t& ,
                               bool ) {
    bool isChanged = myShowItems.pop(myPTS);
    for(StHandle<StSubItem> aNewSubItem = myQueue->pop(myPTS); !aNewSubItem.isNull(); aNewSubItem = myQueue->pop(myPTS)) {
        isChanged = true;
        myShowItems.add(aNewSubItem);
    }

    const StGLVCorner aCorner = parseCorner(params.Place->getValue());
    bool toResize = myCorner.v != aCorner;
    myCorner.v = aCorner;
    switch(myCorner.v) {
        case ST_VCORNER_TOP: {
            const int aDisp = myRoot->scale((int )params.TopDY->getValue()) + myRoot->getRootMargins().top;
            if(getRectPx().top() != aDisp) {
                toResize = true;
                changeRectPx().moveTopTo(aDisp);
            }
            myFormatter.setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                       StGLTextFormatter::ST_ALIGN_Y_TOP);
            break;
        }
        case ST_VCORNER_CENTER: {
            if(getRectPx().top() != 0) {
                toResize = true;
                changeRectPx().moveTopTo(0);
            }
            myFormatter.setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                       StGLTextFormatter::ST_ALIGN_Y_CENTER);
            break;
        }
        case ST_VCORNER_BOTTOM: {
            const int aDisp = -myRoot->scale((int )params.BottomDY->getValue()) - myRoot->getRootMargins().bottom;
            if(getRectPx().top() != aDisp) {
                toResize = true;
                changeRectPx().moveTopTo(aDisp);
            }
            myFormatter.setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                       StGLTextFormatter::ST_ALIGN_Y_BOTTOM);
            break;
        }
    }
    if(toResize) {
        stglResize();
    }

    StGLContext& aCtx = getContext();
    if(isChanged) {
        setText(myShowItems.Text);

        if(!myShowItems.Image.isNull()) {
            myTexture.init(aCtx, myShowItems.Image);
        } else {
            myTexture.release(aCtx);
        }

        StString aLog;
        /**for(size_t anId = 0; anId < myShowItems.size(); ++anId) {
            aLog += ST_STRING(" from ") + myShowItems[anId]->myTimeStart + " to " + myShowItems[anId]->myTimeEnd + "\n";
        }
        ST_DEBUG_LOG("(" + myPTS + ") myShowItems.myText= '" + myShowItems.myText + "'\n" + aLog);*/
    }

    const FontSize aNewSize = (FontSize )(int )params.FontSize->getValue();
    if(!myText.isEmpty()
    && aNewSize != mySize) {
        mySize = aNewSize;
        myToRecompute = true;

        myFont->stglInit(aCtx, getFontSize(), myRoot->getResolution());
    }
}

void StGLSubtitles::stglDraw(unsigned int theView) {
    if(!myIsInitialized || !isVisible()) {
        return;
    }

    StGLContext& aCtx = getContext();
    if(myFormatter.getParser() != (StGLTextFormatter::Parser )params.Parser->getValue()) {
        myFormatter.setupParser((StGLTextFormatter::Parser )params.Parser->getValue());
        myToRecompute = true;
    }
    if(!myText.isEmpty()) {
        formatText(aCtx);

        switch(theView) {
            case ST_DRAW_LEFT:
                myTextDX = -params.Parallax->getValue() * GLfloat(0.5 * 0.001 * myRoot->getRootRectGl().width());
                break;
            case ST_DRAW_RIGHT:
                myTextDX =  params.Parallax->getValue() * GLfloat(0.5 * 0.001 * myRoot->getRootRectGl().width());
                break;
            case ST_DRAW_MONO:
            default:
                myTextDX = 0.0f;
                break;
        }
        StGLTextArea::stglDraw(theView);
    }

    if(!myTexture.isValid()
    || !myImgProgram->isValid()) {
        return;
    }

    StHandle<StStereoParams> aParams;
    StFormat aStFormat = StFormat_Mono;
    unsigned int aView = theView;
    float aSampleRatio = 1.0f;
    StVec2<int> aFrameDims(0, 0);
    if(StGLImageRegion* anImgRegion = !params.ToApplyStereo.isNull() && params.ToApplyStereo->getValue()
                                    ? dynamic_cast<StGLImageRegion*>(myParent)
                                    : NULL) {
        aParams = anImgRegion->getSource();
        if(!aParams.isNull()) {
            aStFormat = aParams->StereoFormat;
            aSampleRatio = anImgRegion->getSampleRatio();
            aFrameDims   = anImgRegion->getFrameSize();
            if(aParams->ToSwapLR) {
                // apply swap flag
                switch(aStFormat) {
                    case StFormat_SideBySide_LR: aStFormat = StFormat_SideBySide_RL; break;
                    case StFormat_SideBySide_RL: aStFormat = StFormat_SideBySide_LR; break;
                    case StFormat_TopBottom_LR:  aStFormat = StFormat_TopBottom_RL;  break;
                    case StFormat_TopBottom_RL:  aStFormat = StFormat_TopBottom_LR;  break;
                    default: break;
                }
            }
            // swap views
            if(aStFormat == StFormat_SideBySide_RL) {
                aStFormat = StFormat_SideBySide_LR;
                if(theView == ST_DRAW_RIGHT) {
                    aView = ST_DRAW_LEFT;
                } else if(theView == ST_DRAW_LEFT) {
                    aView = ST_DRAW_RIGHT;
                }
            } else if(aStFormat == StFormat_TopBottom_RL) {
                aStFormat = StFormat_TopBottom_LR;
                if(theView == ST_DRAW_RIGHT) {
                    aView = ST_DRAW_LEFT;
                } else if(theView == ST_DRAW_LEFT) {
                    aView = ST_DRAW_RIGHT;
                }
            }
            // reset
            if(aStFormat != StFormat_SideBySide_LR
            && aStFormat != StFormat_TopBottom_LR) {
                aSampleRatio = 1.0f;
            }
        }
    }

    // update vertices
    StVec2<int> anImgSize (myTexture.getSizeX(), myTexture.getSizeY()), anOffset (0, 0);
    StArray<StGLVec2> aVertices(4), aTexCoords(4);
    aTexCoords[0] = StGLVec2(1.0f, 0.0f);
    aTexCoords[1] = StGLVec2(1.0f, 1.0f);
    aTexCoords[2] = StGLVec2(0.0f, 0.0f);
    aTexCoords[3] = StGLVec2(0.0f, 1.0f);
    if(aSampleRatio >= 1.0f) {
        anImgSize.x() = int(double(anImgSize.x()) * aSampleRatio);
    } else {
        anImgSize.y() = int(double(anImgSize.y()) / aSampleRatio);
    }
    const double aFontScale = double(getRoot()->getScale()) * myShowItems.Scale * params.FontSize->getValue() / params.FontSize->getDefValue();
    anImgSize.x() = int(double(anImgSize.x()) * aFontScale);
    anImgSize.y() = int(double(anImgSize.y()) * aFontScale);

    switch(aStFormat) {
        case StFormat_SideBySide_LR: {
            anImgSize.x() /= 2;
            const int anOffsetX = int(aFontScale * ((aFrameDims.x() * 2 - myTexture.getSizeX()) / 2));
            if(aView == ST_DRAW_LEFT) {
                aTexCoords[0].x() = aTexCoords[1].x() = 0.5f;
                aTexCoords[2].x() = aTexCoords[3].x() = 0.0f;
                anOffset.x() = anOffsetX;
            } else if(aView == ST_DRAW_RIGHT) {
                aTexCoords[0].x() = aTexCoords[1].x() = 1.0f;
                aTexCoords[2].x() = aTexCoords[3].x() = 0.5f;
                anOffset.x() = -anOffsetX;
            }
            break;
        }
        case StFormat_TopBottom_LR: {
            anImgSize.y() /= 2;
            const int anOffsetY = int(aFontScale * ((aFrameDims.y() * 2 - myTexture.getSizeY()) / 2));
            if(aView == ST_DRAW_LEFT) {
                aTexCoords[0].y() = aTexCoords[2].y() = 0.0f;
                aTexCoords[1].y() = aTexCoords[3].y() = 0.5f;
                anOffset.y() = -anOffsetY;
            } else if(aView == ST_DRAW_RIGHT) {
                aTexCoords[0].y() = aTexCoords[2].y() = 0.5f;
                aTexCoords[1].y() = aTexCoords[3].y() = 1.0f;
                anOffset.y() = anOffsetY;
            }
            break;
        }
        default: {
            break;
        }
    }
    if(aView == ST_DRAW_LEFT) {
        anOffset.x() -= (int )params.Parallax->getValue();
    } else if(aView == ST_DRAW_RIGHT) {
        anOffset.x() += (int )params.Parallax->getValue();
    }

    StRectI_t aRect = getRectPxAbsolute();
    aRect.bottom() += anOffset.y();
    aRect.top()   = aRect.bottom() - anImgSize.y();
    aRect.left()  = aRect.left() + aRect.width() / 2 - anImgSize.x() / 2 + anOffset.x();
    aRect.right() = aRect.left() + anImgSize.x();
    myRoot->getRectGl(aRect, aVertices);
    myVertBuf.init(aCtx, aVertices);
    myTCrdBuf.init(aCtx, aTexCoords);

    aCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    aCtx.core20fwd->glEnable(GL_BLEND);
    myTexture.bind(aCtx);
    myImgProgram->use(aCtx);

    myVertBuf.bindVertexAttrib(aCtx, myImgProgram->getVVertexLoc());
    myTCrdBuf.bindVertexAttrib(aCtx, myImgProgram->getVTexCoordLoc());

    aCtx.core20fwd->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    myTCrdBuf.unBindVertexAttrib(aCtx, myImgProgram->getVTexCoordLoc());
    myVertBuf.unBindVertexAttrib(aCtx, myImgProgram->getVVertexLoc());

    myImgProgram->unuse(aCtx);
    myTexture.unbind(aCtx);
    aCtx.core20fwd->glDisable(GL_BLEND);
}

void StGLSubtitles::stglResize() {
    changeRectPx().right() = (getParent()->getRectPx().width() / 5) * 3;
    myTextWidth = (GLfloat )getRectPx().width();
    myToRecompute = true;
    StGLTextArea::stglResize();

    // update projection matrix
    if(!myImgProgram.isNull()) {
        StGLContext& aCtx = getContext();
        myImgProgram->use(aCtx);
        myImgProgram->setProjMat(aCtx, getRoot()->getScreenProjection());
        myImgProgram->unuse(aCtx);
    }
}

const StHandle<StSubQueue>& StGLSubtitles::getQueue() const {
    return myQueue;
}

void StGLSubtitles::setPTS(const double thePTS) {
    myPTS = thePTS;
}
