/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
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
     * Returns name for format.
     */
    ST_CPPEXPORT StString formatToString(StFormatEnum theFormatEnum);

    /**
     * Return enumeration value from the string.
     */
    ST_CPPEXPORT StFormatEnum formatFromString(const StString& theFormatString);

};

#endif //__StFormatEnum_h_
