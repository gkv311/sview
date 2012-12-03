/**
 * Copyright © 2011-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLDevicesMenu_h_
#define __StGLDevicesMenu_h_

#include <StGLWidgets/StGLMenu.h>

// forward declarations
class StWindow;
typedef struct tagStSDOptionsList StSDOptionsList_t;

/**
 * This class intended to create standard devices menu
 * (including change device submenu and renderer plugin options).
 */
class ST_LOCAL StGLDevicesMenu : public StGLMenu {

        public:

    /**
     * Main constructor.
     * @param theParent (StGLWidget* ) - parent widget;
     * @param theWindow (StWindow* ) - window instance;
     * @param theLabelChangeDevice (const StString& ) - label for "Change Device" submenu;
     * @param theLabelAboutPlugin (const StString& ) - label for "About Plugin" menu item;
     * @param theOrient (const int ) - menu orientation.
     */
    StGLDevicesMenu(StGLWidget* theParent,
                    StWindow* theWindow,
                    const StString& theLabelChangeDevice,
                    const StString& theLabelAboutPlugin,
                    const int theOrient = MENU_VERTICAL);

    /**
     * Just return class name.
     */
    virtual const StString& getClassName();

    /**
     * Will automatically update tracked menu item text.
     */
    virtual void stglUpdate(const StPointD_t& theCursorZo);

    /**
     * You may assign the menu item to track active device changes
     * and automatically set new title.
     */
    void setTrackedItem(StGLMenuItem* theParentItem) {
        myParentItem = theParentItem;
    }

    /**
     * Returns active device title.
     */
    const StString& getTitle() const {
        return myActiveDevice;
    }

    /**
     * Returns true if active device was changed by renderer plugin.
     */
    bool isDeviceChanged();

        private: //!< private callback Slots

    /**
     * Will show message box with renderer plugin about information.
     */
    void doAboutRenderer(const size_t );

        private:

    /**
     * Access to the shared option structure.
     */
    StSDOptionsList_t* getSharedInfo() const;

    /**
     * Change device submenu.
     */
    StGLMenu* createChangeDeviceMenu(StGLWidget* theParent);

        private:

    StGLMenuItem*          myParentItem;     //!< tracked menu item (optional)
    StWindow*              myWindow;         //!< link to the window instance
    StHandle<StInt32Param> myActiveDevParam;
    StString               myActiveDevice;   //!< active device title
    int                    myActiveDeviceId; //!< active device id (in loaded Renderer plugin)

};

#endif //__StGLDevicesMenu_h_
