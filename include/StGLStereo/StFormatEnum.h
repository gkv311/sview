/**
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
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
typedef enum tagStFormatEnum {
    ST_V_SRC_AUTODETECT = -1,
    ST_V_SRC_MONO = 0,                 //!< whole frame is one Mono image
    ST_V_SRC_SIDE_BY_SIDE = 1,         //!< frame is two Full Right + Left images placed horizontally
    ST_V_SRC_PARALLEL_PAIR = 2,        //!< frame is two Full Left + Right images placed horizontally
    ST_V_SRC_OVER_UNDER_RL = 3,        //!< frame is two Full Right + Left images placed vertically
    ST_V_SRC_OVER_UNDER_LR = 4,        //!< frame is two Full Left + Right images placed vertically
    ST_V_SRC_ROW_INTERLACE = 5,        //!< each row is Right or Left image
    ST_V_SRC_VERTICAL_INTERLACE = 6,   //!< each vertical line is Right or Left image
    ST_V_SRC_SEPARATE_FRAMES = 7,      //!< Left and Right images stored in separate frames
    ST_V_SRC_PAGE_FLIP = 8,            //!< 2 sequenced frames is Full Right and Left images
    ST_V_SRC_ANAGLYPH_RED_CYAN = 9,
    ST_V_SRC_ANAGLYPH_G_RB = 10,
    ST_V_SRC_ANAGLYPH_YELLOW_BLUE = 11,
    ST_V_SRC_TILED_4X = 12,            //!< Sisvel 3D frame format to fit 720p stereopair into 1080p frame
} StFormatEnum;

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
    ST_CPPEXPORT StString formatToString(StFormatEnum theFormatEnum);

    /**
     * Return enumeration value from the string.
     */
    ST_CPPEXPORT StFormatEnum formatFromString(const StString& theFormatString);

    /**
     * Return source format with reversed views.
     */
    ST_CPPEXPORT StFormatEnum formatReversed(const StFormatEnum theFormatEnum);

    /**
     * Detect stereoscopic format from file name tags:
     * - ou,ab,ba  Over/Under, Above/Below
     * - sbs,lr,rl SideBySide, Left/Right
     * - 2d        Mono
     * - half,q    Anamorph suffix
     * or file extension:
     * - jps,pns   SideBySide (Right/Left order)
     */
    ST_CPPEXPORT StFormatEnum formatFromName(const StString& theFileName,
                                             bool&           theIsAnamorph);

    /**
     * Function tries to detect side-by-side stereo format based on aspect ratio criteria.
     * @param theRatio image ratio
     * @return autodetected mono/stereo format
     */
    ST_CPPEXPORT StFormatEnum formatFromRatio(const GLfloat theRatio);

};

#endif // __StFormatEnum_h_
