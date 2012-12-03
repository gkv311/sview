/**
 * Copyright © 2007-2012 Kirill Gavrilov <kirill@sview.ru>
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
class ST_LOCAL StImageViewer : public StDrawerInterface {

    friend class StImageViewGUI;

        public:

    static const char* ST_DRAWER_PLUGIN_NAME;

        public: //! @name interface methods' implementations

    StImageViewer();
    virtual ~StImageViewer();
    virtual StDrawerInterface* getLibImpl() { return this; }
    virtual bool init(StWindowInterface* theWindow);
    virtual bool open(const StOpenInfo& stOpenInfo);
    virtual void parseCallback(StMessage_t* stMessages);
    virtual void stglDraw(unsigned int theView);

        public: //! @name callback Slots

    // TODO (Kirill Gavrilov#9) move to the StImageLoader thread
    static SV_THREAD_FUNCTION doOpenFileDialogThread(void* theArg) {
        struct ThreadArgs {
            StImageViewer* receiverPtr; size_t filesCount;
        };
        ThreadArgs* threadArgs = (ThreadArgs* )theArg;
        threadArgs->receiverPtr->doOpenFileDialog(threadArgs->filesCount);
        delete threadArgs;
        return SV_THREAD_RETURN 0;
    }
    static void doOpenFileDialog(void* receiverPtr, size_t filesCount) {
        struct ThreadArgs {
            StImageViewer* receiverPtr; size_t filesCount;
        };
        ThreadArgs* threadArgs = new ThreadArgs();
        threadArgs->receiverPtr = (StImageViewer* )receiverPtr;
        threadArgs->filesCount  = filesCount;
        threadArgs->receiverPtr->params.isFullscreen->setValue(false); // workaround
        StThread(doOpenFileDialogThread, (void* )threadArgs);
    }
    void doOpenFileDialog(const size_t filesCount = 1);

    void doOpen2FilesDialog(const size_t dummy = 0);
    void doSaveImageAs(const size_t theImgType) { myLoader->doSaveImageAs(theImgType); }
    void doListFirst(const size_t dummy = 0);
    void doListPrev(const size_t dummy = 0);
    void doListNext(const size_t dummy = 0);
    void doListLast(const size_t dummy = 0);
    void doSlideShow(const size_t dummy = 0);
    void doQuit(const size_t dummy = 0);

    void doReset(const size_t dummy = 0);

    void doUpdateStateLoading();
    void doUpdateStateLoaded();

    /**
     * Handler for new file loaded event.
     */
    void doLoaded();

    // callback keys
    void keysStereo(bool* keysMap);
    void keysSrcFormat(bool* keysMap);
    void keysFileWalk(bool* keysMap);
    void keysCommon(bool* keysMap);

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

        private: //! @name private callback Slots

    void doFullscreen(const bool theIsFullscreen);
    void doSwitchSrcFormat(const int32_t theSrcFormat);

        private:

    void parseArguments(const StArgumentsMap& theArguments);

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
