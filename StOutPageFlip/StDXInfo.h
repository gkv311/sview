/**
 * Copyright © 2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * StOutPageFlip library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StOutPageFlip library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
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
