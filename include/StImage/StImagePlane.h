/**
 * Copyright © 2010-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StImagePlane_h_
#define __StImagePlane_h_

#include <StStrings/StString.h>
#include "StPixelRGB.h"

/**
 * Class represents rectangle image plain.
 */
class ST_LOCAL StImagePlane {

        public:

    /**
     * This enumeration define packed image plane formats.
     */
    typedef enum tagFormat {
        ImgUNKNOWN = 0, //!< not supported or unknown format
        ImgGray    = 1, //!< 1 byte per pixel (1-component plane, could be part of multiple-plane image)
        ImgRGB,         //!< 3 bytes packed RGB image plane
        ImgBGR,         //!< same as RGB but with different components order
        ImgRGB32,       //!< 4 bytes packed RGB image plane (1 byte is dummy)
        ImgBGR32,       //!< same as RGB but with different components order
        ImgRGBA,        //!< 4 bytes packed RGBA image plane
        ImgBGRA,        //!< same as RGBA but with different components order
        ImgGrayF,       //!< 1 float  (4-bytes) per pixel (1-component plane)
        ImgRGBF,        //!< 3 floats (12-bytes) RGB image plane
        ImgBGRF,        //!< same as RGBF but with different components order
        ImgRGBAF,       //!< 4 floats (16-bytes) RGBA image plane
        ImgBGRAF,       //!< same as RGBAF but with different components order
    } ImgFormat;

    static StString formatImgFormat(ImgFormat theImgFormat);
    StString formatImgFormat() const { return formatImgFormat(myImgFormat); }

        protected:

    GLubyte*    myDataPtr; // pointer to the data
    size_t      mySizeBPP; // bytes per pixel
    size_t        mySizeX; // width
    size_t        mySizeY; // height
    size_t mySizeRowBytes; // number of bytes per line (in most cases equal to 3 * sizeX)
    ImgFormat myImgFormat; // image plane pixel format
    bool   myIsOwnPointer; // if data was allocated by this class - flag is true
    bool    myIsTopBottom; // indicates image data is from Top to the Down. or from Bottom to the Up

        private:

    void setFormat(ImgFormat thePixelFormat);

        public:

    ImgFormat getFormat() const {
        return myImgFormat;
    }

    /**
     * Returns true if image data stored from Top to the Down (default).
     * Some external APIs can return bottom-up data
     * (topmost scanlines starts from the bottom in memory).
     * Thus this flag is to indicate such data and process it correctly.
     * However you should understand that methods ::getData() and so on
     * IGNORES that flag and always assume data is in normal top-down format!
     * @return true if image data is top-down.
     */
    bool isTopDown() const {
        return myIsTopBottom;
    }

    /**
     * Setup scanlines order in memory - top-down (default) or bottom-up.
     * @param theIsTopDown (bool ) - top-down flag.
     */
    void setTopDown(bool theIsTopDown) {
        myIsTopBottom = theIsTopDown;
    }

    /**
     * Access data methods.
     * User should MANUALLY select method according to image data format.
     */

    /**
     * @return data pointer to requested position.
     * Could be used as general method to copy data from one buffer to another.
     */
    const GLubyte* getData(const size_t theRow = 0, const size_t theCol = 0) const {
        ST_DEBUG_ASSERT((myDataPtr != NULL) && (theRow < mySizeY) && (theCol < mySizeX));
        return &myDataPtr[mySizeRowBytes * theRow + mySizeBPP * theCol];
    }

    /**
     * @return data pointer to requested position.
     */
    GLubyte* changeData(const size_t theRow = 0, const size_t theCol = 0) {
        ST_DEBUG_ASSERT((myDataPtr != NULL) && (theRow < mySizeY) && (theCol < mySizeX));
        return &myDataPtr[mySizeRowBytes * theRow + mySizeBPP * theCol];
    }

    GLubyte getFirstByte(const size_t theRow, const size_t theCol) const {
        return *getData(theRow, theCol);
    }

    GLubyte& changeFirstByte(const size_t theRow, const size_t theCol) {
        return *changeData(theRow, theCol);
    }

        public:

    /**
     * Access to RGB data.
     */
    const StPixelRGB& getPixelRGB(const size_t theRow, const size_t theCol) const {
        return *((StPixelRGB* )getData(theRow, theCol));
    }

    StPixelRGB& changePixelRGB(const size_t theRow, const size_t theCol) {
        return *((StPixelRGB* )changeData(theRow, theCol));
    }

        public:

    /**
     * @return width / height.
     */
    GLfloat getRatio() const {
        return (mySizeY > 0) ? (GLfloat(mySizeX) / GLfloat(mySizeY)) : 1.0f;
    }

    /**
     * @return bytes reserved for one pixel.
     */
    size_t getSizePixelBytes() const {
        return mySizeBPP;
    }

    /**
     * @return image plane width in pixels.
     */
    size_t getSizeX() const {
        return mySizeX;
    }

    /**
     * @return image plane height in pixels.
     */
    size_t getSizeY() const {
        return mySizeY;
    }

    /**
     * @return bytes reserved per row.
     * In some case may be larger than needed to store packed row
     * (for alignment etc.).
     */
    size_t getSizeRowBytes() const {
        return mySizeRowBytes;
    }

    /**
     * @return bytes allocated for the whole image plane.
     */
    size_t getSizeBytes() const {
        return mySizeRowBytes * mySizeY;
    }

    /**
     * @return true if data is NULL.
     */
    bool isNull() const {
        return myDataPtr == NULL;
    }

    /**
     * Compute the maximal row alignment for current row size.
     * @return maximal row alignment in bytes (up to 16 bytes).
     */
    size_t getMaxRowAligment() const {
        size_t anAlignment = 2;
        for(; anAlignment <= 16; anAlignment <<= 1) {
            if((mySizeRowBytes % anAlignment) != 0 || (size_t(myDataPtr) % anAlignment) != 0) {
                return (anAlignment >> 1);
            }
        }
        return anAlignment;
    }

    /**
     * @return the extra bytes in the row.
     */
    size_t getRowExtraBytes() const {
        return mySizeRowBytes - mySizeX * mySizeBPP;
    }

        public:

    /**
     * Empty constructor. Initialize the NULL image plane.
     */
    StImagePlane();
    virtual ~StImagePlane();

    /**
     * Initialize image plane as wrapper over alien data.
     * Data will not be copied!
     */
    bool initWrapper(ImgFormat thePixelFormat,
                     GLubyte*      theDataPtr,
                     const size_t  theSizeX,
                     const size_t  theSizeY,
                     const size_t  theSizeRowBytes = 0);

    /**
     * Initialize image plane with required dimensions.
     */
    bool initTrash(ImgFormat thePixelFormat,
                   const size_t theSizeX,
                   const size_t theSizeY,
                   const size_t theSizeRowBytes = 0);

    /**
     * Same as initTrash() but fill the buffer with zeros
     * (will be black for most formats).
     */
    bool initZero(ImgFormat thePixelFormat,
                  const size_t theSizeX,
                  const size_t theSizeY,
                  const size_t theSizeRowBytes = 0,
                  const int theValue = 0);

    /**
     * Initialize as copy (data will be copied!).
     */
    bool initCopy(const StImagePlane& theCopy);

    /**
     * Initialize as wrapper (data will not be copied).
     */
    bool initWrapper(const StImagePlane& theCopy);

    /**
     * Method initialize the cross-eyed stereoscopic image plane.
     */
    bool initSideBySide(const StImagePlane& theImageL,
                        const StImagePlane& theImageR,
                        const int theSeparationDx = 0,
                        const int theSeparationDy = 0,
                        const int theValue = 0);

    bool fill(const StImagePlane& theCopy);

    /**
     * Smart method to correctly deallocate image plane.
     */
    void nullify(ImgFormat thePixelFormat = StImagePlane::ImgGray);

};

#endif //__StImagePlane_h_
