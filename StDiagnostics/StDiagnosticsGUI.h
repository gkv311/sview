/**
 * Copyright © 2010 Kirill Gavrilov <kirill@sview.ru>
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
class ST_LOCAL StDiagnosticsGUI : public StGLRootWidget {

        public:

    StDiagnostics*    stPlugin; // link to the main class
    StTranslations   stLangMap; // translated strings map

    StGeometryTest* stGeometry;

        public:

    StDiagnosticsGUI(StDiagnostics* stPlugin);
    virtual ~StDiagnosticsGUI();

    // StGLWidget overrides
    virtual void stglUpdate(const StPointD_t& pointZo);
    virtual void stglResize(const StRectI_t& winRectPx);
    virtual void setVisibility(const StPointD_t& , bool );

};

#endif //__StDiagnosticsGUI_h_
