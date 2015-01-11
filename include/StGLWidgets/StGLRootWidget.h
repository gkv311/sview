/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLRootWidget_h_
#define __StGLRootWidget_h_

#include <StGLWidgets/StGLShare.h>
#include <StGLWidgets/StGLWidget.h>
#include <StGL/StGLFontManager.h>
#include <StThreads/StResourceManager.h>

#include <StTemplates/StHandle.h>

/**
 * Full OpenGL-window widget, must be ROOT for other widgets.
 */
class StGLRootWidget : public StGLWidget {

        public:

    enum ScaleAdjust {
        ScaleAdjust_Small,
        ScaleAdjust_Normal,
        ScaleAdjust_Big,
    };

    /**
     * List of standard icon sizes.
     */
    enum IconSize {
        IconSize_16,
        IconSize_24,
        IconSize_32,
        IconSize_48,
        IconSize_64,
        IconSize_72,
        IconSize_96,
        IconSize_128,
        IconSize_144,
        IconSize_192,
        IconSize_256,
        IconSizeNb
    };

    /**
     * Colors of standard elements.
     */
    enum Color {
        Color_Menu,
        Color_MenuHighlighted,
        Color_MenuClicked,
        Color_MenuText,
        Color_MenuIcon,
        Color_MessageBox,
        Color_MessageText,
        Color_IconActive,
        Color_NB
    };

    /**
     * Images of standard elements.
     */
    enum IconImage {
        IconImage_CheckboxOff,
        IconImage_CheckboxOn,
        IconImage_RadioButtonOff,
        IconImage_RadioButtonOn,
        IconImage_NB
    };

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StGLRootWidget(const StHandle<StResourceManager>& theResMgr);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StGLRootWidget();

    /**
     * Process initialization.
     * @return true on success
     */
    ST_CPPEXPORT virtual bool stglInit();

    /**
     * Draw all children.
     * Root widget caches OpenGL state (like viewport).
     */
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView);

    /**
     * Return color of standard element.
     */
    ST_LOCAL const StGLVec4& getColorForElement(const StGLRootWidget::Color theElement) const { return myColors[theElement]; }

    /**
     * Return icon of standard element.
     */
    ST_LOCAL const StString& getIcon(const StGLRootWidget::IconImage theIcon) const { return myIcons[theIcon]; }

    /**
     * Function iterate children and self to change clicking state.
     * @param theCursorZo point in Zero2One coordinates
     * @param theMouseBtn mouse button id
     */
    inline bool tryClick(const StPointD_t& theCursorZo,
                         const int&        theMouseBtn) {
        bool isItemClicked = false;
        return tryClick(theCursorZo, theMouseBtn, isItemClicked);
    }

    ST_CPPEXPORT virtual bool tryClick(const StPointD_t& theCursorZo,
                                       const int&        theMouseBtn,
                                       bool&             theIsItemClicked);

    /**
     * Function iterate children and self for unclicking state.
     * @param theCursorZo point in Zero2One coordinates
     * @param theMouseBtn mouse button id
     */
    inline bool tryUnClick(const StPointD_t& theCursorZo,
                           const int&        theMouseBtn) {
        bool isItemUnclicked = false;
        return tryUnClick(theCursorZo, theMouseBtn, isItemUnclicked);
    }

    ST_CPPEXPORT virtual bool tryUnClick(const StPointD_t& theCursorZo,
                                         const int&        theMouseBtn,
                                         bool&             theIsItemUnclicked);

    /**
     * Process key down event. Default implementation redirect event to widget in focus.
     * @param theEvent key event
     * @return true if event has been processed
     */
    ST_CPPEXPORT virtual bool doKeyDown(const StKeyEvent& theEvent);

    /**
     * Process key hold event. Default implementation redirect event to widget in focus.
     * @param theEvent key event
     * @return true if event has been processed
     */
    ST_CPPEXPORT virtual bool doKeyHold(const StKeyEvent& theEvent);

    /**
     * Process key up event. Default implementation redirect event to widget in focus.
     * @param theEvent key event
     * @return true if event has been processed
     */
    ST_CPPEXPORT virtual bool doKeyUp  (const StKeyEvent& theEvent);

    /**
     * Return true if interface should be adopted to mobile device.
     */
    ST_LOCAL bool isMobile() const {
        return myIsMobile;
    }

    /**
     * Setup flag to prefer interface for mobile device.
     */
    ST_LOCAL void setMobile(const bool theValue) {
        myIsMobile = theValue;
    }

    /**
     * @return scale factor for GUI elements (text, icons), 1.0 for normal displays
     */
    ST_LOCAL inline GLfloat getScale() const {
        return myScaleGUI;
    }

    /**
     * @return scaled units
     */
    ST_LOCAL inline int scale(const int thePixels) const {
        return int(myScaleGUI * GLfloat(thePixels) + 0.1f);
    }

    /**
     * Scale specified size to the nearest default icon size.
     */
    ST_CPPEXPORT IconSize scaleIcon(const int theSize) const;

    /**
     * Returns texture for icon with specified default size.
     */
    ST_CPPEXPORT StString iconTexture(const StString& theName,
                                      const IconSize  theSize) const;

    /**
     * @param theScale scale factor for GUI elements (text, icons)
     */
    ST_CPPEXPORT void setScale(const GLfloat     theScale,
                               const ScaleAdjust theScaleAdjust = ScaleAdjust_Normal);

    /**
     * @return resolution DPI, 72 for normal displays
     */
    ST_LOCAL inline unsigned int getResolution() const {
        return myResolution;
    }

    inline GLdouble getRootScaleX() const {
        return myScaleGlX;
    }

    inline GLdouble getRootScaleY() const {
        return myScaleGlY;
    }

    inline StRectD_t getRootRectGl() const {
        return myRectGl;
    }

    ST_LOCAL const StMarginsI& getRootMargins() const {
        return myMarginsPx;
    }

    ST_LOCAL StMarginsI& changeRootMargins() {
        return myMarginsPx;
    }

    /**
     * Convert pixel coordinates (absolute) into GL coordinates.
     */
    ST_CPPEXPORT StRectD_t getRectGl(const StRectI_t& theRectPx) const;

    /**
     * Convert pixel coordinates (absolute) into GL coordinates.
     * Array should be 4 items length or you will got an exception!
     */
    ST_CPPEXPORT void getRectGl(const StRectI_t&   theRectPx,
                                StArray<StGLVec2>& theVertices,
                                const size_t       theFromId = 0) const;

    /**
     * Access the shared resource by unique id.
     */
    ST_CPPEXPORT StGLSharePointer* getShare(const size_t theResId);

    /**
     * Generate next unique id for shared resource.
     */
    ST_CPPEXPORT static size_t generateShareId();

    inline StGLProjCamera* getCamera() {
        return &myProjCamera;
    }

    /**
     * @return widget in focus
     */
    inline StGLWidget* getFocus() {
        return myFocusWidget;
    }

    /**
     * @param theWidget new widget to set focus
     * @return previous widget focus
     */
    ST_CPPEXPORT StGLWidget* setFocus(StGLWidget* theWidget);

    /**
     * @return OpenGL context.
     */
    ST_CPPEXPORT StGLContext& getContext();

    /**
     * @return OpenGL context.
     */
    ST_CPPEXPORT const StHandle<StGLContext>& getContextHandle() const;

    /**
     * Setup OpenGL context.
     */
    ST_CPPEXPORT void setContext(const StHandle<StGLContext>& theCtx);

    /**
     * Access resource manager.
     */
    ST_LOCAL const StHandle<StResourceManager>& getResourceManager() const {
        return myResMgr;
    }

    /**
     * @return reference to shared font manager
     */
    ST_LOCAL StHandle<StGLFontManager>& getFontManager() {
        return myGlFontMgr;
    }

    /**
     * Returns camera projection matrix within to-screen displacement
     * thus it can be used for vertices given in only 2D-coordinates.
     */
    inline const StGLMatrix& getScreenProjection() const {
        return myScrProjMat;
    }

    inline GLfloat getScreenDispX() const {
        return myScrDispX;
    }

    inline GLfloat getLensDist() const {
        return myLensDist;
    }

    inline void setLensDist(const GLfloat theLensDist) {
        myLensDist = theLensDist;
    }

    /**
     * Returns mouse cursor position in GL coordinates.
     */
    inline StPointD_t getCursorZo() {
        return cursorZo;
    }

    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& cursorZo);
    ST_CPPEXPORT virtual void stglResize(const StGLBoxPx&  theRectPx);

    /**
     * @return viewport dimensions from bound GL context (4-indices array)
     */
    inline const GLint* getViewport() const {
        return myViewport;
    }

    /**
     * Computes scissor rectangle in OpenGL viewport.
     * @param theRect        Rectangle in window coordinates
     * @param theScissorRect Scissor rectangle for glScissor() call
     */
    ST_CPPEXPORT void stglScissorRect(const StRectI_t& theRect,
                                      StGLBoxPx&       theScissorRect) const;

    /**
     * Append widget to destroy list.
     * This method should be used to destroy widget within callback processing
     * to prevent corruption during widgets iteration.
     * @param theWidget the widget to destroy
     */
    ST_CPPEXPORT virtual void destroyWithDelay(StGLWidget* theWidget);

    /**
     * Access global flag to perform navigation in menu after first item clicked.
     */
    ST_LOCAL bool isMenuPressed() const {
        return myIsMenuPressed;
    }

    /**
     * Setup global flag to perform navigation in menu after first item clicked.
     */
    ST_LOCAL void setMenuPressed(const bool theIsPressed) {
        myIsMenuPressed = theIsPressed;
    }

        private:

    /**
     * Perform pending destroy requests.
     */
    ST_LOCAL void clearDestroyList();

    ST_LOCAL void setupTextures();

        private:

    StGLSharePointer**        myShareArray;    //!< resources shared within GL context (commonly used)
    size_t                    myShareSize;
    StHandle<StResourceManager> myResMgr;      //!< resources manager
    StGLProjCamera            myProjCamera;    //!< projection camera
    StGLMatrix                myScrProjMat;    //!< projection matrix within translation to the screen
    StHandle<StGLFontManager> myGlFontMgr;     //!< shared font manager
    StHandle<StGLContext>     myGlCtx;         //!< OpenGL context
    GLfloat                   myScrDispX;
    GLfloat                   myLensDist;
    int                       myScrDispXPx;

    StGLVec4                  myColors[Color_NB]; //!< colors of standard elements
    StString                  myIcons[IconImage_NB];

    bool                      myIsMobile;      //!< flag indicating mobile device
    StMarginsI                myMarginsPx;     //!< active area margins in pixels
    StRectD_t                 myRectGl;        //!< rectangle in GL coordinates
    GLdouble                  myScaleGlX;      //!< scale factor to optimize convertion from Pixels -> GL coordinates
    GLdouble                  myScaleGlY;      //!< scale factor to optimize convertion from Pixels -> GL coordinates
    GLfloat                   myScaleGUI;      //!< scale factor for GUI elements (text, icons), 1.0 by default
    unsigned int              myResolution;    //!< resolution in DPI (for text rendering), 72 by default, stored with myScaleGUI applied
    StPointD_t                cursorZo;        //!< mouse cursor position
    GLint                     myViewport[4];   //!< cached GL viewport

    StArrayList<StGLWidget*>  myDestroyList;   //!< list of widgets to be destroyed
    StGLWidget*               myFocusWidget;   //!< widget currently in focus

    bool                      myIsMenuPressed; //!< global flag to perform navigation in menu after first item clicked

};

#endif // __StGLRootWidget_h_
