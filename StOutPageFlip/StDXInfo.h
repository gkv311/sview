/**
 * StOutPageFlip, class providing stereoscopic output for Shutter Glasses displays using StCore toolkit.
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StDXInfo_h_
#define __StDXInfo_h_

struct StDXInfo {

    bool hasNvAdapter;
    bool hasAmdAdapter;
    bool hasAqbsSupport;
    bool hasNvStereoSupport;
    bool hasNvStereoWindowed;

    StDXInfo()
    : hasNvAdapter(false),
      hasAmdAdapter(false),
      hasAqbsSupport(false),
      hasNvStereoSupport(false),
      hasNvStereoWindowed(false)
    {}

};

#endif // __StDXInfo_h_
