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
#include <StGLWidgets/StGLTextArea.h>
#include <StSettings/StTranslations.h>

class StDiagnostics;
class StGeometryTest;
class StGLFpsLabel;

/**
 * Root GUI widget for Diagnostics application.
 */
class StDiagnosticsGUI : public StGLRootWidget {

        public:

    StDiagnostics*           myPlugin;       //!< back-link to the main class
    StHandle<StTranslations> myLangMap;      //!< translated strings map

    StGeometryTest*          myGeomWidget;
    StGLFpsLabel*            myFpsWidget;    //!< FPS widget
    StGLTextArea*            myCntWidgetLT;  //!< top-left     counter
    StGLTextArea*            myCntWidgetBR;  //!< bottom-right counter

    int                      myFrameCounter; //!< frame counter

        public: //!< StGLRootWidget overrides

    ST_LOCAL StDiagnosticsGUI(StDiagnostics* thePlugin);
    ST_LOCAL virtual ~StDiagnosticsGUI();
    ST_LOCAL virtual void stglUpdate(const StPointD_t& thePointZo) ST_ATTR_OVERRIDE;
    ST_LOCAL virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;

    ST_LOCAL void setVisibility(const StPointD_t& , bool );

};

#endif //__StDiagnosticsGUI_h_
