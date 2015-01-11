/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLTextureButton_h_
#define __StGLTextureButton_h_

#include <StGLWidgets/StGLShare.h>
#include <StGLWidgets/StGLWidget.h>

#include <StGL/StGLVertexBuffer.h>
#include <StGL/StGLTexture.h>

#include <StTemplates/StArray.h>

template<> inline void StArray<StGLTexture>::sort() {}

class StGLTextureButton : public StGLWidget {

        public:

    enum Animation {
        Anim_None, //!< no animation
        Anim_Wave  //!< wave animation
    };

        public:

    ST_CPPEXPORT StGLTextureButton(StGLWidget*      theParent,
                                   const int        theLeft = 32,
                                   const int        theTop = 32,
                                   const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                   const size_t     theFacesCount = 1);

    ST_CPPEXPORT virtual ~StGLTextureButton();

    /**
     * Setup color for alpha texture.
     */
    ST_LOCAL void setColor(const StGLVec4& theColor) {
        myColor = theColor;
    }

    /**
     * Get color for alpha texture.
     */
    ST_LOCAL const StGLVec4& getColor() const {
        return myColor;
    }

    ST_LOCAL size_t getFaceId() const {
        return myFaceId;
    }

    ST_CPPEXPORT void setFaceId(const size_t theId);

    ST_LOCAL inline void setTexturePath(const StString& theTexturesPath) {
        setTexturePath(&theTexturesPath, 1);
    }

    ST_CPPEXPORT void setTexturePath(const StString* theTexturesPaths,
                                     const size_t    theCount);

    /**
     * Return extra margins before the texture.
     */
    ST_LOCAL const StMarginsI& getMargins() const {
        return myMargins;
    }

    /**
     * Return extra margins before the texture.
     */
    ST_LOCAL StMarginsI& changeMargins() {
        return myMargins;
    }

    ST_CPPEXPORT virtual void stglResize();
    ST_CPPEXPORT virtual bool stglInit();
    ST_CPPEXPORT virtual void stglDraw(unsigned int view);

    ST_CPPEXPORT virtual bool tryClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemClicked);
    ST_CPPEXPORT virtual bool tryUnClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemUnclicked);

        public:    //! @name Signals

    struct {
        /**
         * Emit callback Slot on button click.
         * @param theUserData (const size_t ) - user predefined data.
         */
        StSignal<void (const size_t )> onBtnClick;
    } signals;

        private:   //! @name callback Slots (private overriders)

    ST_LOCAL void doMouseUnclick(const int theBtnId);

        private:

    ST_LOCAL void glWaveTimerControl();

        protected:

    enum ProgramIndex {
        ProgramIndex_WaveRGB,
        ProgramIndex_WaveAlpha,
        ProgramIndex_NB,
    };

    class Program;
    class ButtonPrograms;

        protected:

    StGLVertexBuffer           myVertBuf;
    StGLVertexBuffer           myTCrdBuf;
    StGLVec4                   myColor;
    size_t                     myFaceId;
    size_t                     myFacesCount;
    Animation                  myAnim;
    StArray<StGLTexture>       myTextures;
    StArray<StString>          myTexturesPaths;

    StGLShare<ButtonPrograms>  myProgram;
    ProgramIndex               myProgramIndex;

    StTimer                    myWaveTimer;
    StMarginsI                 myMargins;

};

/**
 * Passive icon.
 */
class StGLIcon : public StGLTextureButton {

        public:

    ST_CPPEXPORT StGLIcon(StGLWidget*      theParent,
                          const int        theLeft,
                          const int        theTop,
                          const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                          const size_t     theFacesCount = 1);

    ST_CPPEXPORT virtual bool tryClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemClicked);
    ST_CPPEXPORT virtual bool tryUnClick(const StPointD_t& cursorZo, const int& mouseBtn, bool& isItemUnclicked);

};

#endif // __StGLTextureButton_h_
