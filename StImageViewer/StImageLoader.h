/**
 * Copyright © 2007-2019 Kirill Gavrilov <kirill@sview.ru>
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

#ifndef __StImageLoader_h_
#define __StImageLoader_h_

#include <StStrings/StMsgQueue.h>
#include <StFile/StMIMEList.h>
#include <StGL/StPlayList.h>
#include <StGLStereo/StGLTextureQueue.h>
#include <StImage/StImageFile.h>
#include <StImage/StJpegParser.h>
#include <StSlots/StSignal.h>
#include <StStrings/StLangMap.h>
#include <StThreads/StProcess.h>
#include <StThreads/StResourceManager.h>

class StThread;

struct StImageInfo {

    std::shared_ptr<StStereoParams> Id;

    StArgumentsMap           Info;
    StString                 Path;           //!< file path
    StImageFile::ImageType   ImageType;      //!< image type
    StFormat                 StInfoStream;   //!< source format as stored in file metadata
    StFormat                 StInfoFileName; //!< source format detected from file name
    bool                     IsSavable;      //!< indicate that file can be saved without re-encoding

    StImageInfo() : ImageType(StImageFile::ST_TYPE_NONE), StInfoStream(StFormat_AUTO), StInfoFileName(StFormat_AUTO), IsSavable(false) {}

};

/**
 * Auxiliary class to load images from dedicated thread.
 */
class StImageLoader {

        public:

    enum Action {
        Action_NONE,
        Action_Quit,
        Action_SaveJPEG,
        Action_SavePNG,
        Action_SaveInfo,
    };

        public:

    static const char* ST_IMAGES_MIME_STRING;
    static const char* ST_VIDEOS_MIME_STRING;

        public:

    ST_LOCAL const StMIMEList& getMimeListImages() const { return myMimeList; }

    ST_LOCAL const StMIMEList& getMimeListVideo() const { return myVideoMimeList; }

    ST_LOCAL StImageLoader(const StImageFile::ImageClass             theImageLib,
                           const std::shared_ptr<StResourceManager>& theResMgr,
                           const std::shared_ptr<StMsgQueue>&        theMsgQueue,
                           const std::shared_ptr<StLangMap>&         theLangMap,
                           const std::shared_ptr<StPlayList>&        thePlayList,
                           const std::shared_ptr<StGLTextureQueue>&  theTextureQueue,
                           const GLint                               theMaxTexDim);
    ST_LOCAL ~StImageLoader();

    ST_LOCAL const std::shared_ptr<StGLTextureQueue>& getTextureQueue() const {
        return myTextureQueue;
    }

    ST_LOCAL void mainLoop();

    ST_LOCAL void doLoadNext() {
        myLoadNextEvent.set();
    }

    ST_LOCAL void doSaveImageAs(const size_t theImgType) {
        if(myAction == Action_Quit) {
            return;
        }

        if(theImgType == StImageFile::ST_TYPE_JPEG) {
            myAction = Action_SaveJPEG;
        } else if(theImgType == StImageFile::ST_TYPE_PNG) {
            myAction = Action_SavePNG;
        } else {
            ST_ERROR_LOG("Attempt to save in unsupported image format " + theImgType);
            return;
        }
        myLoadNextEvent.set();
    }

    ST_LOCAL void doSaveInfo(const std::shared_ptr<StImageInfo>& theInfo) {
        myLock.lock();
        myInfoToSave = theInfo;
        myAction     = Action_SaveInfo;
        myLock.unlock();
        myLoadNextEvent.set();
    }

    ST_LOCAL std::shared_ptr<StImageInfo> getFileInfo(const std::shared_ptr<StStereoParams>& theParams) const {
        myLock.lock();
        std::shared_ptr<StImageInfo> anInfo = myImgInfo;
        myLock.unlock();
        return (anInfo.get() != nullptr && anInfo->Id == theParams) ? anInfo : std::shared_ptr<StImageInfo>();
    }

    ST_LOCAL void setStereoFormat(const StFormat theSrcFormat) {
        myStFormatByUser = theSrcFormat;
    }

    ST_LOCAL void setImageLib(const StImageFile::ImageClass theImageLib) {
        myImageLib = theImageLib;
    }

    /**
     * Release unused memory as fast as possible.
     */
    ST_LOCAL void setCompressMemory(const bool theToCompress);

    /**
     * Get theater mode.
     */
    ST_LOCAL bool isTheaterMode() const { return myIsTheaterMode; }

    /**
     * Set theater mode.
     */
    ST_LOCAL void setTheaterMode(bool theIsTheater) {
        myIsTheaterMode = theIsTheater;
    }

    /**
     * Stick to panorama 360 mode.
     */
    ST_LOCAL void setStickPano360(bool theToStick) {
        myToStickPano360 = theToStick;
    }

    /**
     * Flip Z within 6x1 cubemap input.
     */
    ST_LOCAL void setFlipCubeZ6x1(bool theToFlip) {
        myToFlipCubeZ6x1 = theToFlip;
    }

    /**
     * Flip Z within 3x2 cubemap input.
     */
    ST_LOCAL void setFlipCubeZ3x2(bool theToFlip) {
        myToFlipCubeZ3x2 = theToFlip;
    }

    /**
     * Set if JPS file should be read as Left/Right (TRUE) of as Right/Left (FALSE).
     */
    ST_LOCAL void setSwapJPS(bool theToSwap) { myToSwapJps = theToSwap; }

        public:  //! @name Signals

    struct {
        /**
         * Emit callback Slot on file load.
         */
        StSignal<void ()> onLoaded;

    } signals;

        private:

    ST_LOCAL bool loadImage(const std::shared_ptr<StFileNode>& theSource,
                            std::shared_ptr<StStereoParams>&   theParams);
    ST_LOCAL bool saveImage(const std::shared_ptr<StFileNode>& theSource,
                            const std::shared_ptr<StStereoParams>& theParams,
                            StImageFile::ImageType theImgType);

    ST_LOCAL bool saveImageInfo(const std::shared_ptr<StImageInfo>& theInfo);

    ST_LOCAL int getSnapshot(StImage* outDataLeft, StImage* outDataRight, bool isForce = false) {
        return myTextureQueue->getSnapshot(outDataLeft, outDataRight, isForce);
    }

    /**
     * Auxiliary method to process image load error.
     */
    ST_LOCAL void processLoadFail(const StString& theErrorDesc);

    /**
     * Fill metadata map from EXIF.
     */
    ST_LOCAL void metadataFromExif(const std::shared_ptr<StExifDir>& theDir,
                                   std::shared_ptr<StImageInfo>&     theInfo);

    ST_LOCAL const StString& tr(const size_t theId) const {
        return myLangMap->getValue(theId);
    }

        private:

    const StMIMEList myMimeList;
    const StMIMEList myVideoMimeList;

    std::shared_ptr<StThread>          myThread;        //!< main loop thread
    std::shared_ptr<StResourceManager> myResMgr;        //!< resource manager
    std::shared_ptr<StLangMap>         myLangMap;       //!< translations dictionary
    std::shared_ptr<StPlayList>        myPlayList;      //!< play list
    mutable StMutex                    myLock;          //!< lock to access not thread-safe properties
    StCondition                        myLoadNextEvent;
    StFormat                           myStFormatByUser;//!< target source format (auto-detect by default)
    GLint                              myMaxTexDim;     //!< value for GL_MAX_TEXTURE_SIZE
    std::shared_ptr<StGLTextureQueue>  myTextureQueue;  //!< decoded frames queue
    std::shared_ptr<StImageInfo>       myImgInfo;       //!< info about currently loaded image
    std::shared_ptr<StImageInfo>       myInfoToSave;    //!< modified info to be saved
    std::shared_ptr<StMsgQueue>        myMsgQueue;      //!< messages queue

    volatile StImageFile::ImageClass myImageLib;
    volatile Action            myAction;
    volatile bool              myIsTheaterMode;  //!< flag indicating theater mode
    volatile bool              myToStickPano360; //!< stick to panorama 360 mode
    volatile bool              myToFlipCubeZ6x1; //!< flip Z within 6x1 cubemap input
    volatile bool              myToFlipCubeZ3x2; //!< flip Z within 3x2 cubemap input
    volatile bool              myToSwapJps;      //!< read JPS as Left/Right instead of Right/Left

        private: //! @name no copies, please

    StImageLoader(const StImageLoader& theCopy);
    const StImageLoader& operator=(const StImageLoader& theCopy);

};

#endif //__StImageLoader_h_
