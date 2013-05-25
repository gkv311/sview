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

#include "StImageLoader.h"
#include "StImagePluginInfo.h"
#include "StImageViewerStrings.h"

#include <StStrings/StLangMap.h>
#include <StImage/StJpegParser.h>
#include <StThreads/StThread.h>

const char* StImageLoader::ST_IMAGES_MIME_STRING = ST_IMAGE_PLUGIN_MIME_CHAR;

static SV_THREAD_FUNCTION threadFunction(void* inStImageLoader) {
    StImageLoader* stImageLoader = (StImageLoader* )inStImageLoader;
    stImageLoader->mainLoop();
    return SV_THREAD_RETURN 0;
}

StImageLoader::StImageLoader(const StImageFile::ImageClass     theImageLib,
                             const StHandle<StMsgQueue>&       theMsgQueue,
                             const StHandle<StLangMap>&        theLangMap,
                             const StHandle<StGLTextureQueue>& theTextureQueue)
: myMimeList(ST_IMAGES_MIME_STRING),
  myLangMap(theLangMap),
  myPlayList(1),
  myLoadNextEvent(false),
  mySrcFormat(ST_V_SRC_AUTODETECT),
  myTextureQueue(theTextureQueue),
  myMsgQueue(theMsgQueue),
  myImageLib(theImageLib),
  myToSave(StImageFile::ST_TYPE_NONE),
  myToQuit(false) {
      myPlayList.setExtensions(myMimeList.getExtensionsList());
      myThread = new StThread(threadFunction, (void* )this);
}

StImageLoader::~StImageLoader() {
    myToQuit = true;
    myLoadNextEvent.set(); // stop the thread
    myThread->wait();
    myThread.nullify();
    ///ST_DEBUG_LOG_AT("Destructor done");
}

void StImageLoader::setCompressMemory(const bool theToCompress) {
    myTextureQueue->setCompressMemory(theToCompress);
}

static StString formatError(const StString& theFilePath, const StString& theImgLibDescr) {
    StString aFileName, aFolderName;
    StFileNode::getFolderAndFile(theFilePath, aFolderName, aFileName);
    ST_ERROR_LOG("Can not load image file \"" + theFilePath + "\" (" + theImgLibDescr + ')');
    return StString("Can not load image file:\n\"") + aFileName + "\"\n" + theImgLibDescr;
}

void StImageLoader::processLoadFail(const StString& theErrorDesc) {
    myMsgQueue->pushError(theErrorDesc);
    myTextureQueue->setConnectedStream(false);
    myTextureQueue->clear();
}

bool StImageLoader::loadImage(const StHandle<StFileNode>& theSource,
                              StHandle<StStereoParams>&   theParams) {
    StString fileToLoadPath = theSource->getPath();
    StImageFile::ImageType anImgType = StImageFile::guessImageType(fileToLoadPath, theSource->getMIME());

    StHandle<StImageFile> stImageL = StImageFile::create(myImageLib, anImgType);
    StHandle<StImageFile> stImageR = StImageFile::create(myImageLib, anImgType);
    if(stImageL.isNull() || stImageR.isNull()) {
        processLoadFail("No any image library was found!");
        return false;
    }

    StTimer aLoadTimer(true);
    StFormatEnum aSrcFormatCurr = mySrcFormat;
    if(anImgType == StImageFile::ST_TYPE_MPO
    || anImgType == StImageFile::ST_TYPE_JPEG) {
        // special procedure to divide MPO (Multi Picture Object)
        StJpegParser aJpegParser;
        double anHParallax = 0.0; // parallax in percents
        if(!aJpegParser.read(fileToLoadPath)) {
            processLoadFail(StString("Can not read the file \"") + fileToLoadPath + '\"');
            return false;
        }

        // read image from memory
        StHandle<StJpegParser::Image> anImg1 = aJpegParser.getImage(0);

        StJpegParser::Orient anOrient = anImg1->getOrientation();
        theParams->setZRotateZero((GLfloat )StJpegParser::getRotationAngle(anOrient));
        anImg1->getParallax(anHParallax);
        if(!stImageL->load(fileToLoadPath, StImageFile::ST_TYPE_JPEG,
                           (uint8_t* )anImg1->myData, (int )anImg1->myLength)) {
            processLoadFail(formatError(fileToLoadPath, stImageL->getState()));
            return false;
        }

        StHandle<StJpegParser::Image> anImg2 = aJpegParser.getImage(1);
        if(!anImg2.isNull() && anImgType == StImageFile::ST_TYPE_MPO) {
            // read image from memory
            anImg2->getParallax(anHParallax); // in MPO parallax generally stored ONLY in second frame
            if(!stImageR->load(fileToLoadPath, StImageFile::ST_TYPE_JPEG,
                               (uint8_t* )anImg2->myData, (int )anImg2->myLength)) {
                processLoadFail(formatError(fileToLoadPath, stImageR->getState()));
                stImageL->close();
                stImageL->nullify();
                return false;
            }

            // convert percents to pixels
            theParams->setSeparationNeutral(GLint(anHParallax * stImageR->getSizeX() * 0.01));
        } else if(anImgType == StImageFile::ST_TYPE_MPO) {
            ST_DEBUG_LOG("MPO image \"" + fileToLoadPath + "\" is invalid!");
        }
    } else if(theSource->size() >= 2) {
        StString fileToLoadPathLeft  = theSource->getValue(0)->getPath();
        StString fileToLoadPathRight = theSource->getValue(1)->getPath();

        // loading image with format autodetection
        if(!stImageL->load(fileToLoadPathLeft)) {
            processLoadFail(formatError(fileToLoadPathLeft, stImageL->getState()));
            return false;
        }
        if(!stImageR->load(fileToLoadPathRight)) {
            processLoadFail(formatError(fileToLoadPathRight, stImageR->getState()));
            stImageL->close();
            stImageL->nullify();
            return false;
        }
    } else {
        if(mySrcFormat == ST_V_SRC_AUTODETECT && (anImgType == StImageFile::ST_TYPE_JPS || anImgType == StImageFile::ST_TYPE_PNS)) {
            aSrcFormatCurr = ST_V_SRC_SIDE_BY_SIDE;
        }
        if(!stImageL->load(fileToLoadPath, anImgType)) {
            processLoadFail(formatError(fileToLoadPath, stImageL->getState()));
            return false;
        }
    }
    const double aLoadTimeMSec = aLoadTimer.getElapsedTimeInMilliSec();

#ifdef __ST_DEBUG__
    if(!stImageL->isNull()) {
        ST_DEBUG_LOG(stImageL->getState());
    }
    if(!stImageR->isNull()) {
        ST_DEBUG_LOG(stImageR->getState());
    }
#endif

    // finally push image data in Texture Queue
    while(myTextureQueue->isFull()) {
        StThread::sleep(10);
    }

    myTextureQueue->setConnectedStream(true);
    if(!stImageR->isNull()) {
        myTextureQueue->push(*stImageL, *stImageR, theParams, ST_V_SRC_SEPARATE_FRAMES, 0.0);
    } else {
        myTextureQueue->push(*stImageL, *stImageR, theParams, aSrcFormatCurr, 0.0);
    }

    StHandle<StImageInfo> anImgInfo = new StImageInfo();
    anImgInfo->myId = theParams;

    StString aTitleString, aFolder;
    if(theSource->size() >= 2) {
        StFileNode::getFolderAndFile(theSource->getValue(0)->getPath(), aFolder, aTitleString);
        anImgInfo->myInfo.add(StArgument("Name (L)", aTitleString));
        StFileNode::getFolderAndFile(theSource->getValue(1)->getPath(), aFolder, aTitleString);
        anImgInfo->myInfo.add(StArgument("Name (R)", aTitleString));
    } else {
        StFileNode::getFolderAndFile(fileToLoadPath, aFolder, aTitleString);
        anImgInfo->myInfo.add(StArgument("Name", aTitleString));
    }
    if(!stImageR->isNull()) {
        anImgInfo->myInfo.add(StArgument("Dimensions (L)",  StString() + stImageL->getSizeX()
                                                               + " x " + stImageL->getSizeY()));
        anImgInfo->myInfo.add(StArgument("Dimensions (R)",  StString() + stImageR->getSizeX()
                                                               + " x " + stImageR->getSizeY()));
        anImgInfo->myInfo.add(StArgument("Color Model (L)", stImageL->formatImgColorModel()));
        anImgInfo->myInfo.add(StArgument("Color Model (R)", stImageR->formatImgColorModel()));
    } else {
        anImgInfo->myInfo.add(StArgument("Dimensions",      StString() + stImageL->getSizeX()
                                                               + " x " + stImageL->getSizeY()));
        anImgInfo->myInfo.add(StArgument("Color Model",     stImageL->formatImgColorModel()));
    }
    anImgInfo->myInfo.add(StArgument("Load time", StString(aLoadTimeMSec) + " msec"));
    myImgInfo = anImgInfo;

    // clean up - close opened files and reset memory
    stImageL->close();
    stImageL->nullify();
    stImageR->close();
    stImageR->nullify();

    myTextureQueue->stglSwapFB(0);

    // indicate new file opened
    signals.onLoaded();
    return true;
}

bool StImageLoader::saveImage(const StHandle<StFileNode>&     theSource,
                              const StHandle<StStereoParams>& theParams,
                              StImageFile::ImageType          theImgType) {
    if(theParams.isNull() || theParams.isNull() || theImgType == StImageFile::ST_TYPE_NONE) {
        stInfo(myLangMap->changeValueId(StImageViewerStrings::DIALOG_NOTHING_TO_SAVE,
                                        "Nothing to save!"));
        return false;
    }

    int result = StGLTextureQueue::SNAPSHOT_NO_NEW;
    StImage dataLeft;
    StImage dataRight;
    if(!theParams->isSwapLR()) {
        result = getSnapshot(&dataLeft, &dataRight, true);
    } else {
        result = getSnapshot(&dataRight, &dataLeft, true);
    }

    if(result == StGLTextureQueue::SNAPSHOT_NO_NEW || dataLeft.isNull()) {
        stInfo(myLangMap->changeValueId(StImageViewerStrings::DIALOG_NO_SNAPSHOT,
                                        "Snapshot not available!"));
        return false;
    }
    StHandle<StImageFile> dataResult = StImageFile::create(myImageLib);
    if(dataResult.isNull()) {
        myMsgQueue->pushError("No any image library was found!");
        return false;
    }

    bool toSaveStereo = !dataRight.isNull();
    if(toSaveStereo && dataResult->initSideBySide(dataLeft, dataRight,
                                                  theParams->getSeparationDx(),
                                                  theParams->getSeparationDy())) {
        dataLeft.nullify();
        dataRight.nullify();
    } else {
        dataResult->initWrapper(dataLeft);
    }

    StString title = myLangMap->changeValueId(StImageViewerStrings::DIALOG_SAVE_SNAPSHOT,
                                              "Choose location to save snapshot");
    StMIMEList filter;
    StString saveExt;
    if(toSaveStereo) {
        switch(theImgType) {
            case StImageFile::ST_TYPE_PNG:
                saveExt = ST_PNS_EXT;
                filter.add(StMIME(ST_PNS_MIME, saveExt, ST_PNS_DESC));
                break;
            case StImageFile::ST_TYPE_JPEG:
                saveExt = ST_JPS_EXT;
                filter.add(StMIME(ST_JPS_MIME, saveExt, ST_JPS_DESC));
                break;
            default:
                return false;
        }
    } else {
        switch(theImgType) {
            case StImageFile::ST_TYPE_PNG:
                saveExt = ST_PNG_EXT;
                filter.add(StMIME(ST_PNG_MIME, saveExt, ST_PNG_DESC));
                break;
            case StImageFile::ST_TYPE_JPEG:
                saveExt = ST_JPG_EXT;
                filter.add(StMIME(ST_JPG_MIME, saveExt, ST_JPEG_DESC));
                break;
            default:
                return false;
        }
    }

    StString fileToSave;
    if(StFileNode::openFileDialog(theSource->getFolderPath(), title, filter, fileToSave, true)) {
        if(StFileNode::getExtension(fileToSave) != saveExt) {
            fileToSave += StString('.') + saveExt;
        }

        bool toSave = !StFileNode::isFileExists(fileToSave);
        if(!toSave) {
            if(stQuestion("File already exists!\nOverride the file?")) {
                toSave = StFileNode::removeFile(fileToSave);
                if(!toSave) {
                    myMsgQueue->pushError("Could not remove the file!");
                    return false;
                }
            }
        }

        if(toSave) {
            ST_DEBUG_LOG("Save snapshot to the path '" + fileToSave + '\'');
            StString strSaveState;
            if(!dataResult->save(fileToSave, theImgType)) {
                // TODO (Kirill Gavrilov#7)
                myMsgQueue->pushError(dataResult->getState());
                return false;
            }
            if(!dataResult->getState().isEmpty()) {
                ST_DEBUG_LOG(dataResult->getState());
            }
            // TODO (Kirill Gavrilov#8) - update playlist (append new file)
        }
    }
    return true;
}

void StImageLoader::mainLoop() {
    StHandle<StFileNode> aFileToLoad;
    StHandle<StStereoParams> aFileParams;
    for(;;) {
        myLoadNextEvent.wait();
        if(myToQuit) {
            // exit the loop
            return;
        } else if(myToSave != StImageFile::ST_TYPE_NONE) {
            StImageFile::ImageType anImgType = myToSave;
            myToSave = StImageFile::ST_TYPE_NONE;
            myLoadNextEvent.reset();
            // save current image (set as current in playlist)
            if(myPlayList.getCurrentFile(aFileToLoad, aFileParams)) {
                saveImage(aFileToLoad, aFileParams, anImgType);
            }
        } else {
            // load next image (set as current in playlist)
            myLoadNextEvent.reset();
            if(myPlayList.getCurrentFile(aFileToLoad, aFileParams)) {
                loadImage(aFileToLoad, aFileParams);
            }
        }
    }
}
