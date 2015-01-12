/**
 * Copyright © 2007-2014 Kirill Gavrilov <kirill@sview.ru>
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

class StThread;

struct StImageInfo {

    StHandle<StStereoParams> Id;
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

        public:

    ST_LOCAL const StMIMEList& getMimeList() const {
        return myMimeList;
    }

    ST_LOCAL StImageLoader(const StImageFile::ImageClass     theImageLib,
                           const StHandle<StMsgQueue>&       theMsgQueue,
                           const StHandle<StLangMap>&        theLangMap,
                           const StHandle<StGLTextureQueue>& theTextureQueue,
                           const GLint                       theMaxTexDim);
    ST_LOCAL ~StImageLoader();

    ST_LOCAL inline const StHandle<StGLTextureQueue>& getTextureQueue() const {
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

    ST_LOCAL void doSaveInfo(const StHandle<StImageInfo>& theInfo) {
        myLock.lock();
        myInfoToSave = theInfo;
        myAction     = Action_SaveInfo;
        myLock.unlock();
        myLoadNextEvent.set();
    }

    ST_LOCAL StHandle<StImageInfo> getFileInfo(const StHandle<StStereoParams>& theParams) const {
        myLock.lock();
        StHandle<StImageInfo> anInfo = myImgInfo;
        myLock.unlock();
        return (!anInfo.isNull() && anInfo->Id == theParams) ? anInfo : NULL;
    }

    ST_LOCAL StPlayList& getPlayList() {
        return myPlayList;
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

        public:  //! @name Signals

    struct {
        /**
         * Emit callback Slot on file load.
         */
        StSignal<void ()> onLoaded;

    } signals;

        private:

    ST_LOCAL bool loadImage(const StHandle<StFileNode>& theSource,
                            StHandle<StStereoParams>&   theParams);
    ST_LOCAL bool saveImage(const StHandle<StFileNode>& theSource,
                            const StHandle<StStereoParams>& theParams,
                            StImageFile::ImageType theImgType);

    ST_LOCAL bool saveImageInfo(const StHandle<StImageInfo>& theInfo);

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
    ST_LOCAL void metadataFromExif(const StHandle<StExifDir>& theDir,
                                   StHandle<StImageInfo>&     theInfo);

    ST_LOCAL const StString& tr(const size_t theId) const {
        return myLangMap->getValue(theId);
    }

        private:

    const StMIMEList           myMimeList;
    StHandle<StThread>         myThread;        //!< main loop thread
    StHandle<StLangMap>        myLangMap;       //!< translations dictionary
    StPlayList                 myPlayList;      //!< play list
    mutable StMutex            myLock;          //!< lock to access not thread-safe properties
    StCondition                myLoadNextEvent;
    StFormat                   myStFormatByUser;//!< target source format (auto-detect by default)
    GLint                      myMaxTexDim;     //!< value for GL_MAX_TEXTURE_SIZE
    StHandle<StGLTextureQueue> myTextureQueue;  //!< decoded frames queue
    StHandle<StImageInfo>      myImgInfo;       //!< info about currently loaded image
    StHandle<StImageInfo>      myInfoToSave;    //!< modified info to be saved
    StHandle<StMsgQueue>       myMsgQueue;      //!< messages queue

    volatile StImageFile::ImageClass myImageLib;
    volatile Action            myAction;

        private: //! @name no copies, please

    StImageLoader(const StImageLoader& theCopy);
    const StImageLoader& operator=(const StImageLoader& theCopy);

};

#endif //__StImageLoader_h_
