/**
 * Copyright © 2009-2012 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StFPSControl_h_
#define __StFPSControl_h_

#include "StFPSMeter.h"
#include "StThread.h"

/**
 * Class extend FPS measurements features with possibility
 * to adjust FPS to target using thread sleeping.
 */
class StFPSControl : public StFPSMeter {

        public:

    StFPSControl()
    : StFPSMeter(),
      mySleeper(10),
      myTargetFps(-1.0),
      myDecCount(0),
      myIsIncreased(false) {
        //
    }

    virtual ~StFPSControl() {
        //
    }

    /**
     * Increment frames counter.
     */
    virtual bool nextFrame() {
        const double aPrevFPS = getAverage();
        if(StFPSMeter::nextFrame()) {
            const double aNewFPS = getAverage();
            // compute sleep time to get target FPS
            if(myTargetFps > 0.0) {
                //ST_DEBUG_LOG("adjustFPSToTarget " + myTargetFps);
                adjustFPSToTarget(aNewFPS);
            } else if(myTargetFps == 0.0) {
                //ST_DEBUG_LOG("adjustFPSToMax " + myTargetFps);
                adjustFPSToMax(aPrevFPS, aNewFPS);
            }
            return true;
        }
        return false;
    }

    /**
     * @param theFps (const double& ) - target fps.
     */
    void setTargetFPS(const double theFps) {
        myTargetFps = theFps;
    }

    /**
     * Sleep the thread to fit the target FPS.
     * If target FPS is 0.0 or system (GPU) can't reach the limit
     * sleep time is a highest value not reduced the FPS.
     * If target FPS set to -1.0 sleep ignored.
     * Notice: on some system (Windows) you should adjust system timer before call!
     */
    void sleepToTarget() {
        if(myTargetFps >= 0.0) {
            mySleeper.sleep();
        }
    }

        private:

    class StSleeper {

            public:

        StSleeper(const int theSlTime)
        : myCurrTimer(0) {
            mySlTimes[0] = mySlTimes[1] = mySlTimes[2] = mySlTimes[3] = theSlTime;
        }

        bool isZero() const {
            return mySlTimes[0] <= 1 && mySlTimes[1] <= 1 && mySlTimes[2] <= 1 && mySlTimes[3] <= 1;
        }

        void zero() {
            mySlTimes[0] = mySlTimes[1] = mySlTimes[2] = mySlTimes[3] = 1;
        }

        void inc() {
            size_t toInc = 0;
            if(mySlTimes[0] + mySlTimes[1] <= mySlTimes[2] + mySlTimes[3]) {
                toInc = (mySlTimes[0] <= mySlTimes[1]) ? 0 : 1;
            } else {
                toInc = (mySlTimes[2] <= mySlTimes[3]) ? 2 : 3;
            }
            ++mySlTimes[toInc];
            //ST_DEBUG_LOG("sleepToTrg++ " + mySlTimes[0] + " " + mySlTimes[1] + " " + mySlTimes[2] + " " + mySlTimes[3]);
        }

        void dec() {
            if(isZero()) {
                return;
            }
            size_t toDec = 0;
            if(mySlTimes[0] + mySlTimes[1] >= mySlTimes[2] + mySlTimes[3]) {
                toDec = (mySlTimes[0] >= mySlTimes[1]) ? 0 : 1;
            } else {
                toDec = (mySlTimes[2] >= mySlTimes[3]) ? 2 : 3;
            }
            --mySlTimes[toDec];
            //ST_DEBUG_LOG("sleepToTrg-- " + mySlTimes[0] + " " + mySlTimes[1] + " " + mySlTimes[2] + " " + mySlTimes[3]);
        }

        void sleep() {
            StThread::sleep(value());
            next();
        }

            private:

        size_t myCurrTimer;
        int    mySlTimes[4];

        int value() const {
            return mySlTimes[myCurrTimer];
        }

        void next() {
            if(++myCurrTimer > 3) {
                myCurrTimer = 0;
            }
        }

    };

        private:

    /**
     * Try to adjust sleep timers to tend to target FPS.
     */
    void adjustFPSToTarget(const double theNewFps) {
        const double aFpsPrecision = myTargetFps * 0.01; // 1%
        const double aDiffFps = theNewFps - myTargetFps;
        if(aDiffFps > aFpsPrecision) {
            mySleeper.inc();
            if(aDiffFps >= 8.0) {
                // great diff - reduce time to adjust
                for(size_t anIter = 0; anIter < 3; ++anIter) {
                    mySleeper.inc();
                }
            }
        } else if(aDiffFps < -aFpsPrecision) {
            if(mySleeper.isZero()) {
                // TODO (Kirill Gavrilov#7) if target FPS higher than maximal
                //                          - no mehanism to reduce CPU utilization
                return;
            }
            mySleeper.dec();
            if(aDiffFps <= -8.0) {
                // great diff - reduce time to adjust
                for(size_t anIter = 0; anIter < 3; ++anIter) {
                    mySleeper.dec();
                }
            }
        }
    }

    /**
     * Try to reduce CPU utilization (using sleep timers)
     * with minimal affect to FPS.
     */
    void adjustFPSToMax(const double thePrevFps,
                        const double theNewFps) {
        static const double ST_FPS_MINIMUM = 30.0; // considered as minimal FPS...
        const double aFpsPrecision = 0.5 * (theNewFps + thePrevFps) * 0.01; // 1%
        const double aDiffFps = theNewFps - thePrevFps;
        if(myIsIncreased && (aDiffFps < -aFpsPrecision || theNewFps < ST_FPS_MINIMUM)) {
            // increased sleep time affect in FPS slowdown
            // - reduce sleep time
            mySleeper.dec();
            myIsIncreased = false;
            ++myDecCount;
        } else if(myDecCount > 0 && (aDiffFps > aFpsPrecision)) {
            // decreased sleep time affect in FPS increasing
            // - reduce sleep time more to tend to FPS maximum
            mySleeper.dec();
            ++myDecCount;
        } else {
            // try to reduce CPU utilization while not affecting FPS
            // - increase sleep time
            mySleeper.inc();
            myIsIncreased = true;
            if(myDecCount > 0) {
                --myDecCount;
            }
        }
    }

        private:

    StSleeper mySleeper;
    double    myTargetFps;   //!< target average FPS
    int       myDecCount;
    bool      myIsIncreased;

};

#endif // __StFPSControl_h_
