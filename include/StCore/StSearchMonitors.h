/**
 * StCore, window system independent C++ toolkit for writing OpenGL applications.
 * Copyright © 2009-2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StSearchMonitors_h_
#define __StSearchMonitors_h_

#include <StCore/StMonitor.h>

#include <StTemplates/StArrayList.h>

/**
 * This class provides:
 *  - access to list of monitors
 *  - methods to initialize list within current system configuration
 *  - methods to find monitor within specified point
 */
class StSearchMonitors : public StArrayList<StMonitor> {

        public:

    /**
     * Empty constructor.
     */
    ST_CPPEXPORT StSearchMonitors();

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StSearchMonitors();

    /**
     * Read user settings and if they are empty - search all connected displays.
     * @param theForced re-initialize cached monitors state if true (notice - calls with delay less than 30 seconds will be ignored)
     */
    ST_CPPEXPORT void init(const bool theForced = false);

    /**
     * Read user settings
     */
    ST_CPPEXPORT void initFromConfig();

    /**
     * Search for connected displays
     */
    ST_CPPEXPORT void initFromSystem();

    ST_LOCAL inline StMonitor& operator[](const size_t theId) {
        return changeValue((theId < size()) ? theId : 0);
    }

    ST_LOCAL inline const StMonitor& operator[](const size_t theId) const {
        return getValue((theId < size()) ? theId : 0);
    }

    /**
     * Get monitor from point.
     */
    ST_CPPEXPORT StMonitor& operator[](const StPointI_t& thePoint);

    /**
     * Get monitor from point.
     */
    ST_CPPEXPORT const StMonitor& operator[](const StPointI_t& thePoint) const;

        private:

    /**
     * Just try to compute displays' configuration from known summary resolution
     */
    ST_LOCAL void findMonitorsBlind(const int rootX, const int rootY);
#ifdef _WIN32
    /**
     * Function retrieves displays' configuration from WinAPI
     */
    ST_LOCAL void findMonitorsWinAPI();
#elif defined(__APPLE__)
    ST_LOCAL void findMonitorsCocoa();
#elif defined(__ANDROID__)
    /**
     * There is no way to retrieve displays from global context - setup it externally.
     */
    ST_LOCAL static void setupGlobalDisplay(const StMonitor& theDisplay);
    friend class StAndroidGlue;
#elif defined(__linux__)
    /**
     * Function retrieves displays' configuration from XRandr extension
     */
    ST_LOCAL void findMonitorsXRandr();
    ST_LOCAL static bool getXRootSize(int& sizeX, int& sizeY);
#endif

    /**
     * Initialize global instance.
     */
    ST_LOCAL void initGlobal();

        public:

    ST_CPPEXPORT static void listEDID(StArrayList<StEDIDParser>& theEdids);

    /**
     * Register this instance as updater of global state
     * (e.g. window listens to messages about configuration changes).
     */
    ST_CPPEXPORT void registerUpdater(const bool theIsUpdater);

        protected:

    bool myIsUpdater; //!< flag indicating updating listener

};

#endif //__StSearchMonitors_h_
