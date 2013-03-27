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

#ifndef __StTestImageLib_h_
#define __StTestImageLib_h_

#include "StTest.h"
#include <StThreads/StThreads.h>
#include <StImage/StImageFile.h>

/**
 * Tests image libraries performance.
 */
class ST_LOCAL StTestImageLib : public StTest {

        public:

    /**
     * Main constructor.
     */
    StTestImageLib(const StString& theFile);

    virtual void perform();

        private:

    /**
     * Load image using specified image loader.
     */
    bool testLoadSpeed(StImageFile& theLoader);

        private:

    StString               myFilePath;
    StImageFile::ImageType myImgType;
    uint8_t*               myDataPtr;
    int                    myDataSize;

};

#endif // __StTestImageLib_h_
