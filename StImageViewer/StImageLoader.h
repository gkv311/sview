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

#ifndef __StImageLoader_h_
#define __StImageLoader_h_

#include <StStrings/StString.h>
#include <StFile/StMIMEList.h>
#include <StGL/StPlayList.h>
#include <StGLStereo/StGLTextureQueue.h>
#include <StImage/StImageFile.h>
#include <StSlots/StSignal.h>

class StLangMap;

struct ST_LOCAL StImageInfo {

    StHandle<StStereoParams> myId;
    StArgumentsMap           myInfo;

};

/**
 * Auxiliary class to load images from dedicated thread.
 */
class ST_LOCAL StImageLoader {

        public:

    static const char* ST_IMAGES_MIME_STRING;

    const StMIMEList& getMimeList() const {
        return myMimeList;
    }

    StImageLoader(const StImageFile::ImageClass     theImageLib,
                  const StHandle<StLangMap>&        theLangMap,
                  const StHandle<StGLTextureQueue>& theTextureQueue);
    ~StImageLoader();

    void mainLoop();

    void doLoadNext() {
        myLoadNextEvent.set();
    }

    void doSaveImageAs(const size_t theImgType) {
        myToSave = StImageFile::ImageType(theImgType);
        myLoadNextEvent.set();
    }

    StHandle<StImageInfo> getFileInfo(const StHandle<StStereoParams>& theParams) const {
        StHandle<StImageInfo> anInfo = myImgInfo;
        return (!anInfo.isNull() && anInfo->myId == theParams) ? anInfo : NULL;
    }

    StPlayList& getPlayList() {
        return myPlayList;
    }

    StFormatEnum getSrcFormat() const {
        return mySrcFormat;
    }

    void setSrcFormat(const StFormatEnum theSrcFormat) {
        mySrcFormat = theSrcFormat;
    }

    void setImageLib(const StImageFile::ImageClass theImageLib) {
        myImageLib = theImageLib;
    }

    /**
     * Release unused memory as fast as possible.
     */
    void setCompressMemory(const bool theToCompress);

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

    bool loadImage(const StHandle<StFileNode>& theSource,
                   StHandle<StStereoParams>&   theParams);
    bool saveImage(const StHandle<StFileNode>& theSource,
                   const StHandle<StStereoParams>& theParams,
                   StImageFile::ImageType theImgType);

    int getSnapshot(StImage* outDataLeft, StImage* outDataRight, bool isForce = false) {
        return myTextureQueue->getSnapshot(outDataLeft, outDataRight, isForce);
    }

        private:

    const StMIMEList           myMimeList;
    StHandle<StThread>         myThread;        //!< main loop thread
    StHandle<StLangMap>        myLangMap;       //!< translations dictionary
    StPlayList                 myPlayList;      //!< play list
    StEvent                    myLoadNextEvent;
    StFormatEnum               mySrcFormat;     //!< target source format (auto-detect by default)
    StHandle<StGLTextureQueue> myTextureQueue;  //!< decoded frames queue
    StHandle<StImageInfo>      myImgInfo;       //!< info about currently loaded image

    volatile StImageFile::ImageClass myImageLib;
    volatile StImageFile::ImageType  myToSave;
    volatile bool              myToQuit;

        private:

    // no copies, please
    StImageLoader(const StImageLoader& theCopy);
    const StImageLoader& operator=(const StImageLoader& theCopy);

};

#endif //__StImageLoader_h_
