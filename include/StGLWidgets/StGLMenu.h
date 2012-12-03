/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLMenu_h_
#define __StGLMenu_h_

#include <StGLWidgets/StGLTextArea.h>
#include <StGL/StGLVertexBuffer.h>

// forward declarations
class StBoolParam;
class StInt32Param;
class StFloat32Param;
class StGLMenuItem;
class StGLMenuProgram;

/**
 * Widget represents classical menu object.
 */
class ST_LOCAL StGLMenu : public StGLWidget {

        public:

    enum {
        MENU_VERTICAL = 0,
        MENU_HORIZONTAL = 1,
    };

    // recursively delete all submenus and that this menu itself
    // should be used only for dynamic menu recreation
    static void DeleteWithSubMenus(StGLMenu* theMenu);

    StGLMenu(StGLWidget* theParent,
             const int theLeft, const int theTop,
             const int theOrient = MENU_VERTICAL,
             const bool theIsRootMenu = false);

    virtual ~StGLMenu();

    virtual const StString& getClassName();
    virtual void setVisibility(bool isVisible, bool isForce);
    virtual void stglResize(const StRectI_t& theWinRectPx);
    virtual bool stglInit();
    virtual void stglDraw(unsigned int theView);
    virtual bool tryUnClick(const StPointD_t& theCursorZo, const int& theMouseBtn, bool& theIsItemUnclicked);

    bool isRootMenu() const {
        return myIsRootMenu;
    }

    bool isActive() const {
        return myIsActive;
    }

    void setActive(const bool isActive) {
        myIsActive = isActive;
    }

    int getOrient() const {
        return myOrient;
    }

    /**
     * Update all children menus layout.
     */
    void stglUpdateSubmenuLayout();

    StGLMenuItem* addItem(const StString& theLabel,
                          const size_t    theUserData = 0);
    StGLMenuItem* addItem(const StString& theLabel,
                          StGLMenu*       theSubMenu);

    /**
     * Append checkbox menu item.
     * @param theLabel        - menu item text;
     * @param theTrackedValue - tracked boolean value;
     * @return created menu item widget.
     */
    StGLMenuItem* addItem(const StString&              theLabel,
                          const StHandle<StBoolParam>& theTrackedValue);

    /**
     * Append radio button menu item.
     * @param theLabel        - menu item text;
     * @param theTrackedValue - tracked integer (enumeration) value;
     * @param theOnValue      - associated integer (enumeration) value for this radio button;
     * @return created menu item widget.
     */
    StGLMenuItem* addItem(const StString&               theLabel,
                          const StHandle<StInt32Param>& theTrackedValue,
                          const int32_t                 theOnValue);

    /**
     * Append radio button menu item.
     * @param theLabel        - menu item text;
     * @param theTrackedValue - tracked float value;
     * @param theOnValue      - associated float value for this radio button;
     * @return created menu item widget.
     */
    StGLMenuItem* addItem(const StString&                 theLabel,
                          const StHandle<StFloat32Param>& theTrackedValue,
                          const float                     theOnValue);

        private:

    void stglResize();

        private: //! @name private fields

    StGLShare<StGLMenuProgram> myProgram;
    StGLVertexBuffer           myVertexBuf;
    StGLVec4                   myColorVec;
    int                        myOrient;
    int                        myItemHeight;
    int                        myWidth;
    bool                       myIsRootMenu;
    bool                       myIsActive;
    bool                       myIsInitialized;

};

#endif //__StGLMenu_h_
