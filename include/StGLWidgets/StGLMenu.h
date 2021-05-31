/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2009-2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLMenu_h_
#define __StGLMenu_h_

#include <StGLWidgets/StGLTextArea.h>
#include <StGL/StGLVertexBuffer.h>

// forward declarations
class StAction;
class StEnumParam;
class StBoolParam;
class StBoolParamNamed;
class StInt32Param;
class StFloat32Param;
class StGLMenuItem;

/**
 * Widget represents classical menu object.
 */
class StGLMenu : public StGLWidget {

        public:

    enum {
        MENU_VERTICAL,         //!< vertical menu with offset reserved for check box icon
        MENU_VERTICAL_COMPACT, //!< vertical menu without extra offset
        MENU_HORIZONTAL,       //!< horizontal menu
        MENU_ZERO,             //!< menu for single item
    };

    // recursively delete all submenus and that this menu itself
    // should be used only for dynamic menu recreation
    ST_CPPEXPORT static void DeleteWithSubMenus(StGLMenu* theMenu);

    ST_CPPEXPORT StGLMenu(StGLWidget* theParent,
                          const int   theLeft,
                          const int   theTop,
                          const int   theOrient = MENU_VERTICAL,
                          const bool  theIsRootMenu = false);

    ST_CPPEXPORT virtual ~StGLMenu();

    ST_CPPEXPORT virtual void setOpacity(const float theOpacity, bool theToSetChildren) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglResize() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool stglInit() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool doKeyDown(const StKeyEvent& theEvent) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool doScroll(const StScrollEvent& theEvent) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool tryUnClick(const StClickEvent& theEvent, bool& theIsItemUnclicked) ST_ATTR_OVERRIDE;

    inline bool isRootMenu() const {
        return myIsRootMenu;
    }

    ST_CPPEXPORT void setContextual(const bool theValue);

    inline bool isActive() const {
        return myIsActive;
    }

    inline void setActive(const bool isActive) {
        myIsActive = isActive;
    }

    /**
     * Skip mouse unclick event - keep menu in active state.
     */
    inline void setKeepActive() {
        myKeepActive = true;
    }

    inline int getOrient() const {
        return myOrient;
    }

    /**
     * @return height of item of standard size
     */
    ST_LOCAL int getItemHeight() const {
        return myItemHeight;
    }

    /**
     * Setup height of item.
     */
    ST_LOCAL void setItemHeight(const int theHeight) {
        myItemHeight = theHeight;
    }

    /**
     * @return maximum width of item in this menu
     */
    ST_LOCAL int getItemWidth() const {
        return myWidth;
    }

    /**
     * Setup maximum width of item in this menu.
     */
    ST_LOCAL void setItemWidth(const int theWidth) {
        myWidth = theWidth;
    }

    /**
     * Return minimum width of item in this menu.
     */
    ST_LOCAL int getItemWidthMin() const {
        return myWidthMin;
    }

    /**
     * Setup minimum width of item in this menu.
     */
    ST_LOCAL void setItemWidthMin(const int theWidth) {
        myWidthMin = theWidth;
    }

    /**
     * Setup background color of menu.
     */
    inline void setColor(const StGLVec3& theColor) {
        myColorVec.rgb() = theColor;
    }

    /**
     * Setup background color of menu.
     */
    ST_LOCAL void setColor(const StGLVec4& theColor) {
        myColorVec = theColor;
    }

    /**
     * Draw bounds rectangle (off by default).
     */
    inline void setShowBounds(const bool theToShow) {
        myToDrawBounds = theToShow;
    }

    /**
     * Update all children menus layout.
     */
    ST_CPPEXPORT void stglUpdateSubmenuLayout();

    ST_CPPEXPORT StGLMenuItem* addItem(const StString& theLabel,
                                       const size_t    theUserData = 0);
    ST_CPPEXPORT StGLMenuItem* addItem(const StString& theLabel,
                                       StGLMenu*       theSubMenu);

    /**
     * Append checkbox menu item.
     * @param theLabel        menu item text
     * @param theTrackedValue tracked boolean value
     * @return created menu item widget
     */
    ST_CPPEXPORT StGLMenuItem* addItem(const StString&              theLabel,
                                       const StHandle<StBoolParam>& theTrackedValue);

    /**
     * Append checkbox menu item.
     * @param theTrackedValue tracked boolean value
     * @return created menu item widget
     */
    ST_CPPEXPORT StGLMenuItem* addItem(const StHandle<StBoolParamNamed>& theTrackedValue);

    /**
     * Append radio button menu item.
     * @param theLabel        menu item text
     * @param theTrackedValue tracked integer (enumeration) value
     * @param theOnValue      associated integer (enumeration) value for this radio button
     * @return created menu item widget
     */
    ST_CPPEXPORT StGLMenuItem* addItem(const StString&               theLabel,
                                       const StHandle<StInt32Param>& theTrackedValue,
                                       const int32_t                 theOnValue);

    /**
     * Append radio button menu item.
     * @param theTrackedValue tracked integer (enumeration) value
     * @param theOnValue      associated integer (enumeration) value for this radio button
     * @return created menu item widget
     */
    ST_CPPEXPORT StGLMenuItem* addItem(const StHandle<StEnumParam>& theTrackedValue,
                                       const int32_t                theOnValue);

    /**
     * Append radio button menu item.
     * @param theLabel        menu item text
     * @param theTrackedValue tracked float value
     * @param theOnValue      associated float value for this radio button
     * @return created menu item widget
     */
    ST_CPPEXPORT StGLMenuItem* addItem(const StString&                 theLabel,
                                       const StHandle<StFloat32Param>& theTrackedValue,
                                       const float                     theOnValue);

    /**
     * Append menu item with bound action.
     * @param theLabel  menu item text
     * @param theAction action to invoke on click
     * @return created menu item widget
     */
    ST_CPPEXPORT StGLMenuItem* addItem(const StString&           theLabel,
                                       const StHandle<StAction>& theAction,
                                       StGLMenu*                 theSubMenu = NULL);

        protected: //! @name protected fields

    StGLVertexBuffer           myVertexBuf;
    StGLVertexBuffer           myVertexBndBuf;
    StGLVec4                   myColorVec;
    int                        myOrient;
    int                        myItemHeight;
    int                        myWidthMin;
    int                        myWidth;
    bool                       myIsRootMenu;    //!< the root menu does not show sub-menus until first click
    bool                       myIsContextual;
    bool                       myIsActive;
    bool                       myKeepActive;
    bool                       myIsInitialized;
    bool                       myToDrawBounds;

};

#endif //__StGLMenu_h_
