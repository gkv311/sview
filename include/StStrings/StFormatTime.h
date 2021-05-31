/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StFormatTime_h__
#define __StFormatTime_h__

#include "StString.h"
#include "stUtfTools.h"

// TODO (Kirill Gavrilov#9#) implement full class
class ST_LOCAL StFormatTime {

        private:

    unsigned int        hours;
    unsigned int      minutes;
    unsigned int      seconds;
    unsigned int milliSeconds;

        public:

    StFormatTime(double inSeconds)
    : hours(0),
      minutes(0),
      seconds(0),
      milliSeconds(0) {
        static const double SECONDS_IN_HOUR = 3600.0;
        static const double SECONDS_IN_MINUTE = 60.0;
        static const double SECOND_IN_HOUR = 1.0 / SECONDS_IN_HOUR;
        static const double SECOND_IN_MINUTE = 1.0 / SECONDS_IN_MINUTE;
        hours = (unsigned int )(inSeconds * SECOND_IN_HOUR);
        inSeconds -= (double )hours * SECONDS_IN_HOUR;
        minutes = (unsigned int )(inSeconds * SECOND_IN_MINUTE);
        inSeconds -= (double )minutes * SECONDS_IN_MINUTE;
        seconds = (unsigned int )inSeconds;
        inSeconds -= (double )seconds;
        milliSeconds = (unsigned int )(1000.0 * inSeconds);
    }

    ~StFormatTime() {
        //
    }

    StString toStringFull() const {
        stUtf8_t aBuffer[64];
        stsprintf(aBuffer, 64, "%02u:%02u:%02u,%05u", hours, minutes, seconds, milliSeconds);
        return aBuffer;
    }

    StString toString() const {
        stUtf8_t aBuffer[64];
        if(hours <= 0) {
            stsprintf(aBuffer, 64, "%02u:%02u", minutes, seconds);
        } else {
            stsprintf(aBuffer, 64, "%02u:%02u:%02u", hours, minutes, seconds);
        }
        return aBuffer;
    }

    static StString formatSeconds(const double& seconds) {
        if(seconds < 0.0) {
            return StString("-") + StFormatTime(-seconds).toString();
        } else {
            return StFormatTime(seconds).toString();
        }
    }

};

#endif //__StFormatTime_h__
