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

#include "StDiagnosticsGUI.h"
#include "StDiagnostics.h"
#include "StGeometryTest.h"

StDiagnosticsGUI::StDiagnosticsGUI(StDiagnostics* thePlugin)
: StGLRootWidget(),
  myPlugin(thePlugin),
  myLangMap(new StTranslations(StDiagnostics::ST_DRAWER_PLUGIN_NAME)),
  myGeomWidget(NULL) {
    //
    myGeomWidget = new StGeometryTest(this);
}

StDiagnosticsGUI::~StDiagnosticsGUI() {
    //
}

void StDiagnosticsGUI::setVisibility(const StPointD_t& , bool ) {
    // always visible
    StGLRootWidget::setVisibility(true, true);
    myGeomWidget->setVisibility(true, true);
}

void StDiagnosticsGUI::stglUpdate(const StPointD_t& thePointZo) {
    StGLRootWidget::stglUpdate(thePointZo);
}

void StDiagnosticsGUI::stglResize(const StRectI_t& theWinRectPx) {
    StGLRootWidget::stglResize(theWinRectPx);
}
