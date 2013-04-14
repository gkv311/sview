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

#ifndef __StGeometryTest_h_
#define __StGeometryTest_h_

#include <StGL/StGLVertexBuffer.h>
#include <StGLWidgets/StGLWidget.h>
#include <StGLMesh/StGLCircle.h>
#include <StGLMesh/StGLQuads.h>

#include "StColorProgram.h"

/**
 * Geometry test widget.
 */
class StGeometryTest : public StGLWidget {

        private:

    StGLVec2 myPixelSize;
    StGLVec2 myCellSize;

    StGLMesh   myGrid;
    StGLCircle myCircles[5];
    StGLQuads  myColors;
    StGLQuads  myBrightness;

    StColorProgram stProgram;

    ST_LOCAL void resizeGrid(const StRectI_t& winRectPx);
    ST_LOCAL void resizeColor();
    ST_LOCAL void resizeBrightness();

        public:

    ST_LOCAL StGeometryTest(StGLWidget* parent);
    ST_LOCAL virtual ~StGeometryTest();

    ST_LOCAL virtual const StString& getClassName();

    ST_LOCAL virtual void stglResize(const StRectI_t& winRectPx);
    ST_LOCAL virtual bool stglInit();
    ST_LOCAL virtual void stglDraw(unsigned int view);

};

#endif //__StGeometryTest_h_
