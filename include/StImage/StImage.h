/**
 * Copyright © 2010-2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StImage_h_
#define __StImage_h_

#include "StImagePlane.h"

/**
 * Interface to share arbitrary memory buffer using reference counter.
 */
class StBufferCounter {

        public:

    /**
     * Empty constructor.
     */
    StBufferCounter() {}

    /**
     * Create the new reference (e.g. increment counter).
     * If theOther has the same type, than the ref counter will be reused.
     * Otherwise then new counter will be allocated.
     */
    virtual void createReference(StHandle<StBufferCounter>& theOther) const = 0;

    /**
     * Release current reference.
     */
    virtual void releaseReference() = 0;

    /**
     * Destructor.
     */
    virtual ~StBufferCounter() {}

};

/**
 * Class defining 2D image consisting from one or more color planes.
 */
class StImage {

        public: //! @name enumerations

    typedef enum tagImgColorModel {
        ImgColor_RGB,     //!< Red, Green, Blue - generic color model on PC (native for GPU and Displays)
        ImgColor_RGBA,    //!< same as RGB but has Alpha channel to perform color blending with background
        ImgColor_GRAY,    //!< just gray scale
        ImgColor_YUV,     //!< luma/brightness (Y) + chrominance (UV color plane) - widely used in cinema
        ImgColor_YUVA,    //!< luma/brightness (Y) + chrominance (UV color plane) + Alpha
        ImgColor_XYZ,     //!< XYZ
        ImgColor_CMYK,    //!< Cyan, Magenta, Yellow and Black - generally used in printing process
        ImgColor_HSV,     //!< Hue, Saturation, Value (also known as HSB - Hue, Saturation, Brightness)
        ImgColor_HSL,     //!< Hue, Saturation, Lightness/Luminance (also known as HLS or HSI - Hue, Saturation, Intensity))
    } ImgColorModel;

    typedef enum tagImgColorScale {
        ImgScale_Full,    //!< full range (use all bits)
        ImgScale_Mpeg,    //!< YUV  8 bits per component Y   16..235;   U and V   16..240
                          //!< YUV 16 bits per component Y 4096..60160; U and V 4096..61440
        ImgScale_Mpeg9,   //!< YUV  9 bits in 16 bits    Y   32..470;   U and V   32..480
        ImgScale_Mpeg10,  //!< YUV 10 bits in 16 bits    Y   64..940;   U and V   64..960
        ImgScale_Jpeg9,   //!< 9  bits in 16 bits 0...511
        ImgScale_Jpeg10,  //!< 10 bits in 16 bits 0..1023
        ImgScale_NvFull,  //!< full range (use all bits)
        ImgScale_NvMpeg,  //!< YUV  8 bits per component Y   16..235;   U and V   16..240
    } ImgColorScale;

    ST_CPPEXPORT static StString formatImgColorModel(ImgColorModel theColorModel);
    ST_LOCAL inline StString formatImgColorModel() const { return formatImgColorModel(myColorModel); }

    /**
     * Format image pixel format.
     * @sa stAV::PIX_FMT::getString()
     */
    ST_CPPEXPORT const char* formatImgPixelFormat() const;

        public: //! @name initializers

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StImage();
    ST_CPPEXPORT virtual ~StImage();

    /**
     * Initialize as copy (data will be copied!).
     */
    ST_CPPEXPORT bool initCopy(const StImage& theCopy,
                               const bool     theIsCompact);

    /**
     * Initialize as wrapper (data will not be copied).
     */
    ST_CPPEXPORT bool initWrapper(const StImage& theCopy);

    /**
     * Initialize as reference to the shared buffer.
     */
    ST_CPPEXPORT bool initReference(const StImage& theCopy);

    /**
     * Initialize as reference to the shared buffer.
     */
    ST_CPPEXPORT bool initReference(const StImage&                   theCopy,
                                    const StHandle<StBufferCounter>& theRef);

    /**
     * Initialize as wrapper of input data in RGB format
     * or tries to convert data to RGB.
     */
    ST_CPPEXPORT bool initRGB(const StImage& theCopy);

    /**
     * Method initialize the cross-eyed stereoscopic image.
     */
    ST_CPPEXPORT bool initSideBySide(const StImage& theImageL,
                                     const StImage& theImageR,
                                     const int theSeparationDx,
                                     const int theSeparationDy);

    ST_CPPEXPORT bool fill(const StImage& theCopy,
                           const bool     theIsCompact);

    /**
     * Advanced initializer for further scaling - creates an image from specified one with:
     * - same format and list of planes
     * - plane size limited by specified dimension
     * - undefined data
     */
    ST_CPPEXPORT bool initTrashLimited(const StImage& theRef,
                                       const size_t   theSizeX,
                                       const size_t   theSizeY);

        public: //! @name image information

    /**
     * @return true if all color components are stored in one plane (thus - interleaved).
     */
    inline bool isPacked() const {
        return myPlanes[1].isNull();
    }

    /**
     * @return true if color components are stored in separate image planes.
     */
    inline bool isPlanar() const {
        return !myPlanes[1].isNull();
    }

    /**
     * @return color model
     */
    inline ImgColorModel getColorModel() const {
        return myColorModel;
    }

    /**
     * Setup color model.
     */
    inline void setColorModel(const ImgColorModel theColorModel) {
        myColorModel = theColorModel;
    }

    /**
     * @return color scale (range)
     */
    inline ImgColorScale getColorScale() const {
        return myColorScale;
    }

    /**
     * Setup color scale (range).
     */
    inline void setColorScale(const ImgColorScale theColorScale) {
        myColorScale = theColorScale;
    }

    /**
     * Determine the color model from image plane.
     * Valid only for packed image.
     */
    inline void setColorModelPacked(const StImagePlane::ImgFormat theImgFormat) {
        switch(theImgFormat) {
            case StImagePlane::ImgGray:
            case StImagePlane::ImgGrayF:
            case StImagePlane::ImgGray16:
                myColorModel = ImgColor_GRAY; return;
            case StImagePlane::ImgRGBA:
            case StImagePlane::ImgBGRA:
            case StImagePlane::ImgRGBAF:
            case StImagePlane::ImgBGRAF:
                myColorModel = ImgColor_RGBA; return;
            case StImagePlane::ImgRGB:
            case StImagePlane::ImgBGR:
            case StImagePlane::ImgRGBF:
            case StImagePlane::ImgBGRF:
            default:
                myColorModel = ImgColor_RGB; return;
        }
    }

    /**
     * @return width / height.
     */
    inline GLfloat getRatio() const {
        size_t aSizeY = getSizeY();
        return (aSizeY > 0) ? (GLfloat(getSizeX()) / GLfloat(aSizeY)) : 1.0f;
    }

    /**
     * Return true if rows are packed in top-bottom order.
     */
    ST_LOCAL bool isTopDown() const {
        return myPlanes[0].isTopDown();
    }

    /**
     * @return image width in pixels.
     */
    inline size_t getSizeX() const {
        return myPlanes[0].getSizeX();
    }

    /**
     * @return image height in pixels.
     */
    inline size_t getSizeY() const {
        return myPlanes[0].getSizeY();
    }

    /**
     * Access to the image plane by ID (from 0 to 3).
     */
    inline const StImagePlane& getPlane(const size_t theId = 0) const {
        ST_ASSERT(theId < 4, "StImage::getPlane() - Out of range access");
        return myPlanes[theId];
    }

    /**
     * Access to the image plane by ID (from 0 to 3).
     */
    inline StImagePlane& changePlane(const size_t theId = 0) {
        ST_ASSERT(theId < 4, "StImage::changePlane() - Out of range access");
        return myPlanes[theId];
    }

    /**
     * Returns Pixel Aspect Ratio.
     * If pixels are quads than PAR = 1.0 (normal case).
     * PAR should be used to compute correct DAR (Display Aspect Ratio).
     */
    inline GLfloat getPixelRatio() const {
        return myPAR;
    }

    inline void setPixelRatio(const GLfloat thePAR) {
        myPAR = thePAR;
    }

    /**
     * @return true if data is NULL.
     */
    ST_CPPEXPORT bool isNull() const;

    /**
     * Release all color planes and reference counter.
     */
    ST_CPPEXPORT void nullify();

    /**
     * Return reference counter for shared memory buffer.
     */
    ST_LOCAL const StHandle<StBufferCounter>& getBufferCounter() const { return myBufCounter; }

    /**
     * Dangerous method to assign reference counter for shared memory buffer.
     */
    ST_LOCAL void setBufferCounter(const StHandle<StBufferCounter>& theCounter) { myBufCounter = theCounter; }

        protected:

    ST_LOCAL inline StString getDescription() const {
       return StString() + getSizeX() + " x " + getSizeY()
            + ", " + formatImgColorModel()
            + ", " + getPlane().formatImgFormat();
    }

        private:

    /**
     * Decode yuv420p pixel into RGB pixel.
     */
    ST_CPPEXPORT StPixelRGB getRGBFromYUV(const size_t theRow,
                                          const size_t theCol) const;

    inline float getScaleFactorX(const size_t thePlane) const {
        return float(getPlane(thePlane).getSizeX()) / float(getPlane(0).getSizeX());
    }

    inline float getScaleFactorY(const size_t thePlane) const {
        return float(getPlane(thePlane).getSizeY()) / float(getPlane(0).getSizeY());
    }

    inline size_t getScaledCol(const size_t thePlane,
                               const size_t theCol) const {
        return size_t(getScaleFactorX(thePlane) * float(theCol));
    }

    inline size_t getScaledRow(const size_t thePlane,
                               const size_t theRow) const {
        return size_t(getScaleFactorY(thePlane) * float(theRow));
    }

    template<typename Type>
    static inline uint8_t clamp(const Type x) {
        return x < 0x00 ? 0x00 : (x > 0xff ? 0xff : uint8_t(x));
    }

    /**
     * Avoid copy.
     */
    StImage(const StImage& theCopy);

        private:

    StImagePlane  myPlanes[4];  //!< color planes (only 1 used for packed formats)
    GLfloat       myPAR;        //!< pixel aspect ratio
    ImgColorModel myColorModel; //!< color model (RGB/YUV...)
    ImgColorScale myColorScale; //!< color scale (range)
    StHandle<StBufferCounter>
                  myBufCounter; //!< reference counter for shared memory buffer

};

#endif // __StImage_h_
