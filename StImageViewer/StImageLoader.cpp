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

#include "StImageLoader.h"
#include "StImagePluginInfo.h"
#include "StImageViewerStrings.h"
#include "StImageViewerGUI.h"

#include <StAV/StAVImage.h>
#include <StThreads/StThread.h>

using namespace StImageViewerStrings;

const char* StImageLoader::ST_IMAGES_MIME_STRING = ST_IMAGE_PLUGIN_MIME_CHAR;

namespace {

    static StString formatError(const StString& theFilePath,
                                const StString& theImgLibDescr) {
        StString aFileName, aFolderName;
        StFileNode::getFolderAndFile(theFilePath, aFolderName, aFileName);
        ST_ERROR_LOG("Can not load image file \"" + theFilePath + "\" (" + theImgLibDescr + ')');
        return StString("Can not load image file:\n\"") + aFileName + "\"\n" + theImgLibDescr;
    }

    static SV_THREAD_FUNCTION threadFunction(void* theImageLoader) {
        StImageLoader* anImageLoader = (StImageLoader* )theImageLoader;
        anImageLoader->mainLoop();
        return SV_THREAD_RETURN 0;
    }

}

StImageLoader::StImageLoader(const StImageFile::ImageClass     theImageLib,
                             const StHandle<StMsgQueue>&       theMsgQueue,
                             const StHandle<StLangMap>&        theLangMap,
                             const StHandle<StGLTextureQueue>& theTextureQueue,
                             const GLint                       theMaxTexDim)
: myMimeList(ST_IMAGES_MIME_STRING),
  myLangMap(theLangMap),
  myPlayList(1),
  myLoadNextEvent(false),
  myStFormatByUser(ST_V_SRC_AUTODETECT),
  myMaxTexDim(theMaxTexDim),
  myTextureQueue(theTextureQueue),
  myMsgQueue(theMsgQueue),
  myImageLib(theImageLib),
  myAction(Action_NONE) {
      myPlayList.setExtensions(myMimeList.getExtensionsList());
      myThread = new StThread(threadFunction, (void* )this);
}

StImageLoader::~StImageLoader() {
    myAction = Action_Quit;
    myLoadNextEvent.set(); // stop the thread
    myThread->wait();
    myThread.nullify();
}

void StImageLoader::setCompressMemory(const bool theToCompress) {
    myTextureQueue->setCompressMemory(theToCompress);
}

void StImageLoader::processLoadFail(const StString& theErrorDesc) {
    myMsgQueue->pushError(theErrorDesc);
    myTextureQueue->setConnectedStream(false);
    myTextureQueue->clear();
}

void StImageLoader::metadataFromExif(const StHandle<StExifDir>& theDir,
                                     StHandle<StImageInfo>&     theInfo) {
    if(theDir.isNull()) {
        return;
    }

    if(!theDir->CameraMaker.isEmpty()) {
        StDictEntry& anEntry  = theInfo->Info.addChange("Exif.Image.Make");
        anEntry.changeValue() = theDir->CameraMaker;
    }
    if(!theDir->CameraModel.isEmpty()) {
        StDictEntry& anEntry  = theInfo->Info.addChange("Exif.Image.Model");
        anEntry.changeValue() = theDir->CameraModel;
    }
    if(!theDir->UserComment.isEmpty()) {
        StDictEntry& anEntry  = theInfo->Info.addChange("Exif.UserComment");
        anEntry.changeValue() = theDir->UserComment;
    }

    for(size_t anExifId = 0; anExifId < theDir->SubDirs.size(); ++anExifId) {
        metadataFromExif(theDir->SubDirs[anExifId], theInfo);
    }
}

inline StHandle<StImage> scaledImage(StHandle<StImageFile>& theRef,
                                     const size_t           theMaxSize) {
    if(theRef->isNull()) {
        return theRef;
    } else if(theRef->getSizeX() <= theMaxSize
           && theRef->getSizeY() <= theMaxSize) {
        return theRef;
    }

    StHandle<StImage> anImage = new StImage();
    const size_t aSizeX = stMin(theRef->getSizeX(), theMaxSize);
    const size_t aSizeY = stMin(theRef->getSizeY(), theMaxSize);
    if(!anImage->initTrashLimited(*theRef, aSizeX, aSizeY)
    || !StAVImage::resize(*theRef, *anImage)) {
        ST_ERROR_LOG("Scale failed!");
        return theRef;
    }
    theRef->close();
    return anImage;
}

bool StImageLoader::loadImage(const StHandle<StFileNode>& theSource,
                              StHandle<StStereoParams>&   theParams) {
    const StString               aFilePath = theSource->getPath();
    const StImageFile::ImageType anImgType = StImageFile::guessImageType(aFilePath, theSource->getMIME());

    StHandle<StImageFile> anImageFileL = StImageFile::create(myImageLib, anImgType);
    StHandle<StImageFile> anImageFileR = StImageFile::create(myImageLib, anImgType);
    if(anImageFileL.isNull()
    || anImageFileR.isNull()) {
        processLoadFail("No any image library was found!");
        return false;
    }

    StHandle<StImageInfo> anImgInfo = new StImageInfo();
    anImgInfo->Id        = theParams;
    anImgInfo->Path      = aFilePath;
    anImgInfo->ImageType = anImgType;
    anImgInfo->IsSavable = false;

    StString aTitleString, aFolder;
    if(theSource->size() >= 2) {
        StString aTitleString2;
        StFileNode::getFolderAndFile(theSource->getValue(0)->getPath(), aFolder, aTitleString);
        StFileNode::getFolderAndFile(theSource->getValue(1)->getPath(), aFolder, aTitleString2);
        anImgInfo->Info.add(StArgument(tr(INFO_FILE_NAME),
                                       aTitleString  + " " + tr(INFO_LEFT) + "\n"
                                     + aTitleString2 + " " + tr(INFO_RIGHT)));
    } else {
        StFileNode::getFolderAndFile(aFilePath, aFolder, aTitleString);
        anImgInfo->Info.add(StArgument(tr(INFO_FILE_NAME), aTitleString));
    }

    StTimer aLoadTimer(true);
    StFormatEnum aSrcFormatCurr = myStFormatByUser;
    if(anImgType == StImageFile::ST_TYPE_MPO
    || anImgType == StImageFile::ST_TYPE_JPEG
    || anImgType == StImageFile::ST_TYPE_JPS) {
        // special procedure to divide MPO (Multi Picture Object)
        StJpegParser aParser;
        double anHParallax = 0.0; // parallax in percents
        const bool isParsed = aParser.readFile(aFilePath);

        StHandle<StJpegParser::Image> anImg1, anImg2;
        size_t aMaxSizeX = 0;
        size_t aMaxSizeY = 0;
        for(StHandle<StJpegParser::Image> anImgIter = aParser.getImage(0); !anImgIter.isNull();
            anImgIter = anImgIter->Next) {
            aMaxSizeX = stMax(aMaxSizeX, anImgIter->SizeX);
            aMaxSizeY = stMax(aMaxSizeY, anImgIter->SizeY);
        }

        int anImgCounter = 1;
        for(StHandle<StJpegParser::Image> anImgIter = aParser.getImage(0); !anImgIter.isNull();
            anImgIter = anImgIter->Next, ++anImgCounter) {
            if(anImgIter->SizeX == aMaxSizeX
            && anImgIter->SizeY == aMaxSizeY) {
                if(anImg1.isNull()) {
                    anImg1 = anImgIter;
                    continue;
                } else if(anImg2.isNull()) {
                    anImg2 = anImgIter;
                    continue;
                }
            }
            anImgInfo->Info.add(StArgument(tr(INFO_DIMENSIONS) + (" (") + anImgCounter + ")",
                                           StString() + anImgIter->SizeX + " x " + anImgIter->SizeY));
        }

        // copy metadata
        if(!aParser.getComment().isEmpty()) {
            StDictEntry& anEntry  = anImgInfo->Info.addChange("Jpeg.Comment");
            anEntry.changeValue() = aParser.getComment();
        }
        if(!aParser.getJpsComment().isEmpty()) {
            StDictEntry& anEntry  = anImgInfo->Info.addChange("Jpeg.JpsComment");
            anEntry.changeValue() = aParser.getJpsComment();
        }
        if(!anImg1.isNull()) {
            for(size_t anExifId = 0; anExifId < anImg1->Exif.size(); ++anExifId) {
                metadataFromExif(anImg1->Exif[anExifId], anImgInfo);
            }
            const StString aTime = anImg1->getDateTime();
            if(!aTime.isEmpty()) {
                StDictEntry& anEntry  = anImgInfo->Info.addChange("Exif.Image.DateTime");
                anEntry.changeValue() = aTime;
            }
        }
        if(myStFormatByUser == ST_V_SRC_AUTODETECT
        && aParser.getSrcFormat() != ST_V_SRC_AUTODETECT) {
            aSrcFormatCurr = aParser.getSrcFormat();
        }

        //aParser.fillDictionary(anImgInfo->Info, true);
        if(!isParsed) {
            processLoadFail(StString("Can not read the file \"") + aFilePath + '\"');
            return false;
        }

        anImgInfo->IsSavable = anImg2.isNull();
        anImgInfo->StInfoStream = aParser.getSrcFormat();
        if(anImgInfo->StInfoStream != ST_V_SRC_AUTODETECT) {
            StDictEntry& anEntry  = anImgInfo->Info.addChange("Jpeg.JpsStereo");
            anEntry.changeValue() = tr(StImageViewerGUI::trSrcFormatId(anImgInfo->StInfoStream));
        }

        // read image from memory
        const StJpegParser::Orient anOrient = anImg1->getOrientation();
        theParams->setZRotateZero((GLfloat )StJpegParser::getRotationAngle(anOrient));
        anImg1->getParallax(anHParallax);
        if(!anImageFileL->load(aFilePath, StImageFile::ST_TYPE_JPEG,
                               (uint8_t* )anImg1->Data, (int )anImg1->Length)
        && !anImageFileL->load(aFilePath, StImageFile::ST_TYPE_JPEG,
                               (uint8_t* )aParser.getBuffer(), (int )aParser.getSize())) {
            processLoadFail(formatError(aFilePath, anImageFileL->getState()));
            return false;
        }

        if(!anImg2.isNull()) {
            // read image from memory
            anImg2->getParallax(anHParallax); // in MPO parallax generally stored ONLY in second frame
            if(!anImageFileR->load(aFilePath, StImageFile::ST_TYPE_JPEG,
                                   (uint8_t* )anImg2->Data, (int )anImg2->Length)) {
                processLoadFail(formatError(aFilePath, anImageFileR->getState()));
                return false;
            }

            // convert percents to pixels
            const GLint aParallaxPx = GLint(anHParallax * anImageFileR->getSizeX() * 0.01);
            if(aParallaxPx != 0) {
                StDictEntry& anEntry  = anImgInfo->Info.addChange("Exif.Fujifilm.Parallax");
                anEntry.changeValue() = StString(anHParallax);
            }
            theParams->setSeparationNeutral(aParallaxPx);
        } else if(anImgType == StImageFile::ST_TYPE_MPO) {
            ST_DEBUG_LOG("MPO image \"" + aFilePath + "\" is invalid!");
        }
    } else if(theSource->size() >= 2) {
        const StString aFilePathLeft  = theSource->getValue(0)->getPath();
        const StString aFilePathRight = theSource->getValue(1)->getPath();

        // loading image with format autodetection
        if(!anImageFileL->load(aFilePathLeft)) {
            processLoadFail(formatError(aFilePathLeft, anImageFileL->getState()));
            return false;
        }
        if(!anImageFileR->load(aFilePathRight)) {
            processLoadFail(formatError(aFilePathRight, anImageFileR->getState()));
            return false;
        }
    } else {
        if(!anImageFileL->load(aFilePath, anImgType)) {
            processLoadFail(formatError(aFilePath, anImageFileL->getState()));
            return false;
        }

        anImgInfo->StInfoStream = anImageFileL->getFormat();
        if(myStFormatByUser == ST_V_SRC_AUTODETECT) {
            aSrcFormatCurr = anImgInfo->StInfoStream;
        }
    }
    const double aLoadTimeMSec = aLoadTimer.getElapsedTimeInMilliSec();

    // scale down image if it does not fit texture limits
    StHandle<StImage> anImageL = scaledImage(anImageFileL, size_t(myMaxTexDim));
    StHandle<StImage> anImageR = scaledImage(anImageFileR, size_t(myMaxTexDim));
#ifdef __ST_DEBUG__
    const double aScaleTimeMSec = aLoadTimer.getElapsedTimeInMilliSec() - aLoadTimeMSec;
    if(anImageL != anImageFileL) {
        ST_DEBUG_LOG("Image is downscaled to fit texture limits in " + aScaleTimeMSec + " ms!");
    }
#endif

    // detect information from file name
    bool isAnamorphByName = false;
    anImgInfo->StInfoFileName = st::formatFromName(aTitleString, isAnamorphByName);
    if(aSrcFormatCurr == ST_V_SRC_AUTODETECT
    && anImgInfo->StInfoFileName != ST_V_SRC_AUTODETECT) {
        aSrcFormatCurr = anImgInfo->StInfoFileName;
    }

    // try detection based on aspect ratio
    /*if(aSrcFormatCurr == ST_V_SRC_AUTODETECT
    && !anImageL->isNull()) {
        aSrcFormatCurr = st::formatFromRatio(anImageL->getRatio());
    }*/

#ifdef __ST_DEBUG__
    if(!anImageFileL->isNull()) {
        ST_DEBUG_LOG(anImageFileL->getState());
    }
    if(!anImageFileR->isNull()) {
        ST_DEBUG_LOG(anImageFileR->getState());
    }
#endif

    // finally push image data in Texture Queue
    while(myTextureQueue->isFull()) {
        StThread::sleep(10);
    }

    myTextureQueue->setConnectedStream(true);
    if(!anImageR->isNull()) {
        myTextureQueue->push(*anImageL, *anImageR, theParams, ST_V_SRC_SEPARATE_FRAMES, 0.0);
    } else {
        myTextureQueue->push(*anImageL, *anImageR, theParams, aSrcFormatCurr, 0.0);
    }

    if(!stAreEqual(anImageFileL->getPixelRatio(), 1.0f, 0.001f)) {
        anImgInfo->Info.add(StArgument(tr(INFO_PIXEL_RATIO),
                                       StString(anImageFileL->getPixelRatio())));
    }
    const StString aModelL = anImageFileL->formatImgColorModel();
    if(!anImageFileR->isNull()) {
        anImgInfo->Info.add(StArgument(tr(INFO_DIMENSIONS), StString()
                                     + anImageFileL->getSizeX() + " x " + anImageFileL->getSizeY() + " " + tr(INFO_LEFT) + "\n"
                                     + anImageFileR->getSizeX() + " x " + anImageFileR->getSizeY() + " " + tr(INFO_RIGHT)));
        const StString aModelR = anImageFileR->formatImgColorModel();
        if(aModelL == aModelR) {
            anImgInfo->Info.add(StArgument(tr(INFO_COLOR_MODEL), aModelL));
        } else {
            anImgInfo->Info.add(StArgument(tr(INFO_COLOR_MODEL),
                                aModelL + " " + tr(INFO_LEFT) + "\n"
                              + aModelR + " " + tr(INFO_RIGHT)));
        }
    } else {
        anImgInfo->Info.add(StArgument(tr(INFO_DIMENSIONS), StString()
                                     + anImageFileL->getSizeX() + " x " + anImageFileL->getSizeY()));
        anImgInfo->Info.add(StArgument(tr(INFO_COLOR_MODEL),
                                       aModelL));
    }
    anImgInfo->Info.add(StArgument(tr(INFO_LOAD_TIME), StString(aLoadTimeMSec) + " " + tr(INFO_TIME_MSEC)));
    myLock.lock();
    myImgInfo = anImgInfo;
    myLock.unlock();

    // clean up - close opened files and reset memory
    anImageL.nullify();
    anImageR.nullify();
    anImageFileL.nullify();
    anImageFileR.nullify();

    myTextureQueue->stglSwapFB(0);

    // indicate new file opened
    signals.onLoaded();
    return true;
}

bool StImageLoader::saveImage(const StHandle<StFileNode>&     theSource,
                              const StHandle<StStereoParams>& theParams,
                              StImageFile::ImageType          theImgType) {
    if(theParams.isNull()
    || theImgType == StImageFile::ST_TYPE_NONE) {
        myMsgQueue->pushError(tr(DIALOG_NOTHING_TO_SAVE));
        return false;
    }

    int aResult = StGLTextureQueue::SNAPSHOT_NO_NEW;
    StImage aDataLeft, aDataRight;
    if(!theParams->ToSwapLR) {
        aResult = getSnapshot(&aDataLeft, &aDataRight, true);
    } else {
        aResult = getSnapshot(&aDataRight, &aDataLeft, true);
    }

    if(aResult == StGLTextureQueue::SNAPSHOT_NO_NEW
    || aDataLeft.isNull()) {
        myMsgQueue->pushInfo(tr(DIALOG_NO_SNAPSHOT));
        return false;
    }
    StHandle<StImageFile> aDataResult = StImageFile::create(myImageLib);
    if(aDataResult.isNull()) {
        myMsgQueue->pushError(stCString("No any image library was found!"));
        return false;
    }

    const bool toSaveStereo = !aDataRight.isNull();
    if(toSaveStereo
    && aDataResult->initSideBySide(aDataLeft, aDataRight,
                                   theParams->getSeparationDx(),
                                   theParams->getSeparationDy())) {
        aDataLeft.nullify();
        aDataRight.nullify();
    } else {
        aDataResult->initWrapper(aDataLeft);
    }

    const StString& aTitle = myLangMap->getValue(StImageViewerStrings::DIALOG_SAVE_SNAPSHOT);
    StMIMEList aFilter;
    StString aSaveExt;
    if(toSaveStereo) {
        switch(theImgType) {
            case StImageFile::ST_TYPE_PNG:
                aSaveExt = ST_PNS_EXT;
                aFilter.add(StMIME(ST_PNS_MIME, aSaveExt, ST_PNS_DESC));
                break;
            case StImageFile::ST_TYPE_JPEG:
                aSaveExt = ST_JPS_EXT;
                aFilter.add(StMIME(ST_JPS_MIME, aSaveExt, ST_JPS_DESC));
                break;
            default:
                return false;
        }
    } else {
        switch(theImgType) {
            case StImageFile::ST_TYPE_PNG:
                aSaveExt = ST_PNG_EXT;
                aFilter.add(StMIME(ST_PNG_MIME, aSaveExt, ST_PNG_DESC));
                break;
            case StImageFile::ST_TYPE_JPEG:
                aSaveExt = ST_JPG_EXT;
                aFilter.add(StMIME(ST_JPG_MIME, aSaveExt, ST_JPEG_DESC));
                break;
            default:
                return false;
        }
    }

    StString aFileToSave;
    if(StFileNode::openFileDialog(theSource->getFolderPath(), aTitle, aFilter, aFileToSave, true)) {
        if(StFileNode::getExtension(aFileToSave) != aSaveExt) {
            aFileToSave += StString('.') + aSaveExt;
        }

        bool toSave = !StFileNode::isFileExists(aFileToSave);
        if(!toSave) {
            if(stQuestion("File already exists!\nOverride the file?")) {
                toSave = StFileNode::removeFile(aFileToSave);
                if(!toSave) {
                    myMsgQueue->pushError(stCString("Could not remove the file!"));
                    return false;
                }
            }
        }

        if(toSave) {
            ST_DEBUG_LOG("Save snapshot to the path '" + aFileToSave + '\'');
            StString strSaveState;
            if(!aDataResult->save(aFileToSave, theImgType,
                                  toSaveStereo ? ST_V_SRC_SIDE_BY_SIDE : ST_V_SRC_AUTODETECT)) {
                // TODO (Kirill Gavrilov#7)
                myMsgQueue->pushError(aDataResult->getState());
                return false;
            }
            if(!aDataResult->getState().isEmpty()) {
                ST_DEBUG_LOG(aDataResult->getState());
            }
            // TODO (Kirill Gavrilov#8) - update playlist (append new file)
        }
    }
    return true;
}

bool StImageLoader::saveImageInfo(const StHandle<StImageInfo>& theInfo) {
    if(theInfo.isNull()
    || theInfo->Path.isEmpty()) {
        myMsgQueue->pushError(tr(DIALOG_NOTHING_TO_SAVE));
        return false;
    } else if(theInfo->ImageType != StImageFile::ST_TYPE_JPEG
           && theInfo->ImageType != StImageFile::ST_TYPE_JPS) {
        myMsgQueue->pushError(stCString("Operation is unavailable for this image type"));
        return false;
    }

    StFormatEnum aSrcFormat = theInfo->Id->StereoFormat;
    if(theInfo->Id->ToSwapLR) {
        aSrcFormat = st::formatReversed(aSrcFormat);
    }

    StJpegParser aParser;
    if(!aParser.readFile(theInfo->Path)) {
        myMsgQueue->pushError(tr(DIALOG_NOTHING_TO_SAVE));
        return false;
    }

    aParser.setupJps(aSrcFormat);
    if(!aParser.saveFile(theInfo->Path)) {
        myMsgQueue->pushError(StString("File can not be saved at path '") + theInfo->Path + "'!");
        return false;
    }
    return true;
}

void StImageLoader::mainLoop() {
    StHandle<StFileNode>     aFileToLoad;
    StHandle<StStereoParams> aFileParams;
    for(;;) {
        myLoadNextEvent.wait();
        switch(myAction) {
            case Action_Quit: {
                // exit the loop
                return;
            }
            case Action_SaveJPEG:
            case Action_SavePNG: {
                StImageFile::ImageType anImgType = (myAction == Action_SaveJPEG)
                                                 ? StImageFile::ST_TYPE_JPEG
                                                 : StImageFile::ST_TYPE_PNG;
                myAction = Action_NONE;
                myLoadNextEvent.reset();
                // save current image (set as current in playlist)
                if(myPlayList.getCurrentFile(aFileToLoad, aFileParams)) {
                    saveImage(aFileToLoad, aFileParams, anImgType);
                }
                break;
            }
            case Action_SaveInfo: {
                myLock.lock();
                StHandle<StImageInfo> anInfo = myInfoToSave;
                myInfoToSave.nullify();
                myAction = Action_NONE;
                myLock.unlock();
                myLoadNextEvent.reset();
                if(!saveImageInfo(anInfo)) {
                    break;
                }
                // re-load image file
            }
            case Action_NONE:
            default: {
                // load next image (set as current in playlist)
                myLoadNextEvent.reset();
                if(myPlayList.getCurrentFile(aFileToLoad, aFileParams)) {
                    loadImage(aFileToLoad, aFileParams);
                }
                break;
            }
        }
    }
}
