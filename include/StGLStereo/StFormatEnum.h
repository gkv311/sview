/**
 * Copyright © 2009-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
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

enum StCubemap {
    StCubemap_AUTO = -1, //!< try to detect from metadata
    StCubemap_OFF  =  0, //!< no cubemap data
    StCubemap_Packed     //!< cubemap data packed into single image frame - 6 horizontally stacked planes
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
     */
    ST_CPPEXPORT StFormat formatFromName(const StString& theFileName,
                                         bool&           theIsAnamorph);

    /**
     * Function tries to detect side-by-side stereo format based on aspect ratio criteria.
     * @param theRatio image ratio
     * @return autodetected mono/stereo format
     */
    ST_CPPEXPORT StFormat formatFromRatio(const GLfloat theRatio);

};

#endif // __StFormatEnum_h_
