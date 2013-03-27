/**
 * Copyright © 2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StTests program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StTests program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "StTestImageLib.h"

#include <StStrings/stConsole.h>
#include <StImage/StLibAVImage.h>
#include <StImage/StDevILImage.h>
#include <StImage/StFreeImage.h>
#include <StImage/StWebPImage.h>
#include <StFile/StRawFile.h>

StTestImageLib::StTestImageLib(const StString& theFile)
: myFilePath(theFile),
  myImgType(StImageFile::guessImageType(theFile, StMIME())),
  myDataPtr(NULL),
  myDataSize(0) {
    //
}

bool StTestImageLib::testLoadSpeed(StImageFile& theLoader) {
    myTimer.restart();
    if(!theLoader.load(myFilePath, myImgType, myDataPtr, myDataSize)) {
        st::cout << stostream_text("  Error! ") << theLoader.getState() << stostream_text("\n");
        return false;
    }

    st::cout << stostream_text("  loaded in:\t")   << myTimer.getElapsedTimeInMilliSec() << stostream_text(" msec\n");
    st::cout << stostream_text("  dimensions:\t")  << theLoader.getSizeX() << stostream_text("x") << theLoader.getSizeY() << stostream_text("\n");
    st::cout << stostream_text("  color model:\t") << theLoader.formatImgColorModel() << stostream_text("\n");

    theLoader.close();
    return true;
}

void StTestImageLib::perform() {
    st::cout << stostream_text("Image library speed tests\n");
    st::cout << stostream_text("  file:   \t'") << myFilePath << stostream_text("'\n");
    if(myImgType == StImageFile::ST_TYPE_NONE) {
        st::cout << stostream_text("  image has unsupported format.\n");
        return;
    }

    myTimer.restart();
    StRawFile aRawFile(myFilePath);
    if(!aRawFile.readFile()) {
        st::cout << stostream_text("  file can not be read.\n");
        return;
    }
    st::cout << stostream_text("  read in:\t") << myTimer.getElapsedTimeInMilliSec() << stostream_text(" msec\n");
    myDataPtr  = (uint8_t* )aRawFile.getBuffer();
    myDataSize = (int )aRawFile.getSize();

    StHandle<StImageFile> aLoader;

    st::cout << stostream_text("FFmpeg:\n");
    if(StLibAVImage::init()) {
        aLoader = new StLibAVImage();
        testLoadSpeed(*aLoader);
    } else {
        st::cout << stostream_text("  library is unavailable! Skipped.\n");
    }

    st::cout << stostream_text("FreeImage:\n");
    if(StFreeImage::init()) {
        aLoader = new StFreeImage();
        testLoadSpeed(*aLoader);
    } else {
        st::cout << stostream_text("  library is unavailable! Skipped.\n");
    }

    st::cout << stostream_text("DevIL:\n");
    if(StDevILImage::init()) {
        aLoader = new StDevILImage();
        testLoadSpeed(*aLoader);
    } else {
        st::cout << stostream_text("  library is unavailable! Skipped.\n");
    }

    st::cout << stostream_text("WebP:\n");
    if(StWebPImage::init()) {
        aLoader = new StWebPImage();
        testLoadSpeed(*aLoader);
    } else {
        st::cout << stostream_text("  library is unavailable! Skipped.\n");
    }
}
