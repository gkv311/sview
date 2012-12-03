/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
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

class ST_LOCAL StSearchMonitors : public StArrayList<StMonitor> {

        public:

    StSearchMonitors()
    : StArrayList<StMonitor>(2) {
        //
    }

    virtual ~StSearchMonitors() {
        //
    }

    /**
     * Read user settings and if they are empty - search all connected displays
     */
    void init();

    /**
     * Read user settings
     */
    void initFromConfig();

    /**
     * Search for connected displays
     */
    void initFromSystem();

    StMonitor& operator[](const size_t id) {
        return (id < size()) ? changeValue(id) : changeValue(0);
    }

    /**
     * Get monitor from point.
     */
    StMonitor& operator[](const StPointI_t& iPoint) {
        for(size_t id = 0; id < size(); ++id) {
            if(getValue(id).getVRect().isPointIn(iPoint)) {
                return changeValue(id);
            }
        }
        return changeValue(0); // return first anyway...
    }

    /**
     * Just try to compute displays' configuration from known summary resolution
     */
    void findMonitorsBlind(const int rootX, const int rootY);
#if (defined(_WIN32) || defined(__WIN32__))
    /**
     * Function retrieves displays' configuration from WinAPI
     */
    void findMonitorsWinAPI();
#elif (defined(__APPLE__))
    void findMonitorsCocoa();
#elif (defined(__linux__) || defined(__linux))
    /**
     * Function retrieves displays' configuration from XRandr extension
     */
    void findMonitorsXRandr();
    static bool getXRootSize(int& sizeX, int& sizeY);
    /**
     * Function retrieves displays' configuration from ADLsdk (AMD Catalyst)
     */
    void findMonitorsADLsdk();
#endif

    static void listEDID(StArrayList<StEDIDParser>& theEdids);

};

#endif //__StSearchMonitors_h_
