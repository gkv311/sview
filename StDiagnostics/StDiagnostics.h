/**
 * Copyright © 2010-2012 Kirill Gavrilov <kirill@sview.ru>
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

class ST_LOCAL StDiagnostics : public StDrawerInterface {

        public:

    static const StString ST_DRAWER_PLUGIN_NAME;

    StDiagnostics();
    ~StDiagnostics();

    // interface methods' implementations
    StDrawerInterface* getLibImpl() {
        return this;
    }
    bool init(StWindowInterface* inStWin);
    bool open(const StOpenInfo& stOpenInfo);
    void parseCallback(StMessage_t* stMessages);
    void stglDraw(unsigned int view);

    StWindow* getStWindow() { return stWin; }

        public: //!< callback Slots

    void doSwitchFullscreen(const size_t dummy = 0);

        private:

    StHandle<StGLContext> myContext;
    StWindow*             stWin;      //!< pointer to StWindow, created by Output plugin
    StSettings*           stSettings; //!< current plugin local settings
    StDiagnosticsGUI*     stGUI;      //!< GUI elements
    bool                  bQuit;

};

#endif //__StDiagnostics_h_
