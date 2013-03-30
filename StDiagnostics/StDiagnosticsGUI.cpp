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

#include <StCore/StWindow.h>
#include <StGLWidgets/StGLFpsLabel.h>

StDiagnosticsGUI::StDiagnosticsGUI(StDiagnostics* thePlugin)
: StGLRootWidget(),
  myPlugin(thePlugin),
  myLangMap(new StTranslations(StDiagnostics::ST_DRAWER_PLUGIN_NAME)),
  myGeomWidget(NULL),
  myFpsWidget(NULL),
  myCntWidgetLT(NULL),
  myCntWidgetBR(NULL),
  myFrameCounter(0) {
    myGeomWidget  = new StGeometryTest(this);

    // FPS widget
    myFpsWidget = new StGLFpsLabel(this);
    myFpsWidget->signals.onBtnClick.connect(myPlugin, &StDiagnostics::doFpsClick);

    // counters
    myCntWidgetLT = new StGLTextArea(this,  32,  32, StGLCorner(ST_VCORNER_TOP,    ST_HCORNER_LEFT),  128, 32);
    myCntWidgetBR = new StGLTextArea(this, -32, -32, StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_RIGHT), 128, 32);
    myCntWidgetLT->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                  StGLTextFormatter::ST_ALIGN_Y_CENTER);
    myCntWidgetBR->setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                                  StGLTextFormatter::ST_ALIGN_Y_CENTER);
    myCntWidgetLT->setBorder(true);
    myCntWidgetBR->setBorder(true);
    myCntWidgetLT->setBackColor(StGLVec3(0.77f, 0.77f, 0.77f));
    myCntWidgetBR->setBackColor(StGLVec3(0.77f, 0.77f, 0.77f));
    myCntWidgetLT->setText("0000");
    myCntWidgetBR->setText("0000");
}

StDiagnosticsGUI::~StDiagnosticsGUI() {
    //
}

void StDiagnosticsGUI::setVisibility(const StPointD_t& , bool ) {
    // always visible
    StGLRootWidget::setVisibility(true, true);
    myGeomWidget->setVisibility(true, true);
    myFpsWidget->setVisibility(true, true);
    myCntWidgetLT->setVisibility(true, true);
    myCntWidgetBR->setVisibility(true, true);

    myFpsWidget->update(myPlugin->getWindow()->isStereoOutput(),
                        myPlugin->getWindow()->stglGetTargetFps());

    char aBuffer[128];
    stsprintf(aBuffer, 128, "%04u", myFrameCounter++);
    if(myFrameCounter > 9999) {
        myFrameCounter = 0;
    }
    const StString aCntString = aBuffer;

    myCntWidgetLT->setText(aCntString);
    myCntWidgetBR->setText(aCntString);
}

void StDiagnosticsGUI::stglUpdate(const StPointD_t& thePointZo) {
    StGLRootWidget::stglUpdate(thePointZo);
}

void StDiagnosticsGUI::stglResize(const StRectI_t& theWinRectPx) {
    StGLRootWidget::stglResize(theWinRectPx);
}
