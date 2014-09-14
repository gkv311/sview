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

#include <StCore/StApplication.h>

class StGLContext;
class StSettings;
class StWindow;
class StDiagnosticsGUI;

class StDiagnostics : public StApplication {

        public:

    static const StString ST_DRAWER_PLUGIN_NAME;

    ST_CPPEXPORT StDiagnostics(const StHandle<StResourceManager>& theResMgr,
                               const StNativeWin_t                theParentWin = (StNativeWin_t )NULL,
                               const StHandle<StOpenInfo>&        theOpenInfo  = NULL);
    ST_CPPEXPORT virtual ~StDiagnostics();

    ST_CPPEXPORT virtual bool open();
    ST_CPPEXPORT virtual void beforeDraw();
    ST_CPPEXPORT virtual void stglDraw(unsigned int view);

        private: //! @name window events slots

    ST_LOCAL virtual void doResize   (const StSizeEvent&  theEvent);
    ST_LOCAL virtual void doKeyDown  (const StKeyEvent&   theEvent);
    ST_LOCAL virtual void doKeyHold  (const StKeyEvent&   theEvent);
    ST_LOCAL virtual void doKeyUp    (const StKeyEvent&   theEvent);
    ST_LOCAL virtual void doMouseDown(const StClickEvent& theEvent);
    ST_LOCAL virtual void doMouseUp  (const StClickEvent& theEvent);

        public: //! @name Properties

    struct {

        StHandle<StBoolParam> IsFullscreen; //!< fullscreen state

    } params;

        public: //!< @name callback Slots

    ST_LOCAL void doFullscreen(const bool theIsFullscreen);
    ST_LOCAL void doStereoMode(const size_t theMode);
    ST_LOCAL void doFpsClick(const size_t dummy = 0);

        private:

    /**
     * Actions identifiers.
     */
    enum ActionId {
        Action_Fullscreen,
        Action_StereoModeOn,
        Action_StereoModeOff,
    };

        private:

    StHandle<StGLContext>      myContext;
    StHandle<StSettings>       mySettings; //!< settings manager for Diagnostics plugin
    StHandle<StDiagnosticsGUI> myGUI;      //!< GUI root widget

        private:

    friend class StDiagnosticsGUI;

};

#endif //__StDiagnostics_h_
