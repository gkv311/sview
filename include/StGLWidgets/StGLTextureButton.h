/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLTextureButton_h_
#define __StGLTextureButton_h_

#include <StGLWidgets/StGLRootWidget.h>

#include <StGL/StGLVertexBuffer.h>
#include <StGL/StGLTexture.h>

class StAction;

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
     * Return action to be triggered on button click.
     */
    ST_LOCAL const StHandle<StAction>& getAction() const { return myAction; }

    /**
     * Set action to be triggered on button click.
     */
    ST_CPPEXPORT void setAction(const StHandle<StAction>& theAction);

    /**
     * Return scale factor to be applied to the widget opacity, 1.0 by default.
     */
    ST_LOCAL float getOpacityScale() const { return myOpacityScale; }

    /**
     * Setup scale factor to be applied to the widget opacity.
     */
    ST_LOCAL void setOpacityScale(const float theValue) { myOpacityScale = theValue; }

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

    ST_CPPEXPORT virtual void stglResize() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool stglInit() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglDraw  (unsigned int theView) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& theCursorZo,
                                         bool theIsPreciseInput) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool tryClick  (const StClickEvent& theEvent, bool& theIsItemClicked)   ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool tryUnClick(const StClickEvent& theEvent, bool& theIsItemUnclicked) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool doScroll(const StScrollEvent& theEvent) ST_ATTR_OVERRIDE;

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

    StHandle<StAction>         myAction;       //!< action on button click
    StGLVertexBuffer           myVertBuf;      //!< vertices VBO
    StGLVertexBuffer           myTCrdBuf;      //!< texture coordinates VBO
    StGLVec4                   myColor;        //!< button color for alpha-textures
    StGLVec4                   myShadowColor;  //!< shadow color for alpha-textures
    StHandle<StGLTextureArray> myTextures;     //!< list of textures (button faces)
    size_t                     myFaceId;       //!< active button face
    float                      myOpacityScale; //!< scale factor to be applied to the widget opacity

    StGLShare<ButtonPrograms>  myProgram;      //!< button program
    ProgramIndex               myProgramIndex; //!< active program index

    StTimer                    myHoldTimer;    //!< timer to handle button hold event
    double                     myHoldDuration; //!< button holding time saved within the last event
    StTimer                    myWaveTimer;    //!< animation timer
    float                      myAnimTime;     //!< animation time
    Animation                  myAnim;         //!< active animation mode
    bool                       myToDrawShadow; //!< flag to display shadows for alpha-textures

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

    ST_CPPEXPORT virtual ~StGLIcon();

    ST_CPPEXPORT virtual bool tryClick  (const StClickEvent& theEvent, bool& theIsItemClicked)   ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool tryUnClick(const StClickEvent& theEvent, bool& theIsItemUnclicked) ST_ATTR_OVERRIDE;

    /**
     * Define externally managed textures.
     */
    ST_LOCAL void setExternalTextures(const StHandle<StGLTextureArray>& theTextures) {
        myTextures          = theTextures;
        myIsExternalTexture = true;
    }

        protected:

    bool myIsExternalTexture; //!< flag indicating that assigned texture should not be released

};

#endif // __StGLTextureButton_h_
