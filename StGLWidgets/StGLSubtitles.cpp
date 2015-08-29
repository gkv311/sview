/**
 * Copyright © 2010-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLSubtitles.h>

#include <StGLCore/StGLCore20.h>
#include <StGL/StGLProgram.h>
#include <StGLWidgets/StGLRootWidget.h>

namespace {
    static const size_t SHARE_IMAGE_PROGRAM_ID = StGLRootWidget::generateShareId();
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
: StArrayList<StHandle <StSubItem> >(8) {
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
        Image.initCopy(anImage);
    } else {
        Image.nullify();
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
        Image.initCopy(anImage);
    }

    StArrayList<StHandle <StSubItem> >::add(theItem);
}

inline StGLVCorner parseCorner(int theVal) {
    return (StGLVCorner )theVal;
}

StGLSubtitles::StGLSubtitles(StGLWidget*                     theParent,
                             const StHandle<StSubQueue>&     theSubQueue,
                             const StHandle<StInt32Param>&   thePlace,
                             const StHandle<StFloat32Param>& theTopDY,
                             const StHandle<StFloat32Param>& theBottomDY,
                             const StHandle<StFloat32Param>& theFontSize,
                             const StHandle<StFloat32Param>& theParallax,
                             const StHandle<StEnumParam>&    theParser)
: StGLTextArea(theParent,
               0, 0,
               StGLCorner(parseCorner(thePlace->getValue()), ST_HCORNER_CENTER),
               theParent->getRoot()->scale(800), theParent->getRoot()->scale(160)),
  myPlace(thePlace),
  myTopDY(theTopDY),
  myBottomDY(theBottomDY),
  myFontSize(theFontSize),
  myParallax(theParallax),
  myParser(theParser),
  myQueue(theSubQueue),
  myPTS(0.0),
  myImgProgram(getRoot()->getShare(SHARE_IMAGE_PROGRAM_ID)) {
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
    const FontSize     aSize       = (FontSize )(int )myFontSize->getValue();
    const unsigned int aResolution = getRoot()->getFontManager()->getResolution();
    for(size_t anIter = 0; anIter < StFTFont::SubsetsNB; ++anIter) {
        StHandle<StGLFontEntry>& aFontGlSrc = myFont->changeFont((StFTFont::Subset )anIter);
        if(aFontGlSrc.isNull()) {
            continue;
        }

        StHandle<StFTFont> aFontFt = new StFTFont(aLib);
        for(int aStyleIt = 0; aStyleIt < StFTFont::StylesNB; ++aStyleIt) {
            aFontFt->load(aFontGlSrc->getFont()->getFilePath((StFTFont::Style )aStyleIt), (StFTFont::Style )aStyleIt);
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
        StArray<StGLVec2> aTexCoords(4);
        aTexCoords[0] = StGLVec2(1.0f, 0.0f);
        aTexCoords[1] = StGLVec2(1.0f, 1.0f);
        aTexCoords[2] = StGLVec2(0.0f, 0.0f);
        aTexCoords[3] = StGLVec2(0.0f, 1.0f);

        StGLContext& aCtx = getContext();
        myVertBuf.init(aCtx, aDummyVert);
        myTCrdBuf.init(aCtx, aTexCoords);

        if(myImgProgram.isNull()) {
            myImgProgram.create(getRoot()->getContextHandle(), new StImgProgram());
            myImgProgram->init(aCtx);
        }
    }
    return StGLTextArea::stglInit();
}

void StGLSubtitles::stglUpdate(const StPointD_t& ) {
    bool isChanged = myShowItems.pop(myPTS);
    for(StHandle<StSubItem> aNewSubItem = myQueue->pop(myPTS); !aNewSubItem.isNull(); aNewSubItem = myQueue->pop(myPTS)) {
        isChanged = true;
        myShowItems.add(aNewSubItem);
    }

    const StGLVCorner aCorner = parseCorner(myPlace->getValue());
    bool toResize = myCorner.v != aCorner;
    myCorner.v = aCorner;
    switch(myCorner.v) {
        case ST_VCORNER_TOP: {
            const int aDisp = myRoot->scale((int )myTopDY->getValue());
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
            const int aDisp = -myRoot->scale((int )myBottomDY->getValue());
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

    const FontSize aNewSize = (FontSize )(int )myFontSize->getValue();
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
    if(myFormatter.getParser() != (StGLTextFormatter::Parser )myParser->getValue()) {
        myFormatter.setupParser((StGLTextFormatter::Parser )myParser->getValue());
        myToRecompute = true;
    }
    if(!myText.isEmpty()) {
        formatText(aCtx);

        switch(theView) {
            case ST_DRAW_LEFT:
                myTextDX = -myParallax->getValue() * GLfloat(0.5 * 0.001 * myRoot->getRootRectGl().width());
                break;
            case ST_DRAW_RIGHT:
                myTextDX =  myParallax->getValue() * GLfloat(0.5 * 0.001 * myRoot->getRootRectGl().width());
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

    aCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    aCtx.core20fwd->glEnable(GL_BLEND);
    myTexture.bind(aCtx);

    // update vertices
    StRectI_t aRect = getRectPxAbsolute();
    aRect.top()   = aRect.bottom() - myTexture.getSizeY();
    aRect.left()  = aRect.left() + aRect.width() / 2 - myTexture.getSizeX() / 2;
    aRect.right() = aRect.left() + myTexture.getSizeX();

    StArray<StGLVec2> aVertices(4);
    myRoot->getRectGl(aRect, aVertices);
    myVertBuf.init(aCtx, aVertices);

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
