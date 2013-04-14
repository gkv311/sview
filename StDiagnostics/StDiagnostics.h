/**
 * Copyright © 2010-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * StDiagnostics program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StDiagnostics program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StDiagnostics_h_
#define __StDiagnostics_h_

#include <StCore/StDrawerInterface.h>

class StGLContext;
class StSettings;
class StWindow;
class StDiagnosticsGUI;

class StDiagnostics : public StDrawerInterface {

        public:

    static const StString ST_DRAWER_PLUGIN_NAME;

    ST_LOCAL StDiagnostics();
    ST_LOCAL ~StDiagnostics();

    ST_LOCAL StHandle<StWindow>& getWindow() {
        return myWindow;
    }

    // interface methods' implementations
    ST_LOCAL StDrawerInterface* getLibImpl() {
        return this;
    }
    ST_LOCAL bool init(StWindowInterface* inStWin);
    ST_LOCAL bool open(const StOpenInfo& stOpenInfo);
    ST_LOCAL void parseCallback(StMessage_t* stMessages);
    ST_LOCAL void stglDraw(unsigned int view);

        public: //!< callback Slots

    ST_LOCAL void doSwitchFullscreen(const size_t dummy = 0);
    ST_LOCAL void doFpsClick(const size_t dummy = 0);

        private:

    StHandle<StGLContext>      myContext;
    StHandle<StWindow>         myWindow;   //!< wrapper over Output plugin's StWindow instance
    StHandle<StSettings>       mySettings; //!< settings manager for Diagnostics plugin
    StHandle<StDiagnosticsGUI> myGUI;      //!< GUI root widget
    bool                       myToQuit;

};

#endif //__StDiagnostics_h_
