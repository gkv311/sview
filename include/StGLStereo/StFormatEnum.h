/**
 * Copyright © 2009-2019 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StFormatEnum_h_
#define __StFormatEnum_h_

#include <StStrings/StString.h>

/**
 * Enumeration of available stereo source formats
 */
enum StFormat {
    StFormat_AUTO = -1,
    StFormat_Mono = 0,              //!< whole frame is one Mono image
    StFormat_SideBySide_LR,         //!< frame is two Full Left + Right images placed horizontally
    StFormat_SideBySide_RL,         //!< frame is two Full Right + Left images placed horizontally
    StFormat_TopBottom_LR,          //!< over/under - frame is two Full Left + Right images placed vertically
    StFormat_TopBottom_RL,          //!< over/under - frame is two Full Right + Left images placed vertically
    StFormat_Rows,                  //!< row-interlaced - each row is Right or Left image
    StFormat_Columns,               //!< column-interlaced - each vertical line is Right or Left image
    StFormat_SeparateFrames,        //!< Left and Right images stored in separate frames
    StFormat_FrameSequence,         //!< 2 sequenced frames is Full Right and Left images
    StFormat_AnaglyphRedCyan,
    StFormat_AnaglyphGreenMagenta,
    StFormat_AnaglyphYellowBlue,
    StFormat_Tiled4x,               //!< Sisvel 3D frame format to fit 720p stereopair into 1080p frame
    StFormat_NB
};

enum StPairRatio {
    StPairRatio_1,
    StPairRatio_HalfWidth,
    StPairRatio_HalfHeight,
};

enum StCubemap {
    StCubemap_AUTO = -1, //!< try to detect from metadata
    StCubemap_OFF  =  0, //!< no cubemap data
    StCubemap_Packed,    //!< cubemap data packed into single image frame - 6 horizontally stacked planes
    StCubemap_PackedEAC  //!< cubemap data packed into single image frame (EAC layout)
};

enum StPanorama {
    StPanorama_OFF  =  0,     //!< no cubemap data
    StPanorama_Sphere,        //!< spherical panorama 360 degrees - 2:1
    StPanorama_Hemisphere,    //!< spherical panorama 180 degrees - 1:1
    StPanorama_Cubemap6_1,    //!< cubemap data packed into single image frame - 6:1 in OpenGL enum order (px nx py ny pz nz)
    StPanorama_Cubemap1_6,    //!< cubemap data packed into single image frame - 1:6 in OpenGL enum order
    StPanorama_Cubemap3_2,    //!< cubemap data packed into single image frame - 3:2 in OpenGL enum order
    StPanorama_Cubemap3_2ytb, //!< cubemap data packed into single image frame - 3:2 in custom order (px nz nx, ny pz py)
                              //!  one row defines 3 horizontally stacked sides of the cube, and other 3 vertically stacked sides
    StPanorama_Cubemap2_3ytb  //!< cubemap data packed into single image frame - 2:3 in custom order (90 counterclockwise transposed 3x2 layout)
};

namespace st {

    /**
     * List of pre-defined frame ratios.
     */
    namespace videoRatio {
        // TV
        static const GLfloat TV_OVERUNDER       = 4.0f  / 6.0f;  //!< 0.6(6):1 ~ 4:6   Over/Under
        static const GLfloat TV_NORMAL          = 4.0f  / 3.0f;  //!< 1.3(3):1 ~ 4:3   Mono
        static const GLfloat TV_SIDEBYSIDE      = 8.0f  / 3.0f;  //!< 2.6(6):1 ~ 8:3   SideBySide

        // Widescreen
        static const GLfloat WIDE_NORMAL        = 16.0f / 9.0f;  //!< 1.7(7):1 ~ 16:9  Mono
        static const GLfloat WIDE_PC            = 16.0f / 10.0f; //!< 1.6:1    ~ 16:10 Mono
        static const GLfloat WIDE_SIDEBYSIDE    = 32.0f / 9.0f;  //!< 3.5(5):1 ~ 32:9  SideBySide

        // Cinemascope
        static const GLfloat CINEMASCOPE        = 29.0f / 9.0f;  //!< 3.2(2):1 ~ 29:9  Mono

        static const GLfloat USERDEF_SIDEBYSIDE = 2.86f;
    }

    /**
     * Returns name for format.
     */
    ST_CPPEXPORT StString formatToString(StFormat theFormatEnum);

    /**
     * Return enumeration value from the string.
     */
    ST_CPPEXPORT StFormat formatFromString(const StString& theFormatString);

    /**
     * Return source format with reversed views.
     */
    ST_CPPEXPORT StFormat formatReversed(const StFormat theFormatEnum);

    /**
     * Detect stereoscopic format from file name tags:
     * - ou,ab,ba  Over/Under, Above/Below
     * - sbs,lr,rl SideBySide, Left/Right
     * - 2d        Mono
     * - half,q    Anamorph suffix
     * or file extension:
     * - jps,pns   SideBySide (Right/Left order)
     * @param theFileName  [in] file name to parse
     * @param theToSwapJps [in] if TRUE, then JPS/PNS file extension will be treated as Left/Right instead of Right/Left
     * @param theIsAnamorph [out] flag indicating anamorphic aspect ratio
     */
    ST_CPPEXPORT StFormat formatFromName(const StString& theFileName,
                                         const bool      theToSwapJps,
                                         bool&           theIsAnamorph);

    /**
     * Function tries to detect side-by-side stereo format based on aspect ratio criteria.
     * @param theRatio image ratio
     * @return autodetected mono/stereo format
     */
    ST_CPPEXPORT StFormat formatFromRatio(const GLfloat theRatio);

    ST_LOCAL inline StPairRatio formatToPairRatio(const StFormat theFormat) {
        switch(theFormat) {
            case StFormat_SideBySide_LR:
            case StFormat_SideBySide_RL:
            case StFormat_Columns:
                return StPairRatio_HalfWidth;
            case StFormat_TopBottom_LR:
            case StFormat_TopBottom_RL:
            case StFormat_Rows:
                return StPairRatio_HalfHeight;
            default:
                return StPairRatio_1;
        }
    }

    /**
     * Probe panorama mode from image dimensions.
     */
    ST_LOCAL inline StPanorama probePanorama(StFormat theFormat,
                                             size_t   theSrc1SizeX,
                                             size_t   theSrc1SizeY,
                                             size_t   theSrc2SizeX,
                                             size_t   theSrc2SizeY) {
        StPairRatio aPairRatio = st::formatToPairRatio(theFormat);
        if(aPairRatio == StPairRatio_HalfWidth) {
            theSrc1SizeX /= 2;
        } else if(aPairRatio == StPairRatio_HalfHeight) {
            theSrc1SizeY /= 2;
        }
        if(theSrc1SizeX < 8
        || theSrc1SizeY < 8) {
            return StPanorama_OFF;
        }

        if(theSrc1SizeX / 2 == theSrc1SizeY
        && theSrc2SizeX / 2 == theSrc2SizeY) {
            return StPanorama_Sphere;
        } else if(theSrc1SizeX / 6 == theSrc1SizeY
               && theSrc2SizeX / 6 == theSrc2SizeY) {
            return StPanorama_Cubemap6_1;
        } else if(theSrc1SizeY / 6 == theSrc1SizeX
               && theSrc2SizeY / 6 == theSrc2SizeX) {
            return StPanorama_Cubemap1_6;
        } else if(theSrc1SizeX / 3 == theSrc1SizeY / 2
               && theSrc2SizeX / 3 == theSrc2SizeY / 2) {
            return StPanorama_Cubemap3_2;
        } else if(theSrc1SizeX == theSrc1SizeY
               && theSrc2SizeX == theSrc2SizeY) {
            return StPanorama_Hemisphere;
        }
        return StPanorama_OFF;
    }

};

#endif // __StFormatEnum_h_
