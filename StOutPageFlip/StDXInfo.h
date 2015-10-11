/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StDXInfo_h_
#define __StDXInfo_h_

struct StDXInfo {

    bool hasNvAdapter;
    bool hasAmdAdapter;
    bool hasAqbsSupport;
    bool hasNvStereoSupport;

    StDXInfo()
    : hasNvAdapter(false),
      hasAmdAdapter(false),
      hasAqbsSupport(false),
      hasNvStereoSupport(false) {}

};

#endif // __StDXInfo_h_
