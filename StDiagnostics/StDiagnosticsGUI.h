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

#ifndef __StDiagnosticsGUI_h_
#define __StDiagnosticsGUI_h_

#include <StGLWidgets/StGLWidget.h>
#include <StGLWidgets/StGLRootWidget.h>
#include <StSettings/StTranslations.h>

class StDiagnostics;
class StGeometryTest;

/**
 * Root GUI widget for Diagnostics application.
 */
class ST_LOCAL StDiagnosticsGUI : public StGLRootWidget {

        public:

    StDiagnostics*           myPlugin;     //!< back-link to the main class
    StHandle<StTranslations> myLangMap;    //!< translated strings map

    StGeometryTest*          myGeomWidget;

        public: //!< StGLRootWidget overrides

    StDiagnosticsGUI(StDiagnostics* thePlugin);
    virtual ~StDiagnosticsGUI();
    virtual void stglUpdate(const StPointD_t& thePointZo);
    virtual void stglResize(const StRectI_t& theWinRectPx);
    virtual void setVisibility(const StPointD_t& , bool );

};

#endif //__StDiagnosticsGUI_h_
