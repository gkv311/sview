/**
 * Copyright © 2012-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StWebPImage_h_
#define __StWebPImage_h_

#include "StImageFile.h"

// define StHandle template specialization
class StWebPImage;
ST_DEFINE_HANDLE(StWebPImage, StImageFile);

/**
 * This class implements image load/save operations using WebP library.
 */
class StWebPImage : public StImageFile {

        public:

    /**
     * Should be called at application start.
     */
    ST_CPPEXPORT static bool init();

        public:

    ST_CPPEXPORT StWebPImage();
    ST_CPPEXPORT virtual ~StWebPImage();

    ST_LOCAL virtual StHandle<StImageFile> createEmpty() const ST_ATTR_OVERRIDE { return new StWebPImage(); }

    ST_CPPEXPORT virtual void close() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool loadExtra(const StString& theFilePath,
                                        ImageType       theImageType,
                                        uint8_t*        theDataPtr,
                                        int             theDataSize,
                                        bool            theIsOnlyRGB) ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool save(const StString& theFilePath,
                                   ImageType       theImageType,
                                   StFormat        theSrcFormat) ST_ATTR_OVERRIDE;

        public:

    typedef struct {
        int width;
        int height;
        int has_alpha;

        // Unused for now
        int bitstream_version;
        int no_incremental_decoding;
        int rotate;
        int uv_sampling;
        uint32_t pad[3];
    } WebPBitstreamFeatures; // Features gathered from the bitstream

    typedef struct {
        int bypass_filtering;
        int no_fancy_upsampling;
        int use_cropping;
        int crop_left,    crop_top;
        int crop_width,   crop_height;
        int use_scaling;
        int scaled_width, scaled_height;
        int use_threads;

        // Unused for now
        int force_rotation;
        int no_enhancement;
        uint32_t pad[6];
    } WebPDecoderOptions; // Decoding options

    typedef struct {     // view as RGBA
        uint8_t* rgba;   // pointer to RGBA samples
        int      stride; // stride in bytes from one scanline to the next.
        size_t   size;   // total size of the *rgba buffer.
    } WebPRGBABuffer;

    typedef struct {
        uint8_t* y, *u, *v, *a;    // pointer to luma, chroma U/V, alpha samples
        int    y_stride;           // luma stride
        int    u_stride, v_stride; // chroma strides
        int    a_stride;           // alpha stride
        size_t y_size;             // luma plane size
        size_t u_size, v_size;     // chroma planes size
        size_t a_size;             // alpha-plane size
    } WebPYUVABuffer;

    typedef enum {
        MODE_RGB  = 0,  MODE_RGBA = 1,
        MODE_BGR  = 2,  MODE_BGRA = 3,
        MODE_ARGB = 4,
        MODE_YUV  = 11, MODE_YUVA = 12,  // yuv 4:2:0
    } WEBP_CSP_MODE; // Colorspaces

    typedef struct {
        WEBP_CSP_MODE  colorspace;
        int            width;
        int            height;
        int            is_external_memory;
        union {
            WebPRGBABuffer RGBA;
            WebPYUVABuffer YUVA;
        } u;
        uint32_t       pad[4]; // padding for later use
        uint8_t*       private_memory;
    } WebPDecBuffer; // Output buffer

    typedef struct {
      WebPBitstreamFeatures input;
      WebPDecBuffer         output;
      WebPDecoderOptions    options;
    } WebPDecoderConfig; // Main object storing the configuration for advanced decoding.

        private:

    ST_LOCAL bool loadInternal(const StString& theFilePath,
                               const uint8_t*  theDataPtr,
                               const int       theDataSize,
                               const bool      theIsRGB);

        private:

    WebPDecoderConfig myConfig;
    bool              myIsCompat;

};

#endif // __StWebPImage_h_
