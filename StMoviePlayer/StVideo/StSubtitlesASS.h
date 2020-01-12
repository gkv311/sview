/**
 * Copyright © 2011-2020 Kirill Gavrilov <kirill@sview.ru>
 *
 * StMoviePlayer program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StMoviePlayer program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StSubtitlesASS_h_
#define __StSubtitlesASS_h_

#include <StTemplates/StHandle.h>
#include <StStrings/StString.h>

// forward declarations
class StSubItem;

/**
 * ASS subtitles parser.
 */
class StSubtitlesASS {

        public:

    /**
     * Main constructor.
     */
    ST_LOCAL StSubtitlesASS();

    /**
     * Destructor.
     */
    ST_LOCAL ~StSubtitlesASS();

    /**
     * @param theHeader header text
     * @param theSize   header length
     * @return true on success
     */
    ST_LOCAL bool init(const char* theHeader,
                       const int   theSize);

    /**
     * Returns true if ASS header provide valid info.
     */
    ST_LOCAL bool isValid() const {
        return myIdText     != -1
            && myIdPtsStart != -1
            && myIdPtsEnd   != -1;
    }

    /**
     * Parse the dialog event and returns a subtitle item.
     */
    ST_LOCAL StHandle<StSubItem> parseEvent(const StString& theString,
                                            double thePts,
                                            double theDuration);

        private:

    enum {
        HEADER_ID_NONE,
        HEADER_ID_INFO,
        HEADER_ID_V4STYLES,
        HEADER_ID_EVENTS,
    };

        private:

    ST_LOCAL void parseStyle(StString& theText);

        private:

    int myElementsNb; //!< elements number in dialog event
    int myIdLayer;    //!< fields ids in dialog event
    int myIdPtsStart;
    int myIdPtsEnd;
    int myIdStyle;
    int myIdName;
    int myIdMarginL;
    int myIdMarginR;
    int myIdMarginV;
    int myIdEffect;
    int myIdText;

};

#endif // __StSubtitlesASS_h_
