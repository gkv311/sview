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

#include "StGeometryTest.h"

#include <StGL/StGLContext.h>
#include <StGLCore/StGLCore20.h>
#include <StGLWidgets/StGLRootWidget.h>

StGeometryTest::StGeometryTest(StGLWidget* parent)
: StGLWidget(parent),
  myPixelSize(0.01f, 0.01f),
  myCellSize(0.1f, 0.1f),
  myGrid(GL_LINES),
  stProgram() {
    //
}

StGeometryTest::~StGeometryTest() {
    StGLContext& aCtx = getContext();
    stProgram.release(aCtx);
    myGrid.release(aCtx);
    for(size_t anIter = 0; anIter < 5; ++anIter) {
        myCircles[anIter].release(aCtx);
    }
    myColors.release(aCtx);
    myBrightness.release(aCtx);
}

void StGeometryTest::resizeGrid(const StRectI_t& winRectPx) {
    StGLContext& aCtx = getContext();

    // grid
    size_t linesCountV = 16 + 1;
    size_t cellSizePx  = 10;
    size_t linesCountH = 16 + 1;

    if(winRectPx.width() > winRectPx.height()) {
        cellSizePx = winRectPx.width() / (linesCountV - 1);
        linesCountH = (winRectPx.height() / cellSizePx) + 1;
    } else {
        cellSizePx = winRectPx.height() / (linesCountH - 1);
        linesCountV = (winRectPx.width() / cellSizePx) + 1;
    }

    myCellSize.x() = 2.0f * GLfloat(cellSizePx) / GLfloat(winRectPx.width());
    myCellSize.y() = 2.0f * GLfloat(cellSizePx) / GLfloat(winRectPx.height());

    size_t vertixesCount = (linesCountH + linesCountV) * 2;
    StArray<StGLVec4> vertArray(vertixesCount);

    // insert black gap to make quads
    StGLVec2 blackGap(GLfloat(winRectPx.width()  - cellSizePx * (linesCountV - 1)) / GLfloat(winRectPx.width()),
                      GLfloat(winRectPx.height() - cellSizePx * (linesCountH - 1)) / GLfloat(winRectPx.height()));

    StGLVec2 bottomLeft = StGLVec2(-1.0f) + blackGap;
    StGLVec2 fat = StGLVec2( 2.0f) - blackGap * 2.0f;

    // horizontal lines
    for(size_t lineId = 0; lineId < linesCountH; ++lineId) {
        GLfloat anY = bottomLeft.y() + fat.y() * (GLfloat(lineId) / GLfloat(linesCountH - 1));
        vertArray[2 * lineId]     = StGLVec4(-1.0f, anY, 0.0f, 1.0f);
        vertArray[2 * lineId + 1] = StGLVec4( 1.0f, anY, 0.0f, 1.0f);
    }

    // vertical lines
    for(size_t lineId = 0; lineId < linesCountV; ++lineId) {
        GLfloat anX = bottomLeft.x() + fat.x() * (GLfloat(lineId) / GLfloat(linesCountV - 1));
        vertArray[2 * linesCountH + 2 * lineId]     = StGLVec4(anX, -1.0f, 0.0f, 1.0f);
        vertArray[2 * linesCountH + 2 * lineId + 1] = StGLVec4(anX,  1.0f, 0.0f, 1.0f);
    }

    myGrid.changeVBO(ST_VBO_VERTEX)->init(aCtx, vertArray);

    // white color
    StArray<StGLVec4> lColorsArray(vertixesCount, StGLVec4(1.0f));
    myGrid.changeVBO(ST_VBO_COLORS)->init(aCtx, lColorsArray);

    // bottom left circle
    myCircles[0].create(StGLVec3(bottomLeft + myCellSize),
                        myCellSize.x(), myCellSize.y(), 64);
    // bottom right circle
    myCircles[1].create(StGLVec3(bottomLeft + myCellSize * StGLVec2(GLfloat(linesCountV - 2), 1.0f)),
                        myCellSize.x(), myCellSize.y(), 64);
    // top left circle
    myCircles[2].create(StGLVec3(bottomLeft + myCellSize * StGLVec2(1.0f, GLfloat(linesCountH - 2))),
                        myCellSize.x(), myCellSize.y(), 64);
    // top right circle
    myCircles[3].create(StGLVec3(bottomLeft + myCellSize * StGLVec2(GLfloat(linesCountV - 2), GLfloat(linesCountH - 2))),
                        myCellSize.x(), myCellSize.y(), 64);
    // center circle
    GLfloat minSize = 0.5f * (((linesCountV < linesCountH) ? linesCountV : linesCountH) - 1);
    myCircles[4].create(StGLVec3(0.0f),
                        myCellSize.x() * minSize, myCellSize.y() * minSize, 64);
    // white color
    for(size_t aCircleId = 0; aCircleId < 5; ++aCircleId) {
        myCircles[aCircleId].computeMesh();
        myCircles[aCircleId].initColorsArray(StGLVec4(1.0f));
        myCircles[aCircleId].initVBOs(aCtx);
    }
}

/**
 * Simple function to setup common values in the array.
 */
static inline void setValues(StArray<StGLVec4>& stVec4Array, const StGLVec4& value,
                             const size_t fromId, const size_t count) {
    for(size_t anId = fromId; anId < (fromId + count); ++anId) {
        stVec4Array[anId] = value;
    }
}

void StGeometryTest::resizeColor() {
    static const size_t COLOR_QUADS   = 8;
    static const size_t VERT_PER_QUAD = 6;
    StGLContext& aCtx = getContext();
    StGLVec2 rectSize(myCellSize.x(), 2.0f * myCellSize.y());

    // move to the center
    StGLVec2 bottomLeft(-1.0f + 0.5f * (2.0f - GLfloat(COLOR_QUADS) * rectSize.x()),
                        -1.0f + 2.0f * rectSize.y());

    GLfloat anYBottom = bottomLeft.y()                + 4.0f * myPixelSize.x();
    GLfloat anYTop    = bottomLeft.y() + rectSize.y() - 4.0f * myPixelSize.x();

    // setup vertices
    StArray<StGLVec4> aVertArray(COLOR_QUADS * VERT_PER_QUAD);
    for(size_t quadId = 0; quadId < COLOR_QUADS; ++quadId) {
        GLfloat anXLeft  = bottomLeft.x() + GLfloat(quadId + 0) * rectSize.x() + 4.0f * myPixelSize.x();
        GLfloat anXRight = bottomLeft.x() + GLfloat(quadId + 1) * rectSize.x() - 4.0f * myPixelSize.x();
        aVertArray[quadId * VERT_PER_QUAD + 0] = StGLVec4(anXLeft,  anYBottom, 0.0f, 1.0f);
        aVertArray[quadId * VERT_PER_QUAD + 1] = StGLVec4(anXRight, anYBottom, 0.0f, 1.0f);
        aVertArray[quadId * VERT_PER_QUAD + 2] = StGLVec4(anXRight, anYTop,    0.0f, 1.0f);
        aVertArray[quadId * VERT_PER_QUAD + 3] = StGLVec4(anXRight, anYTop,    0.0f, 1.0f);
        aVertArray[quadId * VERT_PER_QUAD + 4] = StGLVec4(anXLeft,  anYBottom, 0.0f, 1.0f);
        aVertArray[quadId * VERT_PER_QUAD + 5] = StGLVec4(anXLeft,  anYTop,    0.0f, 1.0f);
    }
    myColors.changeVBO(ST_VBO_VERTEX)->init(aCtx, aVertArray);

    // setup color
    StArray<StGLVec4> aColorsArray(COLOR_QUADS * VERT_PER_QUAD);
    setValues(aColorsArray, StGLVec4(0.0f, 0.0f, 1.0f, 1.0f), 0 * VERT_PER_QUAD, VERT_PER_QUAD); // blue
    setValues(aColorsArray, StGLVec4(0.0f, 1.0f, 1.0f, 1.0f), 1 * VERT_PER_QUAD, VERT_PER_QUAD); // aqua
    setValues(aColorsArray, StGLVec4(1.0f, 0.0f, 1.0f, 1.0f), 2 * VERT_PER_QUAD, VERT_PER_QUAD); // fuchsia
    setValues(aColorsArray, StGLVec4(1.0f, 0.0f, 0.5f, 1.0f), 3 * VERT_PER_QUAD, VERT_PER_QUAD); // ?
    setValues(aColorsArray, StGLVec4(1.0f, 0.0f, 0.0f, 1.0f), 4 * VERT_PER_QUAD, VERT_PER_QUAD); // red
    setValues(aColorsArray, StGLVec4(1.0f, 0.5f, 0.0f, 1.0f), 5 * VERT_PER_QUAD, VERT_PER_QUAD); // orange
    setValues(aColorsArray, StGLVec4(1.0f, 1.0f, 0.0f, 1.0f), 6 * VERT_PER_QUAD, VERT_PER_QUAD); // yellow
    setValues(aColorsArray, StGLVec4(0.0f, 1.0f, 0.0f, 1.0f), 7 * VERT_PER_QUAD, VERT_PER_QUAD); // green
    myColors.changeVBO(ST_VBO_COLORS)->init(aCtx, aColorsArray);
}

void StGeometryTest::resizeBrightness() {
    static const size_t BR_QUADS = 11;
    static const size_t VERT_PER_QUAD = 6;
    StGLContext& aCtx = getContext();
    StGLVec2 rectSize(myCellSize.x(), 2.0f * myCellSize.y());

    // move to the center
    StGLVec2 bottomLeft(-1.0f + 0.5f * (2.0f - GLfloat(BR_QUADS) * rectSize.x()),
                        -1.0f + rectSize.y());

    GLfloat anYBottom = bottomLeft.y()                + 4.0f * myPixelSize.x();
    GLfloat anYTop    = bottomLeft.y() + rectSize.y() - 4.0f * myPixelSize.x();

    // setup vertices
    StArray<StGLVec4> aVertArray(BR_QUADS * VERT_PER_QUAD);
    for(size_t quadId = 0; quadId < BR_QUADS; ++quadId) {
        GLfloat anXLeft  = bottomLeft.x() + GLfloat(quadId + 0) * rectSize.x() + 4.0f * myPixelSize.x();
        GLfloat anXRight = bottomLeft.x() + GLfloat(quadId + 1) * rectSize.x() - 4.0f * myPixelSize.x();
        aVertArray[quadId * VERT_PER_QUAD + 0] = StGLVec4(anXLeft,  anYBottom, 0.0f, 1.0f);
        aVertArray[quadId * VERT_PER_QUAD + 1] = StGLVec4(anXRight, anYBottom, 0.0f, 1.0f);
        aVertArray[quadId * VERT_PER_QUAD + 2] = StGLVec4(anXRight, anYTop,    0.0f, 1.0f);
        aVertArray[quadId * VERT_PER_QUAD + 3] = StGLVec4(anXRight, anYTop,    0.0f, 1.0f);
        aVertArray[quadId * VERT_PER_QUAD + 4] = StGLVec4(anXLeft,  anYBottom, 0.0f, 1.0f);
        aVertArray[quadId * VERT_PER_QUAD + 5] = StGLVec4(anXLeft,  anYTop,    0.0f, 1.0f);
    }
    myBrightness.changeVBO(ST_VBO_VERTEX)->init(aCtx, aVertArray);

    // setup color (increased brightness): 3% 10% 20% 30% ... 100%
    StArray<StGLVec4> aColorsArray(BR_QUADS * VERT_PER_QUAD);
    StGLVec4 aColor(0.03f, 0.03f, 0.03f, 1.0f);
    StGLVec4 aColorDelta(0.1f, 0.1f, 0.1f, 1.0f);
    for(size_t quadId = 0; quadId < BR_QUADS; ++quadId) {
        setValues(aColorsArray, aColor, quadId * VERT_PER_QUAD, VERT_PER_QUAD);
        if(quadId == 0) {
            aColor  = aColorDelta;
        } else {
            aColor += aColorDelta;
        }
    }
    myBrightness.changeVBO(ST_VBO_COLORS)->init(aCtx, aColorsArray);
}

void StGeometryTest::stglResize() {
    StGLWidget::stglResize();

    myPixelSize.x() = 2.0f / GLfloat(myRoot->getRectPx().width());
    myPixelSize.y() = 2.0f / GLfloat(myRoot->getRectPx().height());

    resizeGrid(myRoot->getRectPx()); // grid lines and circles (main test)
    resizeColor();         // color rectangles
    resizeBrightness();    // brightness rectangles
}

bool StGeometryTest::stglInit() {
    return stProgram.init(getContext());
}

void StGeometryTest::stglDraw(unsigned int ) {
    StGLContext& aCtx = getContext();
    GLint aViewPort[4];
    aCtx.core20fwd->glGetIntegerv(GL_VIEWPORT, aViewPort);
    StGLVec4 transVec(1.0f / (GLfloat )aViewPort[2],
                      1.0f / (GLfloat )aViewPort[3],
                      0.0f, 0.0f);
    StGLVec4 scaleVec(1.0f - 2.0f * transVec.x(),
                      1.0f - 2.0f * transVec.y(),
                      1.0f, 1.0f);

    aCtx.core20fwd->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    aCtx.core20fwd->glEnable(GL_BLEND);
    stProgram.use(aCtx);
    stProgram.setScaleTranslate(aCtx, scaleVec, transVec);

    myGrid.draw(aCtx, stProgram);
    for(size_t aCircleId = 0; aCircleId < 5; ++aCircleId) {
        myCircles[aCircleId].draw(aCtx, stProgram);
    }
    myColors.draw(aCtx, stProgram);
    myBrightness.draw(aCtx, stProgram);

    stProgram.unuse(aCtx);
    aCtx.core20fwd->glDisable(GL_BLEND);
}
