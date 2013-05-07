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

#ifndef __StImageLoader_h_
#define __StImageLoader_h_

#include <StStrings/StString.h>
#include <StFile/StMIMEList.h>
#include <StGL/StPlayList.h>
#include <StGLStereo/StGLTextureQueue.h>
#include <StImage/StImageFile.h>
#include <StSlots/StSignal.h>

class StLangMap;

struct StImageInfo {

    StHandle<StStereoParams> myId;
    StArgumentsMap           myInfo;

};

/**
 * Auxiliary class to load images from dedicated thread.
 */
class StImageLoader {

        public:

    static const char* ST_IMAGES_MIME_STRING;

    ST_LOCAL const StMIMEList& getMimeList() const {
        return myMimeList;
    }

    ST_LOCAL StImageLoader(const StImageFile::ImageClass     theImageLib,
                           const StHandle<StLangMap>&        theLangMap,
                           const StHandle<StGLTextureQueue>& theTextureQueue);
    ST_LOCAL ~StImageLoader();

    ST_LOCAL inline const StHandle<StGLTextureQueue>& getTextureQueue() const {
        return myTextureQueue;
    }

    ST_LOCAL void mainLoop();

    ST_LOCAL void doRelease() {
        signals.onError.disconnect();
    }

    ST_LOCAL void doLoadNext() {
        myLoadNextEvent.set();
    }

    ST_LOCAL void doSaveImageAs(const size_t theImgType) {
        myToSave = StImageFile::ImageType(theImgType);
        myLoadNextEvent.set();
    }

    ST_LOCAL StHandle<StImageInfo> getFileInfo(const StHandle<StStereoParams>& theParams) const {
        StHandle<StImageInfo> anInfo = myImgInfo;
        return (!anInfo.isNull() && anInfo->myId == theParams) ? anInfo : NULL;
    }

    ST_LOCAL StPlayList& getPlayList() {
        return myPlayList;
    }

    ST_LOCAL StFormatEnum getSrcFormat() const {
        return mySrcFormat;
    }

    ST_LOCAL void setSrcFormat(const StFormatEnum theSrcFormat) {
        mySrcFormat = theSrcFormat;
    }

    ST_LOCAL void setImageLib(const StImageFile::ImageClass theImageLib) {
        myImageLib = theImageLib;
    }

    /**
     * Release unused memory as fast as possible.
     */
    ST_LOCAL void setCompressMemory(const bool theToCompress);

        public:  //!< Signals

    struct {
        /**
         * Emit callback Slot on file load.
         */
        StSignal<void ()> onLoaded;

        /**
         * Emit callback Slot on image load error.
         * @param theUserData (const StString& ) - error description.
         */
        StSignal<void (const StString& )> onError;
    } signals;

        private:

    ST_LOCAL bool loadImage(const StHandle<StFileNode>& theSource,
                            StHandle<StStereoParams>&   theParams);
    ST_LOCAL bool saveImage(const StHandle<StFileNode>& theSource,
                            const StHandle<StStereoParams>& theParams,
                            StImageFile::ImageType theImgType);

    ST_LOCAL int getSnapshot(StImage* outDataLeft, StImage* outDataRight, bool isForce = false) {
        return myTextureQueue->getSnapshot(outDataLeft, outDataRight, isForce);
    }

    /**
     * Auxiliary method to process image load error.
     */
    ST_LOCAL void processLoadFail(const StString& theErrorDesc);

        private:

    const StMIMEList           myMimeList;
    StHandle<StThread>         myThread;        //!< main loop thread
    StHandle<StLangMap>        myLangMap;       //!< translations dictionary
    StPlayList                 myPlayList;      //!< play list
    StCondition                myLoadNextEvent;
    StFormatEnum               mySrcFormat;     //!< target source format (auto-detect by default)
    StHandle<StGLTextureQueue> myTextureQueue;  //!< decoded frames queue
    StHandle<StImageInfo>      myImgInfo;       //!< info about currently loaded image

    volatile StImageFile::ImageClass myImageLib;
    volatile StImageFile::ImageType  myToSave;
    volatile bool              myToQuit;

        private: //! @name no copies, please

    StImageLoader(const StImageLoader& theCopy);
    const StImageLoader& operator=(const StImageLoader& theCopy);

};

#endif //__StImageLoader_h_
