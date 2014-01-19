/**
 * Copyright © 2011-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StJpegParser_h_
#define __StJpegParser_h_

#include <StGLStereo/StFormatEnum.h>

#include "StExifDir.h"

/**
 * JPEG format parser (Joint Photographic Experts Group).
 * This class doesn't decode the image but only parses format structure.
 */
class StJpegParser {

        public:

    typedef enum {
        ORIENT_NORM         = 1, //!< 0th row ~ visual top,      0th column ~ visual left-hand  side
        ORIENT_FLIPX        = 2, //!< 0th row ~ visual top,      0th column ~ visual right-hand side
        ORIENT_ROT180       = 3, //!< 0th row ~ visual bottom,   0th column ~ visual right-hand side
        ORIENT_ROT180_FLIPX = 4, //!< 0th row ~ visual bottom,   0th column ~ visual left-hand  side
        ORIENT_ROT90_FLIPX  = 5, //!< 0th row ~ left-hand  side, 0th column ~ visual top
        ORIENT_ROT270       = 6, //!< 0th row ~ right-hand side, 0th column ~ visual top
        ORIENT_ROT270_FLIPX = 7, //!< 0th row ~ right-hand side, 0th column ~ visual bottom
        ORIENT_ROT90        = 8, //!< 0th row ~ left-hand  side, 0th column ~ visual bottom
    } Orient;

    struct Image {
        unsigned char*  myData;   //!< pointer to the data
        size_t          myLength; //!< data length
        StArrayList< StHandle<StExifDir> > myExif; //!< EXIF sections
        StHandle<Image> myNext;   //!< link to the next image in file (if any)
        int16_t         ParX;     //!< Pixel Aspect Ratio
        int16_t         ParY;     //!< Pixel Aspect Ratio

        ST_CPPEXPORT Image();
        ST_CPPEXPORT ~Image();

        /**
         * Reads the parallax information from EXIF (currently - only for Fujifilm MPO).
         * @param theParallax the parallax in per cents
         * @return true if tag found
         */
        ST_CPPEXPORT bool getParallax(double& theParallax) const;

        /**
         * Reads the orientation info from EXIF.
         */
        ST_CPPEXPORT Orient getOrientation() const;
    };

    static int getRotationAngle(const Orient theJpegOri) {
        switch(theJpegOri) {
            case ORIENT_FLIPX:
            case ORIENT_NORM:         return   0;
            case ORIENT_ROT180:
            case ORIENT_ROT180_FLIPX: return 180;
            case ORIENT_ROT270:
            case ORIENT_ROT270_FLIPX: return 270;
            case ORIENT_ROT90:
            case ORIENT_ROT90_FLIPX:  return  90;
        }
        return 0;
    }

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StJpegParser();
    ST_CPPEXPORT ~StJpegParser();

    ST_CPPEXPORT void reset();

    /**
     * Read the file content.
     */
    ST_CPPEXPORT bool read(const StString& theFileName);

    inline size_t getImageCount() const {
        size_t aCount = 0;
        for(StHandle<StJpegParser::Image> anImg = myImages;
            !anImg.isNull(); anImg = anImg->myNext) {
            ++aCount;
        }
        return aCount;
    }

    inline StHandle<StJpegParser::Image> getImage(size_t theImgId) const {
        size_t aCount = 0;
        for(StHandle<StJpegParser::Image> anImg = myImages;
            !anImg.isNull(); anImg = anImg->myNext) {
            if(aCount == theImgId) {
                return anImg;
            }
            ++aCount;
        }
        return StHandle<StJpegParser::Image>();
    }

    inline unsigned char* getData() const {
        return myData;
    }

    inline size_t getDataSize() const {
        return myLength;
    }

    /**
     * @return file comment
     */
    ST_LOCAL const StString& getComment() const {
        return myComment;
    }

    /**
     * @return stereo format stored in file
     */
    ST_LOCAL StFormatEnum getSrcFormat() const {
        return myStFormat;
    }

        private:

    /**
     * Parse the structure.
     */
    ST_CPPEXPORT bool parse();

    /**
     * Parse one image in data.
     */
    ST_CPPEXPORT StHandle<StJpegParser::Image> parseImage(const int      theDepth,
                                                          unsigned char* theDataStart,
                                                          const bool     theToFindSOI = false);

        private:

    StHandle<Image> myImages;   //!< images list
    unsigned char*  myData;     //!< pointer to the data
    size_t          myLength;   //!< data length
    StString        myComment;  //!< string stored in COM segment (directly in JPEG, NOT inside EXIF)
    StFormatEnum    myStFormat; //!< stereo format

};

#endif // __StJpegParser_h_
