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

#include "StSubtitlesASS.h"

#include <StGLWidgets/StSubQueue.h>
#include <cmath>

namespace {
    static const StString HEADER_INFO     = "[Script Info]";
    static const StString HEADER_V4STYLES = "[V4+ Styles]";
    static const StString HEADER_EVENTS   = "[Events]";
    static const StString HEADER_Format   = "Format:";
    static const StString HEADER_Dialog   = "Dialogue:";
    static const StString HEADER_Layer    = "Layer";
    static const StString HEADER_PtsStart = "Start";
    static const StString HEADER_PtsEnd   = "End";
    static const StString HEADER_Text     = "Text";

    // replacements
    static const StString ST_CRLF_REDUNDANT      = "\x0D\x0A";
    static const StString ST_CRLF_REPLACEMENT    = " \x0A";
    static const StString ST_ASS_NEWLINE         = "\\N";
    static const StString ST_ASS_NEWLINE2        = "\\n";
    static const StString ST_NEWLINE_REPLACEMENT = " \x0A";
    static const StString ST_NEWLINE_NORMAL      = '\n';

    static double parseDouble(const StString& theString) {
        StHandle< StArrayList<StString> > aList = theString.split('.', size_t(2));
        if(aList->size() <= 1) {
            return double(std::atoi(theString.toCString()));
        }
        double aUpper = double(std::atoi(aList->getValue(0).toCString()));
        double aLower = double(std::atoi(aList->getValue(1).toCString()));
        return aUpper + aLower * std::pow(0.1, double(aList->getValue(1).getLength()));
    }

    static double parseTime(const StString& theString) {
        StHandle< StArrayList<StString> > aList = theString.split(':', size_t(3));
        double aPTS = 0.0;
        if(aList->isEmpty()) {
            return aPTS;
        }
        size_t anItem = aList->size() - 1;
        // seconds
        aPTS += parseDouble(aList->getValue(anItem).toCString());
        if(--anItem == 0) {
            // incorrect format!
            return aPTS;
        }
        // minutes
        aPTS +=   60.0 * double(std::atoi(aList->getValue(anItem).toCString()));
        if(--anItem == 0) {
            // incorrect format!
            return aPTS;
        }
        // hours
        aPTS += 3600.0 * double(std::atoi(aList->getValue(anItem).toCString()));
        return aPTS;
    }
};

StSubtitlesASS::StSubtitlesASS()
: myElementsNb(0),
  myIdLayer(-1),
  myIdPtsStart(-1),
  myIdPtsEnd(-1),
  myIdStyle(-1),
  myIdName(-1),
  myIdMarginL(-1),
  myIdMarginR(-1),
  myIdMarginV(-1),
  myIdEffect(-1),
  myIdText(-1) {
    //
}

bool StSubtitlesASS::init(const char* theHeader,
                          const int   theSize) {
    // reset state
    myElementsNb =  0;
    myIdLayer    = -1;
    myIdPtsStart = -1;
    myIdPtsEnd   = -1;
    myIdStyle    = -1;
    myIdName     = -1;
    myIdMarginL  = -1;
    myIdMarginR  = -1;
    myIdMarginV  = -1;
    myIdEffect   = -1;
    myIdText     = -1;

    // check input
    if(theHeader == NULL || theSize == 0) {
        return false;
    }

    // parse header
    int aSection = HEADER_ID_NONE;
    ptrdiff_t aLen = 0;
    char* aLineStart = (char* )theHeader;
    char* anIter = aLineStart;
    char* anEnd  = aLineStart + theSize;
    for(; anIter < anEnd; ++anIter) {
        if(*anIter != '\n') {
            continue;
        }

        // new line
        aLen = anIter - aLineStart;
        if(*(anIter - 1) == char(13)) {
            // remove redundant CR symbols
            --aLen;
        }
        if(aLen <= 1) {
            // empty line
            aLineStart = anIter + 1;
            aSection = HEADER_ID_NONE;
            continue;
        }

        // header is probably doesn't contains Unicode symbols... so don't care about multibyte
        StString aLine(aLineStart, size_t(aLen));
        aLineStart = anIter + 1;
        if(aLine.isEqualsIgnoreCase(HEADER_INFO)) {
            aSection = HEADER_ID_INFO;
            continue;
        } else if(aLine.isEqualsIgnoreCase(HEADER_V4STYLES)) {
            aSection = HEADER_ID_V4STYLES;
            continue;
        } else if(aLine.isEqualsIgnoreCase(HEADER_EVENTS)) {
            aSection = HEADER_ID_EVENTS;
            continue;
        }
        switch(aSection) {
            case HEADER_ID_EVENTS: {
                if(aLine.isStartsWithIgnoreCase(HEADER_Format)) {
                    StHandle< StArrayList<StString> > aList = aLine.split(',');
                    myElementsNb = int(aList->size());
                    for(size_t anItem = 0; anItem < aList->size(); ++anItem) {
                        if(aList->getValue(anItem).isEndsWith(HEADER_Layer)) {
                            myIdLayer = int(anItem);
                        } else if(aList->getValue(anItem).isEndsWith(HEADER_PtsStart)) {
                            myIdPtsStart = int(anItem);
                        } else if(aList->getValue(anItem).isEndsWith(HEADER_PtsEnd)) {
                            myIdPtsEnd = int(anItem);
                        } else if(aList->getValue(anItem).isEndsWith(HEADER_Text)) {
                            myIdText = int(anItem);
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
    }
    return isValid();
}

StSubtitlesASS::~StSubtitlesASS() {
    //
}

void StSubtitlesASS::parseStyle(StString& theText) {
    StString aText;
    size_t aStart = 0;
    for(StUtf8Iter anIter = theText.iterator(); *anIter != 0; ++anIter) {
        if(*anIter.getBufferHere() == stUtf8_t('{')
        && *anIter.getBufferNext() == stUtf8_t('\\')) {
            aText += theText.subString(aStart, anIter.getIndex());
            ++anIter;
            ++anIter;

            // parse style codes (convert to HTML codes)
            if(*anIter.getBufferHere() == stUtf8_t('i')) {
                // italic
                if(       *anIter.getBufferNext() == stUtf8_t('1')) {
                    aText += "<i>";
                } else if(*anIter.getBufferNext() == stUtf8_t('0')) {
                    aText += "</i>";
                }
            } else if(*anIter.getBufferHere() == stUtf8_t('b')) {
                // bold
                if(       *anIter.getBufferNext() == stUtf8_t('1')) {
                    aText += "<b>";
                } else if(*anIter.getBufferNext() == stUtf8_t('0')) {
                    aText += "</b>";
                }
            } else if(*anIter.getBufferHere() == stUtf8_t('u')) {
                // underline
                if(       *anIter.getBufferNext() == stUtf8_t('1')) {
                    //aText += "<u>";
                } else if(*anIter.getBufferNext() == stUtf8_t('0')) {
                    //aText += "</u>";
                }
            } else if(*anIter.getBufferHere() == stUtf8_t('s')) {
                // strikeout
                if(       *anIter.getBufferNext() == stUtf8_t('1')) {
                    //aText += "<s>";
                } else if(*anIter.getBufferNext() == stUtf8_t('0')) {
                    //aText += "</s>";
                }
            } else if(*anIter.getBufferHere() == stUtf8_t('f')) {
                if(*anIter.getBufferNext() == stUtf8_t('s')) {
                    // font size
                } else if(*anIter.getBufferNext() == stUtf8_t('n')) {
                    // font name
                }
            }
            // else

            // search for trailing bracket
            while(*anIter != stUtf32_t('}') && *anIter != 0) {
                ++anIter;
            }
            if(*anIter == 0) {
                // broken data
                theText = aText;
                return;
            }
            aStart = anIter.getIndex() + 1;
        }
    }
    if(aStart != 0) {
        aText += theText.subString(aStart, theText.Length);
    }
    if(!aText.isEmpty()) {
        // replace only if text contains style codes
        theText = aText;
    }
}

StHandle<StSubItem> StSubtitlesASS::parseEvent(const StString& theString,
                                               double thePts,
                                               double theDuration) {
    if(!isValid()) {
        return StHandle<StSubItem>();
    }

    double aDuration = theDuration;
    StString* aText = NULL;
    StHandle< StArrayList<StString> > aList = theString.split(',', size_t(myElementsNb));
    if(theString.isStartsWithIgnoreCase(HEADER_Dialog)) {
        if(aList->size() < size_t(myElementsNb)) {
            return StHandle<StSubItem>();
        }

        aDuration = parseTime(aList->getValue(myIdPtsEnd)) - parseTime(aList->getValue(myIdPtsStart));
        if(aDuration <= 0.0) {
            aDuration = theDuration; // why ASS information is wrong here on current FFmpeg?
        }

        aText = &aList->changeValue(myIdText);
    } else {
        // try handling FFmpeg truncated format defined by ff_ass_get_dialog()
        // "ReadOrder,Layer,Style,Speaker,0,0,0,,Text"
        // TODO - where this format is documented?
        if(aList->size() != 9) {
            return StHandle<StSubItem>();
        }

        aText = &aList->changeValue(8);
    }

    // remove trailing newline symbol
    if(aText->isEndsWith(ST_CRLF_REDUNDANT)) {
        *aText = aText->subString(0, aText->getLength() - 2);
    } else if(aText->isEndsWith(ST_NEWLINE_NORMAL)) {
        *aText = aText->subString(0, aText->getLength() - 1);
    }
    if(aText->isEmpty()) {
        // ignore empty strings
        return StHandle<StSubItem>();
    }

    // parse style codes
    parseStyle(*aText);

    aText->replaceFast(ST_CRLF_REDUNDANT, ST_CRLF_REPLACEMENT);
    aText->replaceFast(ST_ASS_NEWLINE,    ST_NEWLINE_REPLACEMENT);
    aText->replaceFast(ST_ASS_NEWLINE2,   ST_NEWLINE_REPLACEMENT);
    StHandle<StSubItem> aNewSubItem = new StSubItem(thePts, thePts + aDuration);
    aNewSubItem->Text = *aText;
    return aNewSubItem;
}
