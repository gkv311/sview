/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StCore library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StCore library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 * If not, see <http://www.gnu.org/licenses/>.
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
     * Read user settings and if they are empty - search all connected displays
     */
    ST_CPPEXPORT void init();

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
#if (defined(_WIN32) || defined(__WIN32__))
    /**
     * Function retrieves displays' configuration from WinAPI
     */
    ST_LOCAL void findMonitorsWinAPI();
#elif (defined(__APPLE__))
    ST_LOCAL void findMonitorsCocoa();
#elif (defined(__linux__) || defined(__linux))
    /**
     * Function retrieves displays' configuration from XRandr extension
     */
    ST_LOCAL void findMonitorsXRandr();
    ST_LOCAL static bool getXRootSize(int& sizeX, int& sizeY);
    /**
     * Function retrieves displays' configuration from ADLsdk (AMD Catalyst)
     */
    ST_LOCAL void findMonitorsADLsdk();
#endif

        public:

    ST_CPPEXPORT static void listEDID(StArrayList<StEDIDParser>& theEdids);

};

#endif //__StSearchMonitors_h_
