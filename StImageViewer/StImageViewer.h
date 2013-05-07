/**
 * Copyright © 2007-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StImageViewer program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StImageViewer program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StImageViewer_h_
#define __StImageViewer_h_

#include <StCore/StApplication.h>

#include <StSettings/StParam.h>

#include "StImageLoader.h"
#include "StImageViewerGUI.h"

// forward declarations
class StGLContext;
class StCheckUpdates;
class StWindow;

/**
 * Image Viewer application.
 */
class StImageViewer : public StApplication {

    friend class StImageViewGUI;

        public:

    static const char* ST_DRAWER_PLUGIN_NAME;

        public: //! @name interface methods' implementations

    /**
     * Main constructor.
     */
    ST_CPPEXPORT StImageViewer(const StNativeWin_t         theParentWin = (StNativeWin_t )NULL,
                               const StHandle<StOpenInfo>& theOpenInfo  = NULL);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StImageViewer();

    /**
     * Open application.
     */
    ST_CPPEXPORT virtual bool open();

    /**
     * Process callback.
     */
    ST_CPPEXPORT virtual void processEvents(const StMessage_t* theEvents);

    /**
     * Draw frame for requested view.
     */
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView);

    /**
     * Reset device - release GL resources in old window and re-create them in new window.
     */
    ST_CPPEXPORT virtual bool resetDevice();

        private:

    /**
     * Process device change.
     */
    ST_LOCAL virtual void doChangeDevice(const int32_t theValue);

        public: //! @name callback Slots

    ST_LOCAL static void doOpenFileDialog(void* theReceiverPtr, size_t theFilesCount);
    ST_LOCAL void doOpenFileDialog(const size_t filesCount = 1);

    ST_LOCAL void doOpen2FilesDialog(const size_t dummy = 0);
    ST_LOCAL void doSaveImageAs(const size_t theImgType) { myLoader->doSaveImageAs(theImgType); }
    ST_LOCAL void doListFirst(const size_t dummy = 0);
    ST_LOCAL void doListPrev(const size_t dummy = 0);
    ST_LOCAL void doListNext(const size_t dummy = 0);
    ST_LOCAL void doListLast(const size_t dummy = 0);
    ST_LOCAL void doSlideShow(const size_t dummy = 0);
    ST_LOCAL void doQuit(const size_t dummy = 0);

    ST_LOCAL void doReset(const size_t dummy = 0);

    ST_LOCAL void doUpdateStateLoading();
    ST_LOCAL void doUpdateStateLoaded();

    /**
     * Handler for new file loaded event.
     */
    ST_LOCAL void doLoaded();

    // callback keys
    ST_LOCAL void keysStereo(bool* keysMap);
    ST_LOCAL void keysSrcFormat(bool* keysMap);
    ST_LOCAL void keysFileWalk(bool* keysMap);
    ST_LOCAL void keysCommon(bool* keysMap);

        public: //! @name Properties

    struct {

        StHandle<StBoolParam>   isFullscreen;     //!< fullscreen state
        StHandle<StBoolParam>   toRestoreRatio;   //!< restore ratio on restart
        StHandle<StInt32Param>  checkUpdatesDays; //!< days count between updates checks
        StHandle<StInt32Param>  srcFormat;        //!< source format
        StHandle<StBoolParam>   ToShowFps;        //!< display FPS meter
        StString                lastFolder;       //!< laster folder used to open / save file
        StImageFile::ImageClass imageLib;         //!< preferred image library
        int                     fpsBound;         //!< limit or not rendering FPS

    } params;

    /**
     * Retrieve current playlist item.
     */
    ST_LOCAL bool getCurrentFile(StHandle<StFileNode>&     theFileNode,
                                 StHandle<StStereoParams>& theParams,
                                 StHandle<StImageInfo>&    theInfo);

        private: //! @name private callback Slots

    ST_LOCAL void doFullscreen(const bool theIsFullscreen);
    ST_LOCAL void doSwitchSrcFormat(const int32_t theSrcFormat);

        private:

    /**
     * Initialization routines.
     */
    ST_LOCAL bool init();

    /**
     * Parse arguments.
     */
    ST_LOCAL void parseArguments(const StArgumentsMap& theArguments);

    /**
     * Release GL resources.
     */
    ST_LOCAL void releaseDevice();

        private: //! @name private fields

    StHandle<StGLContext>      myContext;
    StHandle<StSettings>       mySettings;        //!< settings manager for Image Viewer plugin
    StHandle<StTranslations>   myLangMap;         //!< translated strings map

    StHandle<StImageViewerGUI> myGUI;             //!< GUI root widget
    StHandle<StImageLoader>    myLoader;          //!< main image loader class
    StHandle<StCheckUpdates>   myUpdates;         //!< check updates utility

    StCondition                myEventDialog;     //!< event to prevent showing multiple open/save file dialogs
    StCondition                myEventLoaded;     //!< indicate that new file was open

    StTimer                    mySlideShowTimer;  //!< slideshow stuff
    double                     mySlideShowDelay;

    int32_t                    myLastUpdateDay;
    bool                       myToCheckUpdates;
    bool                       myToSaveSrcFormat; //!< indicates that active source format should be saved or not
    bool                       myEscNoQuit;       //!< if true then Escape will not quit application

};

#endif //__StImageViewer_h_
