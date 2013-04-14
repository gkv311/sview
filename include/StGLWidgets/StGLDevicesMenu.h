/**
 * Copyright © 2011-2013 Kirill Gavrilov <kirill@sview.ru>
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
class StGLFpsLabel;
typedef struct tagStSDOptionsList StSDOptionsList_t;

/**
 * This class intended to create standard devices menu
 * (including change device submenu and renderer plugin options).
 */
class StGLDevicesMenu : public StGLMenu {

        public:

    /**
     * Main constructor.
     * @param theParent            parent widget
     * @param theWindow            window instance
     * @param theLabelChangeDevice label for "Change Device" submenu
     * @param theLabelAboutPlugin  label for "About Plugin" menu item
     * @param theOrient            menu orientation
     */
    ST_CPPEXPORT StGLDevicesMenu(StGLWidget*     theParent,
                                 StWindow*       theWindow,
                                 const StString& theLabelChangeDevice,
                                 const StString& theLabelAboutPlugin,
                                 const StString& theLabelShowFps,
                                 const int       theOrient = MENU_VERTICAL);

    /**
     * Just return class name.
     */
    ST_CPPEXPORT virtual const StString& getClassName();

    /**
     * Will automatically update tracked menu item text.
     */
    ST_CPPEXPORT virtual void stglUpdate(const StPointD_t& theCursorZo);

    /**
     * Draw menu.
     */
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView);

    /**
     * You may assign the menu item to track active device changes
     * and automatically set new title.
     */
    ST_CPPEXPORT void setTrackedItem(StGLMenuItem* theParentItem);

    /**
     * Returns active device title.
     */
    ST_CPPEXPORT const StString& getTitle() const;

    /**
     * Returns true if active device was changed by renderer plugin.
     */
    ST_CPPEXPORT bool isDeviceChanged();

        private: //!< private callback Slots

    /**
     * Will show message box with renderer plugin about information.
     */
    ST_LOCAL void doAboutRenderer(const size_t );

    /**
     * Will show FPS meter.
     */
    ST_LOCAL void doShowFPS(const bool theToShowFps);

        private:

    /**
     * Access to the shared option structure.
     */
    ST_LOCAL StSDOptionsList_t* getSharedInfo() const;

    /**
     * Change device submenu.
     */
    ST_LOCAL StGLMenu* createChangeDeviceMenu(StGLWidget* theParent);

        private:

    StGLMenuItem*          myParentItem;     //!< tracked menu item (optional)
    StWindow*              myWindow;         //!< link to the window instance
    StGLFpsLabel*          myFpsWidget;      //!< FPS meter
    StHandle<StBoolParam>  myToShowFps;      //!< FPS meter visability
    StHandle<StInt32Param> myActiveDevParam;
    StString               myActiveDevice;   //!< active device title
    int                    myActiveDeviceId; //!< active device id (in loaded Renderer plugin)

};

#endif //__StGLDevicesMenu_h_
