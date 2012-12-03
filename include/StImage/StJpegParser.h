/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StJpegParser_h_
#define __StJpegParser_h_

#include "StExifDir.h"

/**
 * JPEG format parser (Joint Photographic Experts Group).
 * Notice this class doesn't decode the image!
 */
class ST_LOCAL StJpegParser {

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

        Image()
        : myData(NULL),
          myLength(0),
          myExif(),
          myNext() {}

        /**
         * Reads the parallax information from EXIF (currently - only for Fujifilm MPO).
         * @param theParallax (double& ) - the parallax in per cents;
         * @return true if tag found.
         */
        bool getParallax(double& theParallax) const;

        /**
         * Reads the orientation info from EXIF.
         */
        Orient getOrientation() const;
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

        private:

    StHandle<Image> myImages; //!< images list
    unsigned char*  myData;   //!< pointer to the data
    size_t          myLength; //!< data length

        private:

    /**
     * Parse the structure.
     */
    bool parse();

    /**
     * Parse one image in data.
     */
    StHandle<StJpegParser::Image> parseImage(unsigned char* theDataStart);

        public:

    /**
     * Empty constructor.
     */
    StJpegParser();
    ~StJpegParser();

    void reset();

    /**
     * Read the file content.
     */
    bool read(const StString& theFileName);

    size_t getImageCount() const {
        size_t aCount = 0;
        for(StHandle<StJpegParser::Image> anImg = myImages;
            !anImg.isNull(); anImg = anImg->myNext) {
            ++aCount;
        }
        return aCount;
    }

    StHandle<StJpegParser::Image> getImage(size_t theImgId) const {
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

};

#endif //__StJpegParser_h_
