/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLTextureButton_h_
#define __StGLTextureButton_h_

#include <StGLWidgets/StGLRootWidget.h>

#include <StGL/StGLVertexBuffer.h>
#include <StGL/StGLTexture.h>

/**
 * Widget of the clickable button with image face.
 */
class StGLTextureButton : public StGLWidget {

        public:

    enum Animation {
        Anim_None, //!< no animation
        Anim_Wave  //!< wave animation
    };

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StGLTextureButton(StGLWidget*      theParent,
                                   const int        theLeft = 32,
                                   const int        theTop = 32,
                                   const StGLCorner theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT),
                                   const size_t     theFacesCount = 1);

    /**
     * Destructor.
     */
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

    /**
     * Setup color of the shadow for the alpha texture.
     */
    ST_LOCAL void setShadowColor(const StGLVec4& theColor) {
        myShadowColor = theColor;
    }

    /**
     * color of the shadow for the alpha texture.
     */
    ST_LOCAL const StGLVec4& getShadowColor() const {
        return myShadowColor;
    }

    /**
     * Return value of text shadow rendering flag (false by default).
     */
    ST_LOCAL bool toDrawShadow() {
        return myToDrawShadow;
    }

    /**
     * Assign value to text shadow rendering flag.
     */
    ST_LOCAL void setDrawShadow(const bool theToDraw) {
        myToDrawShadow = theToDraw;
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

    ST_CPPEXPORT virtual void stglResize();
    ST_CPPEXPORT virtual bool stglInit();
    ST_CPPEXPORT virtual void stglDraw  (unsigned int view);
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& theCursorZo);
    ST_CPPEXPORT virtual bool tryClick  (const StPointD_t& theCursorZo, const int& theMouseBtn, bool& theIsItemClicked);
    ST_CPPEXPORT virtual bool tryUnClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& theIsItemUnclicked);

        public:    //! @name Signals

    struct {
        /**
         * Emit callback Slot on button click.
         * @param theUserData (size_t ) user predefined data
         */
        StSignal<void (const size_t )> onBtnClick;

        /**
         * Emit callback Slot on holding button.
         * @param theUserData (size_t ) user predefined data
         * @param theProgress (double ) holding duration from last update in seconds
         */
        StSignal<void (const size_t , const double )> onBtnHold;
    } signals;

        private:   //! @name callback Slots (private overriders)

    ST_LOCAL void doMouseUnclick(const int theBtnId);

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
    StGLVec4                   myShadowColor;
    StHandle<StGLTextureArray> myTextures;
    size_t                     myFaceId;

    StGLShare<ButtonPrograms>  myProgram;
    ProgramIndex               myProgramIndex;

    StTimer                    myHoldTimer;
    double                     myHoldDuration;
    StTimer                    myWaveTimer;
    float                      myAnimTime;
    Animation                  myAnim;
    bool                       myToDrawShadow;

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
