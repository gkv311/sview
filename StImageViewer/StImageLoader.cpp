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

#include "StImageLoader.h"
#include "StImagePluginInfo.h"
#include "StImageViewerStrings.h"
#include "StImageViewerGUI.h"

#include "../StMoviePlayer/StMoviePlayerInfo.h"

#include <StAV/StAVImage.h>
#include <StThreads/StThread.h>

using namespace StImageViewerStrings;

const char* StImageLoader::ST_IMAGES_MIME_STRING = ST_IMAGE_PLUGIN_MIME_CHAR;
const char* StImageLoader::ST_VIDEOS_MIME_STRING = ST_VIDEO_PLUGIN_MIME_CHAR;

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

StImageLoader::StImageLoader(const StImageFile::ImageClass      theImageLib,
                             const StHandle<StResourceManager>& theResMgr,
                             const StHandle<StMsgQueue>&        theMsgQueue,
                             const StHandle<StLangMap>&         theLangMap,
                             const StHandle<StPlayList>&        thePlayList,
                             const StHandle<StGLTextureQueue>&  theTextureQueue,
                             const GLint                        theMaxTexDim)
: myMimeList(ST_IMAGES_MIME_STRING),
  myVideoMimeList(ST_VIDEOS_MIME_STRING),
  myResMgr(theResMgr),
  myLangMap(theLangMap),
  myPlayList(thePlayList),
  myLoadNextEvent(false),
  myStFormatByUser(StFormat_AUTO),
  myMaxTexDim(theMaxTexDim),
  myTextureQueue(theTextureQueue),
  myMsgQueue(theMsgQueue),
  myImageLib(theImageLib),
  myAction(Action_NONE),
  myIsTheaterMode(false),
  myToStickPano360(false),
  myToFlipCubeZ6x1(false),
  myToFlipCubeZ3x2(false),
  myToSwapJps(false) {
      myPlayList->setExtensions(myMimeList.getExtensionsList());
      myThread = new StThread(threadFunction, (void* )this, "StImageLoader");
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
                                     const StGLDeviceCaps&  theCaps,
                                     const size_t           theMaxSizeX,
                                     const size_t           theMaxSizeY,
                                     StCubemap              theCubemap,
                                     const size_t*          theCubeCoeffs,
                                     StPairRatio            thePairRatio) {
    if(theRef->isNull()) {
        return theRef;
    }

    const bool toConvertRgb = !theCaps.isSupportedFormat(theRef->getPlane().getFormat());
    StImagePlane::ImgFormat anRgbImgFormat = StImagePlane::ImgRGB;
    if(toConvertRgb) {
        switch(theRef->getColorModel()) {
            case StImage::ImgColor_RGB:
                anRgbImgFormat = StImagePlane::ImgRGB;
                break;
            case StImage::ImgColor_RGBA:
                anRgbImgFormat = StImagePlane::ImgRGBA;
                break;
            case StImage::ImgColor_GRAY:
                anRgbImgFormat = StImagePlane::ImgGray;
                break;
            default:
                anRgbImgFormat = StImagePlane::ImgRGB;
                break;
        }
    }

    if(theCubemap == StCubemap_Packed) { // skip scaling for StCubemap_PackedEAC
        size_t aSizesY[4] = {};
        bool toResize = false;
        size_t aMulX = (thePairRatio == StPairRatio_HalfWidth)  ? 2 : 1;
        size_t aMulY = (thePairRatio == StPairRatio_HalfHeight) ? 2 : 1;
        for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
            const StImagePlane& aPlane = theRef->getPlane(aPlaneId);
            aSizesY[aPlaneId] = stMin(aPlane.getSizeX() / (theCubeCoeffs[0] * aMulX), theMaxSizeY);
            if(aSizesY[aPlaneId] * theCubeCoeffs[1] != aPlane.getSizeY() * aMulY) {
                const size_t aSizeAlt = stMin(aPlane.getSizeY() / theCubeCoeffs[1], theMaxSizeY) * aMulY;
                aSizesY[aPlaneId] = stMax(aSizesY[aPlaneId], aSizeAlt);
                toResize = true;
            }
        }
        if(!toResize && !toConvertRgb) {
            return theRef;
        }

        StHandle<StImage> anImage = new StImage();
        if(toConvertRgb) {
            anImage->setColorModelPacked(anRgbImgFormat);
            anImage->setColorScale(StImage::ImgScale_Full);
        } else {
            anImage->setColorModel(theRef->getColorModel());
            anImage->setColorScale(theRef->getColorScale());
        }

        double aRatioX = double(aSizesY[0] * theCubeCoeffs[0] * aMulX) / double(theRef->getSizeX());
        double aRatioY = double(aSizesY[0] * theCubeCoeffs[1] * aMulY) / double(theRef->getSizeY());
        anImage->setPixelRatio(float(double(theRef->getPixelRatio()) * aRatioY / aRatioX));
        for(size_t aPlaneId = 0; aPlaneId < 4; ++aPlaneId) {
            if(toConvertRgb) {
                if(aPlaneId != 0) {
                    continue;
                }
                if(!anImage->changePlane(0).initTrash(anRgbImgFormat,
                                                      aSizesY[0] * theCubeCoeffs[0] * aMulX,
                                                      aSizesY[0] * theCubeCoeffs[1] * aMulY)) {
                    ST_ERROR_LOG("Scale failed!");
                    return theRef;
                }
                continue;
            }

            const StImagePlane& aFromPlane = theRef->getPlane(aPlaneId);
            if(aFromPlane.isNull()) {
                continue;
            }

            size_t aPlanesSizeX = aSizesY[aPlaneId] * theCubeCoeffs[0] * aMulX;
            size_t aSizeRowBytes = aPlanesSizeX * aFromPlane.getSizePixelBytes();
            aSizeRowBytes = aSizeRowBytes + (32 - aSizeRowBytes % 32);
            if(!anImage->changePlane(aPlaneId).initTrash(aFromPlane.getFormat(),
                                                         aSizesY[aPlaneId] * theCubeCoeffs[0] * aMulX,
                                                         aSizesY[aPlaneId] * theCubeCoeffs[1] * aMulY,
                                                         aSizeRowBytes)) {
                ST_ERROR_LOG("Scale failed!");
                return theRef;
            }
        }

        if(!StAVImage::resize(*theRef, *anImage)) {
            ST_ERROR_LOG("Scale failed!");
            return theRef;
        }
        theRef->close();
        return anImage;
    }

    if(theRef->getSizeX() <= theMaxSizeX
    && theRef->getSizeY() <= theMaxSizeY
    && !toConvertRgb) {
        return theRef;
    }

    StHandle<StImage> anImage = new StImage();
    const size_t aSizeX = stMin(theRef->getSizeX(), theMaxSizeX);
    const size_t aSizeY = stMin(theRef->getSizeY(), theMaxSizeY);
    if(toConvertRgb) {
        anImage->setColorModelPacked(anRgbImgFormat);
        anImage->setColorScale(StImage::ImgScale_Full);
        if(!anImage->changePlane().initTrash(anRgbImgFormat, aSizeX, aSizeY)
        || !StAVImage::resize(*theRef, *anImage)) {
            ST_ERROR_LOG("Scale failed!");
            return theRef;
        }
    } else {
        if(!anImage->initTrashLimited(*theRef, aSizeX, aSizeY)
        || !StAVImage::resize(*theRef, *anImage)) {
            ST_ERROR_LOG("Scale failed!");
            return theRef;
        }
    }
    theRef->close();
    return anImage;
}

/**
 * Auxiliary method to format image dimensions.
 */
inline StString formatSize(size_t theSizeX,
                           size_t theSizeY,
                           size_t theSrcSizeX,
                           size_t theSrcSizeY) {
    StString aText = StString() + theSizeX + " x " + theSizeY;
    if(theSizeX != theSrcSizeX
    || theSizeY != theSrcSizeY) {
        aText += StString() + " [" + theSrcSizeX + " x " + theSrcSizeY + "]";
    }
    return aText;
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

    // clear active
    myTextureQueue->clear();

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
    StFormat  aSrcFormatCurr = myStFormatByUser;
    StPanorama aSrcPanorama = StPanorama_OFF;
    if(anImgType == StImageFile::ST_TYPE_MPO
    || anImgType == StImageFile::ST_TYPE_JPEG
    || anImgType == StImageFile::ST_TYPE_JPS) {
        int aFileDescriptor = -1;
        if(StFileNode::isContentProtocolPath(aFilePath)) {
            aFileDescriptor = myResMgr->openFileDescriptor(aFilePath);
        }

        // special procedure to divide MPO (Multi Picture Object)
        StJpegParser aParser;
        double anHParallax = 0.0; // parallax in percents
        const bool isParsed = aParser.readFile(aFilePath, aFileDescriptor);

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
        if(!aParser.getXMP().isEmpty()) {
            StDictEntry& anEntry  = anImgInfo->Info.addChange("Jpeg.XMP");
            anEntry.changeValue() = aParser.getXMP();
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
        if(myStFormatByUser == StFormat_AUTO
        && aParser.getSrcFormat() != StFormat_AUTO) {
            aSrcFormatCurr = aParser.getSrcFormat();
        }
        aSrcPanorama = aParser.getPanorama();

        //aParser.fillDictionary(anImgInfo->Info, true);
        if(!isParsed) {
            processLoadFail(StString("Can not read the file \"") + aFilePath + '\"');
            return false;
        }

        anImgInfo->IsSavable = anImg2.isNull();
        anImgInfo->StInfoStream = aParser.getSrcFormat();
        if(anImgInfo->StInfoStream != StFormat_AUTO) {
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
        StRawFile aRawFileL;
        if(StFileNode::isContentProtocolPath(aFilePathLeft)) {
            int aFileDescriptor = myResMgr->openFileDescriptor(aFilePathLeft);
            aRawFileL.readFile(aFilePathLeft, aFileDescriptor);
        }
        if(!anImageFileL->load(aFilePathLeft, anImgType, (uint8_t* )aRawFileL.getBuffer(), (int )aRawFileL.getSize())) {
            processLoadFail(formatError(aFilePathLeft, anImageFileL->getState()));
            return false;
        }
        aRawFileL.freeBuffer();
        aSrcPanorama = anImageFileL->getPanoramaFormat();

        StRawFile aRawFileR;
        if(StFileNode::isContentProtocolPath(aFilePathRight)) {
            int aFileDescriptor = myResMgr->openFileDescriptor(aFilePathRight);
            aRawFileR.readFile(aFilePathRight, aFileDescriptor);
        }
        if(!anImageFileR->load(aFilePathRight, anImgType, (uint8_t* )aRawFileR.getBuffer(), (int )aRawFileR.getSize())) {
            processLoadFail(formatError(aFilePathRight, anImageFileR->getState()));
            return false;
        }
    } else {
        StRawFile aRawFile;
        if(StFileNode::isContentProtocolPath(aFilePath)) {
            int aFileDescriptor = myResMgr->openFileDescriptor(aFilePath);
            aRawFile.readFile(aFilePath, aFileDescriptor);
        }
        if(!anImageFileL->load(aFilePath, anImgType, (uint8_t* )aRawFile.getBuffer(), (int )aRawFile.getSize())) {
            processLoadFail(formatError(aFilePath, anImageFileL->getState()));
            return false;
        }

        aSrcPanorama = anImageFileL->getPanoramaFormat();
        anImgInfo->StInfoStream = anImageFileL->getFormat();
        if(myStFormatByUser == StFormat_AUTO) {
            aSrcFormatCurr = anImgInfo->StInfoStream;
        }
    }
    const double aLoadTimeMSec = aLoadTimer.getElapsedTimeInMilliSec();

    // copy metadata
    for(size_t aTagIter = 0; aTagIter < anImageFileL->getMetadata().size(); ++aTagIter) {
        const StDictEntry& aTag = anImageFileL->getMetadata().getFromIndex(aTagIter);
        anImgInfo->Info.add(aTag);
    }

    // detect information from file name
    bool isAnamorphByName = false;
    anImgInfo->StInfoFileName = st::formatFromName(aTitleString, myToSwapJps, isAnamorphByName);
    if(aSrcFormatCurr == StFormat_AUTO
    && anImgInfo->StInfoFileName != StFormat_AUTO) {
        aSrcFormatCurr = anImgInfo->StInfoFileName;
    }

    // scale down image if it does not fit texture limits
    size_t aSizeXLim = size_t(myMaxTexDim);
    size_t aSizeYLim = size_t(myMaxTexDim);
    size_t aSizeX1 = anImageFileL->getSizeX();
    size_t aSizeY1 = anImageFileL->getSizeY();
    size_t aSizeX2 = anImageFileR->getSizeX();
    size_t aSizeY2 = anImageFileR->getSizeY();
    theParams->Src1SizeX = aSizeX1;
    theParams->Src1SizeY = aSizeY1;
    theParams->Src2SizeX = aSizeX2;
    theParams->Src2SizeY = aSizeY2;
    StPairRatio aPairRatio = StPairRatio_1;
    if(anImageFileR->isNull()) {
        aPairRatio = st::formatToPairRatio(aSrcFormatCurr);
        if(aPairRatio == StPairRatio_HalfWidth) {
            aSizeXLim *= 2;
            aSizeX1   /= 2;
        } else if(aPairRatio == StPairRatio_HalfHeight) {
            aSizeYLim *= 2;
            aSizeY1   /= 2;
        }
    }

    if(!anImageFileR->isNull()) {
        aSrcFormatCurr = StFormat_SeparateFrames;
    }

    if(aSrcPanorama != StPanorama_OFF) {
        theParams->ViewingMode = StStereoParams::getViewSurfaceForPanoramaSource(aSrcPanorama, true);
    }
    if(myIsTheaterMode && theParams->ViewingMode == StViewSurface_Plain) {
        theParams->ViewingMode = StViewSurface_Theater;
    } else if(!myIsTheaterMode && theParams->ViewingMode == StViewSurface_Theater) {
        theParams->ViewingMode = StViewSurface_Plain;
    }

    if(myToStickPano360
    && theParams->ViewingMode == StViewSurface_Plain) {
        StPanorama aPano = st::probePanorama(aSrcFormatCurr,
                                             theParams->Src1SizeX, theParams->Src1SizeY,
                                             theParams->Src2SizeX, theParams->Src2SizeY);
        theParams->ViewingMode = StStereoParams::getViewSurfaceForPanoramaSource(aPano, true);
    }
    StCubemap aSrcCubemap = StCubemap_OFF;
    if(theParams->ViewingMode == StViewSurface_Cubemap) {
        aSrcCubemap = StCubemap_Packed;
    } else if(theParams->ViewingMode == StViewSurface_CubemapEAC) {
        aSrcCubemap = StCubemap_PackedEAC;
    }

    size_t aCubeCoeffs[2] = {0, 0};
    if(aSrcCubemap == StCubemap_Packed
    || aSrcCubemap == StCubemap_PackedEAC) {
        if(aSizeX1 / 6 == aSizeY1) {
            aCubeCoeffs[0] = 6;
            aCubeCoeffs[1] = 1;
            theParams->ToFlipCubeZ = myToFlipCubeZ6x1;
        } else if(aSizeY1 / 6 == aSizeX1) {
            aCubeCoeffs[0] = 1;
            aCubeCoeffs[1] = 6;
            theParams->ToFlipCubeZ = myToFlipCubeZ6x1;
        } else if(aSizeX1 / 3 == aSizeY1 / 2) {
            aCubeCoeffs[0] = 3;
            aCubeCoeffs[1] = 2;
            theParams->ToFlipCubeZ = myToFlipCubeZ3x2;
        } else if(aSizeX1 / 2 == aSizeY1 / 3) {
            aCubeCoeffs[0] = 2;
            aCubeCoeffs[1] = 3;
            theParams->ToFlipCubeZ = myToFlipCubeZ3x2;
        } else if(aSrcCubemap == StCubemap_PackedEAC) {
            // EAC on ytb is so cruel, that they don't use squared cube sides!
            if(aSizeX1 > aSizeY1) {
                aCubeCoeffs[0] = 3;
                aCubeCoeffs[1] = 2;
                theParams->ToFlipCubeZ = myToFlipCubeZ3x2;
            } else {
                aCubeCoeffs[0] = 2;
                aCubeCoeffs[1] = 3;
                theParams->ToFlipCubeZ = myToFlipCubeZ3x2;
            }
        }
        if(!anImageFileR->isNull()
        && (aSizeX1 != aSizeX2 || aSizeY1 != aSizeY2)) {
            aCubeCoeffs[0] = 0;
        }
        if(aCubeCoeffs[0] == 0) {
            myMsgQueue->pushError(StString("Image(s) has unexpected dimensions: {0}x{1} ({2}x{3})\n"
                                           "Cubemap should has 6 squared images in configuration 6:1 (single row) or 3:2 (two rows).")
                       .format(aSizeX1, aSizeY1, anImageFileL->getSizeX(), anImageFileL->getSizeY()));
            aSrcCubemap = StCubemap_OFF;
        } else {
            aSizeXLim *= aCubeCoeffs[0];
            aSizeYLim *= aCubeCoeffs[1];
        }
    }

    StHandle<StImage> anImageL = scaledImage(anImageFileL, myTextureQueue->getDeviceCaps(), aSizeXLim, aSizeYLim,
                                             aSrcCubemap, aCubeCoeffs, aPairRatio);
    StHandle<StImage> anImageR = scaledImage(anImageFileR, myTextureQueue->getDeviceCaps(), aSizeXLim, aSizeYLim,
                                             aSrcCubemap, aCubeCoeffs, aPairRatio);
#ifdef ST_DEBUG
    const double aScaleTimeMSec = aLoadTimer.getElapsedTimeInMilliSec() - aLoadTimeMSec;
    if(anImageL != anImageFileL) {
        ST_DEBUG_LOG("Image is downscaled to fit texture limits in " + aScaleTimeMSec + " ms!");
    }
#endif

#ifdef ST_DEBUG
    if(!anImageFileL->isNull()) {
        ST_DEBUG_LOG(anImageFileL->getState());
    }
    if(!anImageFileR->isNull()) {
        ST_DEBUG_LOG(anImageFileR->getState());
    }
#endif

    // finally push image data in Texture Queue
    myTextureQueue->setConnectedStream(true);

    {
        StImage anImageRefL, anImageRefR;
        StHandle<StBufferCounter> aRefL = new StImageFileCounter(anImageL);
        anImageRefL.initReference(*anImageL, aRefL);
        if(!anImageR->isNull()) {
            StHandle<StBufferCounter> aRefR = new StImageFileCounter(anImageR);
            anImageRefR.initReference(*anImageR, aRefR);
        }

        myTextureQueue->push(anImageRefL, anImageRefR, theParams, aSrcFormatCurr, aSrcCubemap, 0.0);
    }

    if(!stAreEqual(anImageFileL->getPixelRatio(), 1.0f, 0.001f)) {
        anImgInfo->Info.add(StArgument(tr(INFO_PIXEL_RATIO),
                                       StString(anImageFileL->getPixelRatio())));
    }
    const StString aFormatL = anImageFileL->formatImgPixelFormat();
    if(!anImageFileR->isNull()) {
        anImgInfo->Info.add(StArgument(tr(INFO_DIMENSIONS),
                                       formatSize(anImageL->getSizeX(), anImageL->getSizeY(),
                                                  theParams->Src1SizeX, theParams->Src1SizeY) + " " + tr(INFO_LEFT) + "\n"
                                     + formatSize(anImageR->getSizeX(), anImageR->getSizeY(),
                                                  theParams->Src2SizeX, theParams->Src2SizeY) + " " + tr(INFO_RIGHT)));
        const StString aFormatR = anImageFileR->formatImgPixelFormat();
        if(aFormatL == aFormatR) {
            anImgInfo->Info.add(StArgument(tr(INFO_PIXEL_FORMAT), aFormatL));
        } else {
            anImgInfo->Info.add(StArgument(tr(INFO_PIXEL_FORMAT),
                                aFormatL + " " + tr(INFO_LEFT) + "\n"
                              + aFormatR + " " + tr(INFO_RIGHT)));
        }
    } else {
        anImgInfo->Info.add(StArgument(tr(INFO_DIMENSIONS),
                                       formatSize(anImageL->getSizeX(), anImageL->getSizeY(),
                                                  theParams->Src1SizeX, theParams->Src1SizeY)));
        anImgInfo->Info.add(StArgument(tr(INFO_PIXEL_FORMAT),
                                       aFormatL));
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

    StOpenFileName anOpenInfo;
    anOpenInfo.Title = myLangMap->getValue(StImageViewerStrings::DIALOG_SAVE_SNAPSHOT);
    StString aSaveExt;
    if(toSaveStereo) {
        switch(theImgType) {
            case StImageFile::ST_TYPE_PNG:
                aSaveExt = ST_PNS_EXT;
                anOpenInfo.Filter.add(StMIME(ST_PNS_MIME, aSaveExt, ST_PNS_DESC));
                break;
            case StImageFile::ST_TYPE_JPEG:
                aSaveExt = ST_JPS_EXT;
                anOpenInfo.Filter.add(StMIME(ST_JPS_MIME, aSaveExt, ST_JPS_DESC));
                break;
            default:
                return false;
        }
    } else {
        switch(theImgType) {
            case StImageFile::ST_TYPE_PNG:
                aSaveExt = ST_PNG_EXT;
                anOpenInfo.Filter.add(StMIME(ST_PNG_MIME, aSaveExt, ST_PNG_DESC));
                break;
            case StImageFile::ST_TYPE_JPEG:
                aSaveExt = ST_JPG_EXT;
                anOpenInfo.Filter.add(StMIME(ST_JPG_MIME, aSaveExt, ST_JPEG_DESC));
                break;
            default:
                return false;
        }
    }

    StString aFileNameSrc, aNameSrc, anExtSrc;
    StFileNode::getFolderAndFile(theSource->getPath(), anOpenInfo.Folder, aFileNameSrc);
    StFileNode::getNameAndExtension(aFileNameSrc, aNameSrc, anExtSrc);
    StString aFileToSave = (!anOpenInfo.Folder.isEmpty() ? anOpenInfo.Folder : "") + ST_FILE_SPLITTER + aNameSrc;
    if(StFileNode::openFileDialog(aFileToSave, anOpenInfo, true)) {
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
                                  toSaveStereo ? StFormat_SideBySide_RL : StFormat_AUTO)) {
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

    StFormat aSrcFormat = theInfo->Id->StereoFormat;
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
                if(myPlayList->getCurrentFile(aFileToLoad, aFileParams)) {
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
                if(myPlayList->getCurrentFile(aFileToLoad, aFileParams)) {
                    loadImage(aFileToLoad, aFileParams);
                }
                break;
            }
        }
    }
}
