/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2020 Kirill Gavrilov <kirill@sview.ru>
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
#include <StGL/StGLTexture.h>
#include <StThreads/StResourceManager.h>

template<> inline void StArray<StGLNamedTexture>::sort() {}
typedef StArray<StGLNamedTexture> StGLTextureArray;
class StGLMenuProgram;
class StGLMessageBox;
class StGLTextProgram;
class StGLTextBorderProgram;

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
        Color_ScrollBar,
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
        IconImage_Folder,
        IconImage_File,
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
    ST_CPPEXPORT virtual bool stglInit() ST_ATTR_OVERRIDE;

    /**
     * Draw all children.
     * Root widget caches OpenGL state (like viewport).
     */
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;

    /**
     * Get shared menu program instance.
     */
    ST_LOCAL StGLMenuProgram& getMenuProgram() { return *myMenuProgram; }

    /**
     * Get shared text program instance.
     */
    ST_LOCAL StGLTextProgram& getTextProgram() { return *myTextProgram; }

    /**
     * Get shared text border program instance.
     */
    ST_LOCAL StGLTextBorderProgram& getTextBorderProgram() { return *myTextBorderProgram; }

    /**
     * Return color of standard element.
     */
    ST_LOCAL const StGLVec4& getColorForElement(const StGLRootWidget::Color theElement) const { return myColors[theElement]; }

    /**
     * Return icon of standard element.
     */
    ST_LOCAL const StString& getIcon(const StGLRootWidget::IconImage theIcon) const { return myIcons[theIcon]; }

    ST_LOCAL StHandle<StGLTextureArray>& getCheckboxIcon() { return myCheckboxIcon; }
    ST_LOCAL StHandle<StGLTextureArray>& getRadioIcon()    { return myRadioIcon; }

    /**
     * Function iterate children and self to change clicking state.
     */
    ST_LOCAL bool tryClick(const StClickEvent& theEvent) {
        bool isItemClicked = false;
        return tryClick(theEvent, isItemClicked);
    }

    ST_CPPEXPORT virtual bool tryClick(const StClickEvent& theEvent,
                                       bool&               theIsItemClicked) ST_ATTR_OVERRIDE;

    /**
     * Function iterate children and self for unclicking state.
     */
    ST_LOCAL bool tryUnClick(const StClickEvent& theEvent) {
        bool isItemUnclicked = false;
        return tryUnClick(theEvent, isItemUnclicked);
    }

    ST_CPPEXPORT virtual bool tryUnClick(const StClickEvent& theEvent,
                                         bool&               theIsItemUnclicked) ST_ATTR_OVERRIDE;

    /**
     * Handle scrollong event.
     */
    ST_CPPEXPORT virtual bool doScroll(const StScrollEvent& theEvent) ST_ATTR_OVERRIDE;

    /**
     * Process key down event. Default implementation redirect event to widget in focus.
     * @param theEvent key event
     * @return true if event has been processed
     */
    ST_CPPEXPORT virtual bool doKeyDown(const StKeyEvent& theEvent) ST_ATTR_OVERRIDE;

    /**
     * Process key hold event. Default implementation redirect event to widget in focus.
     * @param theEvent key event
     * @return true if event has been processed
     */
    ST_CPPEXPORT virtual bool doKeyHold(const StKeyEvent& theEvent) ST_ATTR_OVERRIDE;

    /**
     * Process key up event. Default implementation redirect event to widget in focus.
     * @param theEvent key event
     * @return true if event has been processed
     */
    ST_CPPEXPORT virtual bool doKeyUp  (const StKeyEvent& theEvent) ST_ATTR_OVERRIDE;

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
     * Cursor distance between click/unclick events to determine as clicking.
     */
    ST_LOCAL int getClickThreshold() const { return myClickThreshold; }

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
     * @param theSize desired icon size in pixels (will be scaled)
     * @return the icon of standard size nearest to requested one
     */
    ST_CPPEXPORT IconSize scaleIcon(const int theSize) const;

    /**
     * Scale specified size to the nearest default icon size and compute margins.
     * @param theSize desired icon size in pixels (will be scaled)
     * @return the icon of standard size nearest to requested one
     */
    ST_LOCAL IconSize scaleIcon(const int   theSize,
                                StMarginsI& theMargins) const {
        const IconSize anStdSize = scaleIcon(theSize);
        theMargins = iconMargins(anStdSize, theSize);
        return anStdSize;
    }

    /**
     * Compute margins to the icon of standard size to fit into the center of arbitrary size.
     */
    ST_CPPEXPORT StMarginsI iconMargins(StGLRootWidget::IconSize theStdSize,
                                        const int                theSize) const;

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

    /**
     * Return the full width of the window (including non-working area)
     */
    ST_LOCAL int getRootFullSizeX() const {
        return myRectPxFull.width();
    }

    /**
     * Return the full height of the window (including non-working area)
     */
    ST_LOCAL int getRootFullSizeY() const {
        return myRectPxFull.height();
    }

    /**
     * Return GL coordinates for entire root widget (including non-working area).
     */
    ST_LOCAL StRectD_t getRootRectGl() const {
        return myRectGl;
    }

    /**
     * Return GL coordinates for working area within root widget.
     */
    ST_LOCAL StRectD_t getRootWorkRectGl() const {
        return myRectWorkGl;
    }

    ST_LOCAL const StMarginsI& getRootMargins() const {
        return myMarginsPx;
    }

    /**
     * Return scale factor to downscale the main image for VR (e.g. with HMD having great FOV),
     * or 1.0 for normal displays.
     */
    ST_LOCAL double getVrZoomOut() const {
        return myMarginsPx.top != 0
             ? 0.74 //1.0 - double(myMarginsPx.top) / double(getRootFullSizeY())
             : 1.0;
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

    ST_LOCAL const StGLProjCamera* getCamera() const {
        return &myProjCamera;
    }

    ST_LOCAL StGLProjCamera* changeCamera() {
        return &myProjCamera;
    }

    /**
     * @return widget in focus
     */
    ST_LOCAL StGLWidget* getFocus() const {
        return myFocusWidget;
    }

    /**
     * @param theWidget new widget to set focus
     * @return previous widget focus
     */
    ST_CPPEXPORT StGLWidget* setFocus(StGLWidget* theWidget);

    /**
     * Return active modal dialog.
     */
    ST_LOCAL StGLMessageBox* getModalDialog() const {
        return myModalDialog;
    }

    /**
     * Assign new modal dialog.
     * @param theWidget       new modal dialog
     * @param theToReleaseOld flag to release the previous modal dialog
     */
    ST_CPPEXPORT void setModalDialog(StGLMessageBox* theWidget,
                                     const bool      theToReleaseOld = true);

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

    inline void setLensDist(const GLfloat theLensDist) {
        myLensDist = theLensDist;
    }

    /**
     * Returns mouse cursor position in GL coordinates.
     */
    ST_LOCAL const StPointD_t& getCursorZo() const {
        return myCursorZo;
    }

    using StGLWidget::stglResize;
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& theCursorZo,
                                         bool theIsPreciseInput) ST_ATTR_OVERRIDE;

    /**
     * Resize root widget in sync with window.
     * @param theViewPort new viewport (width and height)
     * @param theMargins  new margins defining a working area
     * @param theAspect   new aspect ratio (normally width / height)
     */
    ST_CPPEXPORT virtual void stglResize(const StGLBoxPx& theViewPort,
                                         const StMarginsI& theMargins,
                                         float theAspect);

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
    ST_LOCAL void stglScissorRect2d(const StRectI_t& theRect,
                                    StGLBoxPx& theScissorRect) const {
        stglScissorRectInternal(theRect, true, theScissorRect);
    }

    /**
     * Computes scissor rectangle in OpenGL viewport.
     * @param theRect        Rectangle in window coordinates
     * @param theScissorRect Scissor rectangle for glScissor() call
     */
    ST_LOCAL void stglScissorRect3d(const StRectI_t& theRect,
                                    StGLBoxPx& theScissorRect) const {
        stglScissorRectInternal(theRect, false, theScissorRect);
    }

    /**
     * Append widget to destroy list.
     * This method should be used to destroy widget within callback processing
     * to prevent corruption during widgets iteration.
     * @param theWidget the widget to destroy
     */
    ST_CPPEXPORT virtual void destroyWithDelay(StGLWidget* theWidget) ST_ATTR_OVERRIDE;

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

    /**
    * Computes scissor rectangle in OpenGL viewport.
    * @param theRect [in] Rectangle in window coordinates
    * @param theIs2d [in] 2d/3d flag
    * @param theScissorRect [out] Scissor rectangle for glScissor() call
    */
    ST_CPPEXPORT void stglScissorRectInternal(const StRectI_t& theRect,
                                              const bool theIs2d,
                                              StGLBoxPx& theScissorRect) const;

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

    StHandle<StGLTextureArray> myCheckboxIcon;
    StHandle<StGLTextureArray> myRadioIcon;
    StHandle<StGLMenuProgram>  myMenuProgram;
    StHandle<StGLTextProgram>  myTextProgram;
    StHandle<StGLTextBorderProgram> myTextBorderProgram;

    bool                      myIsMobile;      //!< flag indicating mobile device
    StMarginsI                myMarginsPx;     //!< active area margins in pixels
    StRectD_t                 myRectGl;        //!< rectangle in GL coordinates
    StRectD_t                 myRectWorkGl;    //!< rectangle of working area in GL coordinates
    StRectI_t                 myRectPxFull;    //!< full rectangle (including non-working area)
    GLdouble                  myScaleGlX;      //!< scale factor to optimize convertion from Pixels -> GL coordinates
    GLdouble                  myScaleGlY;      //!< scale factor to optimize convertion from Pixels -> GL coordinates
    GLfloat                   myScaleGUI;      //!< scale factor for GUI elements (text, icons), 1.0 by default
    unsigned int              myResolution;    //!< resolution in DPI (for text rendering), 72 by default, stored with myScaleGUI applied
    StPointD_t                myCursorZo;      //!< mouse cursor position
    GLint                     myViewport[4];   //!< cached GL viewport

    StArrayList<StGLWidget*>  myDestroyList;   //!< list of widgets to be destroyed
    StGLWidget*               myFocusWidget;   //!< widget currently in focus
    StGLMessageBox*           myModalDialog;   //!< active dialog

    bool                      myIsMenuPressed; //!< global flag to perform navigation in menu after first item clicked

        protected:

    IconSize                  myMenuIconSize;  //!< scaled size of menu icon
    int                       myClickThreshold;//!< cursor distance between click/unclick events to determine as clicking

};

#endif // __StGLRootWidget_h_
