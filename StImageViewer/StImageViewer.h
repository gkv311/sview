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

#include <StCore/StDrawerInterface.h>

#include <StSettings/StParam.h>

#include "StImageLoader.h"
#include "StImageViewerGUI.h"

// forward declarations
class StGLContext;
class StCheckUpdates;
class StWindow;

/**
 * Base Drawer class for Image Viewer plugin.
 */
class StImageViewer : public StDrawerInterface {

    friend class StImageViewGUI;

        public:

    static const char* ST_DRAWER_PLUGIN_NAME;

        public: //! @name interface methods' implementations

    ST_CPPEXPORT StImageViewer();
    ST_CPPEXPORT virtual ~StImageViewer();
    ST_CPPEXPORT virtual StDrawerInterface* getLibImpl() { return this; }
    ST_CPPEXPORT virtual bool init(StWindowInterface* theWindow);
    ST_CPPEXPORT virtual bool open(const StOpenInfo& stOpenInfo);
    ST_CPPEXPORT virtual void parseCallback(StMessage_t* stMessages);
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView);

        public: //! @name callback Slots

    // TODO (Kirill Gavrilov#9) move to the StImageLoader thread
    ST_LOCAL static SV_THREAD_FUNCTION doOpenFileDialogThread(void* theArg) {
        struct ThreadArgs {
            StImageViewer* receiverPtr; size_t filesCount;
        };
        ThreadArgs* threadArgs = (ThreadArgs* )theArg;
        threadArgs->receiverPtr->doOpenFileDialog(threadArgs->filesCount);
        delete threadArgs;
        return SV_THREAD_RETURN 0;
    }
    ST_LOCAL static void doOpenFileDialog(void* receiverPtr, size_t filesCount) {
        struct ThreadArgs {
            StImageViewer* receiverPtr; size_t filesCount;
        };
        ThreadArgs* threadArgs = new ThreadArgs();
        threadArgs->receiverPtr = (StImageViewer* )receiverPtr;
        threadArgs->filesCount  = filesCount;
        threadArgs->receiverPtr->params.isFullscreen->setValue(false); // workaround
        StThread(doOpenFileDialogThread, (void* )threadArgs);
    }
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

    ST_LOCAL void parseArguments(const StArgumentsMap& theArguments);

        private: //! @name private fields

    StHandle<StGLContext>      myContext;
    StHandle<StWindow>         myWindow;          //!< wrapper over Output plugin's StWindow instance
    StHandle<StSettings>       mySettings;        //!< settings manager for Image Viewer plugin
    StHandle<StImageViewerGUI> myGUI;             //!< GUI root widget
    StHandle<StImageLoader>    myLoader;          //!< main image loader class
    StHandle<StCheckUpdates>   myUpdates;         //!< check updates utility

    StEvent                    myEventDialog;     //!< event to prevent showing multiple open/save file dialogs
    StEvent                    myEventLoaded;     //!< indicate that new file was open

    StTimer                    mySlideShowTimer;  //!< slideshow stuff
    double                     mySlideShowDelay;

    int32_t                    myLastUpdateDay;
    bool                       myToCheckUpdates;
    bool                       myToSaveSrcFormat; //!< indicates that active source format should be saved or not
    bool                       myEscNoQuit;       //!< if true then Escape will not quit application
    bool                       myToQuit;          //!< drawer quiting switcher

};

#endif //__StImageViewer_h_
