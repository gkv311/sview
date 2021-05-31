/**
 * Copyright © 2011-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StJpegParser_h_
#define __StJpegParser_h_

#include <StFile/StRawFile.h>
#include <StGLStereo/StFormatEnum.h>

#include "StExifDir.h"

/**
 * JPEG format parser (Joint Photographic Experts Group).
 * This class doesn't decode the image but only parses format structure.
 */
class StJpegParser : public StRawFile {

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

    /**
     * Units enumeration used in JFIF section
     */
    enum JfifUnitsXY
    {
        JfifUnitsXY_AspectRatio = 0,
        JfifUnitsXY_DotsPerInch = 1,
        JfifUnitsXY_DotsPerCm   = 2,
    };

    enum Offset {
        Offset_Dqt = 0,
        Offset_Jfif,
        Offset_Exif,      //!< APP1
        Offset_ExifExtra, //!< APP2
        Offset_Jps,       //!< APP3
        Offset_Iptc,      //!< APP13
        Offset_Comment,
        OffsetsNb,
    };

    struct Image {
        unsigned char*  Data;     //!< pointer to the data
        size_t          Length;   //!< data length
        StArrayList< StHandle<StExifDir> >
                        Exif;     //!< EXIF sections
        StHandle<Image> Thumb;    //!< optional thumbnail
        StHandle<Image> Next;     //!< link to the next image in file (if any)
        size_t          SizeX;    //!< image width  in pixels
        size_t          SizeY;    //!< image height in pixels
        uint16_t        ParX;     //!< Pixel Aspect Ratio
        uint16_t        ParY;     //!< Pixel Aspect Ratio

        ST_CPPEXPORT Image();
        ST_CPPEXPORT ~Image();

        /**
         * Read image timestamp property.
         */
        ST_CPPEXPORT StString getDateTime() const;

        /**
         * Read 360Mono and 360Stereo EXIF property.
         */
        ST_CPPEXPORT bool get360PanoMakerNote(bool& theIsStereo) const;

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
    ST_CPPEXPORT StJpegParser(const StCString& theFilePath = stCString(""));

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StJpegParser();

    ST_CPPEXPORT virtual void reset();

    /**
     * Read the file content.
     */
    ST_CPPEXPORT virtual bool readFile(const StCString& theFilePath,
                                       const int        theOpenedFd = -1,
                                       const size_t     theReadMax  = 0) ST_ATTR_OVERRIDE;

    /**
     * Determines images count.
     */
    inline size_t getNbImages() const {
        size_t aCount = 0;
        for(StHandle<StJpegParser::Image> anImg = myImages;
            !anImg.isNull(); anImg = anImg->Next) {
            ++aCount;
        }
        return aCount;
    }

    /**
     * Access image with specified index
     */
    inline StHandle<StJpegParser::Image> getImage(size_t theImgId) const {
        size_t aCount = 0;
        for(StHandle<StJpegParser::Image> anImg = myImages;
            !anImg.isNull(); anImg = anImg->Next) {
            if(aCount == theImgId) {
                return anImg;
            }
            ++aCount;
        }
        return StHandle<StJpegParser::Image>();
    }

    /**
     * @return image data size
     */
    inline size_t getDataSize() const {
        return myLength;
    }

    /**
     * @return file comment in COM section
     */
    ST_LOCAL const StString& getComment() const {
        return myComment;
    }

    /**
     * @return file comment in JPS section
     */
    ST_LOCAL const StString& getJpsComment() const {
        return myJpsComment;
    }

    /**
     * @return XMP
     */
    ST_LOCAL const StString& getXMP() const { return myXMP; }

    /**
     * @return stereo format stored in file
     */
    ST_LOCAL StFormat getSrcFormat() const {
        return myStFormat;
    }

    /**
     * Return panorama format.
     */
    ST_LOCAL StPanorama getPanorama() const { return myPanorama; }

    /**
     * Parse the structure.
     */
    ST_CPPEXPORT bool parse();

    ST_CPPEXPORT void fillDictionary(StDictionary& theDict,
                                     const bool    theToShowUnknown) const;

        public:

    /**
     * Create/modify JPS section.
     */
    ST_CPPEXPORT bool setupJps(const StFormat theFormat);

    /**
     * Override data length.
     */
    ST_LOCAL void setDataSize(const size_t theLength) {
        if(theLength <= myBuffSize) {
            myLength = theLength;
        }
    }

        protected:

    /**
     * Parse one image in data.
     */
    ST_CPPEXPORT StHandle<StJpegParser::Image> parseImage(const int      theImgCount,
                                                          const int      theDepth,
                                                          unsigned char* theDataStart,
                                                          const bool     theToFindSOI);

    /**
     * Create new section at specified offset.
     * @param theMarker  section marker
     * @param theSectLen section length excluding marker
     * @param theOffset  section offset from file beginning (at marker position)
     */
    ST_CPPEXPORT bool insertSection(const uint8_t   theMarker,
                                    const uint16_t  theSectLen,
                                    const ptrdiff_t theOffset);

        protected:

    StHandle<Image> myImages;     //!< images list
    ptrdiff_t       myOffsets[OffsetsNb];
                                  //!< array of offsets in image data, starting from session lenght (zero offset is invalid)
    StString        myComment;    //!< string stored in COM segment (directly in JPEG, NOT inside EXIF)
    StString        myJpsComment; //!< string stored in JPS segment
    StString        myXMP;        //!< string stored in XMP segment
    StFormat        myStFormat;   //!< stereo format
    StPanorama      myPanorama;   //!< panorama format

};

#endif // __StJpegParser_h_
