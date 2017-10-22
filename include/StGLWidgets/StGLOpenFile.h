/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2015-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLOpenFile_h_
#define __StGLOpenFile_h_

#include <StGLWidgets/StGLMessageBox.h>
#include <StGLWidgets/StGLRootWidget.h>
#include <StFile/StMIMEList.h>

class StGLMenu;
class StGLMenuItem;

/**
 * Widget for file system navigation.
 */
class StGLOpenFile : public StGLMessageBox {

        public:

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StGLOpenFile(StGLWidget*     theParent,
                              const StString& theTitle,
                              const StString& theCloseText);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StGLOpenFile();

    /**
     * Define file filter.
     */
    ST_CPPEXPORT void setMimeList(const StMIMEList& theFilter);

    /**
     * Open new folder.
     */
    ST_CPPEXPORT void addHotItem(const StString& theTarget,
                                 const StString& theName = "");

    /**
     * Open new folder.
     */
    ST_CPPEXPORT void openFolder(const StString& theFolder);

        public:    //! @name Signals

    struct {
        /**
         * @param path to selected file
         */
        StSignal<void (StHandle<StString> )> onFileSelected;
    } signals;

        protected:

    /**
     * Assign icon to the item.
     */
    ST_CPPEXPORT void setItemIcon(StGLMenuItem*   theItem,
                                  const StGLVec4& theColor,
                                  const bool      theisFolder);

    /**
     * Handle hot-item click event - just remember item id.
     */
    ST_CPPEXPORT void doHotItemClick(const size_t theItemId);

    /**
     * Handle item click event - just remember item id.
     */
    ST_CPPEXPORT void doFileItemClick(const size_t theItemId);

    /**
     * Handle folder-up event.
     */
    ST_CPPEXPORT void doFolderUpClick(const size_t );

    /**
     * Override unclick to open new folder.
     */
    ST_CPPEXPORT virtual bool tryUnClick(const StClickEvent& theEvent,
                                         bool&               theIsItemUnclicked) ST_ATTR_OVERRIDE;

        protected: //! @name class fields

    StHandle<StGLTextureArray> myTextureFolder;
    StHandle<StGLTextureArray> myTextureFile;
    StGLTextArea*              myCurrentPath;
    StGLMenu*                  myHotList;       //!< widget containing the list of predefined libraries
    StGLMenu*                  myList;          //!< widget containing the file list of currently opened folder
    StArrayList<StString>      myHotPaths;      //!< array of hot-links
    StHandle<StFolder>         myFolder;        //!< currently opened folder
    StMIMEList                 myFilter;        //!< file filter
    StArrayList<StString>      myExtensions;    //!< extensions filter
    StString                   myItemToLoad;    //!< new item to open

        protected: //! @name main file list settings

    StGLVec4                   myHighlightColor;//!< item highlighting
    StGLVec4                   myItemColor;     //!< color of icons and text labels for the list
    StGLVec4                   myFileColor;     //!< color of icons and text labels for the list
    StGLVec4                   myHotColor;      //!< color of icons and text labels for the hot-list
    int                        myHotSizeX;
    int                        myMarginX;
    int                        myIconSizeX;

};

#endif // __StGLOpenFile_h_
